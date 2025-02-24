#include <QFileInfo>
#include <QFileDialog>
#include <QDesktopServices>
#include <QMessageBox>
#include <QHostAddress>
#include <QStandardPaths>

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
                extraSettingWindow = new ExtraSettingWindow(this);
                extraSettingWindow->setup(tcpPortForwarding, udpPortForwarding, customDNS, customProxyDomain, extraArguments);

                connect(extraSettingWindow, &ExtraSettingWindow::applied, this,
				[&](const QString& tcpForwarding, const QString& udpForwarding, const QString& customDNS_, const QString& customProxyDomain_, const QString& extraArg)
                    {
                        tcpPortForwarding = tcpForwarding;
                        udpPortForwarding = udpForwarding;
						customDNS = customDNS_;
						customProxyDomain = customProxyDomain_;
						extraArguments = extraArg;
                    });

                extraSettingWindow->show();
            });

    connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, [&](){
        if (!Utils::credentialCheck(ui->usernameLineEdit->text(), ui->passwordLineEdit->text()))
            return;
        applySettings();
        accept();
    });

    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, [&](){
        if (!Utils::credentialCheck(ui->usernameLineEdit->text(), ui->passwordLineEdit->text()))
            return;
        applySettings();
        loadSettings();
    });

    connect(ui->resetDefaultPushButton, &QPushButton::clicked,
        [&]()
        {
            int status = QMessageBox::warning(this, "警告", "将会重置所有设置，是否继续？", QMessageBox::Ok, QMessageBox::Cancel);
            if (status == QMessageBox::Ok)
            {
                settings->clear();
				Utils::resetDefaultSettings(*settings);
				settings->sync();
                loadSettings();
            }
        });

    connect(ui->importPushButton, &QPushButton::clicked,
            [&]()
            {
                QString filename = QFileDialog::getOpenFileName(this, "选择配置文件",
                    QStandardPaths::writableLocation(QStandardPaths::ConfigLocation),
                    "Config Ini(*.ini);;All Files(*.*)");
                if (filename.isEmpty()) {
                    QMessageBox::critical(this, "错误", "未选择配置文件，不会带来任何更改。");
                    return;
                }
                QSettings newSettings(filename, QSettings::IniFormat);
                for (const auto& key : newSettings.allKeys()) {
                    settings->setValue(key, newSettings.value(key));
                }
                settings->sync();
                loadSettings();
            });

    connect(ui->exportPushButton, &QPushButton::clicked,
            [&]()
            {
                QString filename = QFileDialog::getSaveFileName(this, "选择保存位置",
                    QStandardPaths::writableLocation(QStandardPaths::ConfigLocation),
                    "Config Ini(*.ini);;All Files(*.*)");
                if (filename.isEmpty())
                {
                    QMessageBox::critical(this, "错误", "未选择配置文件保存位置。");
                    return;
                }
                settings->sync();
                if (QFile::exists(filename))
                    QFile::remove(filename);
                QFile::copy(settings->fileName(), filename);
            });

    connect(ui->tunCheckBox, &QCheckBox::toggled,
        [&](bool checked)
        {
            ui->routeCheckBox->setEnabled(checked);
            ui->dnsHijackCheckBox->setEnabled(checked);
        });

    connect(ui->dnsAutoCheckBox, &QCheckBox::toggled,
        [&](bool checked)
        {
            ui->dnsLineEdit->setEnabled(!checked);
        });

    connect(ui->passwordVisibleCheckBox, &QCheckBox::checkStateChanged,
        [&](Qt::CheckState state)
        {
            ui->passwordLineEdit->setEchoMode(state == Qt::Checked ? QLineEdit::Normal : QLineEdit::Password);
        });

    connect(ui->totpSecretVisibleCheckBox, &QCheckBox::checkStateChanged,
        [&](Qt::CheckState state)
        {
            ui->totpSecretLineEdit->setEchoMode(state == Qt::Checked ? QLineEdit::Normal : QLineEdit::Password);
        });
}

SettingWindow::~SettingWindow()
{
    delete ui;
}

