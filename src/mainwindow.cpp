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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <time.h>

///////////////////////////////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_settingsData()
{
    ui->setupUi( this );
    m_settingsDialog = NULL;
    m_statisticsDialog = NULL;
    m_selectInProgress = false;
    m_thread = new ThreadParser();

    createDatabase();

    m_popupMenu = new QMenu(ui->listView);
    m_popupMenu->addAction( QIcon(":/image/play"), tr("Play"), this, SLOT(playDemo()) );

    m_demoModel = new SqlTableModelCheckable( 0, this );
    m_demoModel->setTable( "demos" );
    m_demoModel->setEditStrategy( QSqlTableModel::OnManualSubmit );

    ui->listView->setModel( m_demoModel );
    ui->listView->setModelColumn(1);
    ui->listView->setContextMenuPolicy( Qt::CustomContextMenu );

    m_varModel = new QStandardItemModel(this);
    ui->varTableView->setModel(m_varModel);

    connect( ui->actionInvert, SIGNAL(triggered()), this, SLOT(invertSelection()) );
    connect( ui->actionSlowest, SIGNAL(triggered()), this, SLOT(selectWorst()) );
    connect( ui->actionClean, SIGNAL(triggered()), this, SLOT(unselectAll()) );
    connect( ui->actionBest, SIGNAL(triggered()), this, SLOT(selectFastest()) );
    connect( ui->btnMoveTo, SIGNAL(released()), this, SLOT(moveDemosTo()) );
    connect( ui->btnCopyTo, SIGNAL(released()), this, SLOT(copyDemosTo()) );
    connect( ui->btnDelete, SIGNAL(released()), this, SLOT(deleteDemos()) );
    connect( ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()) );
    connect( ui->actionDemos_folder, SIGNAL(triggered()), this, SLOT(openDemosDialog()) );
    connect( ui->actionFolder, SIGNAL(triggered()), this, SLOT(openDemosDialog()) );
    connect( ui->actionSettings, SIGNAL(triggered()), this, SLOT(openSettingsDialog()) );
    connect( ui->actionParseAll, SIGNAL(triggered()), this, SLOT(parseAllDemo()) );
    connect( ui->actionStatistics, SIGNAL(triggered()), this, SLOT(openStatisticsDialog()) );
    connect( ui->actionAbout, SIGNAL(triggered()), this, SLOT(onAboutClicked()) );
    connect( ui->actionDetails, SIGNAL(triggered()), this, SLOT(onDetailsClicked()) );
    //connect( ui->actionPlay, SIGNAL(triggered()), this, SLOT(playDemo()) );
    connect( ui->lineSearch, SIGNAL(textChanged(QString)), this, SLOT(onSearchDemo(QString)) );
    connect( ui->listView, SIGNAL(selectionChanged(const QModelIndex &)), this, SLOT(processDemo(const QModelIndex &)) );
    connect( ui->listView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(onPopupMenu(const QPoint &)) );
    connect( m_thread, SIGNAL(demoParsed(int, QString, int, int)), this, SLOT(onDemoParsed(int, QString, int, int)) );
    connect( m_thread, SIGNAL(finished()), this, SLOT(onThreadParserFinished()) );
    connect( m_demoModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
             this, SLOT(onBoxChecked(const QModelIndex &, const QModelIndex &)) );

    m_textProgressBar = new QLabel();
    ui->statusBar->addPermanentWidget( m_textProgressBar );
    m_textProgressBar->setVisible( false );
    // Parse all demos : for developper
    ui->actionParseAll->setVisible( false );

    loadSettings();
    createDemosList();
    buildRulesList();

    ui->demoInfosBox->setVisible( false );
    emptyDemoInfos();

    this->adjustSize();
    ui->listView->setFocus();

    #ifdef Q_OS_LINUX
    ui->actionFolder->setIcon(QIcon::fromTheme("folder"));
    ui->actionQuit->setIcon(QIcon::fromTheme("application-exit"));
    ui->actionDemos_folder->setIcon(QIcon::fromTheme("folder"));
    ui->actionSettings->setIcon(QIcon::fromTheme("preferences-system"));
    ui->actionAbout->setIcon(QIcon::fromTheme("help-about"));
    #endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////

