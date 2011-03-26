#ifndef LISTVIEWCUSTOM_H
#define LISTVIEWCUSTOM_H

#include <QListView>

/*
 Subclass QListView to support event on selection change.
 */

class ListViewCustom : public QListView
{
    Q_OBJECT
public:
    explicit ListViewCustom(QWidget *parent = 0);

signals:
    void selectionChanged( const QModelIndex & current );

protected slots:
    void currentChanged( const QModelIndex & current, const QModelIndex & previous );

};

#endif // LISTVIEWCUSTOM_H
