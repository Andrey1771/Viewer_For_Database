#include "protocolprinterheaderview.h"
#include "lineeditextended.h"
#include "translatordata.h"

#include <QApplication>
#include <QSqlTableModel>
#include <QScrollBar>
#include <QSqlQuery>
#include <QTableView>

#include <QSqlRecord>
#include "protocolprinteritemmodel.h"
const QStringList TableNames{"Visual Inspections", "Suspect lists of SRUs", "Acceptance Test Reports", "Scenario Test Results"};// Те таблицы, которые будут поддерживать фильтр с временем

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
//

//
void ProtocolPrinterHeaderView::send()
{
    LineEditExtended *line = dynamic_cast<LineEditExtended*>(QObject::sender());

    QString str = line->text();
    if((this->model->tableName() == TableNames[0]) && (line->getNumber() == 7))//"Visual Inspections"
    {
        ProtocolPrinterHeaderView::checksFilter(str, line, 1);
        return;
    }

    if(this->model->tableName() == TableNames[1] && (line->getNumber() == 7))//"Suspect lists of SRUs"
    {
        ProtocolPrinterHeaderView::checksFilter(str, line, 2);
        return;
    }

    if(this->model->tableName() == TableNames[2] && (line->getNumber() == 9))//"Acceptance Test Reports"
    {
        ProtocolPrinterHeaderView::checksFilter(str, line, 3);
        return;
    }

    if(this->model->tableName() == TableNames[3] && (line->getNumber() == 8))//"Scenario Test Results"
    {
        ProtocolPrinterHeaderView::checksFilter(str, line, 4);
        return;
    }

    ProtocolPrinterHeaderView::checksFilter(str, line, 0);
}


void ProtocolPrinterHeaderView::checksFilter(const QString &str, LineEditExtended *line, int type)
{
    if(type == 0)//Для текста
    {
        if(str!="")
        {
            model->setFilter("");
            line->setFilterMemory(("`"+model->headerData(line->getNumber(), Qt::Orientation::Horizontal).toString()+"`"+" like '%"+str+"%'"));
            model->setFilter(filtersRQData());
        }
        else
        {
            model->setFilter("");
            line->setFilterMemory("");
            model->setFilter(filtersRQData());
        }
        if(model2 != nullptr)
            setPrimaryFilter(makePrimaryFilter());

        return;
    }
    if((type > 0 && type < 5))// Для времени
    {
        if(str.lastIndexOf("-") == -1)
        {
            if(str!="")
            {
                model->setFilter("");
                line->setFilterMemory(("`"+model->headerData(line->getNumber(),Qt::Orientation::Horizontal).toString()+"`"+" like '%"+str+"%'"));
                model->setFilter(filtersRQData());
            }
            else
            {
                model->setFilter("");
                line->setFilterMemory("");
                model->setFilter(filtersRQData());
            }
        }

        QStringList list;
        list = str.split("-");

        if(list.size() == 2) // 11/1/2016 9:11:04 - 05/2/2016 12:11:26
        {
            model->setFilter("");
            line->setFilterMemory(TranslatorData::check(list[0], list[1], line->getNumber(), model, "<="));// [a:b]
            model->setFilter(filtersRQData());
        }

        // > 11/1/2016 9:11:04
        list = str.split(">");
        if(list.size() == 2)
        {
            model->setFilter("");
            line->setFilterMemory(TranslatorData::check(list[1], "", line->getNumber(), model, ">", 1));
            model->setFilter(filtersRQData());
        }

        // < 11/1/2016 9:11:04
        list = str.split("<");
        if(list.size() == 2)
        {
            model->setFilter("");
            line->setFilterMemory(TranslatorData::check("", list[1], line->getNumber(), model, "<", 2));
            model->setFilter(filtersRQData());
        }

        // >= 11/1/2016 9:11:04
        list = str.split(">=");
        if(list.size() == 2)
        {
            model->setFilter("");
            line->setFilterMemory(TranslatorData::check(list[1], "", line->getNumber(), model, ">=", 1));
            model->setFilter(filtersRQData());
        }

        // <= 11/1/2016 9:11:04
        list = str.split("<=");
        if(list.size() == 2)
        {
            model->setFilter("");
            line->setFilterMemory(TranslatorData::check("", list[1], line->getNumber(), model, "<=", 2));
            model->setFilter(filtersRQData());
        }

        if(model2 != nullptr)// заменить
        {
            setPrimaryFilter(makePrimaryFilter());
        }

        return;
    }
}
/**
 * Формирует строку SQL запроса на основе сохр SQL запросов в памяти
 */
const QString ProtocolPrinterHeaderView::filtersRQData()
{
    QString str = "";
    bool k = false;
    for (int i = 0; i<LineVect.size(); ++i)
    {
        if(LineVect[i]->getFilterMemory() != "")
        {
            if(!k)
            {
                str += LineVect[i]->getFilterMemory();
                k = true;
            }
            else
                str += " AND " + LineVect[i]->getFilterMemory();
        }
    }

    return str;
}
//

//
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
void ProtocolPrinterHeaderView::setPrimaryFilter(const QList<int> &list, bool reset)
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

    QString req="(`Session ID` IN (0";

    foreach (int value, list)
        req+= "," + QString::number(value);
    req+="))";

    primaryMemory = req;
    if(LineVect.size()>0)
    {
        header2->LineVect[header2->LineVect.size() - 1]->setFilterMemory(req);
        header2->setFilter(header2->LineVect.size() - 1, req);
    }
    emit header2->LineVect[0]->editingFinished();
}
/**
 * Формирует строку для дополнительного фильтра
 */
QList<int> ProtocolPrinterHeaderView::makePrimaryFilter()
{
    QList <int> list;
    int i = 0;

    while(true)
    {
        if(!model->record(i).value("Session ID").isValid())
            return list;
        list << model->record(i).value("Session ID").toInt();
        i++;
    }
}
/**
 *  очищает дополнительный фильтр, сохраняет в памяти и отправляет сигнал о изменении строки
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
        //listSelect.clear(); //
        //setPrimaryFilter(makePrimaryFilter(), true); //сброс
        return;
    }

    for(int i = 0; i < modelIndexListSelected.size(); i += LineVect.size() - 1)
    {
        for (int j = 0; j < listSelect.size(); ++j)
        {
            if(listSelect[j] == modelIndexListSelected.at(i).row() + 1)
            {
                listSelect.removeAt(j);
                j--;
            }
        }
        listSelect << modelIndexListSelected.at(i).row() + 1;
    }

    for (int i = 0; i < modelIndexListDeselected.size(); i += LineVect.size() - 1) // LineVect.size() - 1 - количество колонок
    {
        listSelect.removeOne(modelIndexListDeselected.at(i).row() + 1);
    }
    if(listSelect.size() == 0)
    {
        setPrimaryFilter(makePrimaryFilter());
        return;
    }
    setPrimaryFilter(listSelect);
}

void ProtocolPrinterHeaderView::clearPrimaryMemory()
{
    primaryMemory = "";
}
//

//
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
//
