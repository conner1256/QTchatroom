#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QJsonObject>
class chatClient : public QObject
{
    Q_OBJECT
public:
    explicit chatClient(QObject *parent = nullptr);

    // 【修改 1】增加 nickname 参数
    void connectToServer(const QHostAddress &address, quint16 port, const QString &nickName);
    // 【新增】公开的 getter 函数，用于获取当前用户名
    QString getUserName() const { return m_userName; }

    // 发送消息函数保持不变
    void sendMessage(const QString &text, const QString &type = "message");
    void disconnectFromHost();

signals:
    void connected();
    void messageReceived(const QString &text);

    // 【新增】列表相关的信号
    void userJoined(const QString &username);
    void userLeft(const QString &username);
    void userListReceived(const QStringList &users);
    void loginError(const QString &reason); // 【新增】

private slots:
    void onReadyRead();
    // 【修改 2】增加连接成功的槽函数，用于发送登录JSON
    void onConnected();

private:
    QTcpSocket *m_clientSocket;
    QString m_userName; // 【修改 3】新增成员变量，用于临时存储用户名
};

#endif // CHATCLIENT_H
