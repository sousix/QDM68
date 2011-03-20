#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <time.h>

///////////////////////////////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi( this );
    m_SettingsDialog = NULL;
    m_selectInProgress = false;
    m_thread = new ThreadParser();

    createDatabase();

    m_demoModel = new SqlTableModelCheckable( this );
    m_demoModel->setTable( "demos" );
    m_demoModel->setEditStrategy( QSqlTableModel::OnManualSubmit );

    ui->listView->setModel( m_demoModel );
    ui->listView->setModelColumn(1);

    m_varModel = new QStandardItemModel(this);
    ui->varTableView->setModel(m_varModel);

    connect( ui->btnInvertSelection, SIGNAL(released()), this, SLOT(invertSelection()) );
    connect( ui->btnSelectWorst, SIGNAL(released()), this, SLOT(selectWorst()) );
    connect( ui->btnUnselectAll, SIGNAL(released()), this, SLOT(unselectAll()) );
    connect( ui->actionDemos_folder, SIGNAL(triggered()), this, SLOT(openDemosDialog()) );
    connect( ui->actionSettings, SIGNAL(triggered()), this, SLOT(openSettingsDialog()) );
    connect( ui->actionParseAll, SIGNAL(triggered()), this, SLOT(parseAllDemo()) );
    connect( ui->btnPlayDemo, SIGNAL(released()), this, SLOT(playDemo()) );
    connect( ui->btnMoreInfos, SIGNAL(released()), this, SLOT(onMoreInfosClicked()) );
    connect( ui->listView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onDemoClicked(const QModelIndex &)) );
    connect( m_thread, SIGNAL(demoParsed(int, QString, int, int)), this, SLOT(onDemoParsed(int, QString, int, int)) );
    connect( m_thread, SIGNAL(finished()), this, SLOT(onThreadParserFinished()) );
    connect( m_demoModel, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
             this, SLOT(onBoxChecked(const QModelIndex &, const QModelIndex &)) );

    m_progressBar = new QProgressBar();
    m_textProgressBar = new QLabel();
    m_progressBar->setMaximum(100);
    ui->statusBar->addPermanentWidget( m_textProgressBar );
    ui->statusBar->addPermanentWidget( m_progressBar );
    m_progressBar->setVisible( false );
    m_textProgressBar->setVisible( false );

    initSettings();
    createDemosList();
    ui->demoInfosBox->setVisible( false );
    this->adjustSize();
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

    m_selectInProgress = true;

    m_demoModel->clearBox();
    while( query->next() )
    {
        indexList = m_demoModel->match( m_demoModel->index(0, 0),
                                        Qt::DisplayRole,
                                        QVariant(query->value(0).toInt()).toInt() );
        if( indexList.count() > 0 )
        {
            m_demoModel->setData( m_demoModel->index(indexList.at(0).row(), 1),
                                  Qt::Checked,
                                  Qt::CheckStateRole );
        }
    }
    m_selectInProgress = false;
    updateSelectionInfos();
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
    QSqlQuery query( "SELECT d.id, d.map, d.physic "
                     "FROM demos as d "
                     "WHERE d.id NOT IN (SELECT id FROM demos AS e WHERE e.map = d.map AND e.physic = d.physic AND e.multi = d.multi ORDER BY time ASC LIMIT 0,1)" );
    buildSelection(&query);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::unselectAll()
{
    m_demoModel->clearBox();
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

    ui->btnCopyTo->setEnabled(hasSelection);
    ui->btnMoveTo->setEnabled(hasSelection);
    ui->btnDelete->setEnabled(hasSelection);

    if( !hasSelection )
        statusBar()->showMessage("");
    else
        statusBar()->showMessage( QString(tr("%1 demo(s) selected")).arg(m_demoModel->countBoxChecked()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::openDemosDialog()
{
    QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
    QString dirName = QFileDialog::getExistingDirectory( this, tr("Select demos folder"), m_demosDir.absolutePath(), options );
    QDir newDir( dirName );

    if ( !dirName.isEmpty() )
    {
        if ( m_demosDir != newDir )
        {
            m_demosDir = newDir;
            createDemosList();
            saveSettings();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::openSettingsDialog()
{
    if( !m_SettingsDialog )
    {
        m_SettingsDialog = new SettingsDialog(this);
        m_SettingsDialog->setRules(&m_rules);
        m_SettingsDialog->setEngineFilename(m_engineFile);
    }

    if( m_SettingsDialog->exec() == 1 )
    {
        if( m_SettingsDialog->hasChanged() )
        {
            m_rules = m_SettingsDialog->getRules();
            m_engineFile = m_SettingsDialog->getEngineFilename();
            saveSettings();
        }
    }else{
        if( m_SettingsDialog->hasChanged() )
        {
            m_SettingsDialog->setRules(&m_rules);
            m_SettingsDialog->setEngineFilename(m_engineFile);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool MainWindow::createDatabase()
 {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    if ( !db.open() )
    {
        QMessageBox::critical(0, qApp->tr("Cannot open database"),
                              qApp->tr( "Unable to establish a database connection.\n"
                                        "This example needs SQLite support. Please read "
                                        "the Qt SQL driver documentation for information how "
                                        "to build it.\n\n"
                                        "Click Cancel to exit."), QMessageBox::Cancel );
        return false;
    }

    QSqlQuery query;
    query.exec("CREATE TABLE demos "
               "(id INT PRIMARY KEY, "
               "filename VARCHAR(100),"
               "map VARCHAR(50),"
               "multi VARCHAR(3),"
               "physic VARCHAR(3),"
               "time VARCHAR(9),"
               "author VARCHAR(50),"
               "country VARCHAR(50),"
               "player_infos TEXT,"
               "rules BLOB)");

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// ([^\[.]*)\[([^\..]*)\.([^\].]*)\]([0-9]{2}\.[0-9]{2}\.[0-9]{3})\(([^\..]*)\.([^\).]*)\)\.dm_.*
// ([^\\[.]*)\\[([^\\..]*)\\.([^\\].]*)\\]([0-9]{2}\\.[0-9]{2}\\.[0-9]{3})\\(([^\\..]*)\\.([^\\).]*)\\)\\.dm_.*

void MainWindow::createDemosList()
{
    if( !m_demosDir.exists() )
    {
        qDebug() << "Directory doesn't exist";
        return;
    }

    QSqlQuery query;
    QRegExp pattern( "([^\\[.]*)\\[([^\\..]*)\\.([^\\].]*)\\]([0-9]{2}\\.[0-9]{2}\\.[0-9]{3})\\(([^\\..]*)\\.([^\\).]*)\\)\\.dm_.*" );
    int i, nbDemos = 0;

    if( !m_demoModel->removeRows(0, m_demoModel->rowCount()) )
        qDebug() << "Cannot remove rows";
    else
        m_demoModel->submitAll(); // Apply changement in DB

    for ( i=0 ; i < m_demosDir.entryInfoList().size() ; i++ )
    {
        if( pattern.exactMatch( m_demosDir.entryInfoList().at(i).fileName()) )
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
    statusBar()->showMessage( QString(tr("%1 demos found")).arg(nbDemos) );

    // Avoid a strange behaviour of Sql table model. Without that, only 256 rows are talen in account.
    // The reasons seems to be that sqlite doesn't know the number of rows affected by a "SELECT"
    if( m_demoModel->rowCount() == 256 )
    {
        while(m_demoModel->canFetchMore())
            m_demoModel->fetchMore();
    }

}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::onDemoParsed( int demoId, QString gameState, int current, int size )
{
    m_progressBar->setMaximum( size );
    m_progressBar->setValue( size - current );
    m_textProgressBar->setText(QString("Analyzing... %1/%2").arg(size - current).arg(size));

    const QModelIndexList indexList = m_demoModel->match( m_demoModel->index(0, 0), Qt::DisplayRole, demoId);

    if( indexList.count() > 0 && gameState != "")
    {
        parseAndSaveGameState( gameState,
                               m_demoModel->index( indexList.at(0).row(), 8),
                               m_demoModel->index( indexList.at(0).row(), 9) );

        QModelIndexList rowSelected = ui->listView->selectionModel()->selectedIndexes();

        // If the demo parsed correspond to the demo selected, we display info
        if( !rowSelected.isEmpty() && rowSelected.at(0).row() == indexList.at(0).row() )
        {
            displayDemosInfos( rowSelected.at(0).row() );
        }
    }

}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::onThreadParserFinished()
{
    m_progressBar->setValue(0);
    m_textProgressBar->setText("");
    m_progressBar->setVisible(false);
    m_textProgressBar->setVisible(false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::onMoreInfosClicked()
{
    ui->demoInfosBox->setVisible( !ui->demoInfosBox->isVisible() );
    this->adjustSize();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::displayDemosInfos( int row )
{
    const QStringList & rulesList = m_demoModel->data( m_demoModel->index( row, 8 ), Qt::DisplayRole ).toString().split ( "\\", QString::SkipEmptyParts );
    const QStringList & playerInfoList = m_demoModel->data( m_demoModel->index( row, 9 ), Qt::DisplayRole ).toString().split ( "\\", QString::SkipEmptyParts );
    int i = 0;
    QMap<QString, QString>::iterator it;

    QMap<QString, QString> rulesMap;
    QStringList pair;
    for( i = 0 ; i < rulesList.size() ; i++ )
    {
        pair = rulesList.at(i).split ( "=", QString::SkipEmptyParts );
        rulesMap.insert( pair.at(0), pair.at(1) );
    }

    m_varModel->clear();
    m_varModel->setRowCount( m_rules.size() );
    m_varModel->setColumnCount(2);

    // Display rules

    it = m_rules.begin();
    i = 0;
    while( it != m_rules.end() )
    {
        // Insert key
        QStandardItem *itemKey = new QStandardItem( it.key() );
        itemKey->setEditable(false);
        m_varModel->setItem( i, 0, itemKey );

        // Insert value
        QStandardItem *itemValue;
        if( rulesMap.contains( it.key() ) )
        {
            itemValue = new QStandardItem( rulesMap.value( it.key() ) );
            if( rulesMap.value( it.key() ) == it.value() )
                itemValue->setIcon(QIcon(":/image/valid"));
            else
                itemValue->setIcon(QIcon(":/image/warning"));
        }else{
            itemValue = new QStandardItem("?");
        }
        itemValue->setEditable(false);
        m_varModel->setItem( i, 1, itemValue );

        it++;
        i++;
    }

    ui->varTableView->resizeColumnsToContents();
    m_varModel->sort(0);

    // Display player(s) infos

    /*QMap<QString, QString> playerInfoMap;
    for( i = 0 ; i < playerInfoMap.size() ; i++ )
    {
        pair = playerInfoList.at(i).split( "=", QString::SkipEmptyParts );
        playerInfoMap.insert( pair.at(0), pair.at(1) );
    }*/

    ui->frameDemo->setHtml( m_demoModel->data( m_demoModel->index( row, 9 ), Qt::DisplayRole).toString() );
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::onDemoClicked( const QModelIndex & index )
{
    const QModelIndex & RulesIndex = m_demoModel->index(index.row(), 8);
    const QModelIndex & PlayerInfoIndex = m_demoModel->index(index.row(), 9);

    if( ui->demoInfosBox->isVisible() )
    {
        if( m_demoModel->data( PlayerInfoIndex, Qt::DisplayRole ).toString() == "" )
        {
            QPair<int, QString> pair;
            pair.first = m_demoModel->data( m_demoModel->index(index.row(), 0), Qt::DisplayRole ).toInt();
            pair.second = m_demosDir.absolutePath() + "/" + m_demoModel->data( m_demoModel->index(index.row(), 1), Qt::DisplayRole ).toString();
            m_thread->addOneDemo(pair);

        }else{
            displayDemosInfos( index.row() );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::playDemo()
{
    QModelIndexList indexList = ui->listView->selectionModel()->selectedIndexes();

    if( !QFile::exists(m_engineFile) )
    {
        QMessageBox msgBox;
        msgBox.setText(QString("File %1 not found").arg(m_engineFile));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
        return;
    }

    // replace "/home/stef/.q3a/defrag/demos/subfolder/" by "subfolder/"
    int strIndex = m_demosDir.absolutePath().indexOf("demos") + 6;
    QString demoFolder =  m_demosDir.absolutePath().replace(0, strIndex, "");
    if( demoFolder != "" )
        demoFolder += "/";

    QString command = m_engineFile +
                      " +set fs_game defrag" +
                      " +demo " +
                      demoFolder +
                      indexList.at(0).data().toString();

    statusBar()->showMessage( QString(tr("Executing '%1'")).arg(command) );
    QProcess::execute(command);
    qDebug() << command;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::parseAllDemo()
{
    QList<QPair<int, QString> > demosList;
    QPair<int, QString> pair;
    for( int i=0 ; i<m_demoModel->rowCount() ; i++ )
    {
        if( m_demoModel->data( m_demoModel->index(i, 8), Qt::DisplayRole ).toString() == "" )
        {
            pair.first = m_demoModel->data( m_demoModel->index(i, 0), Qt::DisplayRole ).toInt();
            pair.second = m_demosDir.absolutePath() + "/" + m_demoModel->data( m_demoModel->index(i, 1), Qt::DisplayRole ).toString();
            demosList.append( pair );
        }
    }

    if( !demosList.isEmpty() )
    {
        m_progressBar->setVisible( true );
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
    playerInfos = "<table valign=\"middle\">";
    while( row < propertyStringList.size() )
    {
        property = propertyStringList.at(row).split( "=", QString::KeepEmptyParts );

        if( m_rules.contains( property.at(0) ) )
            demoRules += propertyStringList.at(row) + "\\";
        else if( property.at(0) == "n" )
            playerName = property.at(1);
        else if( property.at(0) == "hmodel" )
        {
            headModel = property.at(1).split( "/", QString::SkipEmptyParts );
            // Sometimes, we have player type (i.e : klesk), but not skin (i.e : red)
            if( headModel.size() != 2 )
                resource.setFileName( QString(":/image/%1/default").arg( headModel.at(0) ) );
            else
                resource.setFileName( QString(":/image/%1").arg( property.at(1) ) );

            // Check that model exists. If it's a special model not in resource, we use the default skin
            if( !resource.isValid() )
                resource.setFileName( QString(":/image/%1/default").arg( headModel.at(0) ) );
        }
        if( playerName != "" && headModel.size() > 0 )
        {
            nbplayer++;
            playerInfos += "<tr><td bgcolor=\"black\"><img width=\"$size\" width=\"$size\" src=\"" + resource.fileName() + "\"></td><td> " + playerName + "<td></tr>";
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

void MainWindow::initSettings()
{
    if( !QFile::exists(CFG_FILE) )
    {
        m_rules.insert("timescale", "1");
        m_rules.insert("g_synchronousClients", "1");
        m_rules.insert("pmove_fixed", "0");
        m_rules.insert("pmove_msec", "8");
        m_rules.insert("defrag_svfps", "125");
        m_rules.insert("defrag_clfps", "125");
        m_rules.insert("g_speed", "320");
        m_rules.insert("g_gravity", "800");
        m_rules.insert("g_knockback", "1000");
        m_rules.insert("sv_cheats", "0");
        m_engineFile = "";
        m_demosDir = QDir::home();
        saveSettings();

    }else{

        QSettings settings(CFG_FILE, QSettings::IniFormat, this);
        QStringList keys;

        settings.beginGroup("paths");
        m_demosDir.setPath( settings.value("demos_dir", QDir::home().absolutePath()).toString());
        m_engineFile = settings.value("engine","").toString();
        settings.endGroup();
        settings.beginGroup("rules");
        keys = settings.childKeys();
        for (int i = 0; i < keys.size(); ++i)
            m_rules.insert( keys.at(i), settings.value(keys.at(i),"").toString() );
        settings.endGroup();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::saveSettings()
{
    QMap<QString, QString>::iterator it;
    QSettings settings(CFG_FILE, QSettings::IniFormat, this);

    if( !settings.isWritable() )
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Settings cannot be saved."));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
        return;
    }

    settings.beginGroup("paths");
    settings.setValue("demos_dir", m_demosDir.absolutePath());
    settings.setValue("engine", m_engineFile);
    settings.endGroup();
    settings.beginGroup("rules");
    it = m_rules.begin();
    while( it != m_rules.end() )
    {
        settings.setValue(it.key(),it.value());
        it++;
    }
    settings.endGroup();
}
