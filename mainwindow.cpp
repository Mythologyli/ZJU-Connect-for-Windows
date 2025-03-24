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
    networkDetector = new NetworkDetector();

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

    // 高级-创建 L2TP VPN
    connect(ui->createL2tpAction, &QAction::triggered,
            [&]()
            {
                QMessageBox messageBox(this);
                messageBox.setWindowTitle("创建 L2TP VPN");
                messageBox.setText(
                    "是否创建名称为 "
                    + settings->value("L2TP/Name", "ZJUVPN").toString()
                    + " 的 L2TP VPN？\n要创建的 VPN 名称可在设置中更改\n在创建过程中会请求管理员权限"
                );

                messageBox.addButton(QMessageBox::Yes)->setText("是");
                messageBox.addButton(QMessageBox::No)->setText("否");
                messageBox.setDefaultButton(QMessageBox::Yes);

                if (messageBox.exec() == QMessageBox::Yes)
                {
                    disconnect(process, &QProcess::finished, nullptr, nullptr);
                    connect(process, &QProcess::finished, this, [&]()
                    {
                        QString output = Utils::ConsoleOutputToQString(process->readAllStandardError());
                        if (!output.contains("Add-VpnConnection"))
                        {
                            HINSTANCE hInstance = ShellExecute(
                                nullptr,
                                L"runas",
                                L"cmd.exe",
                                L"/c reg add \"HKLM\\System\\CurrentControlSet\\Services\\Rasman\\Parameters\" /v ProhibitIpSec /t REG_DWORD /d 0x1 /f",
                                nullptr,
                                SW_HIDE
                            );

                            if (hInstance > (HINSTANCE) 32)
                            {
                                addLog("创建 L2TP VPN 成功！");

                                QMessageBox::information(
                                    this,
                                    "提示",
                                    "创建成功！\n如果这是您第一次创建 L2TP VPN，请重启电脑"
                                );
                            }
                            else
                            {
                                addLog("创建 L2TP VPN 成功，但添加注册表项失败！");

                                QMessageBox::critical(
                                    this,
                                    "错误",
                                    "创建 VPN 成功，但添加注册表项失败！\n这可能是您拒绝了权限请求导致的\n请打开系统设置-网络-VPN，删除存在的 VPN 后重试"
                                );
                            }
                        }
                        else
                        {
                            addLog("创建 L2TP VPN 失败！");
                            addLog(output);
                            QMessageBox::critical(
                                this,
                                "错误",
                                "创建失败！请检查是否已存在同名 VPN。详细信息：\n" + output
                            );
                        }
                    });

                    process->start(
                        "powershell",
                        QStringList()
                            << "-command"
                            << "Add-VpnConnection"
                            << "-Name"
                            << '"' + settings->value("L2TP/Name", "ZJUVPN").toString() + '"'
                            << "-ServerAddress"
                            << "lns.zju.edu.cn"
                            << "-TunnelType"
                            << "L2tp"
                            << "-EncryptionLevel"
                            << "Optional"
                            << "-AuthenticationMethod"
                            << "('Chap','MSChapv2')"
                            << "-RememberCredential"
                    );
                }
            });

    // 高级-静态路由-设置静态路由
    connect(ui->setRouteAction, &QAction::triggered,
            [&]()
            {
                connect(networkDetector, &NetworkDetector::finished, this,
                        [&](const NetworkDetectResult &result)
                        {
                            disconnect(networkDetector, &NetworkDetector::finished, this, nullptr);
                            networkDetectResult = result;

                            if (!networkDetectResult.isZjuLan)
                            {
                                QMessageBox::critical(
                                    this, "错误",
                                    "未检测到 ZJU 有线网，无法设置静态路由！\n如果你正在使用路由器，请在路由器端设置静态路由"
                                );
                                return;
                            }

                            QMessageBox messageBox(this);
                            messageBox.setWindowTitle("设置静态路由");
                            messageBox.setText(
                                "在使用 L2TP 时，设置静态路由有助于从校网访问 RDP、NHD 等服务\n是否设置网关为 "
                                + networkDetectResult.zjuLanGateway
                                + " 的静态路由？\n在设置过程中会请求管理员权限"
                            );

                            messageBox.addButton(QMessageBox::Yes)->setText("是");
                            messageBox.addButton(QMessageBox::No)->setText("否");
                            messageBox.setDefaultButton(QMessageBox::Yes);

                            if (messageBox.exec() == QMessageBox::Yes)
                            {
                                QString command =
                                    "/c route -p add 10.0.0.0 mask 255.0.0.0 "
                                    + networkDetectResult.zjuLanGateway
                                    + " metric 1 & "
                                    + "route -p add 210.32.0.0 mask 255.255.240.0 "
                                    + networkDetectResult.zjuLanGateway
                                    + " metric 1 & "
                                    + "route -p add 222.205.0.0 mask 255.255.128.0 "
                                    + networkDetectResult.zjuLanGateway
                                    + " metric 1 & "
                                    + "route -p add 210.32.128.0 mask 255.255.224.0 "
                                    + networkDetectResult.zjuLanGateway
                                    + " metric 1 & "
                                    + "route -p add 210.32.160.0 mask 255.255.248.0 "
                                    + networkDetectResult.zjuLanGateway
                                    + " metric 1 & "
                                    + "route -p add 210.32.168.0 mask 255.255.252.0 "
                                    + networkDetectResult.zjuLanGateway
                                    + " metric 1 & "
                                    + "route -p add 210.32.172.0 mask 255.255.254.0 "
                                    + networkDetectResult.zjuLanGateway
                                    + " metric 1 & "
                                    + "route -p add 210.32.174.0 mask 255.255.255.0 "
                                    + networkDetectResult.zjuLanGateway
                                    + " metric 1 & "
                                    + "route -p add 210.32.176.0 mask 255.255.240.0 "
                                    + networkDetectResult.zjuLanGateway
                                    + " metric 1 & "
                                    + "route -p add 58.196.192.0 mask 255.255.224.0 "
                                    + networkDetectResult.zjuLanGateway
                                    + " metric 1 & "
                                    + "route -p add 58.196.224.0 mask 255.255.240.0 "
                                    + networkDetectResult.zjuLanGateway
                                    + " metric 1";

                                HINSTANCE hInstance = ShellExecute(
                                    nullptr,
                                    L"runas",
                                    L"cmd.exe",
                                    command.toStdWString().c_str(),
                                    nullptr,
                                    SW_HIDE
                                );

                                if (hInstance > (HINSTANCE) 32)
                                {
                                    addLog("设置静态路由成功！");

                                    QMessageBox successBox(this);
                                    successBox.setWindowTitle("提示");
                                    successBox.setTextFormat(Qt::RichText);
                                    successBox.setText(
                                        "设置成功！"
                                        "<br>请打开 <a href='http://speedtest.zju.edu.cn'>http://speedtest.zju.edu.cn</a> 测速"
                                        "<br>若显示的 IP 地址为 10.x.x.x，说明静态路由已生效！"
                                    );
                                    successBox.exec();
                                }
                                else
                                {
                                    addLog("设置静态路由失败！");

                                    QMessageBox::critical(
                                        this,
                                        "错误",
                                        "设置失败！\n这可能是您拒绝了权限请求导致的"
                                    );
                                }
                            }
                        });

                networkDetector->start();
            });

    // 高级-静态路由-删除静态路由
    connect(ui->deleteRouteAction, &QAction::triggered,
            [&]()
            {
                QMessageBox messageBox(this);
                messageBox.setWindowTitle("删除静态路由");
                messageBox.setText(
                    "是否删除静态路由？\n在删除过程中会请求管理员权限"
                );

                messageBox.addButton(QMessageBox::Yes)->setText("是");
                messageBox.addButton(QMessageBox::No)->setText("否");
                messageBox.setDefaultButton(QMessageBox::Yes);

                if (messageBox.exec() == QMessageBox::Yes)
                {
                    QString command =
                        QString("/c route -p delete 10.0.0.0 mask 255.0.0.0 & ")
                        + "route -p delete 210.32.0.0 mask 255.255.240.0 & "
                        + "route -p delete 222.205.0.0 mask 255.255.128.0 & "
                        + "route -p delete 210.32.128.0 mask 255.255.224.0 & "
                        + "route -p delete 210.32.160.0 mask 255.255.248.0 & "
                        + "route -p delete 210.32.168.0 mask 255.255.252.0 & "
                        + "route -p delete 210.32.172.0 mask 255.255.254.0 & "
                        + "route -p delete 210.32.174.0 mask 255.255.255.0 & "
                        + "route -p delete 210.32.176.0 mask 255.255.240.0 & "
                        + "route -p delete 58.196.192.0 mask 255.255.224.0 & "
                        + "route -p delete 58.196.224.0 mask 255.255.240.0";

                    HINSTANCE hInstance = ShellExecute(
                        nullptr,
                        L"runas",
                        L"cmd.exe",
                        command.toStdWString().c_str(),
                        nullptr,
                        SW_HIDE
                    );

                    if (hInstance > (HINSTANCE) 32)
                    {
                        addLog("删除静态路由成功！");

                        QMessageBox::information(
                            this,
                            "提示",
                            "删除成功！"
                        );
                    }
                    else
                    {
                        addLog("删除静态路由失败！");

                        QMessageBox::critical(
                            this,
                            "错误",
                            "删除失败！\n这可能是您拒绝了权限请求导致的"
                        );
                    }
                }
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

                if (mode == "RVPN" && isSystemProxySet)
                {
                    ui->pushButton2->click();
                }
                else
                {
                    Utils::clearSystemProxy();
                }

                addLog("已清除系统代理设置");
            });

    // 帮助-网络诊断
    connect(ui->diagnosisAction, &QAction::triggered,
            [&]()
            {
                diagnosisContext = new QObject(this);
                connect(networkDetector, &NetworkDetector::finished, diagnosisContext,
                        [&](const NetworkDetectResult &result)
                        {
                            networkDetectResult = result;

                            delete diagnosisContext;

                            QString resultString = "";
                            if (result.isDefaultDnsAvailable)
                            {
                                resultString += "dns=true&";
                            }
                            else
                            {
                                resultString += "dns=false&";
                            }

                            if (result.isZjuNet)
                            {
                                resultString += "zjunet=true&";
                            }
                            else
                            {
                                resultString += "zjunet=false&";
                            }

                            if (result.isZjuDnsCorrect)
                            {
                                resultString += "zjudns=true&";
                            }
                            else
                            {
                                resultString += "zjudns=false&";
                            }

                            if (result.isZjuWlan)
                            {
                                resultString += "zjuwlan=true&";
                            }
                            else
                            {
                                resultString += "zjuwlan=false&";
                            }

                            if (result.isZjuWlanSecure)
                            {
                                resultString += "zjuwlansecure=true&";
                            }
                            else
                            {
                                resultString += "zjuwlansecure=false&";
                            }

                            if (result.isZjuLan)
                            {
                                resultString += "zjulan=true&";
                                resultString += "zjulangateway=" + result.zjuLanGateway + "&";
                            }
                            else
                            {
                                resultString += "zjulan=false&";
                            }

                            if (result.isInternetAvailable)
                            {
                                resultString += "internet=true&";
                            }
                            else
                            {
                                resultString += "internet=false&";
                            }

                            if (result.isProxyEnabled)
                            {
                                resultString += "proxy=true";
                            }
                            else
                            {
                                resultString += "proxy=false";
                            }

                            auto url = "https://diagnosis.myth.cx/query?" + resultString;
                            auto qrCode = qrGenerator.generateQr(url);
                            auto *dialog = new QDialog(this);
                            auto *layout = new QVBoxLayout();
                            auto *textLabel = new QLabel(dialog);
                            textLabel->setWordWrap(true);
                            textLabel->setText(
                                "请使用手机扫描下方二维码。如当前电脑有网络，也可<a href=\"" + url + "\">点此</a>进行诊断"
                            );
                            textLabel->setTextFormat(Qt::RichText);
                            textLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
                            textLabel->setOpenExternalLinks(true);
                            auto *imageLabel = new QLabel(dialog);
                            imageLabel->setAlignment(Qt::AlignCenter);
                            imageLabel->setPixmap(QPixmap::fromImage(qrCode));
                            layout->addWidget(textLabel);
                            layout->addWidget(imageLabel);
                            dialog->setLayout(layout);
                            dialog->setWindowTitle("网络诊断");
                            dialog->exec();
                        });

                networkDetector->start();
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

    // 检查更新
    QTimer::singleShot(10000, [&]()
    {
        QNetworkRequest request(QUrl("https://zjuconnect.myth.cx/version.json"));
        request.setHeader(
            QNetworkRequest::UserAgentHeader,
            QApplication::applicationName() + " v" + QApplication::applicationVersion()
        );
        networkAccessManager->get(request);
    });

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

                        disconnect(trayIcon, &QSystemTrayIcon::messageClicked, nullptr, nullptr);
                        trayIcon->showMessage(
                            "有新版本可用",
                            "v" + version + "\n更新内容：" + description + "\n点击查看详情",
                            QSystemTrayIcon::Information,
                            10000
                        );

                        connect(trayIcon, &QSystemTrayIcon::messageClicked, this, [&, url]()
                        {
                            disconnect(trayIcon, &QSystemTrayIcon::messageClicked, nullptr, nullptr);

                            QDesktopServices::openUrl(QUrl(url));

                            show();
                            setWindowState(Qt::WindowState::WindowActive);
                        });
                    }
                }
            });

    // 网络接口
    connect(ui->refreshInterfaceButton, &QPushButton::clicked,
            [&]()
            {
                auto currentInterfaceName = ui->interfaceComboBox->currentText();
                ui->interfaceComboBox->clear();
                ui->interfaceComboBox->addItem("默认");
                auto interfaces = QNetworkInterface::allInterfaces();
                bool notDefault = false;
                for (auto &singleInterface: interfaces)
                {
                    if ((singleInterface.type() != QNetworkInterface::Ethernet
                         && singleInterface.type() != QNetworkInterface::Wifi)
                        || !singleInterface.flags().testFlag(QNetworkInterface::IsRunning))
                    {
                        continue;
                    }

                    QString ipv4Address;
                    auto addresses = singleInterface.addressEntries();
                    for (auto &address: addresses)
                    {
                        if (address.ip().protocol() == QAbstractSocket::IPv4Protocol)
                        {
                            ipv4Address = address.ip().toString();
                            break;
                        }
                    }

                    ui->interfaceComboBox->addItem(singleInterface.humanReadableName(), ipv4Address);

                    if (currentInterfaceName == singleInterface.humanReadableName())
                    {
                        notDefault = true;
                        ui->interfaceComboBox->setCurrentText(singleInterface.humanReadableName());
                    }
                }

                if (!notDefault)
                {
                    ui->interfaceComboBox->setCurrentText("默认");
                }
            });

    Utils::setWidgetFixedWhenHidden(ui->interfaceLabel);
    Utils::setWidgetFixedWhenHidden(ui->refreshInterfaceButton);
    Utils::setWidgetFixedWhenHidden(ui->interfaceComboBox);

    ui->interfaceLabel->hide();
    ui->refreshInterfaceButton->hide();
    ui->interfaceComboBox->hide();

    // 更改工作模式
    connect(ui->modeComboBox, &QComboBox::currentTextChanged,
            [&](const QString &text)
            {
                if (text == "有线网 L2TP")
                {
                    setModeToL2tp();
                }
                else if (text == "网页认证登录")
                {
                    setModeToWebLogin();
                }
                else if (text == "RVPN")
                {
                    setModeToZjuConnect();
                }
            });

    // 网络检测
    connect(networkDetector, &NetworkDetector::finished, this,
            [&](const NetworkDetectResult &result)
            {
                networkDetectResult = result;

                QString resultString = "";
                resultString += "网络检测结果 | ";
                if (result.isDefaultDnsAvailable)
                {
                    resultString += "默认 DNS 可用：是 | ";
                }
                else
                {
                    resultString += "默认 DNS 可用：否 | ";
                }

                if (result.isZjuNet)
                {
                    resultString += "ZJU 校园网：是 | ";
                }
                else
                {
                    resultString += "ZJU 校园网：否 | ";
                }

                if (result.isZjuDnsCorrect)
                {
                    resultString += "ZJU 内网 DNS 解析：是 | ";
                }
                else
                {
                    resultString += "ZJU 内网 DNS 解析：否 | ";
                }

                if (result.isZjuWlan)
                {
                    resultString += "ZJUWLAN：是 | ";
                }
                else
                {
                    resultString += "ZJUWLAN：否 | ";
                }

                if (result.isZjuWlanSecure)
                {
                    resultString += "ZJUWLAN-Secure：是 | ";
                }
                else
                {
                    resultString += "ZJUWLAN-Secure：否 | ";
                }

                if (result.isZjuLan)
                {
                    resultString += "ZJU 有线网：是 | ";
                    resultString += "ZJU 有线网网关：" + result.zjuLanGateway + " | ";
                }
                else
                {
                    resultString += "ZJU 有线网：否 | ";
                }

                if (result.isInternetAvailable)
                {
                    resultString += "互联网连接：是 | ";
                }
                else
                {
                    resultString += "互联网连接：否 | ";
                }

                if (result.isProxyEnabled)
                {
                    resultString += "是否启用代理：是";
                }
                else
                {
                    resultString += "是否启用代理：否";
                }

                ui->logPlainTextEdit->appendPlainText(resultString);
            });

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

    auto lastMode = settings->value("Common/LastMode", "RVPN").toString();
    if (lastMode == "有线网 L2TP")
    {
        ui->modeComboBox->setCurrentText("有线网 L2TP");
    }
    else if (lastMode == "网页认证登录")
    {
        ui->modeComboBox->setCurrentText("网页认证登录");
    }
    else
    {
        setModeToZjuConnect();
    }
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
        addLog("警告：配置文件版本高于 2。请使用关闭当前 ZJU Connect 并运行新版本！");
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
                    "ZjuConnectForWindows",
                    "\"" + QCoreApplication::applicationFilePath().replace('/', '\\') + "\" -s"
                );
            }
        }

        // Replace old ineffective check ip
        if (settings->value("L2TP/CheckIp", "223.5.5.5").toString() == "39.156.66.10")
        {
            settings->setValue("L2TP/CheckIp", "223.5.5.5");
            settings->sync();
        }

        return;
    }

    if (settings->contains("EasyConnect/ServerAddress"))
    {
        settings->setValue("ZJUConnect/ServerAddress", settings->value("EasyConnect/ServerAddress"));
        settings->remove("EasyConnect/ServerAddress");
    }

    if (settings->contains("EasyConnect/ServerPort"))
    {
        settings->setValue("ZJUConnect/ServerPort", settings->value("EasyConnect/ServerPort"));
        settings->remove("EasyConnect/ServerPort");
    }

    if (settings->contains("EasyConnect/Username"))
    {
        settings->setValue("Common/Username", settings->value("EasyConnect/Username"));
        settings->remove("EasyConnect/Username");
    }

    if (settings->contains("EasyConnect/Password"))
    {
        settings->setValue("Common/Password", settings->value("EasyConnect/Password"));
        settings->remove("EasyConnect/Password");
    }

    if (settings->contains("GUI/AutoReconnect"))
    {
        settings->setValue("ZJUConnect/AutoReconnect", settings->value("GUI/AutoReconnect"));
        settings->remove("GUI/AutoReconnect");
    }

    if (settings->contains("GUI/ReconnectTime"))
    {
        settings->setValue("ZJUConnect/ReconnectTime", settings->value("GUI/ReconnectTime"));
        settings->remove("GUI/ReconnectTime");
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
        settings->setValue("WebLogin/LastInterface", ui->interfaceComboBox->currentText());
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
    delete networkDetector;

    if (zjuConnectController != nullptr)
    {
        disconnect(zjuConnectController, &ZjuConnectController::finished, nullptr, nullptr);
        delete zjuConnectController;
    }

    delete ui;
}
