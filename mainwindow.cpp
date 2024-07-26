#include <QStandardPaths>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSysInfo>
#include <windows.h>
#include <shellapi.h>
#include <QNetworkInterface>
#include <QClipboard>
#include <QDesktopServices>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "zjuconnectcontroller/zjuconnectcontroller.h"
#include "utils/utils.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    zjuConnectController = nullptr;

    networkAccessManager = new QNetworkAccessManager(this);

    process = new QProcess(this);
    processForL2tp = new QProcess(this);
    processForL2tpCheck = new QProcess(this);
    processForWebLogin = new QProcess(this);
    settings = new QSettings(
        QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/config.ini",
        QSettings::IniFormat
    );
    l2tpCheckTimer = nullptr;
    diagnosisContext = nullptr;

    upgradeSettings();

    isFirstTimeSetMode = true;
    isL2tpLinked = false;
    isL2tpReconnecting = false;
    isWebLogged = false;
    isZjuConnectLinked = false;
    isZjuConnectLoginError = false;
    isZjuConnectAccessDenied = false;
    isSystemProxySet = false;

    ui->setupUi(this);

    setWindowIcon(QIcon(QPixmap(":/resource/icon.png").scaled(
        512, 512, Qt::KeepAspectRatio, Qt::SmoothTransformation
    )));

    setWindowTitle(QApplication::applicationName() + " v" + QApplication::applicationVersion());

    ui->versionLabel->setText(
        "Version: " + QApplication::applicationVersion() + "\n" + QSysInfo::prettyProductName() + "\n"
    );

    // 系统托盘
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(QPixmap(":/resource/icon.png").scaled(
        512, 512, Qt::KeepAspectRatio, Qt::SmoothTransformation
    )));
    trayIcon->setVisible(true);
    trayIcon->setToolTip(QApplication::applicationName());
    connect(trayIcon, &QSystemTrayIcon::activated, this, [&](QSystemTrayIcon::ActivationReason reason)
    {
        switch (reason)
        {
            case QSystemTrayIcon::Context:
                trayMenu->exec(QCursor::pos());
                break;
            default:
                show();
                setWindowState(Qt::WindowState::WindowActive);
        }
    });
    trayIcon->show();

    trayShowAction = new QAction("显示", this);
    trayCloseAction = new QAction("退出", this);
    trayMenu = new QMenu(this);
    trayMenu->addAction(trayShowAction);
    trayMenu->addAction(trayCloseAction);
    connect(trayShowAction, &QAction::triggered, this, [&]()
    {
        show();
        setWindowState(Qt::WindowState::WindowActive);
    });
    connect(trayCloseAction, &QAction::triggered, this, [&]()
    {
        QApplication::quit();
    });

    // 文件-退出
    connect(ui->exitAction, &QAction::triggered,
            [&]()
            {
                QApplication::quit();
            });

    // 文件-设置
    connect(ui->settingAction, &QAction::triggered, this,
            [&]()
            {
                settingWindow = new SettingWindow(this, settings);
                settingWindow->show();
            });

    // 帮助-清除系统代理
    connect(ui->disableProxyAction, &QAction::triggered,
            [&]()
            {
                QMessageBox messageBox(this);
                messageBox.setWindowTitle("清除系统代理");
                messageBox.setText("是否清除系统代理？");

                messageBox.addButton(QMessageBox::Yes)->setText("是");
                messageBox.addButton(QMessageBox::No)->setText("否");
                messageBox.setDefaultButton(QMessageBox::Yes);

                if (messageBox.exec() == QMessageBox::No)
                {
                    return;
                }

                if (isSystemProxySet)
                {
                    ui->pushButton2->click();
                }
                else
                {
                    Utils::clearSystemProxy();
                }

                addLog("已清除系统代理设置");
            });


    // 帮助-关于本软件
    connect(ui->aboutAction, &QAction::triggered,
            [&]()
            {
                Utils::showAboutMessageBox(this);
            });

    // 复制日志
    connect(ui->copyLogPushButton, &QPushButton::clicked,
            [&]()
            {
                auto logText = ui->logPlainTextEdit->toPlainText();
                QApplication::clipboard()->setText(logText);
            }
    );

    connect(this, &MainWindow::SetModeFinished, this, [&]()
    {
        if (isFirstTimeSetMode)
        {
            isFirstTimeSetMode = false;
            if (settings->value("Common/ConnectAfterStart", false).toBool())
            {
                ui->pushButton1->click();
            }
        }
    });

    
    setModeToZjuConnect();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    hide();
}

void MainWindow::addLog(const QString &log)
{
    QString timeString = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    ui->logPlainTextEdit->appendPlainText(timeString + " " + log);
}

void MainWindow::clearLog()
{
    ui->logPlainTextEdit->clear();
    ui->logPlainTextEdit->appendPlainText(
        "Version: " + QApplication::applicationVersion() + "\n" + QSysInfo::prettyProductName());
}

void MainWindow::upgradeSettings()
{
    int configVersion = settings->value("Common/ConfigVersion", 1).toInt();
    if (configVersion > 2)
    {
        addLog("警告：配置文件版本高于 2。请运行新版本！");
    }
    else if (configVersion == 2)
    {
        if (settings->contains("Common/AutoStart"))
        {
            if (settings->value("Common/AutoStart").toBool())
            {
                QSettings autoStartSettings(
                    R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run)",
                    QSettings::NativeFormat
                );
                autoStartSettings.setValue(
                    "HITszConnectForWindows",
                    QCoreApplication::applicationFilePath().replace('/', '\\')
                );
            }
        }

        return;
    }
    settings->sync();
}

void MainWindow::showNotification(const QString &title, const QString &content, QSystemTrayIcon::MessageIcon icon)
{
    disconnect(trayIcon, &QSystemTrayIcon::messageClicked, nullptr, nullptr);
    trayIcon->showMessage(
        title,
        content,
        icon,
        10000
    );

    connect(trayIcon, &QSystemTrayIcon::messageClicked, this, [&]()
    {
        disconnect(trayIcon, &QSystemTrayIcon::messageClicked, nullptr, nullptr);

        show();
        setWindowState(Qt::WindowState::WindowActive);
    });
}

void MainWindow::cleanUpWhenQuit()
{
    // 保存配置
    if (settings->value("Common/ConfigVersion", "1").toInt() <= 2)
    {
        settings->setValue("Common/ConfigVersion", 2);
        settings->setValue("Common/LastMode", mode);
        settings->setValue("ZJUConnect/TunMode", ui->tunCheckBox->isChecked());
        settings->sync();
    }

    // 清除系统代理
    if (isSystemProxySet)
    {
        Utils::clearSystemProxy();
    }
}

MainWindow::~MainWindow()
{
    if (zjuConnectController != nullptr)
    {
        disconnect(zjuConnectController, &ZjuConnectController::finished, nullptr, nullptr);
        delete zjuConnectController;
    }

    delete ui;
}