MainWindow::~MainWindow()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::buildSelection( QSqlQuery * query )
{
    QModelIndexList indexList;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    m_selectInProgress = true;
    m_demoModel->clearBox();
    while( query->next() )
    {
        indexList = m_demoModel->match( m_demoModel->index(0, 0),
                                        Qt::DisplayRole,
                                        QVariant( query->value(0).toInt() ),
                                        1,
                                        Qt::MatchFlags( Qt::MatchExactly ) );

        if( indexList.count() > 0 )
        {
            m_demoModel->setData( m_demoModel->index( indexList.at(0).row(), 1 ),
                                  Qt::Checked,
                                  Qt::CheckStateRole );
        }
    }
    m_selectInProgress = false;
    updateSelectionInfos();

    QApplication::restoreOverrideCursor();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// Only display list of rules without values
void MainWindow::buildRulesList()
{
    int i;
    QMap<QString, QString>::iterator it;

    m_varModel->clear();
    m_varModel->setRowCount( m_settingsData.rules.size() );
    m_varModel->setColumnCount(2);

    it = m_settingsData.rules.begin();
    i = 0;
    while( it != m_settingsData.rules.end() )
    {
        // Insert key
        QStandardItem *item = new QStandardItem( it.key() );
        item->setEditable(false);
        m_varModel->setItem( i, 0, item );

        // Insert empty value
        item = new QStandardItem("");
        item->setEditable(false);
        m_varModel->setItem( i, 1, item );

        i++;
        it++;
    }

    ui->varTableView->resizeColumnsToContents();
    m_varModel->sort(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::invertSelection()
{
    m_selectInProgress = true;
    for( int i=0 ; i<m_demoModel->rowCount() ; i++ )
    {
        if( m_demoModel->data( m_demoModel->index(i, 1), Qt::CheckStateRole ) == Qt::Unchecked )
        {
            m_demoModel->setData( m_demoModel->index(i, 1),
                                  Qt::Checked,
                                  Qt::CheckStateRole );
        }else{
            m_demoModel->setData( m_demoModel->index(i, 1),
                                  Qt::Unchecked,
                                  Qt::CheckStateRole );
        }
    }
    m_selectInProgress = false;
    updateSelectionInfos();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::selectWorst()
{
    QSqlQuery query;
    if( !m_settingsData.distinctPlayer )
    {
        query.exec( "SELECT d.id, d.map, d.physic \
                    FROM demos as d \
                    WHERE d.id NOT IN ( SELECT id FROM demos AS e \
                                        WHERE e.map = d.map \
                                        AND e.physic = d.physic \
                                        AND e.multi = d.multi \
                                        AND " +  m_demoModel->filter() + " \
                                        ORDER BY time ASC \
                                        LIMIT 0,1) \
                    AND " + m_demoModel->filter() );
    }else{
        query.exec( "SELECT d.id, d.map, d.physic, d.author \
                    FROM demos as d \
                    WHERE d.id NOT IN ( SELECT id FROM demos AS e \
                                        WHERE e.map = d.map \
                                        AND e.physic = d.physic \
                                        AND e.multi = d.multi \
                                        AND e.author = d.author \
                                        AND " +  m_demoModel->filter() + " \
                                        ORDER BY time ASC \
                                        LIMIT 0,1) \
                    AND " + m_demoModel->filter() );
    }

    buildSelection( &query );

    if( !m_demoModel->hasBoxChecked() )
        statusBar()->showMessage( tr("No slowest demos found!") );
}
///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::selectFastest()
{
    QSqlQuery query;
    if( !m_settingsData.distinctPlayer )
    {
        query.exec( "SELECT d.id, d.map, d.physic \
                    FROM demos as d \
                    WHERE d.id IN ( SELECT id FROM demos AS e \
                                    WHERE e.map = d.map \
                                    AND e.physic = d.physic \
                                    AND e.multi = d.multi \
                                    AND " +  m_demoModel->filter() + " \
                                    ORDER BY time ASC \
                                    LIMIT 0,1) \
                    AND " + m_demoModel->filter() );
    }else{
        query.exec( "SELECT d.id, d.map, d.physic, d.author \
                    FROM demos as d \
                    WHERE d.id IN ( SELECT id FROM demos AS e \
                                    WHERE e.map = d.map \
                                    AND e.physic = d.physic \
                                    AND e.multi = d.multi \
                                    AND e.author = d.author \
                                    AND " +  m_demoModel->filter() + " \
                                    ORDER BY time ASC \
                                    LIMIT 0,1) \
                    AND " + m_demoModel->filter() );
    }


    buildSelection( &query );
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::unselectAll()
{
    m_demoModel->clearBox();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::copyDemosTo()
{
    QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
    QString dirName = QFileDialog::getExistingDirectory( this, tr("Select folder"), m_demosDir.absolutePath(), options );
    QDir copyDir( dirName );

    if ( dirName.isEmpty() )
        return;

    QProgressDialog progress( tr("Copying files..."), tr("Abort Copy"), 0, m_demoModel->countBoxChecked(), this);
    progress.setWindowModality(Qt::WindowModal);
    int i, j=0;

    for ( i = 0 ; i < m_demoModel->rowCount() ; i++)
    {
        QCoreApplication::processEvents();

        if (progress.wasCanceled())
            break;

        if( m_demoModel->data( m_demoModel->index(i, 1), Qt::CheckStateRole ) == Qt::Checked )
        {
            progress.setValue(j++);
            QFile::copy( m_demosDir.absolutePath() + "/" + m_demoModel->data( m_demoModel->index(i, 1) ).toString(),
                         copyDir.absolutePath() + "/" + m_demoModel->data( m_demoModel->index(i, 1) ).toString() );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::moveDemosTo()
{
    QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
    QString dirName = QFileDialog::getExistingDirectory( this, tr("Select folder"), m_demosDir.absolutePath(), options );
    QDir moveDir( dirName );

    if ( dirName.isEmpty() )
        return;

    bool allFileMoved = true;
    bool isMoved;
    int i = 0;
    while( i < m_demoModel->rowCount() )
    {
        if( m_demoModel->data( m_demoModel->index(i, 1), Qt::CheckStateRole ) == Qt::Checked )
        {
             isMoved = QFile::rename( m_demosDir.absolutePath() + "/" + m_demoModel->data( m_demoModel->index(i, 1) ).toString(),
                                      moveDir.absolutePath() + "/" + m_demoModel->data( m_demoModel->index(i, 1) ).toString() );
             if( isMoved )
                m_demoModel->removeRows( i, 1, m_demoModel->index(i, 1).parent() );

             allFileMoved &= isMoved;
        }
        i++;
    }
    m_demoModel->submitAll();
    displayDemosInfos( currentDemoIndex().row() );

    if( !allFileMoved )
        QMessageBox::warning( this, tr("Warning"), tr("One or more files could not be moved.") );

}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::deleteDemos()
{
    QMessageBox question( QMessageBox::Question,
                            tr("Information"),
                            tr("Are you sure you want to delete %1 files ?").arg( m_demoModel->countBoxChecked() ),
                            QMessageBox::Yes | QMessageBox::Cancel );
    question.setButtonText( QMessageBox::Yes, tr("Yes") );
    question.setButtonText( QMessageBox::Cancel, tr("Cancel") );

    if ( question.exec() == QMessageBox::Cancel )
        return;

    bool isRemoved;
    bool allFileRemoved = true;
    int i = 0;
    while( i < m_demoModel->rowCount() )
    {
        if( m_demoModel->data( m_demoModel->index(i, 1), Qt::CheckStateRole ) == Qt::Checked )
        {
            isRemoved = QFile::remove( m_demosDir.absolutePath() + "/" + m_demoModel->data( m_demoModel->index(i, 1) ).toString() );
            if( isRemoved )
                m_demoModel->removeRows( i, 1, m_demoModel->index(i, 1).parent() );

            allFileRemoved &= isRemoved;
        }
        i++;
    }
    m_demoModel->submitAll();
    displayDemosInfos( currentDemoIndex().row() );

    if( !allFileRemoved )
        QMessageBox::warning( this, tr("Warning"), tr("One or more files could not be deleted.") );
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::onBoxChecked( const QModelIndex & topLeft, const QModelIndex & bottomRight )
{
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);

    // Speed up selection : Do not update UI when selecting worst demos, or when inverting selection
    if( !m_selectInProgress )
    {
        updateSelectionInfos();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::updateSelectionInfos()
{
    bool hasSelection = m_demoModel->hasBoxChecked();

    ui->btnCopyTo->setEnabled( hasSelection );
    ui->btnMoveTo->setEnabled( hasSelection );
    ui->btnDelete->setEnabled( hasSelection );

    if( !hasSelection )
        statusBar()->showMessage("");
    else
        statusBar()->showMessage(QString( tr("%1 demo(s) selected") ).arg( m_demoModel->countBoxChecked() ));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::openDemosDialog()
{
    QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
    QString dirName = QFileDialog::getExistingDirectory( this, tr("Select demos folder"), m_demosDir.absolutePath(), options );
    QDir newDir( dirName );

    if ( !dirName.isEmpty() )
    {
        ui->lineSearch->setText("");
        m_demosDir = newDir;
        createDemosList();
        saveSettings();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::onPopupMenu( const QPoint & position )
{
    Q_UNUSED(position);
    m_popupMenu->exec(QCursor::pos());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool MainWindow::createDatabase()
 {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    if ( !db.open() )
    {
        QMessageBox::critical( 0, "Cannot open database",
                               "Unable to establish a database connection.\n"
                                        "This example needs SQLite support. Please read "
                                        "the Qt SQL driver documentation for information how "
                                        "to build it.\n\n"
                                        "Click Cancel to exit.", QMessageBox::Cancel );
        return false;
    }

    QSqlQuery query;
    query.exec( "CREATE TABLE demos "
                "(id INT PRIMARY KEY, "
                "filename VARCHAR(100),"
                "map VARCHAR(50),"
                "multi VARCHAR(3),"
                "physic VARCHAR(3),"
                "time VARCHAR(9),"
                "author VARCHAR(50),"
                "country VARCHAR(50),"
                "player_infos TEXT,"
                "rules BLOB)" );

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// ([^\[.]*)\[([^\..]*)\.([^\].]*)\]([0-9]{2}\.[0-9]{2}\.[0-9]{3})\(([^\..]*)\.([^\).]*)\)\.dm_.*
// ([^\\[.]*)\\[([^\\..]*)\\.([^\\].]*)\\]([0-9]{2}\\.[0-9]{2}\\.[0-9]{3})\\(([^\\..]*)\\.([^\\).]*)\\)\\.dm_.*

void MainWindow::createDemosList()
{
    if( !m_demosDir.exists() )
        return;

    QSqlQuery query;
    QRegExp pattern( "([^\\[.]*)\\[([^\\..]*)\\.([^\\].]*)\\]([0-9]{2}\\.[0-9]{2}\\.[0-9]{3})\\(([^\\..]*)\\.([^\\).]*)\\)\\.dm_.*" );
    int i, nbDemos = 0;

    if( m_demoModel->removeRows( 0, m_demoModel->rowCount() ) )
        m_demoModel->submitAll(); // Apply changement in DB

    for ( i=0 ; i < m_demosDir.entryInfoList().size() ; i++ )
    {
        if( pattern.exactMatch( m_demosDir.entryInfoList().at(i).fileName() ) )
        {
            nbDemos++;
            pattern.indexIn( m_demosDir.entryInfoList().at(i).fileName() );
            query.exec( QString( "INSERT INTO demos "
                                 "VALUES (%1,'%2','%3','%4','%5','%6','%7','%8', NULL, NULL)").arg(i).arg(  pattern.cap(0),
                                                                                                            pattern.cap(1),
                                                                                                            pattern.cap(2),
                                                                                                            pattern.cap(3),
                                                                                                            pattern.cap(4),
                                                                                                            pattern.cap(5),
                                                                                                            pattern.cap(6) ) );
        }
    }
    m_demoModel->select();
    statusBar()->showMessage( QString(tr("%1 demo(s) found")).arg(nbDemos) );

    this->fetchMoreDemos();

    displayDemosInfos( currentDemoIndex().row() );
    this->setWindowTitle( QString("QDM68 - %1").arg( m_demosDir.absolutePath() ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::onDemoParsed( int demoId, QString gameState, int current, int size )
{
    m_textProgressBar->setText( QString( tr("Analyzing... %1/%2") ).arg(size - current).arg(size));

    const QModelIndexList indexList = m_demoModel->match( m_demoModel->index(0, 0), Qt::DisplayRole, demoId);

    if( indexList.count() > 0 && gameState != "")
    {
        parseAndSaveGameState( gameState,
                               m_demoModel->index( indexList.at(0).row(), 8),
                               m_demoModel->index( indexList.at(0).row(), 9) );

        // If the demo parsed correspond to the demo selected, we display info
        if( currentDemoIndex().row() == indexList.at(0).row() )
        {
            displayDemosInfos( indexList.at(0).row() );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::onThreadParserFinished()
{
    m_textProgressBar->setText( "" );
    m_textProgressBar->setVisible( false );
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::onDetailsClicked()
{
    ui->demoInfosBox->setVisible( !ui->demoInfosBox->isVisible()     );
    this->adjustSize();

    if( currentDemoIndex().isValid() )
        processDemo( currentDemoIndex() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::onAboutClicked()
{
    QString about =  "<h3><b>QDM68 v" SOFTWARE_VERSION "</b></h3>" +
            tr( "Author") + " : Stephane <i>`sOuSiX`</i> C.<br>" +
            tr( "Thanks to") + " : uZu (Linux package)" +
            + "<br><br>" +
            tr( "Created with" ) + " <a href=\"http://qt.nokia.com\">Qt SDK</a><br>" +
            tr( "This software use ") + " <a href=\"http://skuller-vidnoe.narod.ru/q3sdc.htm\">q3sdc</a><br>" +
            tr( "Checkout sources at" ) + " <a href=\"https://github.com/sOuSiX/QDM68\">Github/QDM68</a><br>";

    QMessageBox::about( this, tr( "About QDM68" ), about );
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::onSearchDemo( QString text )
{
    if( text != "" )
        m_demoModel->setFilter( QString(" filename LIKE '\%%1\%' ").arg(text) );
    else
        m_demoModel->setFilter( "1" );

    this->fetchMoreDemos();
    statusBar()->showMessage( QString(tr("%1 demo(s) found")).arg(m_demoModel->rowCount()) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void    MainWindow::displayDemosInfos( int row )
{
    // If invalid row, delete infos, exit function
    if( row < 0 )
    {
        emptyDemoInfos();
        return;
    }

    int i;
    const QStringList & rulesList = m_demoModel->data( m_demoModel->index( row, 8 ) ).toString().split ( "\\", QString::SkipEmptyParts );
    QString varRefName = "";
    QString varRefValue = "";
    QString varDemoValue = "";
    QMap<QString, QString> rulesMap;
    QStringList pair;

    // Set title : Map, physic, time

    /* ui->demoInfosBox->setTitle( m_demoModel->data( m_demoModel->index( row, 2 ), Qt::DisplayRole ).toString()
                                + " (" + m_demoModel->data( m_demoModel->index( row, 4 ), Qt::DisplayRole ).toString() + ") "
                                + m_demoModel->data( m_demoModel->index( row, 5 ), Qt::DisplayRole ).toString() ); */

    // Display rules

    for( i = 0 ; i < rulesList.size() ; i++ )
    {
        pair = rulesList.at(i).split ( "=", QString::SkipEmptyParts );
        rulesMap.insert( pair.at(0), pair.at(1) );
    }

    for( i = 0 ; i < m_varModel->rowCount() ; i++ )
    {
        varRefName = m_varModel->item( i , 0 )->data( Qt::DisplayRole ).toString();
        varRefValue = m_settingsData.rules.value( m_varModel->item( i , 0 )->data( Qt::DisplayRole ).toString() );
        varDemoValue = rulesMap.value( m_varModel->item( i , 0 )->data( Qt::DisplayRole ).toString() );

        if( rulesMap.contains( varRefName ) )
        {
            m_varModel->setData( m_varModel->item( i , 1 )->index(), varDemoValue );
            m_varModel->item( i , 1 )->setEnabled( true );

            if( varRefValue == varDemoValue )
                m_varModel->item( i , 1 )->setIcon( QIcon(":/image/valid") );
            else
                m_varModel->item( i , 1 )->setIcon( QIcon(":/image/warning") );

        }else{
            m_varModel->setData( m_varModel->item( i , 1 )->index(), tr("Unavailable") );
            m_varModel->item( i , 1 )->setIcon( QIcon() );
            m_varModel->item( i , 1 )->setEnabled( false );
        }
    }


    // Display player(s) infos

    ui->frameDemo->setHtml( m_demoModel->data( m_demoModel->index( row, 9 ) ).toString() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::emptyDemoInfos()
{
    ui->frameDemo->setHtml("<div width=\"100%\" height=\"100%\" align=\"center\"><br><br><br><i><font color=\"grey\">" + tr( "No demo selected" ) + "</font></i></div>");
    for( int i = 0 ; i < m_varModel->rowCount() ; i++ )
    {
        m_varModel->item( i, 1 )->setText("");
        m_varModel->item( i, 1 )->setIcon( QIcon() );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::processDemo( const QModelIndex & index )
{
    if( !ui->demoInfosBox->isVisible() || !index.isValid() )
        return;

    const QModelIndex & PlayerInfoIndex = m_demoModel->index( index.row(), 9 );

    // If demo not parsed, send it to threadParser, else display demo infos.
    if( m_demoModel->data( PlayerInfoIndex ).toString() == "" )
    {
        QPair<int, QString> pair;
        pair.first = m_demoModel->data( m_demoModel->index(index.row(), 0) ).toInt();
        pair.second = m_demosDir.absolutePath() + "/" + m_demoModel->data( m_demoModel->index( index.row(), 1 ) ).toString();
        m_thread->addPriorityDemo( pair );
    }else{
        displayDemosInfos( index.row() );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QModelIndex MainWindow::currentDemoIndex()
{
    if( ui->listView->selectionModel()->selectedIndexes().size() > 0 )
        return ui->listView->selectionModel()->selectedIndexes().at(0);

    return QModelIndex();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::playDemo()
{
    QFileInfo engineFile( m_settingsData.engineFile );
    QString appPath = QDir::currentPath();

    if( !currentDemoIndex().isValid() )
    {
        QMessageBox msgBox;
        msgBox.setText( QString( tr("No demos selected") ) );
        msgBox.setIcon( QMessageBox::Warning );
        msgBox.exec();
        return;
    }


    if( !engineFile.exists() )
    {
        QMessageBox msgBox;
        msgBox.setText( QString( tr("Quake 3 engine not found") ) );
        msgBox.setIcon( QMessageBox::Critical );
        msgBox.exec();
        return;
    }

    // replace "/home/stef/.q3a/defrag/demos/subfolder/" by "subfolder/"
    int strIndex = m_demosDir.absolutePath().indexOf("demos") + 6;
    QString demoFolder =  m_demosDir.absolutePath().replace(0, strIndex, "");
    if( demoFolder != "" )
        demoFolder += "/";

    QStringList args;
    args << "+set" << "fs_game" << "defrag" << "+demo"
         << demoFolder + currentDemoIndex().data().toString();

    QDir::setCurrent( engineFile.absolutePath() );
    statusBar()->showMessage( QString( tr("Executing '%1'") ).arg( engineFile.fileName() ) );
    QProcess::startDetached( m_settingsData.engineFile, args );
    QDir::setCurrent( appPath );
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::parseAllDemo()
{
    QList<QPair<int, QString> > demosList;
    QPair<int, QString> pair;
    for( int i=0 ; i<m_demoModel->rowCount() ; i++ )
    {
        if( m_demoModel->data( m_demoModel->index(i, 8) ).toString() == "" )
        {
            pair.first = m_demoModel->data( m_demoModel->index(i, 0) ).toInt();
            pair.second = m_demosDir.absolutePath() + "/" + m_demoModel->data( m_demoModel->index(i, 1) ).toString();
            demosList.append( pair );
        }
    }

    if( !demosList.isEmpty() )
    {
        m_textProgressBar->setVisible( true );
        m_thread->addDemos( &demosList );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::parseAndSaveGameState( QString gameState,
                                        QModelIndex rulesIndex,
                                        QModelIndex playerInfoIndex )
{

    // Convert \sv_floodProtect\1\sv_maxPing\0\sv_minPing\0
    // To sv_floodProtect=1\sv_maxPing=0\sv_minPing=0
    if( gameState[0] == '\\' )
        gameState.remove( 0, 1 );

    int i = 0;
    int j = 0;
    while( i < gameState.size() )
    {
        if( gameState[i] == '\\' )
        {
            if( j%2 == 0 )
                gameState[i] = '=';
            j++;
        }
        i++;
    }

    // Create list of key/value.
    QStringList propertyStringList = gameState.split ( "\\", QString::SkipEmptyParts );
    // Store one pair of key/value
    QStringList property;
    // Contains the list of rules saved in the model (sv_fps, timescale etc...)
    QString demoRules;
    // Contains demos informations saved in the model (Player name, head model etc...)
    QString playerInfos;

    int row = 0, nbplayer = 0;
    QResource resource;
    QStringList headModel;
    QString playerName("");
    QString bgColorAvatar = QColor(QPalette().color(QPalette::Window).lighter(10)).name();
    QString bgColorNick = QColor(QPalette().color(QPalette::Window).lighter(85)).name();
    playerInfos = "<table valign=\"middle\">";
    while( row < propertyStringList.size() )
    {
        property = propertyStringList.at(row).split( "=", QString::KeepEmptyParts );

        if( m_settingsData.rules.contains( property.at(0) ) )
            demoRules += propertyStringList.at(row) + "\\";
        else if( property.at(0) == "n" )
            playerName = property.at(1);
        else if( property.at(0) == "hmodel" )
        {
            headModel = property.at(1).split( "/", QString::SkipEmptyParts );

            if( headModel.size() == 2 )
                resource.setFileName( QString(":/image/%1").arg( property.at(1) ) );

            // Sometimes, we have player type (i.e : klesk), but not skin (i.e : red)
            if ( headModel.size() == 1 || !resource.isValid()  )
                resource.setFileName( QString(":/image/%1/default").arg( headModel.at(0) ) );

            // Check that model exists. If it's a special model not in resource, we use the default skin
            if( !resource.isValid() )
                resource.setFileName( QString(":/image/unknown") );
        }

        if( playerName != "" && headModel.size() > 0 )
        {
            nbplayer++;
            playerInfos += "<tr><td bgcolor=\"" + bgColorAvatar + "\"><img width=\"$size\" width=\"$size\" src=\""
                           + resource.fileName()
                           + "\"></td><td bgcolor=\"" + bgColorNick + "\" width=\"100%\"> "
                           + "<b>" + HtmlPlayerName( playerName ) + "</b>"
                           + "<td></tr>";

            playerName = "";
            headModel.clear();
        }
        row++;
    }
    playerInfos += "</table>";

    if( nbplayer > 3 )
        playerInfos.replace("$size","28");
    else
        playerInfos.replace("$size","48");

    m_demoModel->setData( rulesIndex, demoRules, Qt::EditRole );
    m_demoModel->setData( playerInfoIndex, playerInfos, Qt::EditRole );
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::openStatisticsDialog()
{
    if( !m_statisticsDialog )
        m_statisticsDialog = new StatisticsDialog(this);

    int soloVq3 = 0, soloCpm = 0;
    int multiVq3 = 0, multiCpm = 0;
    int nbMillisecVq3 = 0, nbMillisecCpm = 0;

    QStringList list;
    QSqlQuery query;

    query.exec( "SELECT count(id) "
                "FROM demos "
                "WHERE upper(physic) = 'VQ3' AND upper(multi) = 'MDF' AND " + m_demoModel->filter() );
    if( query.next() )
        multiVq3 = query.value(0).toInt();

    query.exec( "SELECT count(id) "
                "FROM demos "
                "WHERE upper(physic) = 'VQ3' AND upper(multi) = 'DF' AND " + m_demoModel->filter() );
    if( query.next() )
        soloVq3 = query.value(0).toInt();

    query.exec( "SELECT count(id) "
                "FROM demos "
                "WHERE upper(physic) = 'CPM' AND upper(multi) = 'DF' AND " + m_demoModel->filter() );
    if( query.next() )
        soloCpm = query.value(0).toInt();

    query.exec( "SELECT count(id) "
                "FROM demos "
                "WHERE upper(physic) = 'CPM' AND upper(multi) = 'MDF' AND " + m_demoModel->filter() );
    if( query.next() )
        multiCpm = query.value(0).toInt();

    query.exec( "SELECT time "
                "FROM demos "
                "WHERE upper(physic) = 'VQ3' AND " + m_demoModel->filter() );
    while( query.next() )
    {
        list = query.value(0).toString().split('.');
        nbMillisecVq3 += list.at(0).toInt() * 60000 + list.at(1).toInt() * 1000 + list.at(2).toInt();
    }

    query.exec( "SELECT time "
                "FROM demos "
                "WHERE upper(physic) = 'CPM' AND " + m_demoModel->filter() );
    while( query.next() )
    {
        list = query.value(0).toString().split('.');
        nbMillisecCpm += list.at(0).toInt() * 60000 + list.at(1).toInt() * 1000 + list.at(2).toInt();
    }

    m_statisticsDialog->updateStatistics( soloVq3, soloCpm,
                                          multiVq3, multiCpm,
                                          nbMillisecVq3, nbMillisecCpm,
                                          m_demoModel->filter() != "1" );
    m_statisticsDialog->exec();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::openSettingsDialog()
{
    if( !m_settingsDialog )
    {
        m_settingsDialog = new SettingsDialog(this);
        m_settingsDialog->setSettings( m_settingsData );
    }

    if( m_settingsDialog->exec() == 1 )
    {
        QdmSettings newSettings = m_settingsDialog->getSettings();

        if( m_settingsData.isEqual( newSettings ) )
        {
            m_settingsData.copy( newSettings );
            saveSettings();
            // Refresh variable list for current demo
            displayDemosInfos( currentDemoIndex().row() );
        }
    }else{
        m_settingsDialog->setSettings( m_settingsData );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::fetchMoreDemos()
{
    // Avoid a strange behaviour of Sql table model. Without that, only 256 rows are talen in account.
    // The reasons seems to be that sqlite doesn't know the number of rows affected by a "SELECT"
    if( m_demoModel->rowCount() == 256 )
    {
        while(m_demoModel->canFetchMore())
            m_demoModel->fetchMore();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::loadSettings()
{
    // On Windows don't put settings file into "document and settings"
    // On linux, it will be $HOME/.config/QDM68/qdm68.ini
    #ifdef Q_OS_WIN32
        m_settingsDevice = new QSettings( "qdm68.ini", QSettings::IniFormat, this );
    #else
        m_settingsDevice = new QSettings( QSettings::IniFormat, QSettings::UserScope, "QDM68", "qdm68" );
    #endif

    if( m_settingsDevice->allKeys().count() == 0 )
    {
        m_settingsData.rules.insert( "timescale", "1" );
        m_settingsData.rules.insert( "g_synchronousClients", "1" );
        m_settingsData.rules.insert( "pmove_fixed", "0" );
        m_settingsData.rules.insert( "pmove_msec", "8" );
        m_settingsData.rules.insert( "sv_fps", "125" );
        m_settingsData.rules.insert( "com_maxfps", "125" );
        m_settingsData.rules.insert( "g_speed", "320" );
        m_settingsData.rules.insert( "g_gravity", "800" );
        m_settingsData.rules.insert( "g_knockback", "1000" );
        m_settingsData.rules.insert( "sv_cheats", "0" );
        m_settingsData.engineFile = "";
        m_settingsData.distinctPlayer = true;
        m_demosDir = QDir::home();
        saveSettings();
    }else{
        QStringList keys;
        m_settingsDevice->beginGroup( "paths" );
        m_demosDir.setPath( m_settingsDevice->value( "demos_dir", QDir::home().absolutePath()).toString() );
        m_settingsData.engineFile = m_settingsDevice->value( "engine" ,"" ).toString();
        m_settingsData.distinctPlayer = m_settingsDevice->value( "distinct_player" ,"" ).toInt();
        m_settingsDevice->endGroup();

        m_settingsDevice->beginGroup( "rules" );
        keys = m_settingsDevice->childKeys();
        for (int i = 0; i < keys.size(); ++i)
            m_settingsData.rules.insert( keys.at(i), m_settingsDevice->value( keys.at(i),"" ).toString() );
        m_settingsDevice->endGroup();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::saveSettings()
{
    QMap<QString, QString>::iterator it;

    if( !m_settingsDevice->isWritable() )
    {
        QMessageBox msgBox;
        msgBox.setText( tr( "Settings could not be saved." ) );
        msgBox.setIcon( QMessageBox::Warning );
        msgBox.exec();
        return;
    }

    m_settingsDevice->beginGroup( "paths" );
    m_settingsDevice->setValue( "demos_dir", m_demosDir.absolutePath() );
    m_settingsDevice->setValue( "engine", m_settingsData.engineFile );
    m_settingsDevice->setValue( "distinct_player", (int)m_settingsData.distinctPlayer );
    m_settingsDevice->endGroup();
    m_settingsDevice->beginGroup( "rules" );
    it = m_settingsData.rules.begin();
    while( it != m_settingsData.rules.end() )
    {
        m_settingsDevice->setValue(it.key(),it.value());
        it++;
    }
    m_settingsDevice->endGroup();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QString MainWindow::HtmlPlayerName( QString name )
{
    int balises = name.count( QRegExp("\\^[0-9]") );

    name.replace( "^0", "<font color=\"#000000\">" ); // black
    name.replace( "^1", "<font color=\"#dd0000\">" ); // red
    name.replace( "^2", "<font color=\"#00dd00\">" ); // green
    name.replace( "^3", "<font color=\"#ffff00\">" ); // yellow
    name.replace( "^4", "<font color=\"#0000dd\">" ); // blue
    name.replace( "^5", "<font color=\"#00ffff\">" ); // aqua (cyan)
    name.replace( "^6", "<font color=\"#ff00ff\">" ); // fuchsia (magenta)
    name.replace( "^7", "<font color=\"#ffffff\">" ); // white
    name.replace( "^8", "<font color=\"orange\">" );  // orange
    name.replace( "^9", "<font color=\"grey\">" );    // Grey

    for( int i=0 ; i<balises ; i++ )
        name.append( "</font>" );

    // Set default color to 'White'
    name.prepend( "<font color=\"#ffffff\">" );
    name.append( "</font>" );

    return name;
}
