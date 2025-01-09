#ifndef PROXYSETTINGS_H
#define PROXYSETTINGS_H

#include <QProcess>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include "utils.h"

#if defined(Q_OS_WINDOWS)
#include "windows.h"
#include "wininet.h"
#include "ras.h"
#include "raserror.h"
#endif

const QString macOSNetworkSetupPath = "/usr/sbin/networksetup";

void windowsSetProxyForAllConnections(const QString &proxyServer, const QString &bypass)
{
#if defined(Q_OS_WINDOWS)
    INTERNET_PER_CONN_OPTION_LIST optionList;
    INTERNET_PER_CONN_OPTION optionsArr[3];
    unsigned long optionListSize = sizeof(INTERNET_PER_CONN_OPTION_LIST);

    optionsArr[1].dwOption = INTERNET_PER_CONN_FLAGS;
    optionsArr[1].Value.dwValue = 0;
    if (proxyServer != "")
    {
        optionsArr[1].Value.dwValue = PROXY_TYPE_DIRECT | PROXY_TYPE_PROXY;
    }
    else
    {
        optionsArr[1].Value.dwValue = PROXY_TYPE_DIRECT;
    }

    optionsArr[0].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
    auto *proxyServerWStr = (wchar_t *)calloc(sizeof(wchar_t), proxyServer.length() + 1);
    proxyServer.toWCharArray(proxyServerWStr);
    optionsArr[0].Value.pszValue = proxyServerWStr;

    optionsArr[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
    auto *bypassWStr = (wchar_t *)calloc(sizeof(wchar_t), bypass.length() + 1);
    bypass.toWCharArray(bypassWStr);
    optionsArr[2].Value.pszValue = bypassWStr;

    optionList.dwSize = sizeof(INTERNET_PER_CONN_OPTION_LIST);
    optionList.pszConnection = nullptr;
    optionList.dwOptionCount = 3;
    optionList.dwOptionError = 0;
    optionList.pOptions = optionsArr;

    InternetSetOption(nullptr, INTERNET_OPTION_PER_CONNECTION_OPTION, &optionList, optionListSize);

    DWORD dwCb = 0;
    DWORD dwRet = ERROR_SUCCESS;
    DWORD dwEntries = 0;
    LPRASENTRYNAME lpRasEntryName = nullptr;

    dwRet = RasEnumEntries(nullptr, nullptr, lpRasEntryName, &dwCb, &dwEntries);

    if (dwRet == ERROR_BUFFER_TOO_SMALL)
    {
        lpRasEntryName = (LPRASENTRYNAME)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwCb);
        if (lpRasEntryName == nullptr)
        {
            free(proxyServerWStr);
            free(bypassWStr);
            return;
        }
        lpRasEntryName[0].dwSize = sizeof(RASENTRYNAME);

        dwRet = RasEnumEntries(nullptr, nullptr, lpRasEntryName, &dwCb, &dwEntries);

        if (ERROR_SUCCESS == dwRet)
        {
            for (DWORD i = 0; i < dwEntries; i++)
            {
                optionList.pszConnection = lpRasEntryName[i].szEntryName;
                InternetSetOption(nullptr, INTERNET_OPTION_PER_CONNECTION_OPTION, &optionList, optionListSize);
            }
        }

        HeapFree(GetProcessHeap(), 0, lpRasEntryName);
    }

    free(proxyServerWStr);
    free(bypassWStr);
#endif
}

QStringList macOSGetActiveNetworkServices()
{
#if defined(Q_OS_MACOS)
    QStringList activeServices;
    QProcess process;
    process.start(macOSNetworkSetupPath, QStringList() << "-listallnetworkservices");
    process.waitForFinished();
    if (process.error() != QProcess::UnknownError)
    {
        QMessageBox::critical(nullptr, "获取网络服务失败", "执行命令失败：" + process.errorString());
        return {};
    }
    if (process.exitCode() != 0)
    {
        QMessageBox::critical(nullptr, "获取网络服务失败", "无法获取网络服务：" + process.readAllStandardError());
        return {};
    }
    /*
    output will be like this:

    An asterisk (*) denotes that a network service is disabled.
    USB 10/100/1000 LAN
    *AX88179A
    Thunderbolt Bridge
    Wi-Fi
    iPhone USB
    */
    QString output = process.readAllStandardOutput();
    qDebug() << output;
    QStringList lines = output.split('\n');
    lines.removeFirst();
    for (const QString &line : lines)
    {
        if (line.isEmpty())
            continue;
        if (line.startsWith("*"))
            continue;
        activeServices.push_back(line);
    }
    return activeServices;
#else
    return {};
#endif
}

