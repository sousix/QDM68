#ifndef THREADPARSER_H
#define THREADPARSER_H

#include <QThread>
#include <QList>
#include <QString>
#include <QPair>
#include <QMutex>

class ThreadParser : public QThread
{
    Q_OBJECT

public:
    explicit ThreadParser(QObject *parent = 0);
    void addDemos( QList< QPair<int, QString> > * );
    void addPriorityDemo( QPair<int, QString> demo );
    void run();

signals:
    void demoParsed( int demoId, QString gameState, int current, int size );

private:
    QList< QPair<int, QString> > m_demosList;
    QPair<int, QString> m_priorityDemo;
    QMutex m_mutex;
    int m_size;
};

#endif // THREADPARSER_H
