#include "protocolprintertableview.h"
#include "protocolprinteritemmodel.h"

#include <QScrollBar>
#include <QEvent>
#include <QFocusEvent>
#include <QMouseEvent>

#include <QDebug>
ProtocolPrinterTableView::ProtocolPrinterTableView(QWidget *parent) : QTableView (parent)
{
}

void ProtocolPrinterTableView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    emit selectRow(selected.indexes(), deselected.indexes());
    return QTableView::selectionChanged(selected, deselected);
}

void ProtocolPrinterTableView::focusInEvent(QFocusEvent *event)
{
    if(event->type() == QEvent::FocusIn)
        emit selectRow(QModelIndexList(), QModelIndexList());
    return QTableView::focusOutEvent(event);
}

void ProtocolPrinterTableView::mousePressEvent(QMouseEvent *event)
{
    if(event->type() == QEvent::Type::MouseButtonPress && event->button() == Qt::RightButton)
        emit selectionSet();
    return QTableView::mousePressEvent(event);
}

bool ProtocolPrinterTableView::eventFilter(QObject *watched, QEvent *event)
{
    return QTableView::eventFilter(watched, event);
}


void ProtocolPrinterTableView::verticalScrollbarAction(int action)
{
    return QTableView::verticalScrollbarAction(action);
}
