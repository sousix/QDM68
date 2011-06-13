#include <QTime>
#include "statisticsdialog.h"
#include "ui_statisticsdialog.h"

StatisticsDialog::StatisticsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StatisticsDialog)
{
    ui->setupUi(this);

    connect(ui->btnClose, SIGNAL(released()), this, SLOT(close()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

StatisticsDialog::~StatisticsDialog()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void StatisticsDialog::updateStatistics( int nbSoloVq3, int nbSoloCpm,
                                         int nbMultiVq3, int nbMultiCpm,
                                         int nbMillisecVq3, int nbMillisecCpm,
                                         bool isFromSearchResults )
{
    int nbSolo = nbSoloVq3 + nbSoloCpm;
    int nbMulti = nbMultiVq3 + nbMultiCpm;
    int nbVq3 = nbSoloVq3 + nbMultiVq3;
    int nbCpm = nbSoloCpm + nbMultiCpm;
    int nbTotal = nbSolo + nbMulti;

    QTime timeVq3( 0, 0 );
    timeVq3 = timeVq3.addMSecs( nbMillisecVq3 );
    QTime timeCpm( 0, 0 );
    timeCpm = timeCpm.addMSecs( nbMillisecCpm );
    QTime timeTotal ( 0, 0 );
    timeTotal = timeTotal.addMSecs( nbMillisecVq3 + nbMillisecCpm );

    ui->lblTotal->setText( QString( "%1" ).arg( nbTotal ) );

    ui->lblTotalVq3->setText( getStats( nbVq3, nbTotal ) );
    ui->lblTotalCpm->setText( getStats( nbCpm, nbTotal ) );

    ui->lblSolo->setText( getStats( nbSolo, nbTotal ) );
    ui->lblSoloVq3->setText( getStats( nbSoloVq3, nbSolo ) );
    ui->lblSoloCpm->setText( getStats( nbSoloCpm, nbSolo ) );

    ui->lblMulti->setText( getStats( nbMulti, nbTotal ) );
    ui->lblMultiVq3->setText( getStats( nbMultiVq3, nbMulti ) );
    ui->lblMultiCpm->setText( getStats( nbMultiCpm, nbMulti ) );

    ui->lblTimeCpm->setText( timeCpm.toString( "H'h'mm'm'ss's'" ) );
    ui->lblTimeVq3->setText( timeVq3.toString( "H'h'mm'm'ss's'" ) );
    ui->lblTime->setText( timeTotal.toString( "H'h'mm'm'ss's'" ) );

    ui->labelInfoSearch->setVisible( isFromSearchResults );
}

///////////////////////////////////////////////////////////////////////////////////////////////////

QString StatisticsDialog::getStats( int nb, int total )
{
    // Avoid division by zero
    if( total > 0 )
        return QString( "%1 (%2%)"  ).arg( nb ).arg( ( (float)nb / (float)total ) * 100, 0, 'f', 0 );
    else
        return "0 (0%)";
}