enum class macOSProxyType
{
    WebProxy,
    SecureWebProxy,
    SOCKSFirewallProxy
};

bool macOSIsSystemProxySet(macOSProxyType proxyType, const QString networkService)
{
    QStringList args;
    switch (proxyType)
    {
    case macOSProxyType::WebProxy:
        args << "-getwebproxy";
        break;
    case macOSProxyType::SecureWebProxy:
        args << "-getsecurewebproxy";
        break;
    case macOSProxyType::SOCKSFirewallProxy:
        args << "-getsocksfirewallproxy";
        break;
    }
    args << networkService;
    QProcess process;
    process.start(macOSNetworkSetupPath, args);
    process.waitForFinished();
    if (process.error() != QProcess::UnknownError)
    {
        QMessageBox::critical(nullptr, "获取系统代理设置失败", "执行命令失败：" + process.errorString());
        return true;
    }
    if (process.exitCode() != 0)
    {
        QMessageBox::critical(nullptr, "获取系统代理设置失败", "无法获取系统代理设置：" + process.readAllStandardError());
        return true;
    }
    return process.readAllStandardOutput().contains("Enabled: Yes");
}

void macOSSetSystemProxy(macOSProxyType proxyType, const QString &networkService, const QString &proxyServer, int port)
{
    QStringList args;
    switch (proxyType)
    {
    case macOSProxyType::WebProxy:
        args << "-setwebproxy";
        break;
    case macOSProxyType::SecureWebProxy:
        args << "-setsecurewebproxy";
        break;
    case macOSProxyType::SOCKSFirewallProxy:
        args << "-setsocksfirewallproxy";
        break;
    }
    args << networkService << proxyServer << QString::number(port);
    QProcess process;
    process.start(macOSNetworkSetupPath, args);
    process.waitForFinished();
    if (process.error() != QProcess::UnknownError)
    {
        QMessageBox::critical(nullptr, "设置系统代理失败", "执行命令失败：" + process.errorString());
        return;
    }
    if (process.exitCode() != 0)
    {
        QMessageBox::critical(nullptr, "设置系统代理失败", "无法设置系统代理：" + process.readAllStandardError());
        return;
    }
}

void macOSDisableSystemProxy(macOSProxyType proxyType, const QString &networkService)
{
    QStringList args;
    switch (proxyType)
    {
    case macOSProxyType::WebProxy:
        args << "-setwebproxystate";
        break;
    case macOSProxyType::SecureWebProxy:
        args << "-setsecurewebproxystate";
        break;
    case macOSProxyType::SOCKSFirewallProxy:
        args << "-setsocksfirewallproxystate";
        break;
    }
    args << networkService << "off";
    QProcess process;
    process.start(macOSNetworkSetupPath, args);
    process.waitForFinished();
    if (process.error() != QProcess::UnknownError)
    {
        QMessageBox::critical(nullptr, "禁用系统代理失败", "执行命令失败：" + process.errorString());
        return;
    }
    if (process.exitCode() != 0)
    {
        QMessageBox::critical(nullptr, "禁用系统代理失败", "无法禁用系统代理：" + process.readAllStandardError());
        return;
    }
}

void macOSSetProxyBypass(const QString &networkService, const QString &bypass)
{
    QStringList args;
    args << "-setproxybypassdomains";
    args << networkService;
    args << bypass;
    QProcess process;
    process.start(macOSNetworkSetupPath, args);
    process.waitForFinished();
    if (process.error() != QProcess::UnknownError)
    {
        QMessageBox::critical(nullptr, "设置代理绕过失败", "执行命令失败：" + process.errorString());
        return;
    }
    if (process.exitCode() != 0)
    {
        QMessageBox::critical(nullptr, "设置代理绕过失败", "无法设置代理绕过：" + process.readAllStandardError());
        return;
    }
}

using ProcessArgument = QPair<QString, QStringList>;

