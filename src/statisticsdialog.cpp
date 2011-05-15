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
                                         int nbMillisecVq3, int nbMillisecCpm )
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

    // This test avoid division by zero
    if( nbTotal > 0)
    {
        ui->lblTotalVq3->setText( QString( "%1 (%2%)" ).arg( nbVq3 ).arg( ( (float)nbVq3 / (float)nbTotal ) * 100, 0, 'f', 0 ) );
        ui->lblTotalCpm->setText( QString( "%1 (%2%)" ).arg( nbCpm ).arg( ( (float)nbCpm / (float)nbTotal ) * 100, 0, 'f', 0 ) );
    }else{
        ui->lblTotalVq3->setText( "0" );
        ui->lblTotalCpm->setText( "0" );
    }

    if( nbSolo > 0 )
    {
        ui->lblSolo->setText( QString( "%1 (%2%)" ).arg( nbSolo ).arg( ( (float)nbSolo / (float)nbTotal ) * 100, 0, 'f', 0 ) );
        ui->lblSoloVq3->setText( QString( "%1 (%2%)" ).arg( nbSoloVq3 ).arg( ( (float)nbSoloVq3 / (float)nbSolo ) * 100, 0, 'f', 0 ) );
        ui->lblSoloCpm->setText( QString( "%1 (%2%)" ).arg( nbSoloCpm ).arg( ( (float)nbSoloCpm / (float)nbSolo ) * 100, 0, 'f', 0 ) );
    }else{
        ui->lblSolo->setText( "0" );
        ui->lblSoloVq3->setText( "0" );
        ui->lblSoloCpm->setText( "0" );
    }

    if( nbMulti > 0 )
    {
        ui->lblMulti->setText( QString( "%1 (%2%)"  ).arg( nbMulti ).arg( ( (float)nbMulti / (float)nbTotal ) * 100, 0, 'f', 0 ) );
        ui->lblMultiVq3->setText( QString( "%1 (%2%)" ).arg( nbMultiVq3 ).arg( ( (float)nbMultiVq3 / (float)nbMulti ) * 100, 0, 'f', 0 ) );
        ui->lblMultiCpm->setText( QString(" %1 (%2%)" ).arg( nbMultiCpm ).arg( ( (float)nbMultiCpm / (float)nbMulti ) * 100, 0, 'f', 0 ) );
    }else{
        ui->lblMulti->setText( "0" );
        ui->lblMultiVq3->setText( "0" );
        ui->lblMultiCpm->setText( "0" );
    }

    ui->lblTimeCpm->setText( timeCpm.toString( "H'h'mm'm'ss's'" ) );
    ui->lblTimeVq3->setText( timeVq3.toString( "H'h'mm'm'ss's'" ) );
    ui->lblTime->setText( timeTotal.toString( "H'h'mm'm'ss's'" ) );
}
