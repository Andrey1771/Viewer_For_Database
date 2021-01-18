#include "protocolprinterheaderviewextended.h"
#include "newmodel.h"

#include <QTableView>
#include <QScrollBar>

ProtocolPrinterHeaderViewExtended::ProtocolPrinterHeaderViewExtended(QTableView *parent) : ProtocolPrinterHeaderView (parent)
{
    startSetSettings(parent);
}

void ProtocolPrinterHeaderViewExtended::startSetSettings(QTableView *parent)
{
    setSectionsClickable(true);
    setSortIndicatorShown(false);
    setSectionResizeMode(QHeaderView::Interactive);
    setHighlightSections(true);

    connect(this, &ProtocolPrinterHeaderView::sectionResized, this, &ProtocolPrinterHeaderView::adjustPositions);
    connect(parent->horizontalScrollBar(), &QScrollBar::valueChanged, this, &ProtocolPrinterHeaderView::adjustPositions);
    connect(parent->verticalScrollBar(), &QScrollBar::valueChanged, this, &ProtocolPrinterHeaderView::adjustPositions);

    setContextMenuPolicy(Qt::CustomContextMenu);

    this->generateFilters(parent->model()->columnCount(), true);

    this->newModel = dynamic_cast<NewModel*>(parent->model());
    this->model = newModel->model;
}
