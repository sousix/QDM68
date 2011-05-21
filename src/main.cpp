#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Translation
    QString locale = QLocale::system().name().section( '_', 0, 0 );
    QTranslator translator;
    translator.load( "translations/qdm68_" + locale );
    app.installTranslator( &translator );

    MainWindow w;
    QDesktopWidget desktop;

    QRect surface = desktop.screenGeometry();
    int x = surface.width()/2 - w.width()/2;
    int y = surface.height()/2 - w.height()/2;
    w.move( x, y );
    w.show();

    return app.exec();
}
