#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QtSql>
#include "sqltablemodelcheckable.h"
#include "settingsdialog.h"

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

protected:
    bool createDatabase();
    void setSelection( QSqlQuery * query );
    void createDemosList();
    void initSettings();
    void saveSettings();
    void parseDemo();
    void updateSelectionInfos();

protected slots:
    void invertSelection();
    void selectWorst();
    void unselectAll();
    void onBoxChecked( const QModelIndex &, const QModelIndex & );
    void buildSelection( QSqlQuery * query  );
    void openDemosDialog();
    void openSettingsDialog();
    void playDemo();
    void updateDemoInfos( const QModelIndex & );
    void showDemoInfos();

private:
    Ui::MainWindow * ui;

    QDir m_demosDir;
    QString m_engineFile;
    QMap<QString, int> m_rules;
    SqlTableModelCheckable *m_demoModel;
    SettingsDialog * m_SettingsDialog;
    QStandardItemModel * m_varModel;
    bool m_selectInProgress;
};

#endif // MAINWINDOW_H
