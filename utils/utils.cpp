#include <QMessageBox>
#include <QApplication>
#include <QSettings>
#include <QNetworkInterface>

#include "windows.h"
#include "wininet.h"
#include "ras.h"
#include "raserror.h"

#include "utils.h"

void Utils::setWidgetFixedWhenHidden(QWidget *widget)
{
    QSizePolicy originPolicy = widget->sizePolicy();
    originPolicy.setRetainSizeWhenHidden(true);
    widget->setSizePolicy(originPolicy);
}

void Utils::showAboutMessageBox(QWidget *parent)
{
    QMessageBox messageBox(parent);
    messageBox.setWindowTitle("关于本软件");
    messageBox.setTextFormat(Qt::RichText);
    QString aboutText = QApplication::applicationName() + " v" + QApplication::applicationVersion() +
                        "<br>基于 Qt 编写的 ZJUConnect 图形界面" +
                        "<br>作者: <a href='https://myth.cx'>Myth</a>" +
                        "<br>项目主页: <a href='https://github.com/Mythologyli/ZJU-Connect-for-Windows'>https://github.com/Mythologyli/ZJU-Connect-for-Windows</a>" +
                        "<br><br>zju-connect" +
                        "<br>ZJU RVPN 客户端的 Go 语言实现，基于 EasierConnect" +
                        "<br>作者: <a href='https://myth.cx'>Myth</a>" +
                        "<br>项目主页: <a href='https://github.com/Mythologyli/zju-connect'>https://github.com/Mythologyli/zju-connect</a>" +
                        "<br><br>EasierConnect" +
                        "<br>EasyConnect 客户端的开源实现" +
                        "<br>作者: <a href='https://github.com/lyc8503'>lyc8503</a>" +
                        "<br>项目主页: <a href='https://github.com/lyc8503/EasierConnect'>https://github.com/lyc8503/EasierConnect</a>"
                        "<br><br>zju-web-login" +
                        "<br>ZJU 网页认证登录脚本" +
                        "<br>作者: <a href='https://azuk.top'>Azuk 443</a>" +
                        "<br>项目主页: <a href='https://github.com/Mythologyli/zju-web-login'>https://github.com/Mythologyli/zju-web-login</a>";
    messageBox.setText(aboutText);
    messageBox.setIconPixmap(QPixmap(":/resource/icon.png").scaled(
        100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation
    ));
    messageBox.exec();
}

void Utils::SetProxyForAllConnections(const QString &proxyServer, const QString &bypass)
{
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
    auto *proxyServerWStr = (wchar_t *) calloc(sizeof(wchar_t), proxyServer.length() + 1);
    proxyServer.toWCharArray(proxyServerWStr);
    optionsArr[0].Value.pszValue = proxyServerWStr;

    optionsArr[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
    auto *bypassWStr = (wchar_t *) calloc(sizeof(wchar_t), bypass.length() + 1);
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
        lpRasEntryName = (LPRASENTRYNAME) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwCb);
        if (lpRasEntryName == nullptr)
        {
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
        return;
    }

    free(proxyServerWStr);
    free(bypassWStr);
}

void Utils::setSystemProxy(int port)
{
    SetProxyForAllConnections(
        "127.0.0.1:" + QString::number(port),
        "localhost;127.*;172.16.*;172.17.*;172.18.*;172.19.*;172.20.*;172.21.*;172.22.*;172.23.*;172.24.*;172.25.*;172.26.*;172.27.*;172.28.*;172.29.*;172.30.*;172.31.*;<local>"
    );
}

void Utils::clearSystemProxy()
{
    SetProxyForAllConnections("", "");
}

QString Utils::getIpv4Address(const QString &interfaceName)
{
    auto interfaces = QNetworkInterface::allInterfaces();
    for (auto &singleInterface: interfaces)
    {
        if (singleInterface.humanReadableName() == interfaceName)
        {
            auto addresses =
                singleInterface.addressEntries();
            for (auto &address: addresses)
            {
                if (address.ip().protocol() == QAbstractSocket::IPv4Protocol)
                {
                    return address.ip().toString();
                }
            }
        }
    }

    return "";
}
