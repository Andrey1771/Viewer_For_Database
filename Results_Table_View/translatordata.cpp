#include "translatordata.h"
#include "protocolprinteritemmodel.h"

#include <QDebug>

QList<int> specialNumbersColumns = {10};// Для 10 column

TranslatorData::TranslatorData(const QString &str)
{
    dateTime.setTime(QTime(0, 0, 0));
    dateTime.setDate(QDate(0, 0, 0));

    setData(str);
}

void TranslatorData::setData(const QString &str)
{
    QRegExp regExp("[0-9]{1,2}/[0-9]{1,2}/[0-9]{1,4}");
    int pos = regExp.indexIn(str);
    if(pos > -1)
    {
        QString value = regExp.capturedTexts().at(0);
        qDebug() << "value: " << value;
        QList<QString>list = value.split("/");
        dateTime.setDate(QDate(list.at(2).toInt(), list.at(0).toInt(), list.at(1).toInt()));// yyyy-MM-dd
    }
    else
    {
        qDebug() << "Строка фильтра не найдена";
        dateTime.setDate(QDate(-1, -1, -1));//
    }

    regExp.setPattern("[0-9]{1,2}:[0-9]{1,2}:[0-9]{1,4}");
    pos = regExp.indexIn(str);
    if(pos > -1)
    {
        QString value = regExp.capturedTexts().at(0);
        qDebug() << "value: " << value;
        QList<QString>list = value.split(":");
        dateTime.setTime(QTime(list.at(0).toInt(), list.at(1).toInt(), list.at(2).toInt()));
    }
    else
    {
        qDebug() << "Строка фильтра не найдена";
        dateTime.setTime(QTime(-1, -1, -1));//
    }
}

QString TranslatorData::repairQString(const QString& str2, int k)// 11/01/2016 09:11:04-12/01/2016 12:11:26
{
    QString str = "";
    TranslatorData dataQString(str2);
    switch (k)
    {
    case (1):
    {
        return dataQString.dateTime.date().toString("MM/dd/yyyy");
    }
    case (2):
    {
        return dataQString.dateTime.time().toString("HH:mm:ss");
    }
    case (3):
    {
        return dataQString.dateTime.toString("HH:mm:ss MM/dd/yyyy");
    }
    default:
    {
        qDebug() << "Такого типа нет repairQString";
        return "";
    }
    }
}

QString TranslatorData::checkDateTimeWithModel(const QString &list0, const QString &list1, int lineNumber, QAbstractItemModel* model, const QString& type2, MaxMinType maxMinType)
{
    QList <QDateTime> list2;
    QList<QString> sessionIdList;
    fillListAllDateTimeDb(list2, model, lineNumber);
    fillListAllSessionId(sessionIdList, model);

    return checkDateTime(sessionIdList, list0, list1, list2, type2, maxMinType);
}
QString TranslatorData::checkDateTimeWithoutModel(QList<QString>& sessionIdList, const QString &list0, const QString &list1, QList<QDateTime>& listAllDateTimeDb, const QString& type2, MaxMinType maxMinType)
{
    return checkDateTime(sessionIdList, list0, list1, listAllDateTimeDb, type2, maxMinType);
}

QString TranslatorData::checkValueWithModel(const int value0, const int value1, int lineNumber, QAbstractItemModel* model, const QString& type2, MaxMinType maxMinType)
{
    QList <int> list2;
    QList<QString> sessionIdList;
    fillListAllValueDb(list2, model, lineNumber);
    fillListAllSessionId(sessionIdList, model);

    return checkValue(sessionIdList, value0, value1, list2, type2, maxMinType);
}
QString TranslatorData::checkValueWithoutModel(QList<QString>& sessionIdList, const int value0, const int value1, QList<int>& listAllValueDb, const QString& type2, MaxMinType maxMinType)
{
    return checkValue(sessionIdList, value0, value1, listAllValueDb, type2, maxMinType);
}

QString TranslatorData::checkValue(QList<QString> &sessionIdNamesList, int value0, int value1, QList<int> &listAllValueDb, const QString &type2, TranslatorData::MaxMinType maxMinType)
{

    if(maxMinType == MaxMinType::Max)
        value1 = INT_MAX;
    if(maxMinType == MaxMinType::Min)
        value0 = 0;
    return filterMemoryCreator(sessionIdNamesList, listAllValueDb, type2, value0, value1);
}

