#ifndef SQLTABLEMODELCHECKABLE_H
#define SQLTABLEMODELCHECKABLE_H

#include <QtSql>

/*
 Subclass QSqlTableModel to support checkbox data (QSet<QPersistentModelIndex>)
 */

class SqlTableModelCheckable : public QSqlTableModel
{

public:
    SqlTableModelCheckable(QObject * parent = 0, QSqlDatabase db = QSqlDatabase());

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex& index, int role) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role);
    bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );

    bool hasBoxChecked();
    void clearBox();
    int countBoxChecked();

private:
    QSet<QPersistentModelIndex> checklist; // List of checked box. Empty at startup.

    QFont fontBold;
    QFont fontNormal;

};

#endif // SQLTABLEMODELCHECKABLE_H
