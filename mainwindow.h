#ifndef QEASIERCONNECT_MAINWINDOW_H
#define QEASIERCONNECT_MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QProcess>
#include <QNetworkReply>
#include <QSettings>

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

    bool isLinked;
    bool isSystemProxySet;
    int logLineCount;
};

#endif //QEASIERCONNECT_MAINWINDOW_H
