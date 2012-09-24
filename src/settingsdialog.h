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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QStandardItemModel>

typedef struct QdmSettings
{
    QString engineFile;
    bool distinctPlayer;
    QMap<QString, QString> rules;
    QDir demosDir;
    bool showDetails;

    bool isEqual( QdmSettings other )
    {
        return( other.engineFile != this->engineFile ||
                other.rules != this->rules ||
                other.distinctPlayer != this->distinctPlayer ||
                other.demosDir != this->demosDir ||
                other.showDetails != this->showDetails );
    }

    void copy( QdmSettings other )
    {
        this->engineFile = other.engineFile;
        this->rules = other.rules;
        this->distinctPlayer = other.distinctPlayer;
        this->demosDir = other.demosDir;
        this->showDetails = other.showDetails;
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
    QdmSettings m_settings;
};

#endif // SETTINGSDIALOG_H
