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
//

//
void ProtocolPrinterHeaderView::send()
{
    LineEditExtended *line = dynamic_cast<LineEditExtended*>(QObject::sender());

    QString str = line->text();

    if((model->tableName() == tablesNamesList.at(0)) && (line->getNumber() == int(SpecColumnsNumb::SesRunDateTime)))//"Test Reports"
    {
        ProtocolPrinterHeaderView::checksFilter(model, model2, this,  str, line->getFilterMemory(), getFilterMemoryList(/*LineVect*/), line->getNumber(), 1);
        if(model2 != nullptr)
            setPrimaryFilter(this, makePrimaryFilter(model));
        return;
    }
    if((model->tableName() == tablesNamesList.at(0)) && (line->getNumber() == int(SpecColumnsNumb::SesRunTotalTime)))//"Test Reports"
    {
        ProtocolPrinterHeaderView::checksFilter(model, model2, this,  str, line->getFilterMemory(), getFilterMemoryList(/*LineVect*/), line->getNumber(), 1);
        if(model2 != nullptr)
            setPrimaryFilter(this, makePrimaryFilter(model));
        return;
    }

    ProtocolPrinterHeaderView::checksFilter(model, model2, this, str, line->getFilterMemory(), getFilterMemoryList(), line->getNumber(), 0/*LineVect.size()*/);
    if(model2 != nullptr)
        setPrimaryFilter(this, makePrimaryFilter(model));// не уверен, посмотреть в старых коммитах
}

QList<QString *> ProtocolPrinterHeaderView::getFilterMemoryList()
{
    QList<QString*> list;
    for(auto var : LineVect)
        list << &var->getFilterMemory();
    return list;
}
///TODO перенести в отдельный класс, избавиться от лишних переменных, а также избавиться от лишней функции
void ProtocolPrinterHeaderView::checksFilter(QList<QString>& sessionIdList/*Может быть пустым*/, QList<QString> headerList, const QString &str, QString &filterMemory, QList<QDateTime>& listAllDateTimeDb, QList<QString*> filterMemoryList, int lineNumber, int type)
{//Обладает меньшим функционалом
    qDebug() << "STR: " << str;
    if(type == 0)//Для текста
    {
        if(str!="")
        {

            filterMemory = (("`" + headerList.at(lineNumber) + "`"+" like '%" + str+"%'"));
            filtersRQData(filterMemoryList);
        }
        else
        {
            filterMemory = ("");
            filtersRQData(filterMemoryList);
        }


        return;
    }
    if((type > 0 && type < 5))// Для времени
    {
        if(str.lastIndexOf("-") == -1)
        {
            if(str!="")
            {
                filterMemory = (("`" + headerList.at(lineNumber) + "`"+" like '%" + str + "%'"));
                filtersRQData(filterMemoryList);
            }
            else
            {
                filterMemory = ("");
                filtersRQData(filterMemoryList);
            }
        }

        QStringList list;
        list = str.split("-");

        if(list.size() == 2) // 11/1/2016 9:11:04 - 05/2/2016 12:11:26
        {
            filterMemory = TranslatorData::checkWithoutModel(sessionIdList, list[0], list[1], listAllDateTimeDb, "<=");// [a:b]
            filtersRQData(filterMemoryList);
        }

        // > 11/1/2016 9:11:04
        list = str.split(">");
        if(list.size() == 2)
        {
            filterMemory = (TranslatorData::checkWithoutModel(sessionIdList, list[1], "", listAllDateTimeDb, ">", TranslatorData::MaxMinDateTime::Max));
            filtersRQData(filterMemoryList);
        }

        // < 11/1/2016 9:11:04
        list = str.split("<");
        if(list.size() == 2)
        {
            filterMemory = (TranslatorData::checkWithoutModel(sessionIdList, "", list[1], listAllDateTimeDb, "<", TranslatorData::MaxMinDateTime::Min));
            filtersRQData(filterMemoryList);
        }

        // >= 11/1/2016 9:11:04
        list = str.split(">=");
        if(list.size() == 2)
        {
            filterMemory = (TranslatorData::checkWithoutModel(sessionIdList, list[1], "", listAllDateTimeDb, ">=", TranslatorData::MaxMinDateTime::Max));
            filtersRQData(filterMemoryList);
        }

        // <= 11/1/2016 9:11:04
        list = str.split("<=");
        if(list.size() == 2)
        {
            filterMemory = (TranslatorData::checkWithoutModel(sessionIdList, "", list[1], listAllDateTimeDb, "<=", TranslatorData::MaxMinDateTime::Min));
            filtersRQData(filterMemoryList);
        }

        return;
    }
}
void ProtocolPrinterHeaderView::checksFilter(ProtocolPrinterItemModel *model, ProtocolPrinterItemModel* model2, ProtocolPrinterHeaderView *headerView, const QString &str, QString &filterMemory, QList<QString*> filterMemoryList, int lineNumber, int type)
{///TODO перенести в отдельный класс, избавиться от лишних переменных
    qDebug() << "STR: " << str;
    if(type == 0)//Для текста
    {
        if(str!="")
        {
            model->setFilter("");
            filterMemory = (("`"+model->headerData(lineNumber, Qt::Orientation::Horizontal).toString()+"`"+" like '%"+str+"%'"));
            model->setFilter(filtersRQData(filterMemoryList));
        }
        else
        {
            model->setFilter("");
            filterMemory = ("");
            model->setFilter(filtersRQData(filterMemoryList));
        }

        if(model2 != nullptr)
            setPrimaryFilter(headerView, makePrimaryFilter(model));// не уверен, посмотреть в старых коммитах

        return;
    }
    if((type > 0 && type < 5))// Для времени
    {
        if(str.lastIndexOf("-") == -1)
        {
            if(str!="")
            {
                model->setFilter("");
                filterMemory = (("`"+model->headerData(lineNumber, Qt::Orientation::Horizontal).toString()+"`"+" like '%"+str+"%'"));
                model->setFilter(filtersRQData(filterMemoryList));
            }
            else
            {
                model->setFilter("");
                filterMemory = ("");
                model->setFilter(filtersRQData(filterMemoryList));
            }
        }

        QStringList list;
        list = str.split("-");

        if(list.size() == 2) // 11/1/2016 9:11:04 - 05/2/2016 12:11:26
        {                   // 11/1/2016 9:11:04 - 05/2/2026 12:11:26
            model->setFilter("");
            filterMemory = (TranslatorData::checkWithModel(list[0], list[1], lineNumber, model, "<="));// [a:b]
            model->setFilter(filtersRQData(filterMemoryList));
        }

        // > 11/1/2016 9:11:04
        list = str.split(">");
        if(list.size() == 2)
        {
            model->setFilter("");
            filterMemory = (TranslatorData::checkWithModel(list[1], "", lineNumber, model, ">", TranslatorData::MaxMinDateTime::Max));
            model->setFilter(filtersRQData(filterMemoryList));
        }

        // < 11/1/2016 9:11:04
        list = str.split("<");
        if(list.size() == 2)
        {
            model->setFilter("");
            filterMemory = (TranslatorData::checkWithModel("", list[1], lineNumber, model, "<", TranslatorData::MaxMinDateTime::Min));
            model->setFilter(filtersRQData(filterMemoryList));
        }

        // >= 11/1/2016 9:11:04
        list = str.split(">=");
        if(list.size() == 2)
        {
            model->setFilter("");
            filterMemory = (TranslatorData::checkWithModel(list[1], "", lineNumber, model, ">=", TranslatorData::MaxMinDateTime::Max));
            model->setFilter(filtersRQData(filterMemoryList));
        }

        // <= 11/1/2016 9:11:04
        list = str.split("<=");
        if(list.size() == 2)
        {
            model->setFilter("");
            filterMemory = (TranslatorData::checkWithModel("", list[1], lineNumber, model, "<=", TranslatorData::MaxMinDateTime::Min));
            model->setFilter(filtersRQData(filterMemoryList));
        }

        if(model2 != nullptr)// заменить
        {
            setPrimaryFilter(headerView, makePrimaryFilter(model));// не уверен, посмотреть в старых коммитах
        }
        return;
    }
}

