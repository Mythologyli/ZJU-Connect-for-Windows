#ifndef UTILS_H
#define UTILS_H

namespace Utils
{
    QString ConsoleOutputToQString(const QByteArray& byteArray);

    void setWidgetFixedWhenHidden(QWidget* widget);

    void showAboutMessageBox(QWidget* parent = nullptr);

    void SetProxyForAllConnections(const QString& proxyServer, const QString& bypass);

    void setSystemProxy(int port);

    void clearSystemProxy();

    QString getIpv4Address(const QString& interfaceName);
}

#endif //UTILS_H
