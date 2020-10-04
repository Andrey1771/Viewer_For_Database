#include "sqlquerybuilder.h"
#include "translatordata.h"
#include "protocolprinteritemmodel.h"
#include "protocolprinterheaderview.h"

#include <QSqlRecord>

#include <QDebug>

void SQLQueryBuilder::checksFilter(QList<QString>& sessionIdList/*Может быть пустым*/, QList<QString> headerList, const QString &str, QString &filterMemory, QList<QDateTime>& listAllDateTimeDb, QList<QString*> filterMemoryList, int lineNumber, TypeColumn type)
{//Обладает меньшим функционалом
    qDebug() << "STR: " << str;
    switch (type)
    {
    case TypeColumn::String:
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
        break;
    }
    case TypeColumn::DateTime:
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
            filterMemory = TranslatorData::checkDateTimeWithoutModel(sessionIdList, list[0], list[1], listAllDateTimeDb, "<=>=");// [a:b]
            filtersRQData(filterMemoryList);
        }// 11/1/2000 9:11:04 - 05/2/2033 12:11:26

        // > 11/1/2016 9:11:04
        list = str.split(">");
        if(list.size() == 2)
        {
            filterMemory = (TranslatorData::checkDateTimeWithoutModel(sessionIdList, list[1], "", listAllDateTimeDb, ">", TranslatorData::MaxMinDateTime::Max));
            filtersRQData(filterMemoryList);
        }

        // < 11/1/2016 9:11:04
        list = str.split("<");
        if(list.size() == 2)
        {
            filterMemory = (TranslatorData::checkDateTimeWithoutModel(sessionIdList, "", list[1], listAllDateTimeDb, "<", TranslatorData::MaxMinDateTime::Min));
            filtersRQData(filterMemoryList);
        }

        // >= 11/1/2016 9:11:04
        list = str.split(">=");
        if(list.size() == 2)
        {
            filterMemory = (TranslatorData::checkDateTimeWithoutModel(sessionIdList, list[1], "", listAllDateTimeDb, ">=", TranslatorData::MaxMinDateTime::Max));
            filtersRQData(filterMemoryList);
        }

        // <= 11/1/2016 9:11:04
        list = str.split("<=");
        if(list.size() == 2)
        {
            filterMemory = (TranslatorData::checkDateTimeWithoutModel(sessionIdList, "", list[1], listAllDateTimeDb, "<=", TranslatorData::MaxMinDateTime::Min));
            filtersRQData(filterMemoryList);
        }
        break;
    }
    case TypeColumn::Time:
    {
        break;
    }
    case TypeColumn::Value:
    {
        break;
    }
    default:
    {
        qDebug() << "Такого типа пока еще нет";
        break;
    }
    }

    return;
}
void SQLQueryBuilder::checksFilter(ProtocolPrinterItemModel *model, const QString &str, QString &filterMemory, QList<QString*> filterMemoryList, int lineNumber, TypeColumn type)
{
    qDebug() << "STR: " << str;
    switch (type)
    {
    case TypeColumn::String:
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
        break;
    }
    case TypeColumn::DateTime:
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
            filterMemory = (TranslatorData::checkDateTimeWithModel(list[0], list[1], lineNumber, model, "<=>="));// [a:b]
            model->setFilter(filtersRQData(filterMemoryList));
        }

        // > 11/1/2016 9:11:04
        list = str.split(">");
        if(list.size() == 2)
        {
            model->setFilter("");
            filterMemory = (TranslatorData::checkDateTimeWithModel(list[1], "", lineNumber, model, ">", TranslatorData::MaxMinDateTime::Max));
            model->setFilter(filtersRQData(filterMemoryList));
        }

        // < 11/1/2016 9:11:04
        list = str.split("<");
        if(list.size() == 2)
        {
            model->setFilter("");
            filterMemory = (TranslatorData::checkDateTimeWithModel("", list[1], lineNumber, model, "<", TranslatorData::MaxMinDateTime::Min));
            model->setFilter(filtersRQData(filterMemoryList));
        }

        // >= 11/1/2016 9:11:04
        list = str.split(">=");
        if(list.size() == 2)
        {
            model->setFilter("");
            filterMemory = (TranslatorData::checkDateTimeWithModel(list[1], "", lineNumber, model, ">=", TranslatorData::MaxMinDateTime::Max));
            model->setFilter(filtersRQData(filterMemoryList));
        }

        // <= 11/1/2016 9:11:04
        list = str.split("<=");
        if(list.size() == 2)
        {
            model->setFilter("");
            filterMemory = (TranslatorData::checkDateTimeWithModel("", list[1], lineNumber, model, "<=", TranslatorData::MaxMinDateTime::Min));
            model->setFilter(filtersRQData(filterMemoryList));
        }
        break;
    }
    case TypeColumn::Time:
    {
        break;
    }
    case TypeColumn::Value:
    {
        if(str.lastIndexOf("-") == -1)
        {
            bool ok = false;
            str.toInt(&ok);
            if(ok)
            {
                model->setFilter("");
                filterMemory = (("`"+model->headerData(lineNumber, Qt::Orientation::Horizontal).toString() + " = "+str));
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
            filterMemory = (TranslatorData::checkDateTimeWithModel(list[0], list[1], lineNumber, model, "<=>="));// [a:b]
            model->setFilter(filtersRQData(filterMemoryList));
        }

        // > 11/1/2016 9:11:04
        list = str.split(">");
        if(list.size() == 2)
        {
            model->setFilter("");
            filterMemory = (TranslatorData::checkDateTimeWithModel(list[1], "", lineNumber, model, ">", TranslatorData::MaxMinDateTime::Max));
            model->setFilter(filtersRQData(filterMemoryList));
        }

        // < 11/1/2016 9:11:04
        list = str.split("<");
        if(list.size() == 2)
        {
            model->setFilter("");
            filterMemory = (TranslatorData::checkDateTimeWithModel("", list[1], lineNumber, model, "<", TranslatorData::MaxMinDateTime::Min));
            model->setFilter(filtersRQData(filterMemoryList));
        }

        // >= 11/1/2016 9:11:04
        list = str.split(">=");
        if(list.size() == 2)
        {
            model->setFilter("");
            filterMemory = (TranslatorData::checkDateTimeWithModel(list[1], "", lineNumber, model, ">=", TranslatorData::MaxMinDateTime::Max));
            model->setFilter(filtersRQData(filterMemoryList));
        }

        // <= 11/1/2016 9:11:04
        list = str.split("<=");
        if(list.size() == 2)
        {
            model->setFilter("");
            filterMemory = (TranslatorData::checkDateTimeWithModel("", list[1], lineNumber, model, "<=", TranslatorData::MaxMinDateTime::Min));
            model->setFilter(filtersRQData(filterMemoryList));
        }
        break;
    }
    default:
    {
        qDebug() << "Такого типа пока еще нет";
        break;
    }
    }
    return;
}
/**
 * Формирует строку SQL запроса на основе сохр SQL запросов в памяти
 */
QString SQLQueryBuilder::filtersRQData(QList<QString*> filterMemoryList)
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
 * Формирует строку для дополнительного фильтра
 */
QList<int> SQLQueryBuilder::makePrimaryFilter(ProtocolPrinterItemModel* model)
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
