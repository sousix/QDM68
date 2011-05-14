#ifndef SQLTABLEMODELCHECKABLE_H
#define SQLTABLEMODELCHECKABLE_H

#include <QtSql>

/*
 Subclass QSqlTableModel to support checkbox data (using QSet<QPersistentModelIndex>)
 */

class SqlTableModelCheckable : public QSqlTableModel
{

public:
    SqlTableModelCheckable( int primaryKeyColumn, QObject * parent = 0, QSqlDatabase db = QSqlDatabase() );

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole);
    bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );

    bool hasBoxChecked();
    void clearBox();
    int countBoxChecked();

protected:
    int primaryKey( const int row ) const;

private:
    int m_primaryKeyColumn;
    QSet<int> m_idList; // List of checked box. Empty at startup.
    QFont m_fontBold;
    QFont m_fontNormal;

};

#endif // SQLTABLEMODELCHECKABLE_H
