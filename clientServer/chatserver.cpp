#include "chatserver.h"
#include "serverworker.h"
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>

ChatServer::ChatServer(QObject *parent):
    QTcpServer(parent)
{

}

void ChatServer::incomingConnection(qintptr socketDescriptor)
{
    serverWorker *worker = new serverWorker(this);
    if(!worker->setSocketDescriptor(socketDescriptor)){
        worker->deleteLater();
        return;
    }
    connect(worker,&serverWorker::logMessage,this,&ChatServer::logMessage);
    connect(worker,&serverWorker::jsonReceived,this,&ChatServer::jsonReceived);
    // 【修改 1】连接断开信号
    connect(worker, &serverWorker::disconnectFromClient, this, &ChatServer::userDisconnected);
    m_clients.append(worker);
    emit logMessage("新用户连接");
}

void ChatServer::broadCast(const QJsonObject &message, serverWorker *exclude)
{
    for(serverWorker *worker : m_clients){
        worker->sendJson(message);
    }
}

void ChatServer::stopServer()
{
    close();
}

void ChatServer::jsonReceived(serverWorker *sender, const QJsonObject &docObj)
{
    const QJsonValue typeVal = docObj.value("type");
    if(typeVal.isNull()||!typeVal.isString())
        return;
    if(typeVal.toString().compare("message",Qt::CaseInsensitive)==0){
        const QJsonValue textVal = docObj.value("text");
        if(textVal.isNull()||!textVal.isString())
            return;
        const QString text = textVal.toString().trimmed();
        if(text.isEmpty())
            return;
        QJsonObject message;
        message["type"] = "message";
        message["text"] = text;
        message["sender"]=sender->getuserName();
        broadCast(message,sender);
    }
    else if(typeVal.toString().compare("login",Qt::CaseInsensitive)==0){
        const QJsonValue userNameVal = docObj.value("text");
        if(userNameVal.isNull()||!userNameVal.isString())
            return;
        QString newName = userNameVal.toString().trimmed();

        // --- 【新增】检查用户名是否重复 ---
        for (serverWorker *worker : m_clients) {
            if (worker == sender) continue; // 跳过自己
            if (worker->getuserName() == newName) {
                // 发现重名！发送失败消息给这个客户端
                QJsonObject failMsg;
                failMsg["type"] = "login";
                failMsg["success"] = false;
                failMsg["reason"] = "用户名已存在";
                sender->sendJson(failMsg);
                return; // 结束，不再进行后续的广播和列表更新
            }
        }
        // ------------------------------
        sender->setUserName(newName);
        QJsonObject connectedMessage;
        connectedMessage["type"]="newuser";
        connectedMessage["username"]=userNameVal.toString();
        broadCast(connectedMessage,sender);

        // --- 【新增代码】 ---
        // 3. 给这个新用户(sender)发送一份当前在线用户列表
        QJsonObject userListMessage;
        userListMessage["type"] = "userlist";
        QJsonArray userArray;

        for (serverWorker *worker : m_clients) {
            // 遍历所有在线客户端，把有名字的加进去
            // (如果不希望把自己包含在列表中，可以加 if (worker == sender) continue;)
            //if (worker == sender) continue; // 这里选择不包含自己
            if (!worker->getuserName().isEmpty()) {
                userArray.append(worker->getuserName());
            }
        }

        userListMessage["userlist"] = userArray;
        sender->sendJson(userListMessage); // 单独发给新用户
        // -------------------
    }
}

void ChatServer::userDisconnected()
{
    serverWorker *worker = qobject_cast<serverWorker *>(sender());
    if(!worker) return;

    m_clients.removeAll(worker); // 从列表中移除
    QString userName = worker->getuserName();

    if(!userName.isEmpty()) {
        emit logMessage(userName + " 断开连接");
        // --- 【新增代码：告诉其他人他走了】 ---
        QJsonObject disconnectedMessage;
        disconnectedMessage["type"] = "userdisconnected";
        disconnectedMessage["username"] = userName;

        // 广播给剩下的人 (nullptr 表示不排除任何人，发给所有还在列表里的人)
        broadCast(disconnectedMessage, nullptr);
        // 可选：在这里广播一条 "用户离开" 的 JSON 消息给其他人
    }

    worker->deleteLater(); // 销毁对象
}