QString TranslatorData::checkDateTime(QList<QString>& sessionIdNamesList, const QString &list0, const QString &list1, QList<QDateTime>& listAllDateTimeDb, const QString& type2, MaxMinType maxMinType)
{
    QString req="";
    QDateTime dateTime[2];
    QList <QDateTime> list2;

    dateTime[0] = strDateTimeConv(list0);
    dateTime[1] = strDateTimeConv(list1);//00:00:00 - 12:11:26

    if(maxMinType == MaxMinType::Max)
        dateTime[1] = QDateTime::fromString("23:59:59 12/31/9999", "HH:mm:ss MM/dd/yyyy");
    if(maxMinType == MaxMinType::Min)//< 11/1/2016 9:11:04
        dateTime[0] = QDateTime::fromString("00:00:00 01/01/0001", "HH:mm:ss MM/dd/yyyy");

    if(dateTime[0].isValid() && dateTime[1].isValid())
        return filterMemoryCreator(sessionIdNamesList, listAllDateTimeDb, type2, dateTime[0], dateTime[1], TypeDateTimeFilter::DateTime);
    else
    {
        dateTime[0] = strDateConv(list0);
        dateTime[1] = strDateConv(list1);

        if(maxMinType == MaxMinType::Max)
            dateTime[1] = QDateTime::fromString("12/31/9999", "MM/dd/yyyy");
        if(maxMinType == MaxMinType::Min)
            dateTime[0] = QDateTime::fromString("00/00/0000", "MM/dd/yyyy");
        //11/1/2000 9:11:04 - 05/2/2033 12:11:26 9:11:04 - 12:11:26
        if(dateTime[0].isValid() && dateTime[1].isValid())
        {
            return filterMemoryCreator(sessionIdNamesList, listAllDateTimeDb, type2, dateTime[0], dateTime[1], TypeDateTimeFilter::Date);
        }
        else
        {
            dateTime[0] = strTimeConv(list0);
            dateTime[1] = strTimeConv(list1);

            if(maxMinType == MaxMinType::Max)
                dateTime[1] = QDateTime::fromString("23:59:59", "HH:mm:ss");
            if(maxMinType == MaxMinType::Min)
                dateTime[0] = QDateTime::fromString("00:00:00", "HH:mm:ss");

            if(!dateTime[0].isValid() || !dateTime[1].isValid())
            {
                return QString();
            }
            return filterMemoryCreator(sessionIdNamesList, listAllDateTimeDb, type2, dateTime[0], dateTime[1], TypeDateTimeFilter::Time);
        }
    }
}

void TranslatorData::fillListAllDateTimeDb(QList <QDateTime>& list2, QAbstractItemModel *model, int lineNumber)
{
    ProtocolPrinterItemModel *protocolPrinterItemModel = dynamic_cast<class ProtocolPrinterItemModel*>(model);
    assert(protocolPrinterItemModel != nullptr);
    //11/1/2016 9:11:04 - 05/2/2016 12:11:26
    protocolPrinterItemModel->getQuery().exec("SELECT `" + protocolPrinterItemModel->headerData(lineNumber, Qt::Orientation::Horizontal).toString() + "` FROM `" + protocolPrinterItemModel->tableName() + "`;");

    for (auto var : specialNumbersColumns)
    {
        if(lineNumber == var)
        {
            while(protocolPrinterItemModel->getQuery().next())
            {
                auto val = protocolPrinterItemModel->getQuery().value(0).toString();
                list2.push_back(strTimeConv(val));
            }
            return;
        }
    }
    while(protocolPrinterItemModel->getQuery().next())
    {
        auto val = protocolPrinterItemModel->getQuery().value(0).toString();
        list2.push_back(strDateTimeConv(val));
    }
}


void TranslatorData::fillListAllValueDb(QList <int>& list2, QAbstractItemModel *model, int lineNumber)
{
    ProtocolPrinterItemModel *protocolPrinterItemModel = dynamic_cast<class ProtocolPrinterItemModel*>(model);
    assert(protocolPrinterItemModel != nullptr);
    //11/1/2016 9:11:04 - 05/2/2016 12:11:26
    protocolPrinterItemModel->getQuery().exec("SELECT `" + protocolPrinterItemModel->headerData(lineNumber, Qt::Orientation::Horizontal).toString() + "` FROM `" + protocolPrinterItemModel->tableName() + "`;");

    for (auto var : specialNumbersColumns)
    {
        if(lineNumber == var)
        {
            while(protocolPrinterItemModel->getQuery().next())
            {
                auto val = protocolPrinterItemModel->getQuery().value(0).toString();
                list2.push_back(val.toInt());
            }
            return;
        }
    }
    while(protocolPrinterItemModel->getQuery().next())
    {
        auto val = protocolPrinterItemModel->getQuery().value(0).toString();
        list2.push_back(val.toInt());
    }
}