/**
 * Формирует строку SQL запроса на основе сохр SQL запросов в памяти
 */
QString ProtocolPrinterHeaderView::filtersRQData(QList<QString*> filterMemoryList)
{
    QString str = "";
    bool k = false;
    for (auto filterMemory : filterMemoryList)
    {
        if(*filterMemory != "")
        {
            if(!k)
            {
                str += *filterMemory;
                k = true;
            }
            else
                str += " AND " + *filterMemory;
        }
    }
    qDebug() << "SUPER STR: " << str;
    return str;
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
void ProtocolPrinterHeaderView::setPrimaryFilter(ProtocolPrinterHeaderView* headerView, const QList<int> &list, bool reset)
{
    if(reset)
    {
        if(headerView->LineVect.size()>0)
        {
            headerView->header2->LineVect[headerView->header2->LineVect.size() - 1]->setFilterMemory("");
            headerView->header2->setFilter(headerView->header2->LineVect.size() - 1, "");
        }
        emit headerView->header2->LineVect[0]->editingFinished();
        return;
    }

    QString req="(`Session ID` IN (0";

    foreach (int value, list)
        req+= "," + QString::number(value);
    req+="))";

    headerView->primaryMemory = req;
    if(headerView->LineVect.size()>0)
    {
        headerView->header2->LineVect[headerView->header2->LineVect.size() - 1]->setFilterMemory(req);
        headerView->header2->setFilter(headerView->header2->LineVect.size() - 1, req);
    }
    emit headerView->header2->LineVect[0]->editingFinished();
}
/**
 * Формирует строку для дополнительного фильтра
 */
QList<int> ProtocolPrinterHeaderView::makePrimaryFilter(ProtocolPrinterItemModel* model)
{
    QList <int> list;
    int i = 0;

    while(model->record(i).value("Session ID").isValid())
    {
        list << model->record(i).value("Session ID").toInt();
        qDebug() << "List: " << list;
        i++;
    }
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
        listSelect.clear(); //
        //if(model != nullptr && )
        //    setPrimaryFilter(this, makePrimaryFilter(model), true); //сброс
        return;         // не уверен, посмотреть в старых коммитах
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
        setPrimaryFilter(this, makePrimaryFilter(model));
        return;
    }
    setPrimaryFilter(this, listSelect);
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
