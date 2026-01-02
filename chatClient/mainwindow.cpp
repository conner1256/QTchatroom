#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "chatclient.h"
#include <QHostAddress>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentWidget(ui->loginPage);
    m_chatClient = new chatClient(this);
    connect(m_chatClient,&chatClient::connected,this,&MainWindow::connectedToServer);
    connect(m_chatClient,&chatClient::messageReceived,this,&MainWindow::messageReceived);

    // 1. 【新增】处理登录失败
    connect(m_chatClient, &chatClient::loginError, this, [&](const QString &reason){
        QMessageBox::critical(this, "登录失败", reason);
        // 断开连接，让用户可以重新点击登录
        m_chatClient->disconnectFromHost();
        // 确保界面停留在登录页 (假设 index 0 是登录页)
        ui->stackedWidget->setCurrentIndex(0);
    });

    // 【新增】连接用户列表更新
    connect(m_chatClient, &chatClient::userJoined, this, [&](const QString &user){
        QString myName = m_chatClient->getUserName();

        if (user == myName) {
            QListWidgetItem *item = new QListWidgetItem("*" + user);
            item->setForeground(Qt::red);
            ui->userListWidget->addItem(item);
        } else {
            ui->userListWidget->addItem(user);
        }
    });

    connect(m_chatClient, &chatClient::userLeft, this, [&](const QString &user){
        // 查找并删除离开的用户
        QList<QListWidgetItem *> items = ui->userListWidget->findItems(user, Qt::MatchExactly);
        for(auto item : items){
            delete item; // 删除该行
        }
    });

    connect(m_chatClient, &chatClient::userListReceived, this, [&](const QStringList &users){
        ui->userListWidget->clear();

        // 获取我自己的名字
        QString myName = m_chatClient->getUserName();

        for(const QString &user : users) {
            if (user == myName) {
                // 如果是自己，添加星号，并(可选)设置特殊颜色
                QListWidgetItem *item = new QListWidgetItem("*" + user);
                item->setForeground(Qt::red); // 比如设为红色，更显眼
                ui->userListWidget->addItem(item);
            } else {
                // 别人正常显示
                ui->userListWidget->addItem(user);
            }
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btn_publish_clicked()
{
    QString text = ui->chatLineEdit->text().trimmed(); // 去除首尾空格
    if (text.isEmpty()) {
        return;
    }

    // 发送消息
    m_chatClient->sendMessage(text);

    // 3. 【新增】发送后清空输入框
    ui->chatLineEdit->clear();

    // 可选：发送后让输入框继续保持焦点，方便连续打字
    ui->chatLineEdit->setFocus();
}


void MainWindow::on_btn_retreat_clicked()
{
    m_chatClient->disconnectFromHost();
    ui->stackedWidget->setCurrentWidget(ui->loginPage);

}


void MainWindow::on_btn_Login_clicked()
{
    // 1. 获取界面上的 IP 和 用户名
    QString ip = ui->serverEdit->text();
    QString userName = ui->userNameEdit->text();

    // 2. 调用修改后的函数，传入 3 个参数：地址、端口、用户名
    // 注意：这里只需要发起连接，不要在这里调用 sendMessage！
    // 登录消息会在连接成功后的 onConnected 槽函数中自动发送。
    m_chatClient->connectToServer(QHostAddress(ip), 1967, userName);
}
void MainWindow::connectedToServer()
{
    ui->stackedWidget->setCurrentWidget(ui->chatPage);
}

void MainWindow::messageReceived(const QString &text)
{
    ui->chatRoomEdit->append(text);
}

