#ifndef SERVERWORKER_H
#define SERVERWORKER_H

#include <QObject>
#include <QTcpSocket>

class serverWorker : public QObject
{
    Q_OBJECT
public:
    explicit serverWorker(QObject *parent = nullptr);
    virtual bool setSocketDescriptor(qintptr socketDescriptor);
    virtual QString getuserName();
    virtual void setUserName(QString username);


private:
    QTcpSocket * m_serverSocket;
    QString m_userName;

public slots:
    void onReadyRead();
    void sendMessage(const QString &text,const QString &type = "message");
    void sendJson(const QJsonObject &json);

signals:
    void logMessage(const QString &msg);
    void jsonReceived(serverWorker *sender,const QJsonObject &docObj);
    void disconnectFromClient(); // 【新增】断开连接信号
};


#endif // SERVERWORKER_H
