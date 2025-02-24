#ifndef EXTRASETTINGWINDOW_H
#define EXTRASETTINGWINDOW_H

#include <QDialog>
#include "ui_extrasettingwindow.h"

namespace Ui
{
    class ExtraSettingWindow;
}

class ExtraSettingWindow : public QDialog
{
Q_OBJECT

public:
    explicit ExtraSettingWindow(QDialog *parent = nullptr);

    ~ExtraSettingWindow() override;

    void setup(const QString &tcpPortForwarding, const QString &udpPortForwarding, const QString &customDns, const QString &customProxyDomains, const QString &extraArguments);

signals:
    void applied(const QString &tcpPortForwarding, const QString &udpPortForwarding, const QString& customDns, const QString& customProxyDomains, const QString& extraArguments);

private:
    Ui::ExtraSettingWindow *ui;
};

#endif //EXTRASETTINGWINDOW_H
