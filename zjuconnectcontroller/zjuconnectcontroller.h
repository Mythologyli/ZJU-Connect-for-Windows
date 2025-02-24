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
        const QString& totpSecret,
        const QString& server,
        int port,
        const QString& dns,
        bool dnsAuto,
        const QString& secondaryDns,
        int dnsTtl,
        const QString& socksBind,
        const QString& httpBind,
        const QString& shadowsocksUrl,
        const QString& dialDirectProxy,
        bool disableMultiLine,
        bool disableKeepAlive,
        bool skipDomainResource,
        bool disableServerConfig,
        bool proxyAll,
        bool disableZjuDns,
        bool disableZjuConfig,
        bool debugDump,
        bool tunMode,
        bool addRoute,
        bool dnsHijack,
        const QString& tcpPortForwarding,
        const QString& udpPortForwarding,
        const QString& customDNS,
        const QString& customProxyDomain,
        const QString& extraArguments
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
