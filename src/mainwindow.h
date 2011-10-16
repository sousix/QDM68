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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QtSql>
#include "sqltablemodelcheckable.h"
#include "settingsdialog.h"
#include "statisticsdialog.h"
#include "threadparser.h"
#include "listviewcustom.h"

#define SOFTWARE_VERSION "1.0 beta3"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow( QWidget *parent = 0 );
    ~MainWindow();
    static QString HtmlPlayerName( QString );

protected:
    void buildRulesList();
    void buildSelection( QSqlQuery * );
    bool createDatabase();
    void createDemosList();
    void updateSelectionInfos();
    void parseAndSaveGameState( QString, QModelIndex, QModelIndex );
    void displayDemosInfos( int );
    void loadSettings();
    void saveSettings();
    QModelIndex currentDemoIndex();
    void emptyDemoInfos();
    void fetchMoreDemos();

protected slots:
    void invertSelection();
    void selectWorst();
    void selectFastest();
    void unselectAll();
    void onBoxChecked( const QModelIndex &, const QModelIndex & );
    void openDemosDialog();
    void openSettingsDialog();
    void openStatisticsDialog();
    void playDemo();
    void parseAllDemo();
    void processDemo( const QModelIndex & );
    void onDetailsClicked();
    void onDemoParsed( int, QString, int, int );
    void onAboutClicked();
    void onThreadParserFinished();
    void copyDemosTo();
    void moveDemosTo();
    void deleteDemos();
    void onSearchDemo( QString );
    void onPopupMenu( const QPoint & );

private:
    Ui::MainWindow * ui;

    QdmSettings m_settingsData;
    QSettings * m_settingsDevice;
    QDir m_demosDir;
    SqlTableModelCheckable * m_demoModel;
    SettingsDialog * m_settingsDialog;
    StatisticsDialog * m_statisticsDialog;
    QStandardItemModel * m_varModel;
    bool m_selectInProgress;
    QLabel * m_textProgressBar;
    QMenu * m_popupMenu;

    ThreadParser * m_thread;
};

#endif // MAINWINDOW_H
