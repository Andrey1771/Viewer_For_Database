#include "printdialogtableview.h"

PrintDialogTableView::PrintDialogTableView(QWidget *parent) : QTableView (parent)
{

}

QModelIndexList PrintDialogTableView::getSelectedIndex() const
{
    if(!selectedIndexes().isEmpty())
        return selectedIndexes();

    return {};
}

