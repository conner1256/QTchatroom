#include "chatclient.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QDataStream>
#include <QJsonValue>
#include <QJsonArray>

chatClient::chatClient(QObject *parent)
    : QObject{parent}
{
    m_clientSocket = new QTcpSocket(this);

    // 【关键修复 1】连接 connected 信号，确保连接建立后再发送数据
    connect(m_clientSocket, &QTcpSocket::connected, this, &chatClient::onConnected);
    connect(m_clientSocket, &QTcpSocket::readyRead, this, &chatClient::onReadyRead);
}

// 【关键修复 2】保存用户名，并发起连接
void chatClient::connectToServer(const QHostAddress &address, quint16 port, const QString &nickName)
{
    m_userName = nickName;

    // 【新增】这行代码非常关键！
    // abort() 会立即关闭当前连接，并重置 Socket 到 UnconnectedState (初始状态)。
    // 这样无论之前是连着、断了一半、还是正在重连，都能保证现在是一个崭新的开始。
    m_clientSocket->abort();

    m_clientSocket->connectToHost(address, port);
}

// 【关键修复 3】连接建立后，自动发送登录 JSON
void chatClient::onConnected()
{
    emit connected(); // 通知界面连接成功

    if (!m_userName.isEmpty()) {
        // 发送登录消息，type 为 login
        sendMessage(m_userName, "login");
    }
}

void chatClient::sendMessage(const QString &text, const QString &type)
{
    if (m_clientSocket->state() != QAbstractSocket::ConnectedState)
        return;

    if (!text.isEmpty()) {
        QDataStream serverStream(m_clientSocket);
        serverStream.setVersion(QDataStream::Qt_5_12);
        QJsonObject message;
        message["type"] = type;
        message["text"] = text;
        serverStream << QJsonDocument(message).toJson();
    }
}

void chatClient::disconnectFromHost()
{
    m_clientSocket->disconnectFromHost();
}

// 【重点修改】接收并解析 JSON
void chatClient::onReadyRead()
{
    QByteArray jsonData;
    QDataStream socketStream(m_clientSocket);
    socketStream.setVersion(QDataStream::Qt_5_12);

    for (;;) {
        socketStream.startTransaction();
        socketStream >> jsonData;
        if (socketStream.commitTransaction()) {

            // 1. 解析 JSON 文档
            QJsonParseError parseError;
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);

            if (parseError.error == QJsonParseError::NoError && jsonDoc.isObject()) {
                QJsonObject jsonObj = jsonDoc.object();
                const QJsonValue typeVal = jsonObj.value("type");

                if (typeVal.isString()) {
                    QString type = typeVal.toString();
                    QString displayMsg; // 最终要显示在聊天框的文字

                    // --- 情况 A: 普通聊天消息 ---
                    if (type.compare("message", Qt::CaseInsensitive) == 0) {
                        QString text = jsonObj.value("text").toString();
                        QString sender = jsonObj.value("sender").toString();

                        // 格式化： "[张三]: 你好"
                        displayMsg = QString("[%1]: %2").arg(sender, text);
                    }
                    // --- 情况 B: 新用户加入 ---
                    else if (type.compare("newuser", Qt::CaseInsensitive) == 0) {
                        QString username = jsonObj.value("username").toString();

                        // 格式化文字用于显示在聊天窗口
                        displayMsg = QString("--- 欢迎 %1 加入聊天室 ---").arg(username);

                        // 【新增】发送信号更新右侧列表
                        emit userJoined(username);
                    }
                    // --- 情况 C: 用户离开 ---
                    else if (type.compare("userdisconnected", Qt::CaseInsensitive) == 0) {
                        QString username = jsonObj.value("username").toString();

                        // 格式化文字用于显示在聊天窗口
                        displayMsg = QString("--- %1 离开了聊天室 ---").arg(username);

                        // 【新增】发送信号更新右侧列表
                        emit userLeft(username);
                    }
                    // --- 情况 D: 收到当前在线用户列表 (登录时触发) ---
                    else if (type.compare("userlist", Qt::CaseInsensitive) == 0) {
                        // 【新增】解析用户列表
                        QJsonArray userArray = jsonObj.value("userlist").toArray();
                        QStringList users;
                        for (const QJsonValue &val : userArray) {
                            users.append(val.toString());
                        }
                        // 发送信号给 MainWindow 刷新列表
                        emit userListReceived(users);
                    }
                    // --- 【修改】处理登录反馈 ---
                    else if (type.compare("login", Qt::CaseInsensitive) == 0) {
                        bool success = jsonObj.value("success").toBool();
                        if (success) {
                            // 登录成功，什么都不用做，或者发个信号
                            // 之前的 userListReceived 实际上已经承担了刷界面的功能
                        } else {
                            // 登录失败
                            QString reason = jsonObj.value("reason").toString();
                            emit loginError(reason); // 发送错误信号
                        }
                    }

                    // 2. 如果解析出了需要显示的文字消息，发送给界面
                    if (!displayMsg.isEmpty()) {
                        emit messageReceived(displayMsg);
                    }
                }
            }
        } else {
            break;
        }
    }
}
