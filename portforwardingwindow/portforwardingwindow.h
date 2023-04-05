#ifndef PORTFORWARDINGWINDOW_H
#define PORTFORWARDINGWINDOW_H

#include <QWidget>
#include "ui_portforwardingwindow.h"

namespace Ui
{
    class PortForwardingWindow;
}

class PortForwardingWindow : public QWidget
{
Q_OBJECT

public:
    explicit PortForwardingWindow(QWidget *parent = nullptr);

    ~PortForwardingWindow() override;

    void setPortForwarding(const QString &tcpPortForwarding, const QString &udpPortForwarding);

signals:
    void applied(const QString &tcpPortForwarding, const QString &udpPortForwarding);

private:
    Ui::PortForwardingWindow *ui;
};

#endif //PORTFORWARDINGWINDOW_H
