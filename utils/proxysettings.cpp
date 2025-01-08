#ifndef PROXYSETTINGS_H
#define PROXYSETTINGS_H

#include "utils.h"

#if defined(Q_OS_WINDOWS)
#include "windows.h"
#include "wininet.h"
#include "ras.h"
#include "raserror.h"
#endif

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

bool Utils::isSystemProxySet()
{
#if defined(Q_OS_WINDOWS)
    QSettings proxySettings(R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Internet Settings)",
                            QSettings::NativeFormat);
    return proxySettings.value("ProxyEnable", 0).toInt() == 1;
#elif defined(Q_OS_MACOS)
    return false;
#elif defined(Q_OS_LINUX)
    return false;
#endif
}

void Utils::setSystemProxy(int port)
{
    windowsSetProxyForAllConnections("127.0.0.1:" + QString::number(port),
                              "localhost;127.*;172.16.*;172.17.*;172.18.*;172.19.*;172.20.*;172.21.*;172.22.*;172.23.*;"
                              "172.24.*;172.25.*;172.26.*;172.27.*;172.28.*;172.29.*;172.30.*;172.31.*;<local>");
}

void Utils::clearSystemProxy()
{
    windowsSetProxyForAllConnections("", "");
}

#endif // PROXYSETTINGS_H
