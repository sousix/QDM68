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

#ifndef THREADPARSER_H
#define THREADPARSER_H

#include <QThread>
#include <QList>
#include <QString>
#include <QPair>
#include <QMutex>

class ThreadParser : public QThread
{
    Q_OBJECT

public:
    explicit ThreadParser(QObject *parent = 0);
    void addDemos( QList< QPair<int, QString> > * );
    void addPriorityDemo( QPair<int, QString> demo );
    void run();

signals:
    void demoParsed( int demoId, QString gameState, int current, int size );

private:
    QList< QPair<int, QString> > m_demosList;
    QPair<int, QString> m_priorityDemo;
    QMutex m_mutex;
    int m_size;
};

#endif // THREADPARSER_H
