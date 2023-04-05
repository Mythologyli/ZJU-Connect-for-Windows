#include <QIcon>

#include "zjurulewindow.h"

ZjuRuleWindow::ZjuRuleWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ZjuRuleWindow)
{
    ui->setupUi(this);

    setWindowIcon(QIcon(QPixmap(":/resource/icon.png").scaled(
        512, 512, Qt::KeepAspectRatio, Qt::SmoothTransformation
    )));

    setWindowFlag(Qt::Window);
    setWindowModality(Qt::WindowModal);
    setAttribute(Qt::WA_DeleteOnClose);
}

ZjuRuleWindow::~ZjuRuleWindow()
{
    delete ui;
}

void ZjuRuleWindow::setSocks5Port(const QString &port)
{
    QString socks5Url = QString("tg://socks?server=127.0.0.1&port=") +
                        port +
                        "&remarks=ZJU Connect";

    ui->socks5LineEdit->setText(socks5Url);
}
