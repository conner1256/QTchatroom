#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "chatclient.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btn_publish_clicked();

    void on_btn_retreat_clicked();

    void on_btn_Login_clicked();
    void connectedToServer();
    void messageReceived(const QString &text);

private:
    Ui::MainWindow *ui;
    chatClient *m_chatClient;
};
#endif // MAINWINDOW_H
