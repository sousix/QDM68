#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QDesktopWidget desktop;

    QRect surface = desktop.screenGeometry();
    int x = surface.width()/2 - w.width()/2;
    int y = surface.height()/2 - w.height()/2;
    w.move(x,y);
    w.show();

    return a.exec();
}
