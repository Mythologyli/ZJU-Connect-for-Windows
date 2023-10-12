#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utils/utils.h"

void MainWindow::setModeToWebLogin()
{
    mode = "网页认证登录";

    clearLog();

    networkDetector->start();

    ui->interfaceComboBox->clear();
    ui->interfaceComboBox->addItem("默认");
    ui->interfaceComboBox->setCurrentText("默认");
    auto interfaceName = settings->value("WebLogin/LastInterface", "默认").toString();
    if (interfaceName != "默认")
    {
        auto ipv4Address = Utils::getIpv4Address(interfaceName);
        if (!ipv4Address.isEmpty())
        {
            ui->interfaceComboBox->addItem(interfaceName, ipv4Address);
            ui->interfaceComboBox->setCurrentText(interfaceName);
        }
    }

    ui->interfaceComboBox->setCurrentText(settings->value("WebLogin/LastInterface", "默认").toString());
    ui->interfaceLabel->show();
    ui->refreshInterfaceButton->show();
    ui->interfaceComboBox->show();

    // 清除系统代理
    if (isSystemProxySet)
    {
        Utils::clearSystemProxy();
        isSystemProxySet = false;
        addLog("已清除系统代理设置");
    }

    disconnect(ui->pushButton1, &QPushButton::clicked, nullptr, nullptr);
    ui->pushButton1->setText("登录");
    disconnect(ui->pushButton2, &QPushButton::clicked, nullptr, nullptr);
    ui->pushButton2->hide();

    addLog("工作模式设置为：网页认证登录");

    connect(ui->pushButton1, &QPushButton::clicked,
            [&]()
            {
                ui->pushButton1->setEnabled(false);
                ui->modeComboBox->setEnabled(false);

                if (!isWebLogged)
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

                    disconnect(processForWebLogin, &QProcess::finished, nullptr, nullptr);
                    connect(processForWebLogin, &QProcess::finished, this, [&]()
                    {
                        QString output = QString::fromLocal8Bit(processForWebLogin->readAllStandardOutput()).trimmed();
                        if (output.contains("[Login Successful]") || output.contains("[Already Online]"))
                        {
                            isWebLogged = true;
                            ui->pushButton1->setText("注销");
                            addLog("登录成功！");
                            showNotification("网页认证登录", "登录成功！");
                        }
                        else
                        {
                            addLog("登录失败！");

                            if (output.contains("dhcp"))
                            {
                                output = "IP 不在 DHCP 表中！\n"
                                         "这通常表明您正在使用不支持网页认证登录的有线网/ZJUWLAN-Secure\n"
                                         "详细信息为：\n" + output;
                            }
                            else if (output.contains("User not found") || output.contains("Password is error"))
                            {
                                output = "登录失败！\n"
                                         "请检查设置中的网络账号和密码是否设置正确！\n"
                                         "详细信息为：\n" + output;
                            }

                            addLog(output);
                            ui->modeComboBox->setEnabled(true);
                            QMessageBox::critical(this, "错误", output);
                        }

                        ui->pushButton1->setEnabled(true);
                    });

                    QString ip;
                    if (ui->interfaceComboBox->currentText() == "默认")
                    {
                        ip = "default";
                    }
                    else
                    {
                        ip = ui->interfaceComboBox->currentData().toString();
                    }

                    QString url;
                    if (settings->value("WebLogin/EnableCustomUrl", false).toBool())
                    {
                        url = settings->value("WebLogin/CustomUrl", "https://net.zju.edu.cn").toString();
                    }
                    else if (settings->value("WebLogin/IntlUrl", false).toBool())
                    {
                        url = "https://zjuwlan.intl.zju.edu.cn";
                    }
                    else
                    {
                        url = "https://net.zju.edu.cn";
                    }

                    processForWebLogin->start(
                        "weblogin.exe",
                        QStringList()
                            << "login"
                            << "-u"
                            << settings->value("Common/Username").toString()
                            << "-p"
                            << QByteArray::fromBase64(settings->value("Common/Password").toString().toUtf8())
                            << "-i"
                            << ip
                            << "--url"
                            << url
                    );

                    addLog("正在进行网页认证登录，接口：" + ui->interfaceComboBox->currentText());
                }
                else
                {
                    disconnect(processForWebLogin, &QProcess::finished, nullptr, nullptr);
                    connect(processForWebLogin, &QProcess::finished, this, [&]()
                    {
                        isWebLogged = false;

                        ui->pushButton1->setText("登录");
                        addLog("注销成功！");

                        ui->pushButton1->setEnabled(true);
                        ui->modeComboBox->setEnabled(true);
                    });

                    QString ip;
                    if (ui->interfaceComboBox->currentText() == "默认")
                    {
                        ip = "default";
                    }
                    else
                    {
                        ip = ui->interfaceComboBox->currentData().toString();
                    }

                    QString url;
                    if (settings->value("WebLogin/EnableCustomUrl", false).toBool())
                    {
                        url = settings->value("WebLogin/CustomUrl", "https://net.zju.edu.cn").toString();
                    }
                    else if (settings->value("WebLogin/IntlUrl", false).toBool())
                    {
                        url = "https://zjuwlan.intl.zju.edu.cn";
                    }
                    else
                    {
                        url = "https://net.zju.edu.cn";
                    }

                    processForWebLogin->start(
                        "weblogin.exe",
                        QStringList()
                            << "logout"
                            << "-u"
                            << settings->value("Common/Username").toString()
                            << "-i"
                            << ip
                            << "--url"
                            << url
                    );

                    addLog("正在注销网页认证，接口：" + ui->interfaceComboBox->currentText());
                }
            });

    emit SetModeFinished();
}
