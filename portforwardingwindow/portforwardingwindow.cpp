#include "portforwardingwindow.h"
#include "ui_portforwardingwindow.h"

PortForwardingWindow::PortForwardingWindow(QDialog *parent) :
    QDialog(parent),
    ui(new Ui::PortForwardingWindow)
{
    ui->setupUi(this);

    setWindowIcon(QIcon(QPixmap(":/resource/icon.png").scaled(
        512, 512, Qt::KeepAspectRatio, Qt::SmoothTransformation
    )));

    setWindowModality(Qt::WindowModal);
    setAttribute(Qt::WA_DeleteOnClose);

    connect(ui->buttonBox, &QDialogButtonBox::accepted,
            [&]()
            {
                QStringList tcpPortForwardingList;
                QStringList udpPortForwardingList;

                QStringList portForwardingList = ui->textEdit->toPlainText().split("\n");

                for (const auto &forwarding: portForwardingList)
                {
                    QStringList portForwardingItem = forwarding.split(",");

                    if (portForwardingItem.size() == 3)
                    {
                        if (portForwardingItem[0].simplified() == "TCP")
                        {
                            tcpPortForwardingList.append(
                                portForwardingItem[1].simplified() + "-" + portForwardingItem[2].simplified()
                            );
                        }
                        else if (portForwardingItem[0].simplified() == "UDP")
                        {
                            udpPortForwardingList.append(
                                portForwardingItem[1].simplified() + "-" + portForwardingItem[2].simplified()
                            );
                        }
                    }
                }

                emit applied(tcpPortForwardingList.join(","), udpPortForwardingList.join(","));
            });

}

PortForwardingWindow::~PortForwardingWindow()
{
    delete ui;
}

void PortForwardingWindow::setPortForwarding(const QString &tcpPortForwarding, const QString &udpPortForwarding)
{
    QStringList tcpPortForwardingList = tcpPortForwarding.split(",");

    for (const auto &forwarding: tcpPortForwardingList)
    {
        QStringList tcpPortForwardingItem = forwarding.split("-");

        if (tcpPortForwardingItem.size() == 2)
        {
            ui->textEdit->append(
                "TCP," + tcpPortForwardingItem[0].simplified() + "," + tcpPortForwardingItem[1].simplified()
            );
        }
    }

    QStringList udpPortForwardingList = udpPortForwarding.split(",");

    for (const auto &forwarding: udpPortForwardingList)
    {
        QStringList udpPortForwardingItem = forwarding.split("-");

        if (udpPortForwardingItem.size() == 2)
        {
            ui->textEdit->append(
                "UDP," + udpPortForwardingItem[0].simplified() + "," + udpPortForwardingItem[1].simplified()
            );
        }
    }
}
