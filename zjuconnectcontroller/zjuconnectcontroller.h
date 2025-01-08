#ifndef ZJUCONNECTCONTROLLER_H
#define ZJUCONNECTCONTROLLER_H

#include <QtCore>

enum class ZJU_ERROR
{
    NONE,
    INVALID_DETAIL,
    BRUTE_FORCE,
    OTHER_LOGIN_FAILED,
    ACCESS_DENIED,
    LISTEN_FAILED,
    CLIENT_FAILED,
    PROGRAM_NOT_FOUND,
    OTHER,
};

class ZjuConnectController : public QObject
{
Q_OBJECT

public:
    ZjuConnectController();

    ~ZjuConnectController() override;

    void start(
        const QString& program,
        const QString& username,
        const QString& password,
        const QString& server = "",
        int port = 0,
        const QString& dns = "",
        const QString& secondaryDns = "",
        bool disableMultiLine = false,
        bool disableKeepAlive = false,
        bool proxyAll = false,
        const QString& socksBind = "",
        const QString& httpBind = "",
        const QString& shadowsocksUrl = "",
        bool tunMode = false,
        bool addRoute = false,
        bool dnsHijack = false,
        bool debugDump = false,
        const QString& tcpPortForwarding = "",
        const QString& udpPortForwarding = ""
    );

    void stop();


signals:

    void error(ZJU_ERROR err);

    void outputRead(const QString &output);

    void finished();

private:
    QProcess *zjuConnectProcess;

};

#endif //ZJUCONNECTCONTROLLER_H
