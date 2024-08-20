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
    isZjuConnectLinked = false;
    isSystemProxySet = false;
    zjuConnectError = ZJU_ERROR::NONE;

    ui->setupUi(this);

    setWindowIcon(QIcon(QPixmap(":/resource/icon.png").scaled(
        512, 512, Qt::KeepAspectRatio, Qt::SmoothTransformation
    )));

    ui->applicationNameLabel->setText(QApplication::applicationDisplayName());
    ui->versionLabel->setText(
        "当前版本：v" + QApplication::applicationVersion() + "\n正在检查更新……\n"
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

    // 帮助-检查更新
    connect(ui->checkUpdateAction, &QAction::triggered, this,
            [&]()
            {
                checkUpdate();
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

    connect(this, &MainWindow::SetModeFinished, this, 
			[&]()
		    {
		        if (isFirstTimeSetMode)
		        {
		            isFirstTimeSetMode = false;
		            if (settings->value("Common/ConnectAfterStart", false).toBool())
		            {
		                ui->pushButton1->click();
		            }
		        }
			}
    );

    // 自动检查更新
    checkUpdateNAM = new QNetworkAccessManager(this);

    connect(checkUpdateNAM, &QNetworkAccessManager::finished,
        this, [&](QNetworkReply* reply) {
            if (reply->error() != QNetworkReply::NoError)
            {
                addLog("检查更新失败。原因是：" + reply->errorString());
                ui->versionLabel->setText(
                    "当前版本：v" + QApplication::applicationVersion() + "\n检查更新失败\n"
                );
                reply->deleteLater();
                return;
            }

            QJsonObject json = QJsonDocument::fromJson(reply->readAll()).object();
            reply->deleteLater();

            QString nowVersion = QApplication::applicationVersion();
            QString latestVersion = json["tag_name"].toString();
            addLog("检查更新成功。最新版本：" + latestVersion);
            ui->versionLabel->setText(
                "当前版本：v" + nowVersion + "\n"
                "最新版本：" + latestVersion + "\n"
            );

            // 移除开头的 'v'
            if (latestVersion.startsWith('v'))
            {
                latestVersion = latestVersion.mid(1);
            }

            qsizetype nowVersionSuffix, latestVersionSuffix;
            auto nowVersionQ = QVersionNumber::fromString(nowVersion, &nowVersionSuffix);
            auto latestVersionQ = QVersionNumber::fromString(latestVersion, &latestVersionSuffix);

            if (latestVersionQ > nowVersionQ || 
                (latestVersionQ == nowVersionQ && latestVersion.right(latestVersionSuffix) != nowVersion.right(nowVersionSuffix)))
            {
                QMessageBox msgBox;
                msgBox.setText("版本更新");
                msgBox.setInformativeText("存在版本更新：" + latestVersion + "\n"
                    "是否前往 Github 发布页面查看？");
                msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
                msgBox.setDefaultButton(QMessageBox::Ok);

                int ret = msgBox.exec();
                if (ret == QMessageBox::Ok)
                {
                    QDesktopServices::openUrl(QUrl("https://github.com/" + Utils::REPO_NAME + "/releases/latest"));
                }
            }
        });

    if (settings->value("Common/checkUpdateAfterStart", true).toBool())
    {
        checkUpdate();
    }
    else
    {
        ui->versionLabel->setText(
            "当前版本：v" + QApplication::applicationVersion() + "\n自动检查更新已禁用\n"
        );
    }
    
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
    ui->logPlainTextEdit->appendPlainText(timeString + " " + log.trimmed());
}

void MainWindow::clearLog()
{
    ui->logPlainTextEdit->clear();
    ui->logPlainTextEdit->appendPlainText(
        "欢迎使用 " + QApplication::applicationDisplayName() + "\n"
        "当前版本：v" + QApplication::applicationVersion() + "\n"
        "系统版本：" + QSysInfo::prettyProductName());
}

void MainWindow::checkUpdate()
{
    QNetworkRequest request(QUrl("https://api.github.com/repos/" + Utils::REPO_NAME + "/releases/latest"));
    checkUpdateNAM->get(request);
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
                    QApplication::applicationName(),
                    QCoreApplication::applicationFilePath().replace('/', '\\')
                );

                // 删除旧版自启动项
                autoStartSettings.remove("HITszConnectForWindows");
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
