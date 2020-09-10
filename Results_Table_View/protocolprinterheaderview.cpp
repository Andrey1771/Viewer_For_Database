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
    enum class SpecColumnsNumb{
        VisualInspections = 7,
        SuspectlistsofSRUs = 7,
        AcceptanceTestReports = 9,
        ScenarioTestResults = 8
    };


    LineEditExtended *line = dynamic_cast<LineEditExtended*>(QObject::sender());

    QString str = line->text();
    qDebug() << "Номер " << line->getNumber();
    if((this->model->tableName() == TableNames[0]) && (line->getNumber() == int(SpecColumnsNumb::VisualInspections)))//"Visual Inspections"
    {

        ProtocolPrinterHeaderView::checksFilter(model, model2, str, line->getFilterMemory(), getFilterMemoryList(/*LineVect*/), line->getNumber(), LineVect.size(), 1);
        if(model2 != nullptr)
            setPrimaryFilter(makePrimaryFilter(model));
        return;
    }

    if(this->model->tableName() == TableNames[1] && (line->getNumber() == int(SpecColumnsNumb::SuspectlistsofSRUs)))//"Suspect lists of SRUs"
    {
        ProtocolPrinterHeaderView::checksFilter(model, model2, str, line->getFilterMemory(), getFilterMemoryList(), line->getNumber(), LineVect.size(), 2);
        if(model2 != nullptr)
            setPrimaryFilter(makePrimaryFilter(model));
        return;
    }

    if(this->model->tableName() == TableNames[2] && (line->getNumber() == int(SpecColumnsNumb::AcceptanceTestReports)))//"Acceptance Test Reports"
    {
        ProtocolPrinterHeaderView::checksFilter(model, model2, str, line->getFilterMemory(), getFilterMemoryList(), line->getNumber(), LineVect.size(), 3);
        if(model2 != nullptr)
            setPrimaryFilter(makePrimaryFilter(model));
        return;
    }

    if(this->model->tableName() == TableNames[3] && (line->getNumber() == int(SpecColumnsNumb::ScenarioTestResults)))//"Scenario Test Results"
    {
        ProtocolPrinterHeaderView::checksFilter(model, model2, str, line->getFilterMemory(), getFilterMemoryList(), line->getNumber(), LineVect.size(), 4);
        if(model2 != nullptr)
            setPrimaryFilter(makePrimaryFilter(model));
        return;
    }

    ProtocolPrinterHeaderView::checksFilter(model, model2, str, line->getFilterMemory(), getFilterMemoryList(), line->getNumber(), LineVect.size(), 0);
    if(model2 != nullptr)
        setPrimaryFilter(makePrimaryFilter(model));
}

//QList<QString&> ProtocolPrinterHeaderView::getFilterMemoryList(QVector<LineEditExtended *> lineVect)
//{
//    QList<QString&> list;
//    for(auto var : lineVect)
//        list << var->getFilterMemory();
//    return list;
//}

QList<QString *> ProtocolPrinterHeaderView::getFilterMemoryList()
{
    QList<QString*> list;
    for(auto var : LineVect)
        list << &var->getFilterMemory();
    return list;
}
///TODO перенести в отдельный класс, избавиться от лишних переменных, а также избавиться от лишней функции
void ProtocolPrinterHeaderView::checksFilter(QList<QString>headerList, const QString &str, QString &filterMemory, QList<QString *> filterMemoryList, int lineNumber, int count, int type)
{
//    qDebug() << "STR: " << str;
//    if(type == 0)//Для текста
//    {
//        if(str!="")
//        {
//            model->setFilter("");
//            filterMemory = (("`"+model->headerData(lineNumber, Qt::Orientation::Horizontal).toString()+"`"+" like '%"+str+"%'"));
//            model->setFilter(filtersRQData(filterMemoryList));
//        }
//        else
//        {
//            model->setFilter("");
//            filterMemory = ("");
//            model->setFilter(filtersRQData(filterMemoryList));
//        }


//        return;
//    }
//    if((type > 0 && type < 5))// Для времени
//    {
//        if(str.lastIndexOf("-") == -1)
//        {
//            if(str!="")
//            {
//                model->setFilter("");
//                filterMemory = (("`"+model->headerData(lineNumber, Qt::Orientation::Horizontal).toString()+"`"+" like '%"+str+"%'"));
//                model->setFilter(filtersRQData(filterMemoryList));
//            }
//            else
//            {
//                model->setFilter("");
//                filterMemory = ("");
//                model->setFilter(filtersRQData(filterMemoryList));
//            }
//        }

//        QStringList list;
//        list = str.split("-");

//        if(list.size() == 2) // 11/1/2016 9:11:04 - 05/2/2016 12:11:26
//        {
//            model->setFilter("");
//            filterMemory = (TranslatorData::check(list[0], list[1], lineNumber, model, "<="));// [a:b]
//            model->setFilter(filtersRQData(filterMemoryList));
//        }

//        // > 11/1/2016 9:11:04
//        list = str.split(">");
//        if(list.size() == 2)
//        {
//            model->setFilter("");
//            filterMemory = (TranslatorData::check(list[1], "", lineNumber, model, ">", 1));
//            model->setFilter(filtersRQData(filterMemoryList));
//        }

//        // < 11/1/2016 9:11:04
//        list = str.split("<");
//        if(list.size() == 2)
//        {
//            model->setFilter("");
//            filterMemory = (TranslatorData::check("", list[1], lineNumber, model, "<", 2));
//            model->setFilter(filtersRQData(filterMemoryList));
//        }

//        // >= 11/1/2016 9:11:04
//        list = str.split(">=");
//        if(list.size() == 2)
//        {
//            model->setFilter("");
//            filterMemory = (TranslatorData::check(list[1], "", lineNumber, model, ">=", 1));
//            model->setFilter(filtersRQData(filterMemoryList));
//        }

//        // <= 11/1/2016 9:11:04
//        list = str.split("<=");
//        if(list.size() == 2)
//        {
//            model->setFilter("");
//            filterMemory = (TranslatorData::check("", list[1], lineNumber, model, "<=", 2));
//            model->setFilter(filtersRQData(filterMemoryList));
//        }

//        return;
//    }
}
void ProtocolPrinterHeaderView::checksFilter(ProtocolPrinterItemModel *model, ProtocolPrinterItemModel* model2, const QString &str, QString &filterMemory, QList<QString *> filterMemoryList, int lineNumber, int count, int type)
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
        {
            model->setFilter("");
            filterMemory = (TranslatorData::check(list[0], list[1], lineNumber, model, "<="));// [a:b]
            model->setFilter(filtersRQData(filterMemoryList));
        }

        // > 11/1/2016 9:11:04
        list = str.split(">");
        if(list.size() == 2)
        {
            model->setFilter("");
            filterMemory = (TranslatorData::check(list[1], "", lineNumber, model, ">", 1));
            model->setFilter(filtersRQData(filterMemoryList));
        }

        // < 11/1/2016 9:11:04
        list = str.split("<");
        if(list.size() == 2)
        {
            model->setFilter("");
            filterMemory = (TranslatorData::check("", list[1], lineNumber, model, "<", 2));
            model->setFilter(filtersRQData(filterMemoryList));
        }

        // >= 11/1/2016 9:11:04
        list = str.split(">=");
        if(list.size() == 2)
        {
            model->setFilter("");
            filterMemory = (TranslatorData::check(list[1], "", lineNumber, model, ">=", 1));
            model->setFilter(filtersRQData(filterMemoryList));
        }

        // <= 11/1/2016 9:11:04
        list = str.split("<=");
        if(list.size() == 2)
        {
            model->setFilter("");
            filterMemory = (TranslatorData::check("", list[1], lineNumber, model, "<=", 2));
            model->setFilter(filtersRQData(filterMemoryList));
        }

        return;
    }
}
//void ProtocolPrinterHeaderView::checksFilter(const QString &str, LineEditExtended *line, int type)
//{
//    qDebug() << "STR: " <<str;
//    if(type == 0)//Для текста
//    {
//        if(str!="")
//        {
//            model->setFilter("");
//            line->setFilterMemory(("`"+model->headerData(line->getNumber(), Qt::Orientation::Horizontal).toString()+"`"+" like '%"+str+"%'"));
//            model->setFilter(filtersRQData());
//        }
//        else
//        {
//            model->setFilter("");
//            line->setFilterMemory("");
//            model->setFilter(filtersRQData());
//        }
//        if(model2 != nullptr)
//            setPrimaryFilter(makePrimaryFilter());

