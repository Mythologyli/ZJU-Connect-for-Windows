#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QNetworkReply>

#include "zjuconnectcontroller/zjuconnectcontroller.h"
#include "networkdetector/networkdetector.h"
#include "settingwindow/settingwindow.h"
#include "QrCodeGenerator/QrCodeGenerator.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

    ~MainWindow() override;

public slots:
    void cleanUpWhenQuit();

signals:
    void SetModeFinished();

protected:
    void closeEvent(QCloseEvent* e) override;

private:
    void upgradeSettings();

    void clearLog();

    void addLog(const QString& log);

    void showNotification(
        const QString& title,
        const QString& content,
        QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon::Information
    );

    void setModeToL2tp();

    void setModeToWebLogin();

    void setModeToZjuConnect();

    Ui::MainWindow* ui;
    QSystemTrayIcon* trayIcon;
    QMenu* trayMenu;
    QAction* trayShowAction;
    QAction* trayCloseAction;
    ZjuConnectController* zjuConnectController;
    NetworkDetector* networkDetector;
    QNetworkAccessManager* networkAccessManager;
    QSettings* settings;
    QProcess* process;
    QProcess* processForL2tp;
    QProcess* processForL2tpCheck;
    QProcess* processForWebLogin;
    QTimer* l2tpCheckTimer;
    QrCodeGenerator qrGenerator;

    QObject* diagnosisContext;

    SettingWindow* settingWindow;

    QString mode;
    NetworkDetectResult networkDetectResult;

    bool isFirstTimeSetMode;

    bool isL2tpLinked;
    bool isL2tpReconnecting;

    bool isWebLogged;

    bool isZjuConnectLinked;
    bool isZjuConnectLoginError;
    bool isZjuConnectAccessDenied;
    bool isSystemProxySet;
};

#endif //MAINWINDOW_H
