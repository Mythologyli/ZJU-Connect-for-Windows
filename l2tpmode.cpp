#include <QMessageBox>
#include <QTimer>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utils/utils.h"

void MainWindow::setModeToL2tp()
{
    mode = "有线网 L2TP";

    clearLog();

    networkDetector->start();

    ui->interfaceLabel->hide();
    ui->refreshInterfaceButton->hide();
    ui->interfaceComboBox->hide();

    // 清除系统代理
    if (isSystemProxySet)
    {
        Utils::clearSystemProxy();
        isSystemProxySet = false;
        addLog("已清除系统代理设置");
    }

    disconnect(ui->pushButton1, &QPushButton::clicked, nullptr, nullptr);

    if (!isL2tpLinked)
    {
        ui->pushButton1->setText("连接 L2TP");
    }
    else
    {
        ui->pushButton1->setText("断开 L2TP");
    }

    ui->pushButton1->setEnabled(false);

    disconnect(ui->pushButton2, &QPushButton::clicked, nullptr, nullptr);
    ui->pushButton2->hide();

    addLog("工作模式设置为：有线网 L2TP");

    // 检查是否存在 VPN
    disconnect(processForL2tp, &QProcess::finished, nullptr, nullptr);
    connect(processForL2tp, &QProcess::finished, this, [&]()
    {
        QString output = QString::fromLocal8Bit(processForL2tp->readAllStandardOutput()).trimmed();
        if (output.isEmpty())
        {
            addLog("警告：在系统中未发现 VPN 配置！请使用高级 - 创建 L2TP VPN 创建 VPN");
        }
        else
        {
            QString line;
            QTextStream stream(&output);

            bool isVpnInSettingsExist = false;

            while (!stream.atEnd())
            {
                line = stream.readLine();
                if (line.contains("Name") && line.contains(settings->value("L2TP/Name", "ZJUVPN").toString()))
                {
                    auto vpnName = line.split(":")[1].trimmed();
                    if (vpnName == settings->value("L2TP/Name", "ZJUVPN").toString())
                    {
                        isVpnInSettingsExist = true;
                        break;
                    }
                }
            }

            if (!isVpnInSettingsExist)
            {
                addLog(
                    "警告：在系统中未发现名称为 "
                    + settings->value("L2TP/Name", "ZJUVPN").toString()
                    + " 的 VPN，请在设置中修改 VPN 名称"
                );
            }
        }

        ui->pushButton1->setEnabled(true);

        emit SetModeFinished();
    });

    processForL2tp->start("powershell", QStringList() << "-command" << "Get-VpnConnection");

    connect(ui->pushButton1, &QPushButton::clicked,
            [&]()
            {
                ui->pushButton1->setEnabled(false);
                ui->modeComboBox->setEnabled(false);
                ui->createL2tpAction->setEnabled(false);

                QString l2tpName = settings->value("L2TP/Name", "ZJUVPN").toString();
                if (l2tpName.isEmpty())
                {
                    QMessageBox::critical(this, "错误", "L2TP VPN 名称不能为空");
                    return;
                }

                if (!isL2tpLinked)
                {
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

                    disconnect(processForL2tp, &QProcess::finished, nullptr, nullptr);
                    connect(processForL2tp, &QProcess::finished, this, [&]()
                    {
                        QString output = QString::fromLocal8Bit(processForL2tp->readAllStandardOutput()).trimmed();
                        if (output.contains("命令已完成") || output.contains("Command completed successfully"))
                        {
                            isL2tpLinked = true;
                            ui->pushButton1->setText("断开 L2TP");
                            addLog("连接成功！");

                            if (settings->value("L2TP/AutoReconnect", false).toBool())
                            {
                                l2tpCheckTimer = new QTimer(this);
                                connect(l2tpCheckTimer, &QTimer::timeout, this, [&]()
                                {
                                    disconnect(processForL2tpCheck, &QProcess::finished, nullptr, nullptr);
                                    connect(processForL2tpCheck, &QProcess::finished, this, [&]()
                                    {
                                        QString output = QString::fromLocal8Bit(
                                            processForL2tpCheck->readAllStandardOutput()
                                        ).trimmed();
                                        if (output.contains("(0%") && !output.contains("unreachable") &&
                                            !output.contains("无法"))
                                        {
                                            addLog("自动检测结果：L2TP VPN 连接正常");
                                        }
                                        else
                                        {
                                            addLog("自动检测结果：L2TP VPN 连接异常，正在重连...");
                                            isL2tpReconnecting = true;
                                            ui->pushButton1->click();
                                        }
                                    });

                                    processForL2tpCheck->start(
                                        "ping",
                                        QStringList()
                                            << "-n"
                                            << "1"
                                            << settings->value("L2TP/CheckIp", "39.156.66.10").toString()
                                    );
                                });

                                l2tpCheckTimer->start(settings->value("L2TP/CheckTime", 600).toInt() * 1000);
                            }
                        }
                        else
                        {
                            addLog("连接失败！");

                            if (output.contains("623"))
                            {
                                output = "请检查 VPN 名称是否设置正确\n"
                                         "如果你没有创建过 VPN，请使用高级-创建 L2TP VPN\n"
                                         "如果你创建过 VPN，请打开系统设置-网络-VPN，将 VPN 名称填写在本软件设置-L2TP 中\n"
                                         "详细信息为：\n" + output;
                            }
                            else if (output.contains("691"))
                            {
                                output = "请检查设置中的网络账号和密码是否设置正确！\n详细信息为：\n" + output;
                            }
                            else if (output.contains("789"))
                            {
                                output = "连接失败！这个问题通常是注册表造成的\n"
                                         "建议删除现有 VPN，然后使用高级-创建 L2TP VPN\n"
                                         "如果您刚刚使用本程序创建了 VPN，请重启电脑\n详细信息为：\n" + output;
                            }
                            else if (output.contains("868"))
                            {
                                output = "请检查系统 DNS 是否设置正确！\n详细信息为：\n" + output;
                            }

                            addLog(output);
                            ui->modeComboBox->setEnabled(true);
                            ui->createL2tpAction->setEnabled(true);
                            QMessageBox::critical(this, "错误", output);
                        }

                        ui->pushButton1->setEnabled(true);
                    });

                    processForL2tp->start(
                        "rasdial",
                        QStringList()
                            << l2tpName
                            << settings->value("Common/Username").toString()
                            << QByteArray::fromBase64(settings->value("Common/Password").toString().toUtf8())
                    );

                    addLog("正在连接 L2TP VPN: " + l2tpName);
                }
                else
                {
                    if (l2tpCheckTimer != nullptr)
                    {
                        disconnect(processForL2tpCheck, &QProcess::finished, nullptr, nullptr);
                        l2tpCheckTimer->stop();
                        delete l2tpCheckTimer;
                        l2tpCheckTimer = nullptr;
                    }

                    disconnect(processForL2tp, &QProcess::finished, nullptr, nullptr);
                    connect(processForL2tp, &QProcess::finished, this, [&]()
                    {
                        QString output = QString::fromLocal8Bit(processForL2tp->readAllStandardOutput());
                        if (output.contains("命令已完成") || output.contains("Command completed successfully"))
                        {
                            isL2tpLinked = false;

                            ui->pushButton1->setText("连接 L2TP");
                            addLog("断开成功！");
                        }
                        else
                        {
                            addLog("断开失败！");
                            addLog(output);
                            QMessageBox::critical(this, "错误", output);
                        }

                        ui->pushButton1->setEnabled(true);
                        ui->modeComboBox->setEnabled(true);
                        ui->createL2tpAction->setEnabled(true);

                        if (isL2tpReconnecting)
                        {
                            isL2tpReconnecting = false;
                            ui->pushButton1->click();
                        }
                    });

                    processForL2tp->start("rasdial", QStringList() << l2tpName << "/DISCONNECT");

                    addLog("正在断开 L2TP VPN: " + l2tpName);
                }
            });
}
