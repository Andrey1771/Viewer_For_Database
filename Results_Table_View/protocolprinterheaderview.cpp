#include "protocolprinterheaderview.h"
#include "lineeditextended.h"
#include "protocolprinteritemmodel.h"
#include "sqlquerybuilder.h"

#include <QApplication>
#include <QSqlTableModel>
#include <QScrollBar>
#include <QTableView>

#include <QDebug>

ProtocolPrinterHeaderView::ProtocolPrinterHeaderView(QTableView *parent) : QHeaderView (Qt::Horizontal, parent)
{
    startSetSettings(parent);
}

void ProtocolPrinterHeaderView::startSetSettings(QTableView *parent)
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

    this->model = dynamic_cast<ProtocolPrinterItemModel*>(parent->model());
}

//
void ProtocolPrinterHeaderView::update(QAbstractTableModel *model)
{
    this->model = dynamic_cast<ProtocolPrinterItemModel*>(model);
    generateFilters(this->model->columnCount(), true);
    adjustPositions();
}

QSize ProtocolPrinterHeaderView::sizeHint() const
{
    QSize s = QHeaderView::sizeHint();
    if(LineVect.size())
        s.setHeight(s.height() + LineVect.at(0)->sizeHint().height() + 4); // The 4 adds just adds some extra space

    return s;
}

void ProtocolPrinterHeaderView::updateGeometries()
{
    if(LineVect.size())
        setViewportMargins(0, 0, 0, LineVect.at(0)->sizeHint().height());
    else
        setViewportMargins(0, 0, 0, 0);

    QHeaderView::updateGeometries();
    adjustPositions();
}

void ProtocolPrinterHeaderView::adjustPositions()
{
    for(int i=0; i < static_cast<int>(LineVect.size()); ++i)
    {
        QWidget* w = LineVect.at(i);

        int y = QHeaderView::sizeHint().height() + 2;
        if(QHeaderView::sizeHint().height())
        {
            if (QApplication::layoutDirection() == Qt::RightToLeft)
            {
                w->move(width() - (sectionPosition(i) + sectionSize(i) - offset()), y);
                adjPosHint = y;
            }
            else
            {
                w->move(sectionPosition(i) - offset(), y);
                adjPosHint = y;
            }
        }
        else
        {
            if (QApplication::layoutDirection() == Qt::RightToLeft)
                w->move(width() - (sectionPosition(i) + sectionSize(i) - offset()), adjPosHint);
            else
                w->move(sectionPosition(i) - offset(), adjPosHint);
        }
        w->resize(sectionSize(i), w->sizeHint().height());
    }
}

void ProtocolPrinterHeaderView::generateFilters(int number, bool showFirst)
{
    QStringList list;

    if(model2 != nullptr)
    {
        for(int i = 0; i < LineVect.size(); ++i)
        {
            if(LineVect[i] == nullptr)
                break;
            list.push_back(LineVect[i]->text());
        }
    }

    qDeleteAll(LineVect);
    LineVect.clear();

    for(int i=0; i < number; ++i)
    {
        LineEditExtended* l = new LineEditExtended(i, this);
        l->setPlaceholderText("Filter");

        if(!showFirst && i == 0)
            l->setVisible(false);
        else
            l->setVisible(true);

        connect(l, &QLineEdit::editingFinished, this, &ProtocolPrinterHeaderView::send);
        connect(l, &LineEditExtended::clearButtonPressed, this, &ProtocolPrinterHeaderView::send);
        LineVect.push_back(l);
    }

    for (int i = 0; i < LineVect.size() && i < list.size() ; ++i)
        LineVect[i]->setText(list[i]);

    LineVect.push_back(new LineEditExtended(number, this));
    adjustPositions();

    if(model != nullptr)
        emit updateFiltersComp();
}

void ProtocolPrinterHeaderView::send()
{
    LineEditExtended *line = dynamic_cast<LineEditExtended*>(QObject::sender());

    QString str = line->text();

    if((model->tableName() == tablesNamesList.at(0)) && (line->getNumber() == int(SpecColumnsNumb::SesRunDateTime)))//"Test Reports"
    {
        SQLQueryBuilder::checksFilter(model, str, line->getFilterMemory(), getFilterMemoryList(), line->getNumber(), SQLQueryBuilder::TypeColumn::DateTime);
        if(model2 != nullptr)
            setPrimaryFilter();
        return;
    }
    if((model->tableName() == tablesNamesList.at(0)) && (line->getNumber() == int(SpecColumnsNumb::SesRunTotalTime)))//"Test Reports"
    {
        SQLQueryBuilder::checksFilter(model,  str, line->getFilterMemory(), getFilterMemoryList(), line->getNumber(), SQLQueryBuilder::TypeColumn::DateTime);
        if(model2 != nullptr)
            setPrimaryFilter();
        return;
    }

    SQLQueryBuilder::checksFilter(model, str, line->getFilterMemory(), getFilterMemoryList(), line->getNumber(), SQLQueryBuilder::TypeColumn::String/*LineVect.size()*/);
    if(model2 != nullptr)
        setPrimaryFilter();
}