void linuxSetSystemProxy(const QString &proxyServer, int httpPort, int socksPort, const QString &bypass)
{
    QList<ProcessArgument> actions;
    actions << ProcessArgument{"gsettings", {"set", "org.gnome.system.proxy", "mode", "manual"}};
    //
    bool isKDE = qEnvironmentVariable("XDG_SESSION_DESKTOP") == "KDE" ||
                 qEnvironmentVariable("XDG_SESSION_DESKTOP") == "plasma";
    const auto configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);

    //
    // Configure HTTP Proxies for HTTP, FTP and HTTPS
    // if (hasHTTP)
    {
        // iterate over protocols...
        for (const auto &protocol : QStringList{"http", "ftp", "https"})
        {
            // for GNOME:
            {
                actions << ProcessArgument{"gsettings",
                                           {"set", "org.gnome.system.proxy." + protocol, "host", proxyServer}};
                actions << ProcessArgument{"gsettings",
                                           {"set", "org.gnome.system.proxy." + protocol, "port", QString::number(httpPort)}};
            }

            // for KDE:
            if (isKDE)
            {
                actions << ProcessArgument{"kwriteconfig5",
                                           {"--file", configPath + "/kioslaverc", //
                                            "--group", "Proxy Settings",          //
                                            "--key", protocol + "Proxy",          //
                                            "http://" + proxyServer + " " + QString::number(httpPort)}};
            }
        }
    }

    // Configure SOCKS5 Proxies
    // if (hasSOCKS)
    {
        // for GNOME:
        {
            actions << ProcessArgument{"gsettings", {"set", "org.gnome.system.proxy.socks", "host", proxyServer}};
            actions << ProcessArgument{"gsettings",
                                       {"set", "org.gnome.system.proxy.socks", "port", QString::number(socksPort)}};

            // for KDE:
            if (isKDE)
            {
                actions << ProcessArgument{"kwriteconfig5",
                                           {"--file", configPath + "/kioslaverc", //
                                            "--group", "Proxy Settings",          //
                                            "--key", "socksProxy",                //
                                            "socks://" + proxyServer + " " + QString::number(socksPort)}};
            }
        }
    }
    // Setting Proxy Mode to Manual
    {
        // for GNOME:
        {
            actions << ProcessArgument{"gsettings", {"set", "org.gnome.system.proxy", "mode", "manual"}};
            QStringList bypassList = bypass.split(";");
            QString ignoreHosts = "[\"" + bypassList.join("\",\"") + "\"]";
            actions << ProcessArgument{"gsettings", {"set", "org.gnome.system.proxy", "ignore-hosts", ignoreHosts}};
        }

        // for KDE:
        if (isKDE)
        {
            actions << ProcessArgument{"kwriteconfig5",
                                       {"--file", configPath + "/kioslaverc", //
                                        "--group", "Proxy Settings",          //
                                        "--key", "ProxyType", "1"}};
            actions << ProcessArgument{"kwriteconfig5",
                                       {"--file", configPath + "/kioslaverc", //
                                        "--group", "Proxy Settings",          //
                                        "--key", "NoProxyFor", bypass}};
        }
    }

    // Notify kioslaves to reload system proxy configuration.
    if (isKDE)
    {
        actions << ProcessArgument{"dbus-send",
                                   {"--type=signal", "/KIO/Scheduler",                 //
                                    "org.kde.KIO.Scheduler.reparseSlaveConfiguration", //
                                    "string:''"}};
    }
    // Execute them all!
    //
    // note: do not use std::all_of / any_of / none_of,
    // because those are short-circuit and cannot guarantee atomicity.
    QList<bool> results;
    for (const auto &action : actions)
    {
        // execute and get the code
        const auto returnCode = QProcess::execute(action.first, action.second);
        // print out the commands and result codes
        qDebug() << QStringLiteral("[%1] Program: %2, Args: %3").arg(returnCode).arg(action.first).arg(action.second.join(";"));
        // give the code back
        results << (returnCode == QProcess::NormalExit);
    }

    if (results.count(true) != actions.size())
    {
        QMessageBox::critical(nullptr, "设置系统代理失败", "存在失败的命令");
    }
}

