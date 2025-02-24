#include "extrasettingwindow.h"
#include "ui_extrasettingwindow.h"

ExtraSettingWindow::ExtraSettingWindow(QDialog *parent) :
    QDialog(parent),
    ui(new Ui::ExtraSettingWindow)
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

                QStringList portForwardingList = ui->forwardTextEdit->toPlainText().split("\n");

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

				QStringList customDnsList;

				for (const auto& hosts : ui->hostsTextEdit->toPlainText().split("\n"))
				{
					QStringList hostsItem = hosts.split(" ");
					if (hostsItem.size() == 2)
					{
						customDnsList.append(
							hostsItem[1].simplified() + ":" + hostsItem[0].simplified()
						);
					}
				}

				QStringList customProxyDomainsList = ui->proxyTextEdit->toPlainText().split("\n");

                emit applied(
                    tcpPortForwardingList.join(","),
                    udpPortForwardingList.join(","),
                    customDnsList.join(","),
                    customProxyDomainsList.join(","),
                    ui->extraArgTextEdit->toPlainText().replace("\n", " ")
                );
            });

}

ExtraSettingWindow::~ExtraSettingWindow()
{
    delete ui;
}

void ExtraSettingWindow::setup(const QString& tcpPortForwarding, const QString& udpPortForwarding, const QString& customDns, const QString& customProxyDomains, const QString& extraArguments)
{
    QStringList tcpPortForwardingList = tcpPortForwarding.split(",");

    for (const auto &forwarding: tcpPortForwardingList)
    {
        QStringList tcpPortForwardingItem = forwarding.split("-");

        if (tcpPortForwardingItem.size() == 2)
        {
            ui->forwardTextEdit->append(
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
            ui->forwardTextEdit->append(
                "UDP," + udpPortForwardingItem[0].simplified() + "," + udpPortForwardingItem[1].simplified()
            );
        }
    }

    QStringList customDnsList = customDns.split(",");

	for (const auto& hosts : customDnsList)
	{
		QStringList hostsItem = hosts.split(":");

		if (hostsItem.size() == 2)
		{
			ui->hostsTextEdit->append(
				hostsItem[1].simplified() + " " + hostsItem[0].simplified()
			);
		}
	}

	QStringList customProxyDomainsList = customProxyDomains.split(",");

	for (const auto& domain : customProxyDomainsList)
	{
		ui->proxyTextEdit->append(domain.trimmed());
	}

	ui->extraArgTextEdit->setPlainText(extraArguments);
}