/**
 * Изменяет структуру работы, теперь влияет на еще одну модель
 **/
void ProtocolPrinterHeaderView::setSecondFilterDatabase(ProtocolPrinterItemModel *model2, ProtocolPrinterHeaderView *header2)
{
    this->model2 = model2;
    this->header2 = header2;
}
/**
 * Формирует строку SQL запроса для дополнительного фильтра, сохраняет в памяти и отправляет сигнал о изменении строки
 */
void ProtocolPrinterHeaderView::setPrimaryFilter(bool reset, QList<int> list)
{
    if(reset)
    {
        if(LineVect.size()>0)
        {
            header2->LineVect[header2->LineVect.size() - 1]->setFilterMemory("");
            header2->setFilter(header2->LineVect.size() - 1, "");
        }
        emit header2->LineVect[0]->editingFinished();
        return;
    }

    if(!list.size())
        list = SQLQueryBuilder::makePrimaryFilter(model);

    QString req="(`Session ID` IN (0";
    foreach (int value, list)
        req+= "," + QString::number(value);////??????!!!!!! РАБОТАЕТ НЕВЕРНО ????
    req+="))";

    primaryMemory = req;
    if(LineVect.size()>0)
    {
        header2->LineVect[header2->LineVect.size() - 1]->setFilterMemory(req);
        header2->setFilter(header2->LineVect.size() - 1, req);
    }
    emit header2->LineVect[0]->editingFinished();
}

QList<QString *> ProtocolPrinterHeaderView::getFilterMemoryList()
{
    QList<QString*> list;
    for(auto var : LineVect)
        list << &var->getFilterMemory();
    return list;
}

/**
 *  очищает дополнительный фильтр, сохраняет в памяти и отправляет сигнал об изменении строки
 */
void ProtocolPrinterHeaderView::updateFilter2()
{
    listSelect.clear();
    header2->LineVect[header2->LineVect.size() - 1]->setFilterMemory("");
    header2->setFilter(header2->LineVect.size() - 1, "");
    primaryMemory = "";
    emit header2->LineVect[0]->editingFinished();
}

void ProtocolPrinterHeaderView::rowSelect(QModelIndexList modelIndexListSelected, QModelIndexList modelIndexListDeselected)
{
    if(modelIndexListSelected.isEmpty() && modelIndexListDeselected.isEmpty())
    {
        listSelect.clear();
        return;
    }

    for(int i = 0; i < modelIndexListSelected.size(); i += LineVect.size() - 1)
    {
        for (int j = 0; j < listSelect.size(); ++j)
        {
            if(listSelect[j] == modelIndexListSelected.at(i).data().toInt())
            {
                listSelect.removeAt(j);
                j--;
            }
        }
        qDebug() << "list Select: " << listSelect;
        qDebug() << "list Select Model: " << modelIndexListSelected.at(i).data().toInt();
        listSelect << modelIndexListSelected.at(i).data().toInt();
    }

    for (int i = 0; i < modelIndexListDeselected.size(); i += LineVect.size() - 1) // LineVect.size() - 1 - количество колонок
    {
        listSelect.removeOne(modelIndexListDeselected.at(i).data().toInt());
    }
    if(listSelect.size() == 0)
    {
        setPrimaryFilter();
        return;
    }
    setPrimaryFilter(false, listSelect);
}

void ProtocolPrinterHeaderView::clearPrimaryMemory()
{
    primaryMemory = "";
}

void ProtocolPrinterHeaderView::linesUpdate()
{
    for (int i = 0; i < LineVect.size() ; ++i)
        LineVect[i]->setText(LineVect[i]->getFilterMemory());
}

void ProtocolPrinterHeaderView::clearFilters()
{
    for(QLineEdit* filterLineEdit : LineVect)
        filterLineEdit->clear();
}

void ProtocolPrinterHeaderView::setFilter(int column, const QString& value)
{
    if(column < LineVect.size())
        LineVect.at(column)->setText(value);
}
