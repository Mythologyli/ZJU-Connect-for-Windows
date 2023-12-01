#include "networkdetector.h"
#include "../utils/utils.h"

NetworkDetector::NetworkDetector()
{
    process = new QProcess(this);
}

void NetworkDetector::start()
{
    result.isDefaultDnsAvailable = false;
    result.isZjuNet = false;
    result.isZjuDnsCorrect = false;
    result.isZjuWlan = false;
    result.isZjuWlanSecure = false;
    result.isZjuLan = false;
    result.zjuLanGateway = "";
    result.isInternetAvailable = false;
    result.isProxyEnabled = false;

    disconnect(process, &QProcess::finished, this, nullptr);
    connect(process, &QProcess::finished, this, [&]()
    {
        disconnect(process, &QProcess::finished, this, nullptr);
        connect(process, &QProcess::finished, this, [&]()
        {
            QString output = Utils::ConsoleOutputToQString(process->readAllStandardOutput());
            if (!output.contains("timed out"))
            {
                result.isDefaultDnsAvailable = true;

                if (output.contains("10."))
                {
                    result.isZjuNet = true;
                    result.isZjuDnsCorrect = true;
                    checkZjuWlan();
                    return;
                }
            }
            else
            {
                result.isDefaultDnsAvailable = false;
            }

            disconnect(process, &QProcess::finished, this, nullptr);
            connect(process, &QProcess::finished, this, [&]()
            {
                QString output = Utils::ConsoleOutputToQString(process->readAllStandardOutput());
                if (!output.contains("timed out"))
                {
                    result.isZjuNet = true;
                    checkZjuWlan();
                }

                checkInternet();
            });

            process->start("nslookup", QStringList() << "dns1.zju.edu.cn" << "10.10.0.21");
        });

        process->start("nslookup", QStringList() << "dns1.zju.edu.cn");
    });

    process->start("ipconfig", QStringList() << "/flushdns");
}

void NetworkDetector::checkZjuWlan()
{
    disconnect(process, &QProcess::finished, this, nullptr);
    connect(process, &QProcess::finished, this, [&]()
    {
        QString output = Utils::ConsoleOutputToQString(process->readAllStandardOutput());
        if (output.contains("ZJUWLAN"))
        {
            if (output.contains("ZJUWLAN-Secure"))
            {
                result.isZjuWlanSecure = true;
            }
            else
            {
                result.isZjuWlan = true;
            }

            checkZjuLan();
            return;
        }

        checkZjuLan();
    });

    process->start("netsh", QStringList() << "wlan" << "show" << "interfaces");
}

void NetworkDetector::checkZjuLan()
{
    disconnect(process, &QProcess::finished, this, nullptr);
    connect(process, &QProcess::finished, this, [&]()
    {
        QString output = Utils::ConsoleOutputToQString(process->readAllStandardOutput());
        QString line;
        QTextStream stream(&output);

        bool waitForIp = false;
        bool waitForGateway = false;
        QString ip;

        while (!stream.atEnd())
        {
            line = stream.readLine();
            if (line.contains("以太网") || line.contains("Ethernet"))
            {
                waitForIp = true;
            }
            else if ((line.contains("IP 地址") || line.contains("IP Address")) && waitForIp)
            {
                ip = line.split(":").at(1).trimmed();
                if (ip.startsWith("10."))
                {
                    waitForGateway = true;
                }

                waitForIp = false;
            }
            else if ((line.contains("默认网关") || line.contains("Default Gateway")) && waitForGateway)
            {
                result.zjuLanGateway = line.split(":").at(1).trimmed();

                if (result.zjuLanGateway.startsWith("10."))
                {
                    result.isZjuLan = true;
                    checkInternet();
                    return;
                }

                result.zjuLanGateway = "";
            }
            else if ((line.contains("接口 \"") || line.contains("interface \"")) && (waitForIp || waitForGateway))
            {
                waitForIp = false;
                waitForGateway = false;
            }
        }

        checkInternet();
    });

    process->start("netsh", QStringList() << "interface" << "ipv4" << "show" << "addresses");
}

void NetworkDetector::checkInternet()
{
    disconnect(process, &QProcess::finished, this, nullptr);
    connect(process, &QProcess::finished, this, [&]()
    {
        QString output = Utils::ConsoleOutputToQString(process->readAllStandardOutput());
        if (output.contains("(0%") && !output.contains("unreachable") && !output.contains("无法"))
        {
            result.isInternetAvailable = true;
            checkProxy();
            return;
        }

        checkProxy();
    });

    process->start("ping", QStringList() << "39.156.66.10" << "-n" << "1");
}

void NetworkDetector::checkProxy()
{
    QSettings proxySettings(
        R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Internet Settings)",
        QSettings::NativeFormat
    );

    result.isProxyEnabled = proxySettings.value("ProxyEnable", 0).toInt() == 1;

    emit finished(result);
}
