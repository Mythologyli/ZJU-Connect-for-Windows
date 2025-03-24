#include <QCommandLineParser>

#include "SingleApplication"

#include "mainwindow.h"

int main(int argc, char* argv[])
{
    SingleApplication app(argc, argv);
    QApplication::setApplicationName("ZJU Connect for Windows");
    QApplication::setApplicationVersion("1.1.3");

    QApplication::setFont(QFont("Microsoft YaHei UI", 9));

    QCommandLineParser parser;
    parser.setApplicationDescription("ZJU Connect for Windows");
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption slientOption("s", "Hide the main window");
    parser.addOption(slientOption);
    parser.process(app);

    MainWindow mainWindow;

    QObject::connect(&app, &SingleApplication::aboutToQuit, &mainWindow, &MainWindow::cleanUpWhenQuit);

    if (!parser.isSet(slientOption))
    {
        mainWindow.show();
    }

    return QApplication::exec();
}
