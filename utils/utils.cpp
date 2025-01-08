#include <QMessageBox>
#include <QApplication>
#include <QNetworkInterface>
#include <QTextCodec>
#include <QSettings>
#include <QProcess>
#include <QtSystemDetection>
#include <QDir>
#include <QFileInfo>

#include "utils.h"

QString Utils::ConsoleOutputToQString(const QByteArray &byteArray)
{
    static QString codeName = "";
    if (codeName == "UTF-8")
    {
        return QTextCodec::codecForName("UTF-8")->toUnicode(byteArray);;
    }
    else if (codeName == "GBK")
    {
        return QTextCodec::codecForName("GBK")->toUnicode(byteArray);;
    }
    else if (codeName == "locale")
    {
        return QString::fromLocal8Bit(byteArray);
    }
    else
    {
        QTextCodec *utf8Codec = QTextCodec::codecForName("UTF-8");
        QString utf8Str = utf8Codec->toUnicode(byteArray);
        QByteArray utf8ByteArrayBack = utf8Codec->fromUnicode(utf8Str);

        QTextCodec *gbkCodec = QTextCodec::codecForName("GBK");
        QString gbkStr = gbkCodec->toUnicode(byteArray);
        QByteArray gbkByteArrayBack = gbkCodec->fromUnicode(gbkStr);

        if (utf8ByteArrayBack == byteArray && gbkByteArrayBack != byteArray)
        {
            codeName = "UTF-8";
            return utf8Str;
        }
        else if (gbkByteArrayBack == byteArray && utf8ByteArrayBack != byteArray)
        {
            codeName = "GBK";
            return gbkStr;
        }
        else if (gbkByteArrayBack == byteArray && utf8ByteArrayBack == byteArray)
        {
            return utf8Str;
        }
        else
        {
            codeName = "locale";
            return QString::fromLocal8Bit(byteArray);
        }
    }
}

void Utils::setWidgetFixedWhenHidden(QWidget *widget)
{
    QSizePolicy originPolicy = widget->sizePolicy();
    originPolicy.setRetainSizeWhenHidden(true);
    widget->setSizePolicy(originPolicy);
}

void Utils::showAboutMessageBox(QWidget *parent)
{
    QMessageBox messageBox(parent);
    messageBox.setWindowTitle("关于");
    messageBox.setTextFormat(Qt::RichText);
    QString aboutText = QApplication::applicationDisplayName() + " " + QApplication::applicationVersion() +
                        "<br>针对哈工大深圳的修改版 ZJU-Connect-for-Windows" +
                        "<br>作者：<a href='https://github.com/chenx-dust'>Chenx Dust</a>" +
                        "<br>项目主页：<a href='https://github.com/" + REPO_NAME + "'>https://github.com/" + REPO_NAME + "</a>" +
                        "<br><br>ZJU-Connect-for-Windows" +
                        "<br>基于 Qt 编写的 ZJUConnect 图形界面" +
                        "<br>作者：<a href='https://myth.cx'>Myth</a>" +
                        "<br>项目主页：<a href='https://github.com/Mythologyli/ZJU-Connect-for-Windows'>https://github.com/Mythologyli/ZJU-Connect-for-Windows</a>" +
                        "<br><br>zju-connect" +
                        "<br>ZJU RVPN 客户端的 Go 语言实现，基于 EasierConnect" +
                        "<br>作者：<a href='https://myth.cx'>Myth</a>" +
                        "<br>项目主页：<a href='https://github.com/Mythologyli/zju-connect'>https://github.com/Mythologyli/zju-connect</a>" +
                        "<br><br>EasierConnect" +
                        "<br>EasyConnect 客户端的开源实现" +
                        "<br>作者：<a href='https://github.com/lyc8503'>lyc8503</a>" +
                        "<br>项目主页：<a href='https://github.com/lyc8503/EasierConnect'>https://github.com/lyc8503/EasierConnect</a>"
                        "<br><br>zju-web-login" +
                        "<br>ZJU 网页认证登录脚本" +
                        "<br>作者：<a href='https://azuk.top'>Azuk 443</a>" +
                        "<br>项目主页：<a href='https://github.com/Mythologyli/zju-web-login'>https://github.com/Mythologyli/zju-web-login</a>";
    messageBox.setText(aboutText);
    messageBox.setIconPixmap(QPixmap(":/resource/icon.png").scaled(
        100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation
    ));
    messageBox.exec();
}

