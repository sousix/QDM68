#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QMessageBox>

SettingsDialog::SettingsDialog(QWidget * parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    m_hasChanged = false;
    m_model = new QStandardItemModel(this);
    ui->tblRules->setModel(m_model);

    connect(ui->btnEngine, SIGNAL(released()), this, SLOT(openEngineDialog()));
    connect(m_model, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(onItemChange()));
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
    m_hasChanged = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool SettingsDialog::hasChanged()
{
    return m_hasChanged;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SettingsDialog::openEngineDialog()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select quake 3 engine"));

    if ( !fileName.isEmpty() )
    {
        ui->leEngine->setText(fileName);
        m_hasChanged = true;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SettingsDialog::setRules( QMap<QString, int> * rules )
{
    QMap<QString, int>::iterator it;
    int row;

    m_model->clear();
    m_model->setRowCount(rules->size());
    m_model->setColumnCount(2);
    m_model->setHeaderData(0, Qt::Horizontal, tr("CVar"));
    m_model->setHeaderData(1, Qt::Horizontal, tr("Value"));

    row = 0;
    it = rules->begin();
    while( it != rules->end() )
    {
        QStandardItem *itemKey = new QStandardItem(QString("%0").arg(it.key()));
        itemKey->setEditable(false);
        m_model->setItem(row, 0, itemKey);

        QStandardItem *itemValue = new QStandardItem(QString("%0").arg(it.value()));
        m_model->setItem(row, 1, itemValue);

        it++;
        row++;
    }

    ui->tblRules->resizeColumnsToContents();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QMap<QString, int> SettingsDialog::getRules()
{
    QMap<QString, int> rules;

    for( int i=0 ; i<m_model->rowCount() ; i++ )
    {
        rules.insert(m_model->item(i, 0)->text(),
                     m_model->item(i, 1)->text().toInt());
    }

    return rules;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QString SettingsDialog::getEngineFilename()
{
    return ui->leEngine->text();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SettingsDialog::setEngineFilename( QString filename )
{
    ui->leEngine->setText(filename);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void SettingsDialog::onItemChange()
{
    m_hasChanged = true;
}
