#include "sqlquerybuilder.h"
#include "translatordata.h"
#include "protocolprinteritemmodel.h"
#include "protocolprinterheaderview.h"

#include <QSqlRecord>
#include <QDebug>

void SQLQueryBuilder::checksFilter(QList<QString>& sessionIdList/*Может быть пустым*/, QList<QString> headerList, const QString &str, QString &filterMemory, QList<QDateTime>& listAllDateTimeDb, QList<int> valuesList, QList<QString*> filterMemoryList, int lineNumber, TypeColumn type)
{//Обладает меньшим функционалом
    qDebug() << "STR: " << str;
    switch (type)
    {
    case TypeColumn::String:
    {
        if(str!="")
        {

            filterMemory = (("`" + headerList.at(lineNumber) + "`"+" like '%" + str+"%'"));
        }
        else
        {
            filterMemory = ("");
        }
        break;
    }
    case TypeColumn::DateTime:
    {
        QStringList list;
        list = str.split("-");

        if(list.size() == 2) // 11/1/2016 9:11:04 - 05/2/2016 12:11:26
        {
            filterMemory = TranslatorData::checkDateTimeWithoutModel(sessionIdList, list[0], list[1], listAllDateTimeDb, "<=>=");// [a:b]
            break;
        }// 11/1/2000 9:11:04 - 05/2/2033 12:11:26

        // > 11/1/2016 9:11:04
        list = str.split(">");
        if(list.size() == 2)
        {
            filterMemory = (TranslatorData::checkDateTimeWithoutModel(sessionIdList, list[1], "", listAllDateTimeDb, ">", TranslatorData::MaxMinType::Max));
            break;
        }

        // < 11/1/2016 9:11:04
        list = str.split("<");
        if(list.size() == 2)
        {
            filterMemory = (TranslatorData::checkDateTimeWithoutModel(sessionIdList, "", list[1], listAllDateTimeDb, "<", TranslatorData::MaxMinType::Min));
            break;
        }

        // >= 11/1/2016 9:11:04
        list = str.split(">=");
        if(list.size() == 2)
        {
            filterMemory = (TranslatorData::checkDateTimeWithoutModel(sessionIdList, list[1], "", listAllDateTimeDb, ">=", TranslatorData::MaxMinType::Max));
            break;
        }

        // <= 11/1/2016 9:11:04
        list = str.split("<=");
        if(list.size() == 2)
        {
            filterMemory = (TranslatorData::checkDateTimeWithoutModel(sessionIdList, "", list[1], listAllDateTimeDb, "<=", TranslatorData::MaxMinType::Min));
            break;
        }

        if(str.lastIndexOf("-") == -1)
        {
            if(str!="")
            {
                filterMemory = (("`" + headerList.at(lineNumber) + "`"+" like '%" + str + "%'"));
            }
            else
            {
                filterMemory = ("");
            }
            break;
        }
        break;
    }
    case TypeColumn::Time:
    {
        break;
    }
    case TypeColumn::Value:
    {
        QStringList list;
        list = str.split(":");

        if(list.size() == 2) // 11/1/2016 9:11:04 - 05/2/2016 12:11:26
        {                   // 11/1/2016 9:11:04 - 05/2/2026 12:11:26
            filterMemory = TranslatorData::checkValueWithoutModel(sessionIdList, list.at(0).toInt(), list.at(1).toInt(), valuesList, "<=>=");// [a:b]
            break;
        }

        // > 11/1/2016 9:11:04
        list = str.split(">");
        if(list.size() == 2)
        {
            filterMemory = (TranslatorData::checkValueWithoutModel(sessionIdList, list.at(1).toInt(), -1/*не используется*/, valuesList, ">", TranslatorData::MaxMinType::Max));
            break;
        }

        // < 11/1/2016 9:11:04
        list = str.split("<");
        if(list.size() == 2)
        {
            filterMemory = (TranslatorData::checkValueWithoutModel(sessionIdList, list.at(1).toInt(), -1/*не используется*/, valuesList, "<", TranslatorData::MaxMinType::Min));
            break;
        }

        // >= 11/1/2016 9:11:04
        list = str.split(">=");
        if(list.size() == 2)
        {
            filterMemory = (TranslatorData::checkValueWithoutModel(sessionIdList, list.at(1).toInt(), -1/*не используется*/, valuesList, ">=", TranslatorData::MaxMinType::Max));
            break;
        }

        // <= 11/1/2016 9:11:04
        list = str.split("<=");
        if(list.size() == 2)
        {
            filterMemory = (TranslatorData::checkValueWithoutModel(sessionIdList, list.at(1).toInt(), -1/*не используется*/, valuesList, "<=", TranslatorData::MaxMinType::Min));
            break;
        }

        if(str.lastIndexOf(":") == -1)
        {
            bool ok = false;
            str.toInt(&ok);
            if(ok)
            {
                filterMemory = (TranslatorData::checkValueWithoutModel(sessionIdList, str.toInt(), -1/*не используется*/, valuesList, ""));//TranslatorData::fillListAllValueDb(valuesList, )
            }
            else
            {
                filterMemory = ("");
            }
            break;
        }
        break;
    }
    }
    filtersRQData(filterMemoryList);
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
            filterMemory = (("`"+model->headerData(lineNumber, Qt::Orientation::Horizontal).toString()+"`"+" like '%"+str+"%'"));
        }
        else
        {
            filterMemory = ("");
        }
        break;
    }
    case TypeColumn::DateTime:
    {
        QStringList list;
        list = str.split("-");

        if(list.size() == 2) // 11/1/2016 9:11:04 - 05/2/2016 12:11:26
        {                   // 11/1/2016 9:11:04 - 05/2/2026 12:11:26
            filterMemory = (TranslatorData::checkDateTimeWithModel(list[0], list[1], lineNumber, model, "<=>="));// [a:b]
            break;
        }

        // >= 11/1/2016 9:11:04
        list = str.split(">=");
        if(list.size() == 2)
        {
            filterMemory = (TranslatorData::checkDateTimeWithModel(list[1], "", lineNumber, model, ">=", TranslatorData::MaxMinType::Max));
            break;
        }

        // <= 11/1/2016 9:11:04
        list = str.split("<=");
        if(list.size() == 2)
        {
            filterMemory = (TranslatorData::checkDateTimeWithModel("", list[1], lineNumber, model, "<=", TranslatorData::MaxMinType::Min));
            break;
        }

        // > 11/1/2016 9:11:04
        list = str.split(">");
        if(list.size() == 2)
        {
            filterMemory = (TranslatorData::checkDateTimeWithModel(list[1], "", lineNumber, model, ">", TranslatorData::MaxMinType::Max));
            break;
        }

        // < 11/1/2016 9:11:04
        list = str.split("<");
        if(list.size() == 2)
        {
            filterMemory = (TranslatorData::checkDateTimeWithModel("", list[1], lineNumber, model, "<", TranslatorData::MaxMinType::Min));
            break;
        }

        if(str.lastIndexOf("-") == -1)
        {
            if(str!="")
            {
                filterMemory = (("`"+model->headerData(lineNumber, Qt::Orientation::Horizontal).toString()+"`"+" like '%"+str+"%'"));
            }
            else
            {
                filterMemory = ("");
            }
            break;
        }
        break;
    }
    case TypeColumn::Time:
    {
        break;
    }
    case TypeColumn::Value:
    {
        model->setFilter("");

        QStringList list;
        list = str.split(":");

        if(list.size() == 2) // 11/1/2016 9:11:04 - 05/2/2016 12:11:26
        {                   // 11/1/2016 9:11:04 - 05/2/2026 12:11:26
            filterMemory = (TranslatorData::checkValueWithModel(list[0].toInt(), list[1].toInt(), lineNumber, model, "<=>="));// [a:b]
            break;
        }

        // >= 11/1/2016 9:11:04
        list = str.split(">=");
        if(list.size() == 2)
        {
            filterMemory = (TranslatorData::checkValueWithModel(list[1].toInt(), -1/*Заменится на другое значение*/, lineNumber, model, ">=", TranslatorData::MaxMinType::Max));
            break;
        }

        // <= 11/1/2016 9:11:04
        list = str.split("<=");
        if(list.size() == 2)
        {
            filterMemory = (TranslatorData::checkValueWithModel(-1/*Заменится на другое значение*/, list[1].toInt(), lineNumber, model, "<=", TranslatorData::MaxMinType::Min));
            break;
        }

        // > 11/1/2016 9:11:04
        list = str.split(">");
        if(list.size() == 2)
        {
            filterMemory = (TranslatorData::checkValueWithModel(list[1].toInt(), -1/*Заменится на другое значение*/, lineNumber, model, ">", TranslatorData::MaxMinType::Max));
            break;
        }

        // < 11/1/2016 9:11:04
        list = str.split("<");
        if(list.size() == 2)
        {
            filterMemory = (TranslatorData::checkValueWithModel(-1/*Заменится на другое значение*/, list[1].toInt(), lineNumber, model, "<", TranslatorData::MaxMinType::Min));
            break;
        }

        if(str.lastIndexOf(":") == -1)
        {
            bool ok = false;
            str.toInt(&ok);
            if(ok)
            {

                filterMemory = (TranslatorData::checkValueWithModel(str.toInt(), -1/*не используется*/, lineNumber, model, ""));
                qDebug() << "Magic Size " << filtersRQData(filterMemoryList).size();

            }
            else
            {
                filterMemory = ("");
                qDebug() << "Magic Size " << filtersRQData(filterMemoryList).size();
            }
            break;
        }
        break;

    }
    }
    model->setFilter(filtersRQData(filterMemoryList));
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
QList<int> SQLQueryBuilder::makePrimaryFilter(ProtocolPrinterItemModel *model)
{
    QList <int> list;

    int i = 0;
    while(model->record(i).value("Session ID").isValid())
    {
        list << model->record(i).value("Session ID").toInt();
        qDebug() << "PrimaryFilterList: " << list;
        i++;
    }

    return list;
}
