#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QObject>
#include <QTcpServer>
#include "serverworker.h"

class ChatServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit ChatServer(QObject *parent = nullptr);
protected:
    void incomingConnection(qintptr socketDescriptor) override;
    QVector<serverWorker *>m_clients;
    void broadCast(const QJsonObject &message,serverWorker *exclude);

public slots:
    void stopServer();
    void jsonReceived(serverWorker *sender,const QJsonObject &docObj);
    void userDisconnected(); // 【新增】处理用户断开

signals:
    void logMessage(const QString &msg);

};

#endif // CHATSERVER_H
