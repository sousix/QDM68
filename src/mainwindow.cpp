#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>

extern "C" {
    #include "q3sdc/q3sdc.h"
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_SettingsDialog = NULL;
    m_selectInProgress = false;

    createDatabase();

    m_demoModel = new SqlTableModelCheckable(this);
    m_demoModel->setTable("demos");
    m_demoModel->setEditStrategy(QSqlTableModel::OnFieldChange);

    ui->listView->setModel(m_demoModel);
    ui->listView->setModelColumn(1);

    m_varModel = new QStandardItemModel(this);
    ui->varTableView->setModel(m_varModel);

    connect(ui->btnInvertSelection, SIGNAL(released()), this, SLOT(invertSelection()));
    connect(ui->btnSelectWorst, SIGNAL(released()), this, SLOT(selectWorst()));
    connect(ui->btnUnselectAll, SIGNAL(released()), this, SLOT(unselectAll()));
    connect(ui->actionDemos_folder, SIGNAL(triggered()), this, SLOT(openDemosDialog()));
    connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(openSettingsDialog()));
    connect(ui->btnPlayDemo, SIGNAL(released()), this, SLOT(playDemo()));
    connect(ui->btnMoreInfos, SIGNAL(released()), this, SLOT(showDemoInfos()));
    connect(ui->listView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(updateDemoInfos(const QModelIndex &)));
    connect(m_demoModel,
            SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
            this,
            SLOT(onBoxChecked(const QModelIndex &, const QModelIndex &)));

    initSettings();
    createDemosList();
    ui->demoInfosBox->setVisible(false);
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
        indexList = m_demoModel->match(m_demoModel->index(0, 0),
                                       Qt::DisplayRole,
                                       QVariant(query->value(0).toInt()).toInt());

        if( indexList.count() > 0 )
            m_demoModel->setData(m_demoModel->index(indexList.at(0).row(), 1),
                                 Qt::Checked,
                                 Qt::CheckStateRole);
        else
            qDebug() << query->value(0).toInt();

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
        if( m_demoModel->data( m_demoModel->index(i, 1), Qt::CheckStateRole ) == Qt::Unchecked  )
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
    QSqlQuery query("SELECT d.id, d.map, d.physic "
                    "FROM demos as d "
                    "WHERE d.id NOT IN (SELECT id FROM demos AS e WHERE e.map = d.map AND e.physic = d.physic AND e.multi = d.multi ORDER BY time ASC LIMIT 0,1)");
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

    // Speed up selection : Do not update UI when selecting worst demos,
    // or when inverting selection
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
        statusBar()->showMessage(QString(tr("%1 demo(s) selected")).arg(m_demoModel->countBoxChecked()));
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
    query.exec("create table demos "
               "(id int primary key, "
               "filename varchar(100),"
               "map varchar(50),"
               "multi varchar(3),"
               "physic varchar(3),"
               "time varchar(9),"
               "author varchar(50),"
               "country varchar(50))");

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

    for ( i=0 ; i < m_demosDir.entryInfoList().size() ; i++ )
    {
        if( pattern.exactMatch( m_demosDir.entryInfoList().at(i).fileName()) )
        {
            nbDemos++;
            pattern.indexIn( m_demosDir.entryInfoList().at(i).fileName() );
            query.exec( QString( "INSERT INTO demos "
                                 "VALUES (%1,'%2','%3','%4','%5','%6','%7','%8')").arg(i).arg( pattern.cap(0),
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

    /*query.exec( "SELECT count(*) AS count FROM demos" );
    query.next();
    qDebug() << "Total demos in DB : " << QVariant(query.value(0).toInt()).toInt();
    qDebug() << "Demos checked : " << m_model->countBoxChecked();*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::showDemoInfos()
{
    ui->demoInfosBox->setVisible( !ui->demoInfosBox->isVisible() );
    this->adjustSize();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::updateDemoInfos(const QModelIndex & index)
{
    //if( ui->demoInfosBox->)
    //m_demoModel->index(index.row(), index.column()).data().isNull();
    parseDemo();
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

void MainWindow::parseDemo()
{
    QModelIndexList indexList = ui->listView->selectionModel()->selectedIndexes();
    QString file = m_demosDir.absolutePath() + "/" + indexList.at(0).data().toString();
    int i, j;

    // Parse demo
    qDebug() << "-- Parsing";
    QString demoInfo( q3sdc_parse( file.toAscii().data() ) );

    // Parse infos
    if( demoInfo != "" )
    {
        // Convert \sv_floodProtect\1\sv_maxPing\0\sv_minPing\0
        // To sv_floodProtect=1\sv_maxPing=0\sv_minPing=0
        if( demoInfo[0] == '\\' )
            demoInfo.remove( 0, 1 );

        i = 0;
        j = 0;
        while( i < demoInfo.size() )
        {
            if( demoInfo[i] == '\\' )
            {
                if( j%2 == 0 )
                    demoInfo[i] = '=';
                j++;
            }
            i++;
        }

        // Create list of key/value.
        QStringList list = demoInfo.split ( "\\", QString::SkipEmptyParts );
        QStringList pair;
        int row;

        //list.indexOf()

        m_varModel->clear();
        m_varModel->setRowCount(list.size());
        m_varModel->setColumnCount(2);

        row = 0;
        while( row < list.size() )
        {
            pair = list.at(row).split( "=", QString::KeepEmptyParts );

            QStandardItem *itemKey = new QStandardItem(QString("%0").arg(pair.at(0)));
            itemKey->setEditable(false);
            m_varModel->setItem(row, 0, itemKey);

            QStandardItem *itemValue = new QStandardItem(QString("%0").arg(pair.at(1)));
            m_varModel->setItem(row, 1, itemValue);
            row++;
        }

        ui->varTableView->resizeColumnsToContents();

    }else{
        qDebug() << "-- Cannot parse demo";
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::initSettings()
{
    if( !QFile::exists(CFG_FILE) )
    {
        m_rules.insert("timescale", 1);
        m_rules.insert("g_synchronousClients", 1);
        m_rules.insert("pmove_fixed", 0);
        m_rules.insert("pmove_msec", 8);
        m_rules.insert("sv_fps", 125);
        m_rules.insert("com_maxfps", 125);
        m_rules.insert("g_speed", 320);
        m_rules.insert("g_gravity", 800);
        m_rules.insert("g_knockback", 1000);
        m_rules.insert("sv_cheats", 0);
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
            m_rules.insert( keys.at(i), settings.value(keys.at(i),"").toInt() );
        settings.endGroup();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::saveSettings()
{
    QMap<QString, int>::iterator it;
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



