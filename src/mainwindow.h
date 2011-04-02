#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QtSql>
#include "sqltablemodelcheckable.h"
#include "settingsdialog.h"
#include "threadparser.h"
#include "listviewcustom.h"

#define CFG_FILE "qdm68.cfg"

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
    void initSettings();
    void saveSettings();
    QModelIndex currentDemo();
    void emptyDemoInfos();

protected slots:
    void invertSelection();
    void selectWorst();
    void unselectAll();
    void onBoxChecked( const QModelIndex &, const QModelIndex & );
    void openDemosDialog();
    void openSettingsDialog();
    void playDemo();
    void parseAllDemo();
    void processDemo( const QModelIndex & );
    void onMoreInfosClicked();
    void onDemoParsed( int, QString, int, int );
    void onThreadParserFinished();

private:
    Ui::MainWindow * ui;

    QdmSettings m_settings;
    QDir m_demosDir;
    SqlTableModelCheckable * m_demoModel;
    SettingsDialog * m_SettingsDialog;
    QStandardItemModel * m_varModel;
    bool m_selectInProgress;
    QLabel * m_textProgressBar;

    ThreadParser * m_thread;
};

#endif // MAINWINDOW_H
