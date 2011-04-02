#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QStandardItemModel>

typedef struct QdmSettings
{
    QString engineFile;
    QMap<QString, QString> rules;

    bool isEqual( QdmSettings other )
    {
        return( other.engineFile != this->engineFile ||
                other.rules != this->rules );
    }

    void copy( QdmSettings other )
    {
        this->engineFile = other.engineFile;
        this->rules = other.rules;
    }

} QdmSettings;

///////////////////////////////////////////////////////////////////////

namespace Ui {
    class SettingsDialog;
}

///////////////////////////////////////////////////////////////////////

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    void setSettings( QdmSettings settings );
    QdmSettings getSettings();

protected slots:
    void openEngineDialog();
    void showEvent( QShowEvent * event );

private:
    Ui::SettingsDialog * ui;
    QStandardItemModel * m_model;
};

#endif // SETTINGSDIALOG_H
