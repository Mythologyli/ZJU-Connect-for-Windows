#ifndef NETWORKDETECTOR_H
#define NETWORKDETECTOR_H

#include <QtCore>

struct NetworkDetectResult
{
    bool isDefaultDnsAvailable = false;
    bool isZjuNet = false;
    bool isZjuDnsCorrect = false;
    bool isZjuWlan = false;
    bool isZjuWlanSecure = false;
    bool isZjuLan = false;
    QString zjuLanGateway = "";
    bool isInternetAvailable = false;
    bool isProxyEnabled = false;
};

class NetworkDetector : public QObject
{
Q_OBJECT

public:
    NetworkDetector();

    void start();

private:
    void checkZjuWlan();

    void checkZjuLan();

    void checkInternet();

    void checkProxy();

signals:

    void finished(NetworkDetectResult result);

private:
    QProcess *process;
    NetworkDetectResult result;
};


#endif //NETWORKDETECTOR_H
