#include <QFileInfo>
#include <QDesktopServices>
#include <QMessageBox>
#include <QHostAddress>

#include "settingwindow.h"
#include "ui_settingwindow.h"
#include "../utils/utils.h"

SettingWindow::SettingWindow(QWidget *parent, QSettings *inputSettings) :
    QDialog(parent),
    ui(new Ui::SettingWindow)
{
    ui->setupUi(this);

    this->settings = inputSettings;

    setWindowModality(Qt::WindowModal);
    setAttribute(Qt::WA_DeleteOnClose);

    loadSettings();

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

    auto applySettings = [&]()
        {
            if (settings->value("Common/AutoStart").toBool() != ui->autoStartCheckBox->isChecked())
                Utils::setAutoStart(ui->autoStartCheckBox->isChecked());

            settings->setValue("Common/Username", ui->usernameLineEdit->text());
            settings->setValue("Common/Password", QString(ui->passwordLineEdit->text().toUtf8().toBase64()));
            settings->setValue("Common/AutoStart", ui->autoStartCheckBox->isChecked());
            settings->setValue("Common/ConnectAfterStart", ui->connectAfterStartCheckBox->isChecked());
            settings->setValue("Common/CheckUpdateAfterStart", ui->checkUpdateAfterStartCheckBox->isChecked());

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
            settings->setValue("ZJUConnect/SecondaryDNS", ui->secondaryDnsLineEdit->text());
            settings->setValue("ZJUConnect/ShadowsocksUrl", ui->shadowsocksUrlLineEdit->text());
            settings->setValue("ZJUConnect/SystemProxyBypass", ui->systemProxyBypassLineEdit->text());
            settings->setValue("ZJUConnect/KeepAlive", ui->keepAliveCheckBox->isChecked());
            settings->setValue("ZJUConnect/TunMode", ui->tunCheckBox->isChecked());
            settings->setValue("ZJUConnect/DNSHijack", ui->dnsHijackCheckBox->isChecked());
            settings->setValue("ZJUConnect/OutsideAccess", ui->outsideAccessCheckBox->isChecked());

            settings->setValue("Common/ConfigVersion", Utils::CONFIG_VERSION);

            settings->sync();
        };

    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, [&, applySettings](){
        if (!Utils::credentialCheck(ui->usernameLineEdit->text(), ui->passwordLineEdit->text()))
            return;
        applySettings();
        accept();
    });

    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, [&, applySettings](){
        if (!Utils::credentialCheck(ui->usernameLineEdit->text(), ui->passwordLineEdit->text()))
            return;
        applySettings();
    });

    connect(ui->buttonBox->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked,
        [&]()
        {
            int status = QMessageBox::warning(this, "警告", "将会重置所有设置，是否继续？", QMessageBox::Ok, QMessageBox::Cancel);
            if (status == QMessageBox::Ok)
            {
                settings->clear();
                settings->sync();
                loadSettings();
            }
        });

    connect(ui->tunCheckBox, &QCheckBox::toggled,
        [&](bool checked)
        {
            ui->routeCheckBox->setEnabled(checked);
            ui->dnsHijackCheckBox->setEnabled(checked);
        });

    connect(ui->passwordVisibleCheckBox, &QCheckBox::checkStateChanged,
        [&](Qt::CheckState state)
        {
            ui->passwordLineEdit->setEchoMode(state == Qt::Checked ? QLineEdit::Normal : QLineEdit::Password);
        });
}

SettingWindow::~SettingWindow()
{
    delete ui;
}

void SettingWindow::loadSettings()
{
    ui->usernameLineEdit->setText(settings->value("Common/Username", "").toString());
    ui->passwordLineEdit->setText(
        QByteArray::fromBase64(settings->value("Common/Password", "").toString().toUtf8())
    );

    ui->autoStartCheckBox->setChecked(settings->value("Common/AutoStart", false).toBool());
    ui->connectAfterStartCheckBox->setChecked(settings->value("Common/ConnectAfterStart", false).toBool());
    ui->checkUpdateAfterStartCheckBox->setChecked(settings->value("Common/CheckUpdateAfterStart", true).toBool());

    ui->serverAddressLineEdit->setText(settings->value("ZJUConnect/ServerAddress", "vpn.hitsz.edu.cn").toString());
    ui->serverPortSpinBox->setValue(settings->value("ZJUConnect/ServerPort", 443).toInt());
    ui->dnsLineEdit->setText(settings->value("ZJUConnect/DNS", "10.248.98.30").toString());
    ui->socks5PortSpinBox->setValue(settings->value("ZJUConnect/Socks5Port", 11080).toInt());
    ui->httpPortSpinBox->setValue(settings->value("ZJUConnect/HttpPort", 11081).toInt());
    ui->multiLineCheckBox->setChecked(settings->value("ZJUConnect/MultiLine", false).toBool());
    ui->proxyAllCheckBox->setChecked(settings->value("ZJUConnect/ProxyAll", false).toBool());
    ui->debugCheckBox->setChecked(settings->value("ZJUConnect/Debug", false).toBool());
    tcpPortForwarding = settings->value("ZJUConnect/TcpPortForwarding", "").toString();
    udpPortForwarding = settings->value("ZJUConnect/UdpPortForwarding", "").toString();
    ui->autoReconnectCheckBox->setChecked(settings->value("ZJUConnect/AutoReconnect", false).toBool());
    ui->reconnectTimeSpinBox->setValue(settings->value("ZJUConnect/ReconnectTime", 1).toInt());
    ui->autoSetProxyCheckBox->setChecked(settings->value("ZJUConnect/AutoSetProxy", false).toBool());
    ui->routeCheckBox->setChecked(settings->value("ZJUConnect/Route", false).toBool());
    ui->secondaryDnsLineEdit->setText(settings->value("ZJUConnect/SecondaryDNS", "").toString());
    ui->shadowsocksUrlLineEdit->setText(settings->value("ZJUConnect/ShadowsocksUrl", "").toString());
    ui->systemProxyBypassLineEdit->setText(settings->value("ZJUConnect/SystemProxyBypass", "localhost").toString());
    ui->keepAliveCheckBox->setChecked(settings->value("ZJUConnect/KeepAlive", false).toBool());
    ui->tunCheckBox->setChecked(settings->value("ZJUConnect/TunMode", false).toBool());
    ui->dnsHijackCheckBox->setChecked(settings->value("ZJUConnect/DNSHijack", false).toBool());
    ui->outsideAccessCheckBox->setChecked(settings->value("ZJUConnect/OutsideAccess", false).toBool());

    ui->routeCheckBox->setEnabled(ui->tunCheckBox->isChecked());
    ui->dnsHijackCheckBox->setEnabled(ui->tunCheckBox->isChecked());
}
