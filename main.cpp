#include <QApplication>

#include "SingleApplication"

#include "mainwindow.h"

#ifndef PROJ_VER
#define PROJ_VER "unknown"
#endif

int main(int argc, char *argv[])
{
    SingleApplication app(argc, argv);
    QApplication::setApplicationName("HITsz Connect for Windows");
    QApplication::setApplicationDisplayName("HITsz Connect for Windows");
    QApplication::setApplicationVersion(PROJ_VER);

    // 愚人节彩蛋
    QDate currentDate = QDate::currentDate();
    if (currentDate.month() == 4 && currentDate.day() == 1) {
        QApplication::setApplicationDisplayName("Connect Strike: HashenU for ウィンドウズ");
    }

    MainWindow mainWindow;

    QObject::connect(&app, &SingleApplication::aboutToQuit, &mainWindow, &MainWindow::cleanUpWhenQuit);

    mainWindow.show();

    return QApplication::exec();
}
