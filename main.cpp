#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("ZJU Connect for Windows");
    QApplication::setApplicationVersion("0.1");

    MainWindow mainWindow;
    mainWindow.show();

    return QApplication::exec();
}