void TranslatorData::fillListAllSessionId(QList<QString>& sessionIdList, QAbstractItemModel *model)
{
    ProtocolPrinterItemModel *protocolPrinterItemModel = dynamic_cast<class ProtocolPrinterItemModel*>(model);
    assert(protocolPrinterItemModel != nullptr);
    //11/1/2016 9:11:04 - 05/2/2016 12:11:26

    protocolPrinterItemModel->getQuery().exec("SELECT `Session ID` FROM `"+protocolPrinterItemModel->tableName()+"`");

    while(protocolPrinterItemModel->getQuery().next())
    {
        auto val = protocolPrinterItemModel->getQuery().value(0).toString();
        sessionIdList.push_back(val);
    }
}

QDateTime TranslatorData::strDateTimeConv(const QString& str)
{
    auto dt = QDateTime::fromString(str);
    if (!dt.isValid())
    {
        QString buf = repairQString(str, 3);
        dt = QDateTime::fromString(buf, "HH:mm:ss MM/dd/yyyy");
    }
    return dt;
}
QDateTime TranslatorData::strDateConv(const QString& str)
{
    auto dt = QDateTime::fromString(str);
    if (!dt.isValid())
    {
        QString buf = repairQString(str, 1);
        dt = QDateTime::fromString(buf, "MM/dd/yyyy");
    }
    return dt;
}
QDateTime TranslatorData::strTimeConv(const QString& str)
{
    auto dt = QDateTime::fromString(str);
    if (!dt.isValid())
    {
        QString buf = repairQString(str, 2);
        dt = QDateTime::fromString(buf, "HH:mm:ss");
    }
    return dt;
}
QString TranslatorData::filterMemoryCreator(QList<QString>& sessionIdList, QList<int>& listAllValueDb, const QString& type2, const int value0, const int value1)
{
    QString req;
    sqlValueRequestCreator(sessionIdList, req, listAllValueDb, value0, value1, type2);
    return req;
}
QString TranslatorData::filterMemoryCreator(QList<QString>& sessionIdList, QList<QDateTime>& listAllDateTimeDb, const QString& type2, QDateTime& dateTime0, QDateTime& dateTime1, TypeDateTimeFilter typeFilter)
{
    QString req;
    sqlDateTimeRequestCreator(sessionIdList, req, listAllDateTimeDb, dateTime0, dateTime1, type2, typeFilter);
    return req;
}
                                                                          /*строка может быть очень большой, чтобы избежать лишнего копирования*/
void TranslatorData::sqlValueRequestCreator(QList<QString>& sessionIdList, QString& req, QList<int> &list2,
                                            const int value0, const int value1, const QString& type2)
{
    req = "(`Session ID` IN (0";// такого ID нет

    if(type2 == "")
    {
        for (int i=0; i < list2.size(); ++i)
        {
            if(list2[i] == value0)
                req += ","+ sessionIdList.at(i); //Строка может быть очень большой
        }
    }
    if(type2 == "<")
    {
        for (int i=0; i < list2.size(); ++i)
        {
            if(list2[i] < value1)
                req += ","+ sessionIdList.at(i); //Строка может быть очень большой
        }
    }
    if(type2 == ">")
    {
        for (int i=0; i < list2.size(); ++i)
        {
            if(value0 < list2[i])
                req += ","+ sessionIdList.at(i); //Строка может быть очень большой
        }
    }

    if(type2 == "<=")
    {
        for (int i=0; i < list2.size(); ++i)
        {
            if(list2[i] <= value1)
                req += ","+ sessionIdList.at(i); //Строка может быть очень большой
        }
    }
    if(type2 == ">=")
    {
        for (int i=0; i < list2.size(); ++i)
        {
            if(value0 <= list2[i])
                req += ","+ sessionIdList.at(i); //Строка может быть очень большой
        }
    }
    if(type2 == "<=>=")
    {
        for (int i=0; i< list2.size(); ++i)
        {
            if(value0 <= list2[i] && list2[i] <= value1)
                req += ","+ sessionIdList.at(i); //Строка может быть очень большой
        }

    }
    req += "))";
    return;
}

