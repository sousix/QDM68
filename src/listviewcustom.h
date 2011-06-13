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

#ifndef LISTVIEWCUSTOM_H
#define LISTVIEWCUSTOM_H

#include <QListView>

/*
 Subclass QListView to support event on selection change.
 */

class ListViewCustom : public QListView
{
    Q_OBJECT
public:
    explicit ListViewCustom(QWidget *parent = 0);

signals:
    void selectionChanged( const QModelIndex & current );

protected slots:
    void currentChanged( const QModelIndex & current, const QModelIndex & previous );

};

#endif // LISTVIEWCUSTOM_H
