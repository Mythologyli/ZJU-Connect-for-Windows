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

    ui->serverAddressLineEdit->setText(settings->value("ZJUConnect/ServerAddress", "vpn.hitsz.edu.cn").toString());
    ui->serverPortSpinBox->setValue(settings->value("ZJUConnect/ServerPort", 443).toInt());
    ui->dnsLineEdit->setText(settings->value("ZJUConnect/DNS", "10.248.98.30").toString());
    ui->socks5PortSpinBox->setValue(settings->value("ZJUConnect/Socks5Port", 11080).toInt());
    ui->httpPortSpinBox->setValue(settings->value("ZJUConnect/HttpPort", 11081).toInt());
    ui->multiLineCheckBox->setChecked(settings->value("ZJUConnect/MultiLine", true).toBool());
    ui->proxyAllCheckBox->setChecked(settings->value("ZJUConnect/ProxyAll", false).toBool());
    ui->debugCheckBox->setChecked(settings->value("ZJUConnect/Debug", false).toBool());
    tcpPortForwarding = settings->value("ZJUConnect/TcpPortForwarding", "").toString();
    udpPortForwarding = settings->value("ZJUConnect/UdpPortForwarding", "").toString();
    ui->autoReconnectCheckBox->setChecked(settings->value("ZJUConnect/AutoReconnect", false).toBool());
    ui->reconnectTimeSpinBox->setValue(settings->value("ZJUConnect/ReconnectTime", 1).toInt());
    ui->autoSetProxyCheckBox->setChecked(settings->value("ZJUConnect/AutoSetProxy", false).toBool());
    ui->routeCheckBox->setChecked(settings->value("ZJUConnect/Route", false).toBool());

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

                settings->setValue("Common/Username", ui->usernameLineEdit->text());
                settings->setValue("Common/Password", QString(ui->passwordLineEdit->text().toUtf8().toBase64()));
                settings->setValue("Common/AutoStart", ui->autoStartComboBox->currentText() == "是");
                settings->setValue("Common/ConnectAfterStart", ui->connectAfterStartComboBox->currentText() == "是");

                settings->setValue("ZJUConnect/ServerAddress", ui->serverAddressLineEdit->text());
                settings->setValue("ZJUConnect/ServerPort", ui->serverPortSpinBox->value());
                settings->setValue("ZJUConnect/DNS", ui->dnsLineEdit->text());
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
