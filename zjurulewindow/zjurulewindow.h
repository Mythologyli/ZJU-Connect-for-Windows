#ifndef ZJURULEWINDOWS_H
#define ZJURULEWINDOWS_H

#include <QWidget>

namespace Ui
{
    class ZjuruleWindow;
}

class ZjuruleWindow : public QWidget
{
Q_OBJECT

public:
    explicit ZjuruleWindow(QWidget *parent = nullptr);

    ~ZjuruleWindow() override;

    void setSocks5Port(const QString &port);

private:
    Ui::ZjuruleWindow *ui;
};

#endif //ZJURULEWINDOWS_H
