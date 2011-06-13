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

#ifndef SQLTABLEMODELCHECKABLE_H
#define SQLTABLEMODELCHECKABLE_H

#include <QtSql>

/*
 Subclass QSqlTableModel to support checkbox data (using QSet<QPersistentModelIndex>)
 */

class SqlTableModelCheckable : public QSqlTableModel
{

public:
    SqlTableModelCheckable( int primaryKeyColumn, QObject * parent = 0, QSqlDatabase db = QSqlDatabase() );

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole);
    bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );

    bool hasBoxChecked();
    void clearBox();
    int countBoxChecked();
    QString filter();

protected:
    int primaryKey( const int row ) const;

private:
    int m_primaryKeyColumn;
    QSet<int> m_idList; // List of checked box. Empty at startup.
    QFont m_fontBold;
    QFont m_fontNormal;

};

#endif // SQLTABLEMODELCHECKABLE_H
