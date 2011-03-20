#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QStandardItemModel>

namespace Ui {
    class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    void setRules(QMap<QString, QString> * rules);
    QMap<QString, QString> getRules();

    QString getEngineFilename();
    void setEngineFilename(QString filename);

    bool hasChanged();

protected slots:
    void openEngineDialog();
    void onItemChange();
    void showEvent( QShowEvent * event );

private:
    Ui::SettingsDialog * ui;
    QStandardItemModel * m_model;
    bool m_hasChanged;
};

#endif // SETTINGSDIALOG_H
