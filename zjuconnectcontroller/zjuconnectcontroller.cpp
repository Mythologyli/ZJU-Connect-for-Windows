#include "zjuconnectcontroller.h"
#include "../utils/utils.h"

ZjuConnectController::ZjuConnectController()
{
    zjuConnectProcess = new QProcess(this);

    auto outputProcess = [&](const QString& output)
        {
            emit outputRead(output);

            if (output.contains("Access is denied."))
            {
                emit error(ZJU_ERROR::ACCESS_DENIED);
            }
            else if (output.contains("listen failed"))
            {
                emit error(ZJU_ERROR::LISTEN_FAILED);
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
            else if (output.contains("client setup error"))
            {
                emit error(ZJU_ERROR::CLIENT_FAILED);
            }
            else if (output.contains("panic"))
            {
                emit error(ZJU_ERROR::OTHER);
            }
        };

    connect(zjuConnectProcess, &QProcess::readyReadStandardOutput, this, [&, outputProcess]()
    {
        QString output = Utils::ConsoleOutputToQString(zjuConnectProcess->readAllStandardOutput());

		outputProcess(output);
    });

    connect(zjuConnectProcess, &QProcess::readyReadStandardError, this, [&, outputProcess]()
    {
        QString output = Utils::ConsoleOutputToQString(zjuConnectProcess->readAllStandardError());

		outputProcess(output);
    });

    connect(zjuConnectProcess, &QProcess::errorOccurred, this, [&](QProcess::ProcessError err)
    {
        QString timeString = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        QString errorString = zjuConnectProcess->errorString();
        emit outputRead(timeString + " 退出原因：" + errorString);

        if (errorString.contains("No such file or directory") || errorString.contains("not found") || errorString.contains("找不到"))
        {
            emit outputRead(timeString + " 核心路径：" + zjuConnectProcess->program());
            emit error(ZJU_ERROR::PROGRAM_NOT_FOUND);
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

    if (!dns.isEmpty() || dnsAuto)
    {
        args.append("-zju-dns-server");
		if (dnsAuto)
		{
			args.append("auto");
		}
		else
		{
			args.append(dns);
		}
    }

    if (dnsTtl != 3600)
    {
        args.append("-dns-ttl");
        args.append(QString::number(dnsTtl));
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

    if (disableZjuConfig)
    {
        args.append("-disable-zju-config");
    }

    if (disableZjuDns)
    {
        args.append("-disable-zju-dns");
    }

    if (disableServerConfig)
    {
        args.append("-disable-server-config");
    }

    if (proxyAll)
    {
        args.append("-proxy-all");
    }

    if (skipDomainResource)
    {
        args.append("-skip-domain-resource");
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

    if (!dialDirectProxy.isEmpty())
    {
        args.append("-dial-direct-proxy");
        args.append(dialDirectProxy);
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

    if (!customDNS.isEmpty())
    {
        args.append("-custom-dns");
        args.append(customDNS);
    }

    if (!customProxyDomain.isEmpty())
    {
        args.append("-custom-proxy-domain");
        args.append(customProxyDomain);
    }

    if (!extraArguments.isEmpty())
    {
        args.append(extraArguments.split(" "));
    }

    QString timeString = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    emit outputRead(timeString + " VPN 启动！参数：" + args.join(' '));

    QStringList credentialList({
            "-username", username,
            "-password", password,
        });

    if (!totpSecret.isEmpty())
    {
        emit outputRead(timeString + " 使用了 TOTP");
        credentialList.append("-totp-secret");
        credentialList.append(totpSecret);
    }

    zjuConnectProcess->start(program, credentialList + args);
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