//        return;
//    }
//    if((type > 0 && type < 5))// Для времени
//    {
//        if(str.lastIndexOf("-") == -1)
//        {
//            if(str!="")
//            {
//                model->setFilter("");
//                line->setFilterMemory(("`"+model->headerData(line->getNumber(),Qt::Orientation::Horizontal).toString()+"`"+" like '%"+str+"%'"));
//                model->setFilter(filtersRQData());
//            }
//            else
//            {
//                model->setFilter("");
//                line->setFilterMemory("");
//                model->setFilter(filtersRQData());
//            }
//        }

//        QStringList list;
//        list = str.split("-");

//        if(list.size() == 2) // 11/1/2016 9:11:04 - 05/2/2016 12:11:26
//        {
//            model->setFilter("");
//            line->setFilterMemory(TranslatorData::check(list[0], list[1], line->getNumber(), model, "<="));// [a:b]
//            model->setFilter(filtersRQData());
//        }

//        // > 11/1/2016 9:11:04
//        list = str.split(">");
//        if(list.size() == 2)
//        {
//            model->setFilter("");
//            line->setFilterMemory(TranslatorData::check(list[1], "", line->getNumber(), model, ">", 1));
//            model->setFilter(filtersRQData());
//        }

//        // < 11/1/2016 9:11:04
//        list = str.split("<");
//        if(list.size() == 2)
//        {
//            model->setFilter("");
//            line->setFilterMemory(TranslatorData::check("", list[1], line->getNumber(), model, "<", 2));
//            model->setFilter(filtersRQData());
//        }

//        // >= 11/1/2016 9:11:04
//        list = str.split(">=");
//        if(list.size() == 2)
//        {
//            model->setFilter("");
//            line->setFilterMemory(TranslatorData::check(list[1], "", line->getNumber(), model, ">=", 1));
//            model->setFilter(filtersRQData());
//        }

//        // <= 11/1/2016 9:11:04
//        list = str.split("<=");
//        if(list.size() == 2)
//        {
//            model->setFilter("");
//            line->setFilterMemory(TranslatorData::check("", list[1], line->getNumber(), model, "<=", 2));
//            model->setFilter(filtersRQData());
//        }

//        if(model2 != nullptr)// заменить
//        {
//            setPrimaryFilter(makePrimaryFilter());
//        }

//        return;
//    }
//}
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
    qDebug() << str;
    return str;
}
/*const QString ProtocolPrinterHeaderView::filtersRQData()
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
    qDebug() << str;
    return str;
}*/
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
QList<int> ProtocolPrinterHeaderView::makePrimaryFilter(ProtocolPrinterItemModel* model)
{
    QList <int> list;
    int i = 0;

    while(true)
    {
        if(!model->record(i).value("Session ID").isValid())
            return list;
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
        //listSelect.clear(); //
        //setPrimaryFilter(makePrimaryFilter(), true); //сброс
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
        setPrimaryFilter(makePrimaryFilter(model));
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
