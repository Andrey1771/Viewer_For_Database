#ifndef PRINTDIALOGTABLEVIEW_H
#define PRINTDIALOGTABLEVIEW_H

#include <QTableView>

class PrintDialogTableView : public QTableView
{
    Q_OBJECT
    QItemSelection selectedItems;
public:
    PrintDialogTableView(QWidget *parent = nullptr);
    QModelIndexList getSelectedIndex() const;

};

#endif // PRINTDIALOGTABLEVIEW_H
