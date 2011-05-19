#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QMessageBox>

SettingsDialog::SettingsDialog(QWidget * parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    m_model = new QStandardItemModel(this);
    ui->tblRules->setModel(m_model);

    this->move( parent->x() + (parent->size().rwidth() - this->size().rwidth())/2 ,
                parent->y() + (parent->size().rheight() - this->size().rheight())/2 );

    connect(ui->btnEngine, SIGNAL(released()), this, SLOT(openEngineDialog()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SettingsDialog::showEvent( QShowEvent * event )
{
    Q_UNUSED(event);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SettingsDialog::openEngineDialog()
{
    QFileInfo fileInfo( ui->leEngine->text() );
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select quake 3 engine"), fileInfo.absolutePath());

    if ( !fileName.isEmpty() )
    {
        ui->leEngine->setText( fileName );
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QdmSettings SettingsDialog::getSettings()
{
    QdmSettings settings;

    for( int i=0 ; i<m_model->rowCount() ; i++ )
    {
        settings.rules.insert( m_model->item(i, 0)->text(),
                               m_model->item(i, 1)->text() );
    }
    settings.engineFile = ui->leEngine->text();

    return settings;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SettingsDialog::setSettings( QdmSettings settings )
{
    QMap<QString, QString>::iterator it;
    int row;

    m_model->clear();
    m_model->setRowCount(settings.rules.size());
    m_model->setColumnCount(2);
    m_model->setHeaderData(0, Qt::Horizontal, tr("Variable"));
    m_model->setHeaderData(1, Qt::Horizontal, tr("Value"));

    row = 0;
    it = settings.rules.begin();
    while( it != settings.rules.end() )
    {
        QStandardItem *itemKey = new QStandardItem( it.key() );
        itemKey->setEditable(false);
        m_model->setItem( row, 0, itemKey );

        QStandardItem *itemValue = new QStandardItem( it.value() );
        m_model->setItem( row, 1, itemValue );

        it++;
        row++;
    }
    ui->tblRules->resizeColumnsToContents();

    ui->leEngine->setText( settings.engineFile );
}
