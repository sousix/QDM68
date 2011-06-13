/*
* Copyright (C) 2011 Stephane 'sOuSiX' C.
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public License
* along with this library; see the file COPYING.LIB. If not, write to
* the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
*/

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
