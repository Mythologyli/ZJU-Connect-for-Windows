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
    isSystemProxySet = false;
    logLineCount = 0;

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
                auto zjuRuleWindow = new QWidget(this);
                zjuRuleWindow->setWindowTitle("ZJU Rule");
                zjuRuleWindow->setWindowIcon(QIcon(QPixmap(":/resource/icon.png").scaled(
                    512, 512, Qt::KeepAspectRatio, Qt::SmoothTransformation
                )));
                zjuRuleWindow->resize(400, 100);

                QString socks5Url = QString("tg://socks?server=127.0.0.1&port=") +
                                    ui->socks5PortSpinBox->text() +
                                    "&remarks=ZJU Connect";

                zjuRuleWindow->setLayout(new QVBoxLayout());

                QString socks5UrlLabelText = "你可以将以下订阅链接和你的机场订阅一起添加到 <a href='https://zjurule.xyz'>ZJU Rule</a> 中：";

                auto *socks5UrlLabel = new QLabel(socks5UrlLabelText);
                socks5UrlLabel->setTextFormat(Qt::RichText);
                socks5UrlLabel->setOpenExternalLinks(true);
                socks5UrlLabel->setAlignment(Qt::AlignLeft);
                zjuRuleWindow->layout()->addWidget(socks5UrlLabel);

                auto *socks5LineEdit = new QLineEdit(socks5Url);
                socks5LineEdit->setReadOnly(true);
                zjuRuleWindow->layout()->addWidget(socks5LineEdit);

                QString aboutLabelText = "不明白 ZJU Rule 是什么？请访问<a href='https://www.cc98.org/topic/5257184'>这个帖子</a>";
                auto *aboutLabel = new QLabel(aboutLabelText);
                aboutLabel->setTextFormat(Qt::RichText);
                aboutLabel->setOpenExternalLinks(true);
                aboutLabel->setAlignment(Qt::AlignLeft);
                zjuRuleWindow->layout()->addWidget(aboutLabel);

                zjuRuleWindow->setWindowFlag(Qt::Window);
                zjuRuleWindow->setWindowModality(Qt::WindowModal);
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
                                    "<br><br>ZJUConnect" +
                                    "<br>ZJU RVPN 客户端的 Go 语言实现，基于 EasierConnect" +
                                    "<br>作者: <a href='https://myth.cx'>Myth</a>" +
                                    "<br>项目主页: <a href='https://github.com/Mythologyli/ZJU-Connect'>https://github.com/Mythologyli/ZJU-Connect</a>" +
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
                        ui->logTextBrowser->append(QString(zjuConnectProcess->readAllStandardOutput()));
                    });

                    connect(zjuConnectProcess, &QProcess::readyReadStandardError, this, [&]()
                    {
                        ui->logTextBrowser->append(QString(zjuConnectProcess->readAllStandardError()));
                    });

                    connect(zjuConnectProcess, &QProcess::finished, this, [&]()
                    {
                        if (ui->autoReconnectCheckBox->isChecked() && isLinked)
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

                    if (ui->parseServerCheckBox->isChecked())
                    {
                        args.append("-parse");
                    }

                    if (ui->parseZjuCheckBox->isChecked())
                    {
                        args.append("-parse-zju");
                    }

                    if (ui->useZjuDnsCheckBox->isChecked())
                    {
                        args.append("-use-zju-dns");
                    }

                    if (ui->proxyAllCheckBox->isChecked())
                    {
                        args.append("-proxy-all");
                    }

                    if (ui->debugCheckBox->isChecked())
                    {
                        args.append("-debug-dump");
                    }

                    zjuConnectProcess->start("ZJUConnect.exe", args);
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
