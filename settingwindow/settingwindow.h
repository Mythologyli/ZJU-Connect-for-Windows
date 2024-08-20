#ifndef SETTINGWINDOW_H
#define SETTINGWINDOW_H

#include <QDialog>
#include <QSettings>
#include "ui_settingwindow.h"
#include "../portforwardingwindow/portforwardingwindow.h"

namespace Ui
{
    class SettingWindow;
}

class SettingWindow : public QDialog
{
Q_OBJECT

public:
    explicit SettingWindow(QWidget *parent = nullptr, QSettings *settings = nullptr);

    ~SettingWindow() override;

private:
    void loadSettings();

    Ui::SettingWindow *ui;

    QSettings *settings;

    PortForwardingWindow *portForwardingWindow;

    QString tcpPortForwarding;
    QString udpPortForwarding;
};

#endif //SETTINGWINDOW_H
