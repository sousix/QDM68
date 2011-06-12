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

#define SOFTWARE_VERSION "1.0 beta1"

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

    ThreadParser * m_thread;
};

#endif // MAINWINDOW_H
