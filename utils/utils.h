#ifndef UTILS_H
#define UTILS_H

#include "../networkdetector/networkdetector.h"

namespace Utils
{
    void setWidgetFixedWhenHidden(QWidget *widget);

    void showAboutMessageBox(QWidget *parent = nullptr);

    void setSystemProxy(int port);

    void clearSystemProxy();

    QString getIpv4Address(const QString &interfaceName);
}

#endif //UTILS_H
