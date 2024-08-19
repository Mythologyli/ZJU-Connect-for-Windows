#include <QApplication>

#include "SingleApplication"

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    SingleApplication app(argc, argv);
    QApplication::setApplicationName("HITszConnectForWindows");
    QApplication::setApplicationDisplayName("HITsz Connect for Windows");
    QApplication::setApplicationVersion("1.1.6");

    // Easter Egg
    QDate currentDate = QDate::currentDate();
    if (currentDate.month() == 4 && currentDate.day() == 1) {
        QApplication::setApplicationDisplayName("Connect Strike: HashenU");
    }

    MainWindow mainWindow;

    QObject::connect(&app, &SingleApplication::aboutToQuit, &mainWindow, &MainWindow::cleanUpWhenQuit);

    mainWindow.show();

    return QApplication::exec();
}
