#ifndef PORTFORWARDINGWINDOW_H
#define PORTFORWARDINGWINDOW_H

#include <QDialog>
#include "ui_portforwardingwindow.h"

namespace Ui
{
    class PortForwardingWindow;
}

class PortForwardingWindow : public QDialog
{
Q_OBJECT

public:
    explicit PortForwardingWindow(QDialog *parent = nullptr);

    ~PortForwardingWindow() override;

    void setPortForwarding(const QString &tcpPortForwarding, const QString &udpPortForwarding);

signals:
    void applied(const QString &tcpPortForwarding, const QString &udpPortForwarding);

private:
    Ui::PortForwardingWindow *ui;
};

#endif //PORTFORWARDINGWINDOW_H