void linuxClearSystemProxy()
{
    QList<ProcessArgument> actions;
    const bool isKDE = qEnvironmentVariable("XDG_SESSION_DESKTOP") == "KDE" ||
                       qEnvironmentVariable("XDG_SESSION_DESKTOP") == "plasma";
    const auto configRoot = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);

    // Setting System Proxy Mode to: None
    {
        // for GNOME:
        {
            actions << ProcessArgument{"gsettings", {"set", "org.gnome.system.proxy", "mode", "none"}};
        }

        // for KDE:
        if (isKDE)
        {
            actions << ProcessArgument{"kwriteconfig5",
                                       {"--file", configRoot + "/kioslaverc", //
                                        "--group", "Proxy Settings",          //
                                        "--key", "ProxyType", "0"}};
        }
    }

    // Notify kioslaves to reload system proxy configuration.
    if (isKDE)
    {
        actions << ProcessArgument{"dbus-send",
                                   {"--type=signal", "/KIO/Scheduler",                 //
                                    "org.kde.KIO.Scheduler.reparseSlaveConfiguration", //
                                    "string:''"}};
    }

    // Execute the Actions
    for (const auto &action : actions)
    {
        // execute and get the code
        const auto returnCode = QProcess::execute(action.first, action.second);
        // print out the commands and result codes
        qDebug() << QStringLiteral("[%1] Program: %2, Args: %3").arg(returnCode).arg(action.first).arg(action.second.join(";"));
    }
}

bool linuxIsSystemProxySet()
{
    QProcess process;
    process.start("gsettings", {"get", "org.gnome.system.proxy", "mode"});
    process.waitForFinished();
    return process.readAllStandardOutput().contains("manual");
}

bool Utils::isSystemProxySet()
{
#if defined(Q_OS_WINDOWS)
    QSettings proxySettings(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Internet Settings)",
                            QSettings::NativeFormat);
    return proxySettings.value("ProxyEnable", 0).toInt() == 1;
#elif defined(Q_OS_MACOS)
    QStringList activeServices = macOSGetActiveNetworkServices();
    for (const QString &service : activeServices)
    {
        if (macOSIsSystemProxySet(macOSProxyType::WebProxy, service))
            return true;
        if (macOSIsSystemProxySet(macOSProxyType::SecureWebProxy, service))
            return true;
        if (macOSIsSystemProxySet(macOSProxyType::SOCKSFirewallProxy, service))
            return true;
    }
    return false;
#elif defined(Q_OS_LINUX)
    return linuxIsSystemProxySet();
#endif
}

void Utils::setSystemProxy(int http_port, int socks_port, const QString &bypass)
{
#if defined(Q_OS_WINDOWS)
    windowsSetProxyForAllConnections(
        "127.0.0.1:" + QString::number(http_port),
        bypass);
#elif defined(Q_OS_MACOS)
    QStringList activeServices = macOSGetActiveNetworkServices();
    for (const QString &service : activeServices)
    {
        macOSSetSystemProxy(macOSProxyType::WebProxy, service, "127.0.0.1", http_port);
        macOSSetSystemProxy(macOSProxyType::SecureWebProxy, service, "127.0.0.1", http_port);
        macOSSetSystemProxy(macOSProxyType::SOCKSFirewallProxy, service, "127.0.0.1", socks_port);
        // macOSDisableSystemProxy(macOSProxyType::SOCKSFirewallProxy, service);
        macOSSetProxyBypass(service, bypass);
    }
#elif defined(Q_OS_LINUX)
    linuxSetSystemProxy("127.0.0.1", http_port, socks_port, bypass);
#endif
}

void Utils::clearSystemProxy()
{
#if defined(Q_OS_WINDOWS)
    windowsSetProxyForAllConnections("", "");
#elif defined(Q_OS_MACOS)
    QStringList activeServices = macOSGetActiveNetworkServices();
    for (const QString &service : activeServices)
    {
        macOSDisableSystemProxy(macOSProxyType::WebProxy, service);
        macOSDisableSystemProxy(macOSProxyType::SecureWebProxy, service);
        macOSDisableSystemProxy(macOSProxyType::SOCKSFirewallProxy, service);
    }
#elif defined(Q_OS_LINUX)
    linuxClearSystemProxy();
#endif
}

#endif // PROXYSETTINGS_H
