#include <QApplication>

#include "SingleApplication"

#include "mainwindow.h"
#include "utils/utils.h"

#ifndef PROJ_VER
#define PROJ_VER "unknown"
#endif

int main(int argc, char *argv[])
{
    SingleApplication app(argc, argv);
    QApplication::setApplicationName(Utils::APP_NAME);
    QApplication::setApplicationDisplayName(Utils::APP_NAME);
    QApplication::setApplicationVersion(PROJ_VER);

    QTranslator qtTranslator;
    if (qtTranslator.load(QLocale(QLocale::Chinese, QLocale::SimplifiedChineseScript, QLocale::China),
                          QString("qtbase"), QString("_"), QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
        app.installTranslator(&qtTranslator);
    else
        qDebug() << "Failed to load qtbase_zh_CN.qm";

    MainWindow mainWindow;

    QObject::connect(&app, &SingleApplication::aboutToQuit, &mainWindow, &MainWindow::cleanUpWhenQuit);

    mainWindow.show();

    return QApplication::exec();
}
