#include "threadparser.h"

extern "C" {
    #include "q3sdc/q3sdc.h"
}

ThreadParser::ThreadParser(QObject *parent) :
    QThread(parent), m_demosList()
{
    m_size = 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////

void ThreadParser::addOneDemo( QPair<int, QString> demo )
{
    m_demosList.append( demo );
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
    while( !m_demosList.isEmpty() )
    {
        msleep(5);
        gameState = q3sdc_parse( m_demosList.at(0).second.toAscii().data() );
        gameState.replace("defrag_clfps", "com_maxfps");
        gameState.replace("defrag_svfps", "sv_fps");
        emit( demoParsed( m_demosList.at(0).first, gameState, m_demosList.size(), m_size ) );
        m_demosList.removeFirst();
    }
    m_size = 0;
}

