#include "portforwardingwindow.h"
#include "ui_portforwardingwindow.h"

PortForwardingWindow::PortForwardingWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PortForwardingWindow)
{
    ui->setupUi(this);

    setWindowIcon(QIcon(QPixmap(":/resource/icon.png").scaled(
        512, 512, Qt::KeepAspectRatio, Qt::SmoothTransformation
    )));

    setWindowFlag(Qt::Window);
    setWindowModality(Qt::WindowModal);
    setAttribute(Qt::WA_DeleteOnClose);

    connect(ui->applyPushButton, &QPushButton::clicked,
            [&]()
            {
                QStringList tcpPortForwardingList;
                QStringList udpPortForwardingList;

                QStringList portForwardingList = ui->textEdit->toPlainText().split("\n");

                for (int i = 0; i < portForwardingList.size(); i++)
                {
                    QStringList portForwardingItem = portForwardingList[i].split(",");

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

    connect(ui->cancelPushButton, &QPushButton::clicked,
            [&]()
            {
                close();
            });
}

PortForwardingWindow::~PortForwardingWindow()
{
    delete ui;
}

void PortForwardingWindow::setPortForwarding(const QString &tcpPortForwarding, const QString &udpPortForwarding)
{
    QStringList tcpPortForwardingList = tcpPortForwarding.split(",");

    for (int i = 0; i < tcpPortForwardingList.size(); i++)
    {
        QStringList tcpPortForwardingItem = tcpPortForwardingList[i].split("-");

        if (tcpPortForwardingItem.size() == 2)
        {
            ui->textEdit->append(
                "TCP," + tcpPortForwardingItem[0].simplified() + "," + tcpPortForwardingItem[1].simplified() + "\n"
            );
        }
    }

    QStringList udpPortForwardingList = udpPortForwarding.split(",");

    for (int i = 0; i < udpPortForwardingList.size(); i++)
    {
        QStringList udpPortForwardingItem = udpPortForwardingList[i].split("-");

        if (udpPortForwardingItem.size() == 2)
        {
            ui->textEdit->append(
                "UDP," + udpPortForwardingItem[0].simplified() + "," + udpPortForwardingItem[1].simplified() + "\n"
            );
        }
    }
}
