#include "listviewcustom.h"

ListViewCustom::ListViewCustom(QWidget *parent) : QListView(parent)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void ListViewCustom::currentChanged ( const QModelIndex & current, const QModelIndex & previous )
{
    Q_UNUSED(previous);
    emit( selectionChanged( current ) );
}
