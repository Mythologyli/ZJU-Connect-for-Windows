#include <QStandardPaths>
#include <QMessageBox>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    zjuConnectProcess = nullptr;
    networkAccessManager = new QNetworkAccessManager(this);
    settings = new QSettings(
        QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/config.ini",
        QSettings::IniFormat
    );
    isLinked = false;
    isLoginError = false;
    isSystemProxySet = false;

    ui->setupUi(this);

    setWindowIcon(QIcon(QPixmap(":/resource/icon.png").scaled(
        512, 512, Qt::KeepAspectRatio, Qt::SmoothTransformation
    )));

    setWindowTitle(QApplication::applicationName() + " v" + QApplication::applicationVersion());

    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);

    // 读取配置
    ui->serverAddressLineEdit->setText(settings->value("EasyConnect/ServerAddress", "rvpn.zju.edu.cn").toString());
    ui->serverPortSpinBox->setValue(settings->value("EasyConnect/ServerPort", 443).toInt());
    ui->usernameLineEdit->setText(settings->value("EasyConnect/Username", "").toString());
    ui->passwordLineEdit->setText(
        QByteArray::fromBase64(settings->value("EasyConnect/Password", "").toString().toUtf8())
    );
    ui->socks5PortSpinBox->setValue(settings->value("ZJUConnect/Socks5Port", 1080).toInt());
    ui->httpPortSpinBox->setValue(settings->value("ZJUConnect/HttpPort", 1081).toInt());
    ui->parseServerCheckBox->setChecked(settings->value("ZJUConnect/ParseServer", true).toBool());
    ui->parseZjuCheckBox->setChecked(settings->value("ZJUConnect/ParseZju", true).toBool());
    ui->useZjuDnsCheckBox->setChecked(settings->value("ZJUConnect/UseZjuDns", true).toBool());
    ui->proxyAllCheckBox->setChecked(settings->value("ZJUConnect/ProxyAll", false).toBool());
    ui->debugCheckBox->setChecked(settings->value("ZJUConnect/Debug", false).toBool());
    ui->autoReconnectCheckBox->setChecked(settings->value("GUI/AutoReconnect", false).toBool());
    ui->reconnectTimeSpinBox->setValue(settings->value("GUI/ReconnectTime", 1).toInt());

    // 系统托盘
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(QIcon(QPixmap(":/resource/icon.png").scaled(
        512, 512, Qt::KeepAspectRatio, Qt::SmoothTransformation
    )));
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

    // 高级-ZJU Rule
    connect(ui->zjuRuleAction, &QAction::triggered,
            [&]()
            {
                zjuRuleWindow = new ZjuRuleWindow(this);
                zjuRuleWindow->setSocks5Port(ui->socks5PortSpinBox->text());
                zjuRuleWindow->show();
            });

    // 帮助-关于本软件
    connect(ui->aboutAction, &QAction::triggered,
            [&]()
            {
                QMessageBox messageBox(this);
                messageBox.setWindowTitle("关于本软件");
                messageBox.setTextFormat(Qt::RichText);
                QString aboutText = QApplication::applicationName() + " v" + QApplication::applicationVersion() +
                                    "<br>基于 Qt 编写的 ZJUConnect 图形界面" +
                                    "<br>作者: <a href='https://myth.cx'>Myth</a>" +
                                    "<br>项目主页: <a href='https://github.com/Mythologyli/ZJU-Connect-for-Windows'>https://github.com/Mythologyli/ZJU-Connect-for-Windows</a>" +
                                    "<br><br>zju-connect" +
                                    "<br>ZJU RVPN 客户端的 Go 语言实现，基于 EasierConnect" +
                                    "<br>作者: <a href='https://myth.cx'>Myth</a>" +
                                    "<br>项目主页: <a href='https://github.com/Mythologyli/zju-connect'>https://github.com/Mythologyli/zju-connect</a>" +
                                    "<br><br>EasierConnect" +
                                    "<br>EasyConnect 客户端的开源实现" +
                                    "<br>作者: <a href='https://github.com/lyc8503'>lyc8503</a>" +
                                    "<br>项目主页: <a href='https://github.com/lyc8503/EasierConnect'>https://github.com/lyc8503/EasierConnect</a>";
                messageBox.setText(aboutText);
                messageBox.setIconPixmap(QPixmap(":/resource/icon.png").scaled(
                    100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation
                ));
                messageBox.exec();
            });

    // 连接服务器
    connect(ui->linkPushButton, &QPushButton::clicked,
            [&]()
            {
                if (!isLinked)
                {
                    if (ui->serverAddressLineEdit->text().isEmpty())
                    {
                        QMessageBox::warning(this, "警告", "服务器地址不能为空");
                        return;
                    }

                    if (ui->usernameLineEdit->text().isEmpty())
                    {
                        QMessageBox::warning(this, "警告", "用户名不能为空");
                        return;
                    }

                    if (ui->passwordLineEdit->text().isEmpty())
                    {
                        QMessageBox::warning(this, "警告", "密码不能为空");
                        return;
                    }

                    ui->logTextBrowser->clear();

                    zjuConnectProcess = new QProcess(this);

                    connect(zjuConnectProcess, &QProcess::readyReadStandardOutput, this, [&]()
                    {
                        QString output = QString(zjuConnectProcess->readAllStandardOutput());

                        if (output.contains("Login FAILED"))
                        {
                            isLoginError = true;
                        }

                        ui->logTextBrowser->append(output);
                    });

                    connect(zjuConnectProcess, &QProcess::readyReadStandardError, this, [&]()
                    {
                        QString output = QString(zjuConnectProcess->readAllStandardError());

                        if (output.contains("Login FAILED"))
                        {
                            isLoginError = true;
                        }

                        ui->logTextBrowser->append(output);
                    });

                    connect(zjuConnectProcess, &QProcess::finished, this, [&]()
                    {
                        if (!isLoginError && ui->autoReconnectCheckBox->isChecked() && isLinked)
                        {
                            QTimer::singleShot(ui->reconnectTimeSpinBox->value() * 1000, this, [&]()
                            {
                                if (isLinked)
                                {
                                    delete zjuConnectProcess;
                                    zjuConnectProcess = nullptr;

                                    isLinked = false;
                                    ui->linkPushButton->click();
                                }
                            });

                            return;
                        }

                        delete zjuConnectProcess;
                        zjuConnectProcess = nullptr;

                        isLinked = false;
                        ui->linkPushButton->setText("连接服务器");

                        if (isLoginError)
                        {
                            isLoginError = false;
                            QMessageBox::warning(this, "警告", "登录失败");
                        }
                    });

                    QList<QString> args = QStringList({
                                                          "-password",
                                                          ui->passwordLineEdit->text(),
                                                          "-username",
                                                          ui->usernameLineEdit->text(),
                                                          "-server",
                                                          ui->serverAddressLineEdit->text(),
                                                          "-port",
                                                          ui->serverPortSpinBox->text(),
                                                          "-socks-bind",
                                                          ":" + ui->socks5PortSpinBox->text(),
                                                          "-http-bind",
                                                          ":" + ui->httpPortSpinBox->text()
                                                      });

                    if (!ui->parseServerCheckBox->isChecked())
                    {
                        args.append("-disable-server-config");
                    }

                    if (!ui->parseZjuCheckBox->isChecked())
                    {
                        args.append("-disable-zju-config");
                    }

                    if (!ui->useZjuDnsCheckBox->isChecked())
                    {
                        args.append("-disable-zju-dns");
                    }

                    if (ui->proxyAllCheckBox->isChecked())
                    {
                        args.append("-proxy-all");
                    }

                    if (ui->debugCheckBox->isChecked())
                    {
                        args.append("-debug-dump");
                    }

                    zjuConnectProcess->start("zju-connect.exe", args);
                    zjuConnectProcess->waitForStarted();

                    isLinked = true;
                    ui->linkPushButton->setText("断开服务器");
                }
                else
                {
                    isLinked = false;

                    zjuConnectProcess->kill();
                    zjuConnectProcess->waitForFinished();

                    ui->linkPushButton->setText("连接服务器");
                    QMessageBox::information(this, "提示", "已断开服务器");
                }
            });

    // 设置系统代理
    connect(ui->systemProxyPushButton, &QPushButton::clicked,
            [&]()
            {
                QSettings proxySettings(
                    R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Internet Settings)",
                    QSettings::NativeFormat
                );

                if (!isSystemProxySet)
                {
                    proxySettings.setValue("ProxyEnable", 1);
                    proxySettings.setValue("ProxyServer", "127.0.0.1:" + ui->httpPortSpinBox->text());
                    proxySettings.setValue("ProxyOverride", "");

                    ui->systemProxyPushButton->setText("清除系统代理");

                    isSystemProxySet = true;
                }
                else
                {
                    proxySettings.setValue("ProxyEnable", 0);
                    proxySettings.setValue("ProxyServer", "");
                    proxySettings.setValue("ProxyOverride", "");

                    ui->systemProxyPushButton->setText("设置系统代理");

                    isSystemProxySet = false;
                }
            });

    // 检查更新
    QNetworkRequest request(QUrl("https://zjuconnect.myth.cx/version.json"));
    networkAccessManager->get(request);

    connect(networkAccessManager, &QNetworkAccessManager::finished,
            [&](QNetworkReply *reply)
            {
                if (reply->error() == QNetworkReply::NoError)
                {
                    QJsonDocument jsonDocument = QJsonDocument::fromJson(reply->readAll());
                    QJsonObject jsonObject = jsonDocument.object();
                    QString version = jsonObject["version"].toString();
                    QString url = jsonObject["url"].toString();

                    if (version != QApplication::applicationVersion())
                    {
                        QString text = "有新版本可用：v" + version +
                                       "<br>点击<a href='" + url + "'>此处</a>下载";

                        ui->statusLabel->setText(text);
                    }
                }
            });
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    hide();
}

