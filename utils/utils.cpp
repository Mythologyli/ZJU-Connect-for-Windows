#include <QMessageBox>
#include <QApplication>
#include <QSettings>

#include "utils.h"

void Utils::showAboutMessageBox(QWidget *parent)
{
    QMessageBox messageBox(parent);
    messageBox.setWindowTitle("关于本软件");
    messageBox.setTextFormat(Qt::RichText);
    QString aboutText = QApplication::applicationName() + " v" + QApplication::applicationVersion() +
                        "<br>基于 Qt 编写的 ZJUConnect 图形界面" +
                        "<br>作者: <a href='https://myth.cx'>Myth</a>" +
                        "<br>项目主页: <a href='https://github.com/Mythologyli/ZJU-Connect-for-Windows'>https://github.com/Mythologyli/ZJU-Connect-for-Windows</a>" +
                        "<br><br>zju-connect" +
                        "<br>ZJU RVPN 客户端的 Go 语言实现，基于 EasierConnect" +
                        "<br>作者: <a href='https://myth.cx'>Myth</a>" +
                        "<br>项目主页: <a href='https://github.com/Mythologyli/zju-connect'>https://github.com/Mythologyli/zju-connect</a>" +
                        "<br><br>EasierConnect" +
                        "<br>EasyConnect 客户端的开源实现" +
                        "<br>作者: <a href='https://github.com/lyc8503'>lyc8503</a>" +
                        "<br>项目主页: <a href='https://github.com/lyc8503/EasierConnect'>https://github.com/lyc8503/EasierConnect</a>";
    messageBox.setText(aboutText);
    messageBox.setIconPixmap(QPixmap(":/resource/icon.png").scaled(
        100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation
    ));
    messageBox.exec();
}

void Utils::setSystemProxy(int port)
{
    QSettings proxySettings(
        R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Internet Settings)",
        QSettings::NativeFormat
    );

    proxySettings.setValue("ProxyEnable", 1);
    proxySettings.setValue("ProxyServer", "127.0.0.1:" + QString::number(port));
    proxySettings.setValue("ProxyOverride", "");
}

void Utils::clearSystemProxy()
{
    QSettings proxySettings(
        R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Internet Settings)",
        QSettings::NativeFormat
    );

    proxySettings.setValue("ProxyEnable", 0);
    proxySettings.setValue("ProxyServer", "");
    proxySettings.setValue("ProxyOverride", "");
}
