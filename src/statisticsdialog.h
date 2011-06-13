#ifndef STATISTICSDIALOG_H
#define STATISTICSDIALOG_H

#include <QDialog>

namespace Ui {
    class StatisticsDialog;
}

class StatisticsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StatisticsDialog(QWidget *parent = 0);
    ~StatisticsDialog();
    void updateStatistics( int nbSoloVq3, int nbSoloCpm,
                           int nbMultiVq3, int nbMultiCpm,
                           int nbMillisecVq3, int nbMillisecCpm,
                           bool isFromSearchResults);

    QString getStats( int nb, int total );

private:
    Ui::StatisticsDialog *ui;
};

#endif // STATISTICSDIALOG_H
