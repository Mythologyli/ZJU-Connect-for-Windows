#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utils/utils.h"

void MainWindow::setModeToZjuConnect()
{
    mode = "RVPN";

    clearLog();

    networkDetector->start();

    ui->interfaceLabel->hide();
    ui->refreshInterfaceButton->hide();
    ui->interfaceComboBox->hide();
    ui->tunCheckBox->show();

    zjuConnectController = new ZjuConnectController();

    disconnect(ui->pushButton1, &QPushButton::clicked, nullptr, nullptr);
    ui->pushButton1->setText("连接服务器");
    disconnect(ui->pushButton2, &QPushButton::clicked, nullptr, nullptr);
    ui->pushButton2->setText("设置系统代理");
    ui->pushButton2->hide();

    addLog("工作模式设置为：RVPN");

    // 连接服务器
    connect(zjuConnectController, &ZjuConnectController::outputRead, this, [&](const QString &output)
    {
        ui->logPlainTextEdit->appendPlainText(output);
    });

    connect(zjuConnectController, &ZjuConnectController::loginFailed, this, [&]()
    {
        isZjuConnectLoginError = true;
    });

    connect(zjuConnectController, &ZjuConnectController::finished, this, [&]()
    {
        if (
            !isZjuConnectLoginError &&
            settings->value("ZJUConnect/AutoReconnect", false).toBool() &&
            isZjuConnectLinked
            )
        {
            QTimer::singleShot(settings->value("ZJUConnect/ReconnectTime", 1).toInt() * 1000, this, [&]()
            {
                if (isZjuConnectLinked)
                {
                    zjuConnectController->stop();

                    isZjuConnectLinked = false;
                    ui->pushButton1->click();
                }
            });

            return;
        }

        addLog("RVPN 断开！");
        showNotification("RVPN", "RVPN 断开！", QSystemTrayIcon::MessageIcon::Warning);
        isZjuConnectLinked = false;
        ui->pushButton1->setText("连接服务器");
        ui->modeComboBox->setEnabled(true);

        if (isZjuConnectLoginError)
        {
            isZjuConnectLoginError = false;
            QMessageBox::critical(this, "错误", "登录失败");
        }
    });

    connect(ui->pushButton1, &QPushButton::clicked,
            [&]()
            {
                if (!isZjuConnectLinked)
                {
                    if (settings->value("ZJUConnect/ServerAddress", "rvpn.zju.edu.cn").toString().isEmpty())
                    {
                        QMessageBox::critical(this, "错误", "服务器地址不能为空");
                        return;
                    }

                    if (settings->value("Common/Username", "").toString().isEmpty())
                    {
                        QMessageBox::critical(this, "错误", "用户名不能为空");
                        return;
                    }

                    if (QByteArray::fromBase64(settings->value("Common/Password", "").toString().toUtf8()).isEmpty())
                    {
                        QMessageBox::critical(this, "错误", "密码不能为空");
                        return;
                    }

                    zjuConnectController->start(
                        "zju-connect.exe",
                        settings->value("Common/Username").toString(),
                        QByteArray::fromBase64(settings->value("Common/Password").toString().toUtf8()),
                        settings->value("ZJUConnect/ServerAddress").toString(),
                        settings->value("ZJUConnect/ServerPort", 443).toInt(),
                        !settings->value("ZJUConnect/MultiLine", true).toBool(),
                        settings->value("ZJUConnect/ProxyAll", false).toBool(),
                        "127.0.0.1:" + QString::number(settings->value("ZJUConnect/Socks5Port", 1080).toInt()),
                        "127.0.0.1:" + QString::number(settings->value("ZJUConnect/HttpPort", 1081).toInt()),
                        ui->tunCheckBox->isChecked(),
                        settings->value("ZJUConnect/Route", false).toBool(),
                        settings->value("ZJUConnect/Debug", false).toBool(),
                        settings->value("ZJUConnect/TcpPortForwarding", "").toString(),
                        settings->value("ZJUConnect/UdpPortForwarding", "").toString()
                    );

                    isZjuConnectLinked = true;
                    ui->modeComboBox->setEnabled(false);
                    ui->pushButton1->setText("断开服务器");
                    ui->pushButton2->show();

                    if (settings->value("ZJUConnect/AutoSetProxy", false).toBool())
                    {
                        ui->pushButton2->click();
                    }
                }
                else
                {
                    isZjuConnectLinked = false;

                    zjuConnectController->stop();

                    if (isSystemProxySet)
                    {
                        ui->pushButton2->click();
                    }

                    ui->pushButton1->setText("连接服务器");
                    ui->pushButton2->hide();
                    ui->modeComboBox->setEnabled(true);
                }
            });

    // 设置系统代理
    connect(ui->pushButton2, &QPushButton::clicked,
            [&]()
            {
                if (!isSystemProxySet)
                {
                    if (networkDetectResult.isProxyEnabled)
                    {
                        QMessageBox messageBox(this);
                        messageBox.setWindowTitle("警告");
                        messageBox.setText(
                            "当前已存在系统代理配置\n如您正在使用代理工具，推荐使用高级-ZJU Rule\n是否覆盖当前系统代理配置？"
                        );

                        messageBox.addButton(QMessageBox::Yes)->setText("是");
                        messageBox.addButton(QMessageBox::No)->setText("否");
                        messageBox.setDefaultButton(QMessageBox::Yes);

                        if (messageBox.exec() == QMessageBox::No)
                        {
                            return;
                        }
                    }

                    Utils::setSystemProxy(settings->value("ZJUConnect/HttpPort", 1081).toInt());
                    ui->pushButton2->setText("清除系统代理");
                    isSystemProxySet = true;
                }
                else
                {
                    Utils::clearSystemProxy();
                    ui->pushButton2->setText("设置系统代理");
                    isSystemProxySet = false;
                }
            });

    emit SetModeFinished();
}
