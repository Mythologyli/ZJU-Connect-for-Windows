#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QProcess>
#include <QNetworkReply>
#include <QSettings>

#include "zjurulewindow/zjurulewindow.h"
#include "portforwardingwindow/portforwardingwindow.h"
#include "zjuconnectcontroller/zjuconnectcontroller.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *e) override;

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QAction *trayShowAction;
    QAction *trayCloseAction;
    ZjuConnectController *zjuConnectController;
    QNetworkAccessManager *networkAccessManager;
    QSettings *settings;

    ZjuRuleWindow *zjuRuleWindow;
    PortForwardingWindow *portForwardingWindow;

    bool isLinked;
    bool isLoginError;
    bool isSystemProxySet;

    QString tcpPortForwarding;
    QString udpPortForwarding;
};

#endif //MAINWINDOW_H