QString Utils::getIpv4Address(const QString &interfaceName)
{
    auto interfaces = QNetworkInterface::allInterfaces();
    for (auto &singleInterface: interfaces)
    {
        if (singleInterface.humanReadableName() == interfaceName)
        {
            auto addresses =
                singleInterface.addressEntries();
            for (auto &address: addresses)
            {
                if (address.ip().protocol() == QAbstractSocket::IPv4Protocol)
                {
                    return address.ip().toString();
                }
            }
        }
    }

    return "";
}

QString macOSAppBundlePath()
{
#ifdef Q_OS_MAC
    QDir dir = QDir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    dir.cdUp();
    QString absolutePath = dir.absolutePath();
    // absolutePath will contain a "/" at the end,
    // but we want the clean path to the .app bundle
    if (absolutePath.length() > 0 && absolutePath.right(1) == "/")
    {
        absolutePath.chop(1);
    }
    return absolutePath;
#else
    return "";
#endif
}

QString macOSAppBundleName()
{
#ifdef Q_OS_MAC
    QString bundlePath = macOSAppBundlePath();
    QFileInfo fileInfo(bundlePath);
    return fileInfo.baseName();
#else
    return "";
#endif
}

QString nativeAppPath()
{
    return QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
}

void Utils::setAutoStart(bool enable)
{
#if defined(Q_OS_WINDOWS)
    QSettings autoStartSettings(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run)",
                                QSettings::NativeFormat);
    if (enable)
    {
        autoStartSettings.setValue(
            QApplication::applicationName(),
            nativeAppPath()
        );
    }
    else
    {
        autoStartSettings.remove(QApplication::applicationName());
    }
    autoStartSettings.sync();
#elif defined(Q_OS_MACOS)
    {
        QStringList args;
        args << "-e tell application \"System Events\" to delete login item\"" + macOSAppBundleName() + "\"";
        qDebug() << args;

        // int result = QProcess::execute("osascript", args);
        QProcess process;
        process.start("osascript", args);
        process.waitForFinished();
        qDebug() << process.readAllStandardOutput();
        qDebug() << process.readAllStandardError();
        if (!enable && process.error() != QProcess::UnknownError)
        {
            QMessageBox::critical(nullptr, "取消开机自启动失败", "无法删除登录项：" + process.errorString());
            return;
        }
        if (!enable && process.exitCode() != 0)
        {
            QMessageBox::critical(nullptr, "取消开机自启动失败", "无法删除登录项：" + process.readAllStandardError());
            return;
        }
    }
    if (enable)
    {
        QStringList args;
        args << "-e tell application \"System Events\" to make login item at end with properties {path:\"" +
                    macOSAppBundlePath() + "\", hidden:false}";
        qDebug() << args;

        QProcess process;
        process.start("osascript", args);
        process.waitForFinished();
        qDebug() << process.readAllStandardOutput();
        qDebug() << process.readAllStandardError();
        if (process.error() != QProcess::UnknownError)
        {
            QMessageBox::critical(nullptr, "设置开机自启动失败", "无法创建登录项：" + process.errorString());
            return;
        }
        if (process.exitCode() != 0)
        {
            QMessageBox::critical(nullptr, "设置开机自启动失败", "无法创建登录项：" + process.readAllStandardError());
            return;
        }
    }
#elif defined(Q_OS_LINUX)
    QString autostartPath = QDir::homePath() + "/.config/autostart/";
    QDir dir(autostartPath);
    QString desktopFilePath = autostartPath + QApplication::applicationName() + ".desktop";
    QFile desktopFile(desktopFilePath);

    if (dir.exists() && desktopFile.exists())
    {
        if (!desktopFile.remove())
        {
            QMessageBox::critical(nullptr, "取消开机自启动失败", "无法删除 .desktop 文件：" + desktopFilePath);
            return;
        }
    }

    if (enable)
    {
        if (!dir.exists())
        {
            if (!dir.mkpath("."))
            {
                QMessageBox::critical(nullptr, "设置开机自启动失败", "无法创建 autostart 目录：" + autostartPath);
                return;
            }
        }

        if (!desktopFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::critical(nullptr, "设置开机自启动失败", "无法创建 .desktop 文件：" + desktopFilePath);
            return;
        }

        QTextStream out(&desktopFile);
        out << "[Desktop Entry]\n";
        out << "Type=Application\n";
        out << "Name=" << QApplication::applicationName() << "\n";
        out << "Exec=" << nativeAppPath() << "\n";
        out << "X-GNOME-Autostart-enabled=true\n";
        desktopFile.close();
    }
#endif
}