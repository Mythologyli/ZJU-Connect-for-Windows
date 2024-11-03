#include <QApplication>

#include "SingleApplication"

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    SingleApplication app(argc, argv);
    QApplication::setApplicationName("ZJU Connect for Windows");
    QApplication::setApplicationVersion("1.1.3");

    QApplication::setFont(QFont("Microsoft YaHei UI", 9));

    MainWindow mainWindow;

    QObject::connect(&app, &SingleApplication::aboutToQuit, &mainWindow, &MainWindow::cleanUpWhenQuit);

    mainWindow.show();

    return QApplication::exec();
}
