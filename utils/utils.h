#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QByteArray>
#include <QWidget>

namespace Utils
{
    const inline QString REPO_NAME("PageChen04/EZ4Connect");

    const inline QString APP_NAME("EZ4Connect");

    constexpr inline int CONFIG_VERSION = 4;

    QString ConsoleOutputToQString(const QByteArray &byteArray);

    void setWidgetFixedWhenHidden(QWidget *widget);

    void showAboutMessageBox(QWidget *parent = nullptr);

    bool isSystemProxySet();

    void setSystemProxy(int http_port, int socks_port, const QString &bypass);

    void clearSystemProxy();

    QString getIpv4Address(const QString &interfaceName);

    void setAutoStart(bool enable);

    bool credentialCheck(const QString &username, const QString &password);
}

#endif //UTILS_H
