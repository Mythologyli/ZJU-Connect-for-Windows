#ifndef UTILS_H
#define UTILS_H

namespace Utils
{
    QString ConsoleOutputToQString(const QByteArray& byteArray);

    QByteArray QStringToConsoleOutput(const QString& str);

    void setWidgetFixedWhenHidden(QWidget* widget);

    void showAboutMessageBox(QWidget* parent = nullptr);

    void SetProxyForAllConnections(const QString& proxyServer, const QString& bypass);

    void setSystemProxy(int port);

    void clearSystemProxy();

    QString getIpv4Address(const QString& interfaceName);

    bool sendIcmpEcho(const QString& targetIp, int timeoutMs = 1000);
}

#endif //UTILS_H