void SettingWindow::loadSettings()
{
    ui->configVersionLabel->setText(
        "当前配置文件版本：" + QString::number(settings->value("Common/ConfigVersion").toInt()) +
        "\n程序配置文件版本：" + QString::number(Utils::CONFIG_VERSION)
    );
    ui->usernameLineEdit->setText(settings->value("Credential/Username").toString());
    ui->passwordLineEdit->setText(
        QByteArray::fromBase64(settings->value("Credential/Password").toString().toUtf8())
    );
    ui->totpSecretLineEdit->setText(settings->value("Credential/TOTPSecret").toString());

    ui->autoStartCheckBox->setChecked(settings->value("Common/AutoStart").toBool());
    ui->connectAfterStartCheckBox->setChecked(settings->value("Common/ConnectAfterStart").toBool());
    ui->checkUpdateAfterStartCheckBox->setChecked(settings->value("Common/CheckUpdateAfterStart").toBool());
    ui->autoSetProxyCheckBox->setChecked(settings->value("Common/AutoSetProxy").toBool());
    ui->reconnectTimeSpinBox->setValue(settings->value("Common/ReconnectTime").toInt());
    ui->autoReconnectCheckBox->setChecked(settings->value("Common/AutoReconnect").toBool());
	ui->systemProxyBypassLineEdit->setText(settings->value("Common/SystemProxyBypass").toString());


    ui->serverAddressLineEdit->setText(settings->value("ZJUConnect/ServerAddress").toString());
    ui->serverPortSpinBox->setValue(settings->value("ZJUConnect/ServerPort").toInt());
    ui->dnsLineEdit->setText(settings->value("ZJUConnect/DNS").toString());
    ui->dnsAutoCheckBox->setChecked(settings->value("ZJUConnect/DNSAuto").toBool());
    ui->secondaryDnsLineEdit->setText(settings->value("ZJUConnect/SecondaryDNS").toString());
    ui->dnsTTLSpinBox->setValue(settings->value("ZJUConnect/DNSTTL").toInt());
    ui->socks5PortSpinBox->setValue(settings->value("ZJUConnect/Socks5Port").toInt());
    ui->httpPortSpinBox->setValue(settings->value("ZJUConnect/HttpPort").toInt());
    ui->shadowsocksUrlLineEdit->setText(settings->value("ZJUConnect/ShadowsocksURL").toString());
    ui->dialDirectProxyLineEdit->setText(settings->value("ZJUConnect/DialDirectProxy").toString());


    ui->multiLineCheckBox->setChecked(settings->value("ZJUConnect/MultiLine").toBool());
    ui->keepAliveCheckBox->setChecked(settings->value("ZJUConnect/KeepAlive").toBool());
    ui->outsideAccessCheckBox->setChecked(settings->value("ZJUConnect/OutsideAccess").toBool());

    ui->skipDomainResourceCheckBox->setChecked(settings->value("ZJUConnect/SkipDomainResource").toBool());
    ui->disableServerConfigCheckBox->setChecked(settings->value("ZJUConnect/DisableServerConfig").toBool());
    ui->proxyAllCheckBox->setChecked(settings->value("ZJUConnect/ProxyAll").toBool());
    
    ui->zjuDefaultCheckBox->setChecked(settings->value("ZJUConnect/ZJUDefault").toBool());
    ui->disableDNSCheckBox->setChecked(settings->value("ZJUConnect/DisableZJUDNS").toBool());
    ui->debugCheckBox->setChecked(settings->value("ZJUConnect/Debug").toBool());

    ui->tunCheckBox->setChecked(settings->value("ZJUConnect/TunMode").toBool());
    ui->routeCheckBox->setChecked(settings->value("ZJUConnect/AddRoute").toBool());
    ui->dnsHijackCheckBox->setChecked(settings->value("ZJUConnect/DNSHijack").toBool());

    tcpPortForwarding = settings->value("ZJUConnect/TcpPortForwarding").toString();
    udpPortForwarding = settings->value("ZJUConnect/UdpPortForwarding").toString();
	customDNS = settings->value("ZJUConnect/CustomDNS").toString();
	customProxyDomain = settings->value("ZJUConnect/CustomProxyDomain").toString();
    extraArguments = settings->value("ZJUConnect/ExtraArguments").toString();

    ui->routeCheckBox->setEnabled(ui->tunCheckBox->isChecked());
    ui->dnsHijackCheckBox->setEnabled(ui->tunCheckBox->isChecked());

	ui->dnsLineEdit->setEnabled(!ui->dnsAutoCheckBox->isChecked());
}

