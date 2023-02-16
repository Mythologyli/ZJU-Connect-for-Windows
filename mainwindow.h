#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QProcess>
#include <QNetworkReply>
#include <QSettings>

#include "zjurulewindow/zjurulewindow.h"

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
    QProcess *zjuConnectProcess;
    QNetworkAccessManager *networkAccessManager;
    QSettings *settings;

    ZjuruleWindow *zjuruleWindow;

    bool isLinked;
    bool isLoginError;
    bool isSystemProxySet;
};

#endif //MAINWINDOW_H
