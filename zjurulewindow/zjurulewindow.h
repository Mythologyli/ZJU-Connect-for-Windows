#ifndef ZJURULEWINDOW_H
#define ZJURULEWINDOW_H

#include <QWidget>
#include "ui_zjurulewindow.h"

namespace Ui
{
    class ZjuruleWindow;
}

class ZjuRuleWindow : public QWidget
{
Q_OBJECT

public:
    explicit ZjuRuleWindow(QWidget *parent = nullptr);

    ~ZjuRuleWindow() override;

    void setSocks5Port(const QString &port);

private:
    Ui::ZjuRuleWindow *ui;
};

#endif //ZJURULEWINDOW_H
