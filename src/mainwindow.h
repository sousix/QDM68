#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QtSql>
#include "sqltablemodelcheckable.h"
#include "settingsdialog.h"
#include "threadparser.h"

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
    bool createDatabase();
    void setSelection( QSqlQuery * );
    void createDemosList();
    void initSettings();
    void saveSettings();
    void updateSelectionInfos();
    void parseAndSaveGameState( QString, QModelIndex, QModelIndex );
    void displayDemosInfos( int );

protected slots:
    void invertSelection();
    void selectWorst();
    void unselectAll();
    void onBoxChecked( const QModelIndex &, const QModelIndex & );
    void buildSelection( QSqlQuery * );
    void openDemosDialog();
    void openSettingsDialog();
    void playDemo();
    void parseAllDemo();
    void onDemoClicked( const QModelIndex & );
    void onMoreInfosClicked();
    void onDemoParsed( int, QString, int, int );
    void onThreadParserFinished();

private:
    Ui::MainWindow * ui;

    QDir m_demosDir;
    QString m_engineFile;
    QMap<QString, QString> m_rules;
    SqlTableModelCheckable * m_demoModel;
    SettingsDialog * m_SettingsDialog;
    QStandardItemModel * m_varModel;
    bool m_selectInProgress;
    QProgressBar * m_progressBar;
    QLabel * m_textProgressBar;

    ThreadParser * m_thread;
};

#endif // MAINWINDOW_H
