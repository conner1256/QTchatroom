#include "serverworker.h"
#include<QJsonObject>
#include<QJsonDocument>
#include<QDataStream>
serverWorker::serverWorker(QObject *parent)
    : QObject{parent}
{
    m_serverSocket = new QTcpSocket(this);
    connect(m_serverSocket,&QTcpSocket::readyRead,this,&serverWorker::onReadyRead);
    // 【修改 1】当 Socket 断开时，发送 disconnectFromClient 信号
    connect(m_serverSocket, &QTcpSocket::disconnected, this, &serverWorker::disconnectFromClient);
}

bool serverWorker::setSocketDescriptor(qintptr socketDescriptor)
{
    return m_serverSocket->setSocketDescriptor(socketDescriptor);
}

QString serverWorker::getuserName()
{
    return m_userName;
}

void serverWorker::setUserName(QString username)
{
    m_userName = username;
}

void serverWorker::onReadyRead()
{
    QByteArray jsonData;
    QDataStream socketStream(m_serverSocket);
    socketStream.setVersion(QDataStream::Qt_5_12);
    for(;;){
        socketStream.startTransaction();
        socketStream>>jsonData;
        if(socketStream.commitTransaction()){
            //emit logMessage(QString::fromUtf8(jsonData));
            //sendMessage("I received message");
            QJsonParseError parseError;
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData,&parseError);
            if(parseError.error == QJsonParseError::NoError){
                if(jsonDoc.isObject()){
                    //emit logMessage(QJsonDocument(jsonDoc).toJson(QJsonDocument::Compact));
                    emit jsonReceived(this,jsonDoc.object());
                }
            }
        }
        else{
            break;
        }
    }
}

void serverWorker::sendMessage(const QString &text, const QString &type)
{
    if(m_serverSocket->state()!=QAbstractSocket::ConnectedState)
        return;
    if(!text.isEmpty()){
        QDataStream serverStream(m_serverSocket);
        serverStream.setVersion(QDataStream::Qt_5_12);
        QJsonObject message;
        message["type"]=type;
        message["text"]=text;
        serverStream<<QJsonDocument(message).toJson();
    }
}

void serverWorker::sendJson(const QJsonObject &json)
{
    // 【修改 3】必须先检查连接状态！
    // 如果连接已断开，直接返回，不要打印日志，也不要写数据
    if(m_serverSocket->state() != QAbstractSocket::ConnectedState) {
        return;
    }
    const QByteArray jsonData = QJsonDocument(json).toJson(QJsonDocument::Compact);
    emit logMessage(QLatin1String("sending to:")+getuserName()+QLatin1String("-")+QString::fromUtf8(jsonData));
    QDataStream socketStream(m_serverSocket);
    socketStream.setVersion(QDataStream::Qt_5_12);
    socketStream<<jsonData;
}
