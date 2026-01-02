#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_chatServer = new ChatServer(this);
    connect(m_chatServer,&ChatServer::logMessage,this,&MainWindow::logMessage);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_controlButton_clicked()
{
    // 1. 直接判断服务器当前的真实状态
    if (m_chatServer->isListening()) {
        // --- 当前正在运行，所以逻辑是：停止服务器 ---

        m_chatServer->stopServer();

        // 停止后，按钮功能应变为“启动”，提示变为“已停止”
        ui->controlButton->setText("启动服务器");
        logMessage("已停止服务器");
    }
    else {
        // --- 当前未运行，所以逻辑是：启动服务器 ---

        if (!m_chatServer->listen(QHostAddress::Any, 1967)) {
            QMessageBox::critical(this, "error", "无法启动服务器");
            return;
        }

        // 启动成功后，按钮功能应变为“关闭”，提示变为“已启动”
        ui->controlButton->setText("关闭服务器");
        logMessage("已启动服务器");
    }
}

void MainWindow::logMessage(const QString &msg)
{
    ui->logEdit->appendPlainText(msg);
}
