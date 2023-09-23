#include "zjuconnectcontroller.h"

ZjuConnectController::ZjuConnectController()
{
    zjuConnectProcess = new QProcess(this);

    connect(zjuConnectProcess, &QProcess::readyReadStandardOutput, this, [&]()
    {
        QString output = QString::fromLocal8Bit(zjuConnectProcess->readAllStandardOutput());

        emit outputRead(output);

        if (output.contains("Login FAILED") || output.contains("too many login failures"))
        {
            emit loginFailed();
        }
    });

    connect(zjuConnectProcess, &QProcess::readyReadStandardError, this, [&]()
    {
        QString output = QString::fromLocal8Bit(zjuConnectProcess->readAllStandardError());

        emit outputRead(output);

        if (output.contains("Login FAILED") || output.contains("too many login failures"))
        {
            emit loginFailed();
        }
    });

    connect(zjuConnectProcess, &QProcess::finished, this, [&]()
    {
        emit finished();
    });
}

void ZjuConnectController::start(
    const QString &program,
    const QString &username,
    const QString &password,
    const QString &server,
    int port,
    bool disableMultiLine,
    bool proxyAll,
    const QString &socksBind,
    const QString &httpBind,
    bool debugDump,
    const QString &tcpPortForwarding,
    const QString &udpPortForwarding
)
{
    QList<QString> args = QStringList({"-username", username, "-password", password});

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

    if (disableMultiLine)
    {
        args.append("-disable-multi-line");
    }

    if (proxyAll)
    {
        args.append("-proxy-all");
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

    zjuConnectProcess->start(program, args);
    zjuConnectProcess->waitForStarted();
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
