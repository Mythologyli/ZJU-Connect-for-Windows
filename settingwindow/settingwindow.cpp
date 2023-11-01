#include <QFileInfo>
#include <QDesktopServices>
#include <QMessageBox>
#include <QHostAddress>

#include "settingwindow.h"
#include "ui_settingwindow.h"

SettingWindow::SettingWindow(QWidget *parent, QSettings *inputSettings) :
    QWidget(parent),
    ui(new Ui::SettingWindow)
{
    ui->setupUi(this);

    this->settings = inputSettings;

    setWindowIcon(QIcon(QPixmap(":/resource/icon.png").scaled(
        512, 512, Qt::KeepAspectRatio, Qt::SmoothTransformation
    )));

    setWindowFlag(Qt::Window);
    setWindowModality(Qt::WindowModal);
    setAttribute(Qt::WA_DeleteOnClose);

    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);

    ui->usernameLineEdit->setText(settings->value("Common/Username", "").toString());
    ui->passwordLineEdit->setText(
        QByteArray::fromBase64(settings->value("Common/Password", "").toString().toUtf8())
    );

    if (settings->value("Common/AutoStart", false).toBool())
    {
        ui->autoStartComboBox->setCurrentText("是");
    }
    else
    {
        ui->autoStartComboBox->setCurrentText("否");
    }

    if (settings->value("Common/ConnectAfterStart", false).toBool())
    {
        ui->connectAfterStartComboBox->setCurrentText("是");
    }
    else
    {
        ui->connectAfterStartComboBox->setCurrentText("否");
    }

    ui->serverAddressLineEdit->setText(settings->value("ZJUConnect/ServerAddress", "rvpn.zju.edu.cn").toString());
    ui->serverPortSpinBox->setValue(settings->value("ZJUConnect/ServerPort", 443).toInt());
    ui->socks5PortSpinBox->setValue(settings->value("ZJUConnect/Socks5Port", 1080).toInt());
    ui->httpPortSpinBox->setValue(settings->value("ZJUConnect/HttpPort", 1081).toInt());
    ui->multiLineCheckBox->setChecked(settings->value("ZJUConnect/MultiLine", true).toBool());
    ui->proxyAllCheckBox->setChecked(settings->value("ZJUConnect/ProxyAll", false).toBool());
    ui->debugCheckBox->setChecked(settings->value("ZJUConnect/Debug", false).toBool());
    tcpPortForwarding = settings->value("ZJUConnect/TcpPortForwarding", "").toString();
    udpPortForwarding = settings->value("ZJUConnect/UdpPortForwarding", "").toString();
    ui->autoReconnectCheckBox->setChecked(settings->value("ZJUConnect/AutoReconnect", false).toBool());
    ui->reconnectTimeSpinBox->setValue(settings->value("ZJUConnect/ReconnectTime", 1).toInt());
    ui->autoSetProxyCheckBox->setChecked(settings->value("ZJUConnect/AutoSetProxy", false).toBool());
    ui->routeCheckBox->setChecked(settings->value("ZJUConnect/Route", false).toBool());

    ui->l2tpNameLineEdit->setText(settings->value("L2TP/Name", "ZJUVPN").toString());

    if (settings->value("L2TP/AutoReconnect", false).toBool())
    {
        ui->l2tpAutoCheckComboBox->setCurrentText("是");
    }
    else
    {
        ui->l2tpAutoCheckComboBox->setCurrentText("否");
    }

    ui->l2tpCheckIpLineEdit->setText(settings->value("L2TP/CheckIp", "39.156.66.10").toString());
    ui->l2tpCheckTimeSpinBox->setValue(settings->value("L2TP/CheckTime", 600).toInt());

    if (settings->value("WebLogin/IntlUrl", false).toBool())
    {
        ui->intlUrlComboBox->setCurrentText("是");
    }
    else
    {
        ui->intlUrlComboBox->setCurrentText("否");
    }

    if (settings->value("WebLogin/EnableCustomUrl", false).toBool())
    {
        ui->customUrlComboBox->setCurrentText("是");
    }
    else
    {
        ui->customUrlComboBox->setCurrentText("否");
    }

    ui->customUrlLineEdit->setText(settings->value("WebLogin/CustomUrl", "https://net.zju.edu.cn").toString());

    connect(ui->portForwardingPushButton, &QPushButton::clicked,
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

    connect(ui->applyPushButton, &QPushButton::clicked,
            [&]()
            {
                QRegularExpression re("^[A-Za-z]+$");
                QRegularExpressionMatch match = re.match(ui->l2tpNameLineEdit->text());
                if (!match.hasMatch())
                {
                    ui->tabWidget->setCurrentWidget(ui->l2tpTab);
                    QMessageBox::warning(
                        this,
                        "警告",
                        "建议 L2TP VPN 名称只包含英文字母，否则可能导致系统代理出错！\n"
                        "如果您之前自行配置过 VPN，建议修改此处 L2TP VPN 名称，之后在主界面点击高级-创建 L2TP VPN"
                    );
                }

                if (ui->l2tpCheckIpLineEdit->text().isEmpty() and ui->l2tpAutoCheckComboBox->currentText() == "是")
                {
                    ui->tabWidget->setCurrentWidget(ui->l2tpTab);
                    QMessageBox::critical(this, "错误", "检测使用的 IP 不能为空！");
                    return;
                }

                QHostAddress address(ui->l2tpCheckIpLineEdit->text());
                if (QAbstractSocket::IPv4Protocol != address.protocol())
                {
                    ui->tabWidget->setCurrentWidget(ui->l2tpTab);
                    QMessageBox::critical(this, "错误", "请填写有效的 IPv4 地址！");
                    return;
                }

                settings->setValue("Common/Username", ui->usernameLineEdit->text());
                settings->setValue("Common/Password", QString(ui->passwordLineEdit->text().toUtf8().toBase64()));
                settings->setValue("Common/AutoStart", ui->autoStartComboBox->currentText() == "是");
                settings->setValue("Common/ConnectAfterStart", ui->connectAfterStartComboBox->currentText() == "是");

                settings->setValue("ZJUConnect/ServerAddress", ui->serverAddressLineEdit->text());
                settings->setValue("ZJUConnect/ServerPort", ui->serverPortSpinBox->value());
                settings->setValue("ZJUConnect/Socks5Port", ui->socks5PortSpinBox->value());
                settings->setValue("ZJUConnect/HttpPort", ui->httpPortSpinBox->value());
                settings->setValue("ZJUConnect/MultiLine", ui->multiLineCheckBox->isChecked());
                settings->setValue("ZJUConnect/ProxyAll", ui->proxyAllCheckBox->isChecked());
                settings->setValue("ZJUConnect/Debug", ui->debugCheckBox->isChecked());
                settings->setValue("ZJUConnect/TcpPortForwarding", tcpPortForwarding);
                settings->setValue("ZJUConnect/UdpPortForwarding", udpPortForwarding);
                settings->setValue("ZJUConnect/AutoReconnect", ui->autoReconnectCheckBox->isChecked());
                settings->setValue("ZJUConnect/ReconnectTime", ui->reconnectTimeSpinBox->value());
                settings->setValue("ZJUConnect/AutoSetProxy", ui->autoSetProxyCheckBox->isChecked());
                settings->setValue("ZJUConnect/Route", ui->routeCheckBox->isChecked());

                settings->setValue("L2TP/Name", ui->l2tpNameLineEdit->text());
                settings->setValue("L2TP/AutoReconnect", ui->l2tpAutoCheckComboBox->currentText() == "是");
                settings->setValue("L2TP/CheckIp", ui->l2tpCheckIpLineEdit->text());
                settings->setValue("L2TP/CheckTime", ui->l2tpCheckTimeSpinBox->value());

                settings->setValue("WebLogin/IntlUrl", ui->intlUrlComboBox->currentText() == "是");
                settings->setValue("WebLogin/EnableCustomUrl", ui->customUrlComboBox->currentText() == "是");
                settings->setValue("WebLogin/CustomUrl", ui->customUrlLineEdit->text());

                settings->sync();

                if (settings->value("Common/AutoStart").toBool())
                {
                    QSettings autoStartSettings(
                        R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run)",
                        QSettings::NativeFormat
                    );
                    autoStartSettings.setValue(
                        "ZjuConnectForWindows",
                        QCoreApplication::applicationFilePath().replace('/', '\\')
                    );
                }
                else
                {
                    QSettings autoStartSettings(
                        R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run)",
                        QSettings::NativeFormat
                    );
                    autoStartSettings.remove("ZjuConnectForWindows");
                }

                ui->cancelPushButton->setText("关闭");
            });

    connect(ui->cancelPushButton, &QPushButton::clicked,
            [&]()
            {
                close();
            });
}

SettingWindow::~SettingWindow()
{
    delete ui;
}
