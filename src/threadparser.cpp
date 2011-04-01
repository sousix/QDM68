#include "threadparser.h"

extern "C" {
    #include "q3sdc/q3sdc.h"
}

ThreadParser::ThreadParser(QObject *parent) :
    QThread(parent), m_demosList()
{
    m_size = 0;
    m_priorityDemo.first = -1;
}


///////////////////////////////////////////////////////////////////////////////////////////////////

void ThreadParser::addPriorityDemo( QPair<int, QString> demo )
{
    m_mutex.lock();
    m_priorityDemo = demo;
    m_mutex.unlock();
    if( !isRunning() )
        start();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void ThreadParser::addDemos( QList<QPair<int, QString> > * demosList )
{
    m_demosList.append( *demosList );
    m_size += demosList->size();
    if( !demosList->isEmpty() && !isRunning() )
        start();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void ThreadParser::run()
{
    QString gameState;
    int id;
    while( !m_demosList.isEmpty() || m_priorityDemo.first != -1 )
    {
        msleep(5);
        if( m_priorityDemo.first != -1 )
        {
            m_mutex.lock();
            gameState = q3sdc_parse( m_priorityDemo.second.toAscii().data() );
            id = m_priorityDemo.first;
            m_priorityDemo.first = -1;
            m_mutex.unlock();
        }else{
            gameState = q3sdc_parse( m_demosList.at(0).second.toAscii().data() );
            id = m_demosList.at(0).first;
            m_demosList.removeFirst();
        }
        gameState.replace("defrag_clfps", "com_maxfps");
        gameState.replace("defrag_svfps", "sv_fps");
        emit( demoParsed( id, gameState, m_demosList.size(), m_size ) );
    }
    m_size = 0;
}

