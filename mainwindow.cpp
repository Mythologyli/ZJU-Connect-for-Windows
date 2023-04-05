#include <QStandardPaths>
#include <QMessageBox>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "zjuconnectcontroller/zjuconnectcontroller.h"
#include "utils/utils.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    zjuConnectController = new ZjuConnectController();
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
    ui->multiLineCheckBox->setChecked(settings->value("ZJUConnect/MultiLine", true).toBool());
    ui->proxyAllCheckBox->setChecked(settings->value("ZJUConnect/ProxyAll", false).toBool());
    ui->debugCheckBox->setChecked(settings->value("ZJUConnect/Debug", false).toBool());
    tcpPortForwarding = settings->value("ZJUConnect/TcpPortForwarding", "").toString();
    udpPortForwarding = settings->value("ZJUConnect/UdpPortForwarding", "").toString();
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

    // 高级-端口转发
    connect(ui->portForwardingAction, &QAction::triggered,
            [&]()
            {
                portForwardingWindow = new PortForwardingWindow(this);
                portForwardingWindow->setPortForwarding(tcpPortForwarding, udpPortForwarding);

                connect(portForwardingWindow, &PortForwardingWindow::applied, this,
                        [&](const QString &tcpForwarding, const QString &udpForwarding)
                        {
                            tcpPortForwarding = tcpForwarding;
                            udpPortForwarding = udpForwarding;
                        });

                portForwardingWindow->show();
            });

    // 帮助-关于本软件
    connect(ui->aboutAction, &QAction::triggered,
            [&]()
            {
                Utils::showAboutMessageBox(this);
            });

    // 连接服务器
    connect(zjuConnectController, &ZjuConnectController::outputRead, this, [&](const QString &output)
    {
        ui->logTextBrowser->append(output);
    });

    connect(zjuConnectController, &ZjuConnectController::loginFailed, this, [&]()
    {
        isLoginError = true;
    });

    connect(zjuConnectController, &ZjuConnectController::finished, this, [&]()
    {
        if (!isLoginError && ui->autoReconnectCheckBox->isChecked() && isLinked)
        {
            QTimer::singleShot(ui->reconnectTimeSpinBox->value() * 1000, this, [&]()
            {
                if (isLinked)
                {
                    zjuConnectController->stop();

                    isLinked = false;
                    ui->linkPushButton->click();
                }
            });

            return;
        }

        isLinked = false;
        ui->linkPushButton->setText("连接服务器");

        if (isLoginError)
        {
            isLoginError = false;
            QMessageBox::warning(this, "警告", "登录失败");
        }
        else
        {
            QMessageBox::information(this, "提示", "已断开连接");
        }
    });

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

                    zjuConnectController->start(
                        "zju-connect.exe",
                        ui->usernameLineEdit->text(),
                        ui->passwordLineEdit->text(),
                        ui->serverAddressLineEdit->text(),
                        ui->serverPortSpinBox->text().toInt(),
                        !ui->multiLineCheckBox->isChecked(),
                        ui->proxyAllCheckBox->isChecked(),
                        "127.0.0.1:" + ui->socks5PortSpinBox->text(),
                        "127.0.0.1:" + ui->httpPortSpinBox->text(),
                        ui->debugCheckBox->isChecked(),
                        tcpPortForwarding,
                        udpPortForwarding
                    );

                    isLinked = true;
                    ui->linkPushButton->setText("断开服务器");
                }
                else
                {
                    isLinked = false;

                    zjuConnectController->stop();

                    ui->linkPushButton->setText("连接服务器");
                }
            });

    // 设置系统代理
    connect(ui->systemProxyPushButton, &QPushButton::clicked,
            [&]()
            {
                if (!isSystemProxySet)
                {
                    Utils::setSystemProxy(ui->httpPortSpinBox->text().toInt());
                    ui->systemProxyPushButton->setText("清除系统代理");
                    isSystemProxySet = true;
                }
                else
                {
                    Utils::clearSystemProxy();
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
                    QString description = jsonObject["description"].toString();
                    QString url = jsonObject["url"].toString();

                    if (version != QApplication::applicationVersion())
                    {
                        QString text = "有新版本可用：v" + version +
                                       "<br>更新内容：" + description +
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
    settings->setValue("ZJUConnect/MultiLine", ui->multiLineCheckBox->isChecked());
    settings->setValue("ZJUConnect/ProxyAll", ui->proxyAllCheckBox->isChecked());
    settings->setValue("ZJUConnect/Debug", ui->debugCheckBox->isChecked());
    settings->setValue("ZJUConnect/TcpPortForwarding", tcpPortForwarding);
    settings->setValue("ZJUConnect/UdpPortForwarding", udpPortForwarding);
    settings->setValue("GUI/AutoReconnect", ui->autoReconnectCheckBox->isChecked());
    settings->setValue("GUI/ReconnectTime", ui->reconnectTimeSpinBox->value());
    settings->sync();

    // 清除系统代理
    if (isSystemProxySet)
    {
        Utils::clearSystemProxy();
    }

    disconnect(zjuConnectController, &ZjuConnectController::finished, this, nullptr);

    delete zjuConnectController;

    delete ui;
}