void SettingWindow::applySettings()
{
    if (settings->value("Common/AutoStart", false).toBool() != ui->autoStartCheckBox->isChecked())
        Utils::setAutoStart(ui->autoStartCheckBox->isChecked());

    settings->setValue("Credential/Username", ui->usernameLineEdit->text());
    settings->setValue("Credential/Password", QString(ui->passwordLineEdit->text().toUtf8().toBase64()));
    settings->setValue("Credential/TOTPSecret", ui->totpSecretLineEdit->text());

    settings->setValue("Common/AutoStart", ui->autoStartCheckBox->isChecked());
    settings->setValue("Common/ConnectAfterStart", ui->connectAfterStartCheckBox->isChecked());
    settings->setValue("Common/CheckUpdateAfterStart", ui->checkUpdateAfterStartCheckBox->isChecked());
    settings->setValue("Common/AutoSetProxy", ui->autoSetProxyCheckBox->isChecked());
    settings->setValue("Common/ReconnectTime", ui->reconnectTimeSpinBox->value());
    settings->setValue("Common/AutoReconnect", ui->autoReconnectCheckBox->isChecked());
    settings->setValue("Common/SystemProxyBypass", ui->systemProxyBypassLineEdit->text());


    settings->setValue("ZJUConnect/ServerAddress", ui->serverAddressLineEdit->text());
    settings->setValue("ZJUConnect/ServerPort", ui->serverPortSpinBox->value());
    settings->setValue("ZJUConnect/DNS", ui->dnsLineEdit->text());
    settings->setValue("ZJUConnect/DNSAuto", ui->dnsAutoCheckBox->isChecked());
    settings->setValue("ZJUConnect/SecondaryDNS", ui->secondaryDnsLineEdit->text());
    settings->setValue("ZJUConnect/DNSTTL", ui->dnsTTLSpinBox->value());
    settings->setValue("ZJUConnect/Socks5Port", ui->socks5PortSpinBox->value());
    settings->setValue("ZJUConnect/HttpPort", ui->httpPortSpinBox->value());
    settings->setValue("ZJUConnect/ShadowsocksURL", ui->shadowsocksUrlLineEdit->text());
    settings->setValue("ZJUConnect/DialDirectProxy", ui->dialDirectProxyLineEdit->text());


    settings->setValue("ZJUConnect/MultiLine", ui->multiLineCheckBox->isChecked());
    settings->setValue("ZJUConnect/KeepAlive", ui->keepAliveCheckBox->isChecked());
    settings->setValue("ZJUConnect/OutsideAccess", ui->outsideAccessCheckBox->isChecked());

    settings->setValue("ZJUConnect/SkipDomainResource", ui->skipDomainResourceCheckBox->isChecked());
    settings->setValue("ZJUConnect/DisableServerConfig", ui->disableServerConfigCheckBox->isChecked());
    settings->setValue("ZJUConnect/ProxyAll", ui->proxyAllCheckBox->isChecked());

    settings->setValue("ZJUConnect/DisableZJUDNS", ui->disableDNSCheckBox->isChecked());
    settings->setValue("ZJUConnect/ZJUDefault", ui->zjuDefaultCheckBox->isChecked());
    settings->setValue("ZJUConnect/Debug", ui->debugCheckBox->isChecked());

    settings->setValue("ZJUConnect/TunMode", ui->tunCheckBox->isChecked());
    settings->setValue("ZJUConnect/AddRoute", ui->routeCheckBox->isChecked());
    settings->setValue("ZJUConnect/DNSHijack", ui->dnsHijackCheckBox->isChecked());


    settings->setValue("ZJUConnect/TcpPortForwarding", tcpPortForwarding);
    settings->setValue("ZJUConnect/UdpPortForwarding", udpPortForwarding);
    settings->setValue("ZJUConnect/CustomDNS", customDNS);
    settings->setValue("ZJUConnect/CustomProxyDomain", customProxyDomain);
    settings->setValue("ZJUConnect/ExtraArguments", extraArguments);

    settings->setValue("Common/ConfigVersion", Utils::CONFIG_VERSION);

    settings->sync();
}
