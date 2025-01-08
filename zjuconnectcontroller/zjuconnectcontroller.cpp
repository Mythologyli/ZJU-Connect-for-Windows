#include "zjuconnectcontroller.h"
#include "../utils/utils.h"

ZjuConnectController::ZjuConnectController()
{
    zjuConnectProcess = new QProcess(this);

    connect(zjuConnectProcess, &QProcess::readyReadStandardOutput, this, [&]()
    {
        QString output = Utils::ConsoleOutputToQString(zjuConnectProcess->readAllStandardOutput());

        emit outputRead(output);

        if (output.contains("Access is denied."))
        {
            emit error(ZJU_ERROR::ACCESS_DENIED);
        }
        else if (output.contains("listen failed"))
        {
            emit error(ZJU_ERROR::LISTEN_FAILED);
        }
        else if (output.contains("client setup error"))
        {
            emit error(ZJU_ERROR::CLIENT_FAILED);
        }
        else if (output.contains("Invalid username or password!"))
        {
            emit error(ZJU_ERROR::INVALID_DETAIL);
        }
        else if (output.contains("You are trying brute-force login on this IP address."))
        {
            emit error(ZJU_ERROR::BRUTE_FORCE);
        }
        else if (output.contains("Login failed") || output.contains("too many login failures"))
        {
            emit error(ZJU_ERROR::OTHER_LOGIN_FAILED);
        }
        else if (output.contains("panic"))
        {
            emit error(ZJU_ERROR::OTHER);
        }
    });

    connect(zjuConnectProcess, &QProcess::readyReadStandardError, this, [&]()
    {
        QString output = Utils::ConsoleOutputToQString(zjuConnectProcess->readAllStandardError());

        emit outputRead(output);

        if (output.contains("Access is denied."))
        {
            emit error(ZJU_ERROR::ACCESS_DENIED);
        }
        else if (output.contains("listen failed"))
        {
            emit error(ZJU_ERROR::LISTEN_FAILED);
        }
        else if (output.contains("client setup error"))
        {
            emit error(ZJU_ERROR::CLIENT_FAILED);
        }
    	else if (output.contains("Invalid username or password!"))
        {
            emit error(ZJU_ERROR::INVALID_DETAIL);
        }
        else if (output.contains("You are trying brute-force login on this IP address."))
        {
            emit error(ZJU_ERROR::BRUTE_FORCE);
        }
        else if (output.contains("Login failed") || output.contains("too many login failures"))
        {
            emit error(ZJU_ERROR::OTHER_LOGIN_FAILED);
        }
        else if (output.contains("panic"))
        {
            emit error(ZJU_ERROR::OTHER);
        }
    });

    connect(zjuConnectProcess, &QProcess::errorOccurred, this, [&](QProcess::ProcessError err)
    {
        QString timeString = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        QString errorString = zjuConnectProcess->errorString();
        emit outputRead(timeString + " 错误：" + errorString);

        if (errorString.contains("No such file or directory"))
        {
            emit error(ZJU_ERROR::PROGRAM_NOT_FOUND);
        }
        else
        {
            emit error(ZJU_ERROR::OTHER);
        }
    });

    connect(zjuConnectProcess, &QProcess::finished, this, [&]()
    {
        emit finished();
    });
}

void ZjuConnectController::start(
    const QString& program,
    const QString& username,
    const QString& password,
    const QString& server,
    int port,
    const QString& dns,
    const QString& secondaryDns,
    bool disableMultiLine,
    bool disableKeepAlive,
    bool proxyAll,
    const QString& socksBind,
    const QString& httpBind,
    const QString& shadowsocksUrl,
    bool tunMode,
    bool addRoute,
    bool dnsHijack,
    bool debugDump,
    const QString& tcpPortForwarding,
    const QString& udpPortForwarding
)
{
    QStringList args;

    if (!server.isEmpty())
    {
        args.append("-server");
        args.append(server);
    }

    if (port != 0)
    {
        args.append("-port");
        args.append(QString::number(port));
    }

    if (!dns.isEmpty())
    {
        args.append("-zju-dns-server");
        args.append(dns);
    }

    if (!secondaryDns.isEmpty())
    {
        args.append("-secondary-dns-server");
        args.append(secondaryDns);
    }

    if (disableMultiLine)
    {
        args.append("-disable-multi-line");
    }

    if (disableKeepAlive)
    {
        args.append("-disable-keep-alive");
    }

    if (proxyAll)
    {
        args.append("-proxy-all");
    }

    if (tunMode)
    {
        args.append("-tun-mode");

        if (dnsHijack)
        {
            args.append("-dns-hijack");
        }

        if (addRoute)
        {
            args.append("-add-route");
        }
    }

    if (debugDump)
    {
        args.append("-debug-dump");
    }

    if (!socksBind.isEmpty())
    {
        args.append("-socks-bind");
        args.append(socksBind);
    }

    if (!httpBind.isEmpty())
    {
        args.append("-http-bind");
        args.append(httpBind);
    }

    if (!shadowsocksUrl.isEmpty())
    {
        args.append("-shadowsocks-url");
        args.append(shadowsocksUrl);
    }

    if (!tcpPortForwarding.isEmpty())
    {
        args.append("-tcp-port-forwarding");
        args.append(tcpPortForwarding);
    }

    if (!udpPortForwarding.isEmpty())
    {
        args.append("-udp-port-forwarding");
        args.append(udpPortForwarding);
    }

    QString timeString = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    emit outputRead(timeString + " VPN 启动！参数：" + args.join(' '));

    zjuConnectProcess->start(program, QStringList({ "-username", username, "-password", password }) + args);
    zjuConnectProcess->waitForStarted();
    if (zjuConnectProcess->state() == QProcess::NotRunning)
    {
        emit finished();
    }
}

void ZjuConnectController::stop()
{
    if (zjuConnectProcess->state() == QProcess::NotRunning)
    {
        return;
    }

    zjuConnectProcess->kill();
    zjuConnectProcess->waitForFinished();
}

ZjuConnectController::~ZjuConnectController()
{
    stop();
}