MainWindow::~MainWindow()
{
    // 保存配置
    settings->setValue("EasyConnect/ServerAddress", ui->serverAddressLineEdit->text());
    settings->setValue("EasyConnect/ServerPort", ui->serverPortSpinBox->value());
    settings->setValue("EasyConnect/Username", ui->usernameLineEdit->text());
    settings->setValue("EasyConnect/Password", QString(ui->passwordLineEdit->text().toUtf8().toBase64()));
    settings->setValue("ZJUConnect/Socks5Port", ui->socks5PortSpinBox->value());
    settings->setValue("ZJUConnect/HttpPort", ui->httpPortSpinBox->value());
    settings->setValue("ZJUConnect/ParseServer", ui->parseServerCheckBox->isChecked());
    settings->setValue("ZJUConnect/ParseZju", ui->parseZjuCheckBox->isChecked());
    settings->setValue("ZJUConnect/UseZjuDns", ui->useZjuDnsCheckBox->isChecked());
    settings->setValue("ZJUConnect/ProxyAll", ui->proxyAllCheckBox->isChecked());
    settings->setValue("ZJUConnect/Debug", ui->debugCheckBox->isChecked());
    settings->setValue("GUI/AutoReconnect", ui->autoReconnectCheckBox->isChecked());
    settings->setValue("GUI/ReconnectTime", ui->reconnectTimeSpinBox->value());
    settings->sync();

    if (zjuConnectProcess != nullptr)
    {
        zjuConnectProcess->kill();
        zjuConnectProcess->waitForFinished();
    }

    delete ui;
}
