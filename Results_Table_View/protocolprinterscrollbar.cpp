#include "protocolprinterscrollbar.h"

#include <QEvent>
#include <QDebug>
#include <QTableView>
ProtocolPrinterScrollBar::ProtocolPrinterScrollBar(Qt::Orientation orientation, QWidget *parent) : QScrollBar (orientation, parent)
{
}

bool ProtocolPrinterScrollBar::event(QEvent *event)
{
    return QScrollBar::event(event);
}