void TranslatorData::sqlDateTimeRequestCreator(QList<QString>& sessionIdList, QString& req/*строка может быть очень большой, чтобы избежать лишнего копирования*/, QList <QDateTime>& list2, QDateTime& dateTime0, QDateTime& dateTime1, const QString& type2, TypeDateTimeFilter typeFilter)
{///TODO реализовать паттерн state???
    req = "(`Session ID` IN (0";// такого ID нет
    switch(typeFilter)
    {
    case TypeDateTimeFilter::DateTime:
    {
        // Дата и время
        if(type2 == "<")
        {
            for (int i=0; i < list2.size(); ++i)
            {
                if(list2[i] < dateTime1) // работает неправильно, если есть ошибки в дате(проверку не добавил, т к затратно и все равно потом будет проверка при вводе)
                    req += ","+ sessionIdList.at(i); //Строка может быть очень большой
            }
        }
        if(type2 == ">")
        {
            for (int i=0; i < list2.size(); ++i)
            {
                if(dateTime0 < list2[i]) // работает неправильно, если есть ошибки в дате(проверку не добавил, т к затратно и все равно потом будет проверка при вводе)
                    req += ","+ sessionIdList.at(i); //Строка может быть очень большой
            }
        }

        if(type2 == "<=")
        {
            for (int i=0; i < list2.size(); ++i)
            {
                if(list2[i] <= dateTime1)
                    req += ","+ sessionIdList.at(i); //Строка может быть очень большой
            }
        }
        if(type2 == ">=")
        {
            for (int i=0; i < list2.size(); ++i)
            {
                if(dateTime0 <= list2[i])
                    req += ","+ sessionIdList.at(i); //Строка может быть очень большой
            }
        }
        if(type2 == "<=>=")
        {
            for (int i=0; i< list2.size(); ++i)
            {
                if(dateTime0 <= list2[i] && list2[i] <= dateTime1)
                    req += ","+ sessionIdList.at(i); //Строка может быть очень большой
            }

        }
        req += "))";
        break;
    }
    case TypeDateTimeFilter::Date:
    {//<=08/27/2020 19:10:11
        //Дата  5/1/3377 - 5/1/3877

        if(type2 == "<")
        {
            for (int i=0; i < list2.size(); ++i)
            {
                if(list2[i].date() < dateTime1.date())
                    req += ","+ sessionIdList.at(i); //Строка может быть очень большой
            }
        }

        if(type2 == ">")
        {
            for (int i=0; i < list2.size(); ++i)
            {
                if(dateTime0.date() < list2[i].date())
                    req += ","+ sessionIdList.at(i); //Строка может быть очень большой
            }
        }
        if(type2 == "<=")
        {
            for (int i=0; i < list2.size(); ++i)
            {
                if(list2[i].date() <= dateTime1.date())
                    req += ","+ sessionIdList.at(i); //Строка может быть очень большой
            }
        }
        if(type2 == ">=")
        {
            for (int i=0; i < list2.size(); ++i)
            {
                if(dateTime0.date() <= list2[i].date())
                    req += ","+ sessionIdList.at(i); //Строка может быть очень большой
            }
        }
        if(type2 == "<=>=")
        {
            for (int i=0; i< list2.size(); ++i)
            {
                if((dateTime0.date() <= list2[i].date()) && (list2[i].date() <= dateTime1.date()))
                    req += ","+ sessionIdList.at(i); //Строка может быть очень большой
            }
        }
        req += "))";
        break;
    }
    case TypeDateTimeFilter::Time:
    {
        //Время
        if(type2 == "<")
        {
            for (int i=0; i < list2.size(); ++i)
            {
                if(list2[i].time() < dateTime1.time())
                    req += ","+ sessionIdList.at(i); //Строка может быть очень большой
            }
        }
        if(type2 == ">")
        {
            for (int i=0; i < list2.size(); ++i)
            {
                if(dateTime0.time() < list2[i].time())
                    req += ","+ sessionIdList.at(i); //Строка может быть очень большой
            }
        }
        if(type2 == "<=")
        {
            for (int i=0; i < list2.size(); ++i)
            {
                if(list2[i].time() <= dateTime1.time())
                {
                    req += ","+ sessionIdList.at(i); //Строка может быть очень большой
                }
            }
        }
        if(type2 == ">=")
        {
            for (int i=0; i < list2.size(); ++i)
            {
                if(dateTime0.time() <= list2[i].time())
                {
                    req += ","+ sessionIdList.at(i); //Строка может быть очень большой
                }
            }
        }
        if(type2 == "<=>=")
        {
            for (int i=0; i< list2.size(); ++i)
            {
                if(dateTime0.time() <= list2[i].time() && list2[i].time() <= dateTime1.time())
                {
                    req += ","+ sessionIdList.at(i); //Строка может быть очень большой
                }
            }

        }
        req += "))";//5/1/2544 - 5/1/3877
        break;
    }
    default:
    {
        req = "";
        return ;
    }
    }
    qDebug() << "SQLRequestStr " << req;
    return;
}
