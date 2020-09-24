#include "translatordata.h"
#include "protocolprinteritemmodel.h"

#include <QDateTime>
#include <QSqlDriver>
#include <QTableView>
#include <QDebug>
QList<int> specialNumbersColumns = {10};// Для 10 column
TranslatorData::TranslatorData(const QString &str, int number)
{
    day=0;
    month=0;
    year=0;
    hours=0;
    minutes=0;
    seconds=0;

    if(!setData(str, number))
    {
        day=-1;
        month=-1;
        year=-1;
        hours=-1;
        minutes=-1;
        seconds=-1;
    }
}

bool TranslatorData::setData(const QString &str, int number)// Можно было сделать через регулярку
{
    bool ok = false;
    QString buf = str;
    if(number == 2)
    {
        seconds = buf.rightRef(buf.size()-buf.lastIndexOf(":")-1).toInt(&ok);// не сработает при 11/1/2016 9:11:04 greger
        buf.truncate(buf.lastIndexOf(":"));
        if(!ok)
            return false;

        minutes = buf.rightRef(buf.size()-buf.lastIndexOf(":")-1).toInt(&ok);
        buf.truncate(buf.lastIndexOf(":"));
        if(!ok)
            return false;
        hours = buf.rightRef(buf.size()-buf.lastIndexOf(" ")-1).toInt(&ok);
        buf.truncate(buf.lastIndexOf(" "));
        if(!ok)
            return false;
    }
    if(number == 1)
    {
        year = buf.rightRef(buf.size()-buf.lastIndexOf("/")-1).toInt(&ok);
        buf.truncate(buf.lastIndexOf("/"));
        if(!ok)
            return false;

        month = buf.rightRef(buf.size()-buf.lastIndexOf("/")-1).toInt(&ok);
        buf .truncate(buf.lastIndexOf("/"));
        if(!ok)
            return false;

        day = buf.toInt(&ok);
        if(!ok)
            return false;
    }

    if(number == 3)
    {
        seconds = buf.rightRef(buf.size()-buf.lastIndexOf(":")-1).toInt(&ok);// не сработает при 11/1/2016 9:11:04 greger
        buf.truncate(buf.lastIndexOf(":"));
        if(!ok)
            return false;

        minutes = buf.rightRef(buf.size()-buf.lastIndexOf(":")-1).toInt(&ok);
        buf.truncate(buf.lastIndexOf(":"));
        if(!ok)
            return false;

        hours = buf.rightRef(buf.size()-buf.lastIndexOf(" ")-1).toInt(&ok);
        buf.truncate(buf.lastIndexOf(" "));
        if(!ok)
            return false;

        year = buf.rightRef(buf.size()-buf.lastIndexOf("/")-1).toInt(&ok);
        buf.truncate(buf.lastIndexOf("/"));
        if(!ok)
            return false;

        month = buf.rightRef(buf.size()-buf.lastIndexOf("/")-1).toInt(&ok);
        buf .truncate(buf.lastIndexOf("/"));
        if(!ok)
            return false;

        day = buf.toInt(&ok);
        if(!ok)
            return false;
    }
    return true;
}

QString TranslatorData::repairQString(const QString& str2, int k)// 11/01/2016 09:11:04-12/01/2016 12:11:26
{
    QString str = "";
    TranslatorData dataQString(str2, k);
    if(k == 1)
    {
        str = QString::number(dataQString.day).rightJustified(2,'0') + "/";
        str += QString::number(dataQString.month).rightJustified(2,'0') + "/";
        str += QString::number(dataQString.year).rightJustified(4,'0');
    }
    if(k == 2)
    {
        str = QString::number(dataQString.hours).rightJustified(2,'0') + ":";
        str += QString::number(dataQString.minutes).rightJustified(2,'0') + ":";
        str += QString::number(dataQString.seconds).rightJustified(2,'0');
    }
    if(k==3)
    {
        str = QString::number(dataQString.hours).rightJustified(2,'0') + ":";
        str += QString::number(dataQString.minutes).rightJustified(2,'0') + ":";
        str += QString::number(dataQString.seconds).rightJustified(2,'0');
        str += " ";
        str += QString::number(dataQString.day).rightJustified(2,'0') + "/";
        str += QString::number(dataQString.month).rightJustified(2,'0') + "/";
        str += QString::number(dataQString.year).rightJustified(4,'0');
    }
    return str;
}

QString TranslatorData::checkWithModel(const QString &list0, const QString &list1, int lineNumber, QAbstractItemModel* model, const QString& type2, MaxMinDateTime maxMinType)
{
    QList <QDateTime> list2;
    QList<QString> sessionIdList;
    fillListAllDateTimeDb(list2, model, lineNumber);
    fillListAllSessionId(sessionIdList, model);

    return check(sessionIdList, list0, list1, list2, type2, maxMinType);
}
QString TranslatorData::checkWithoutModel(QList<QString>& sessionIdList, const QString &list0, const QString &list1, QList<QDateTime>& listAllDateTimeDb, const QString& type2, MaxMinDateTime maxMinType)
{
    return check(sessionIdList, list0, list1, listAllDateTimeDb, type2, maxMinType);
}

QString TranslatorData::check(QList<QString>& sessionIdNamesList, const QString &list0, const QString &list1, QList<QDateTime>& listAllDateTimeDb, const QString& type2, MaxMinDateTime maxMinType)
{
    QString req="";
    QDateTime dateTime[2];
    QList <QDateTime> list2;

    dateTime[0] = strDateTimeConv(list0);
    dateTime[1] = strDateTimeConv(list1);//00:00:00 - 12:11:26

    if(maxMinType == MaxMinDateTime::Max)
        dateTime[1] = QDateTime::fromString("23:59:59 12/31/9999", "HH:mm:ss MM/dd/yyyy");
    if(maxMinType == MaxMinDateTime::Min)//< 11/1/2016 9:11:04
        dateTime[0] = QDateTime::fromString("00:00:00 01/01/0001", "HH:mm:ss MM/dd/yyyy");

    if(dateTime[0].isValid() && dateTime[1].isValid())
        return filterMemoryCreator(sessionIdNamesList, listAllDateTimeDb, type2, dateTime[0], dateTime[1], TypeFilter::DateTime);
    else
    {
        dateTime[0] = strDateConv(list0);
        dateTime[1] = strDateConv(list1);

        if(maxMinType == MaxMinDateTime::Max)
            dateTime[1] = QDateTime::fromString("12/31/9999", "MM/dd/yyyy");
        if(maxMinType == MaxMinDateTime::Min)
            dateTime[0] = QDateTime::fromString("00/00/0000", "MM/dd/yyyy");
        //11/1/2000 9:11:04 - 05/2/2033 12:11:26 9:11:04 - 12:11:26
        if(dateTime[0].isValid() && dateTime[1].isValid())
        {
            return filterMemoryCreator(sessionIdNamesList, listAllDateTimeDb, type2, dateTime[0], dateTime[1], TypeFilter::Date);
        }
        else
        {
            dateTime[0] = strTimeConv(list0);
            dateTime[1] = strTimeConv(list1);

            if(maxMinType == MaxMinDateTime::Max)
                dateTime[1] = QDateTime::fromString("23:59:59", "HH:mm:ss");
            if(maxMinType == MaxMinDateTime::Min)
                dateTime[0] = QDateTime::fromString("00:00:00", "HH:mm:ss");

            if(!dateTime[0].isValid() || !dateTime[1].isValid())
            {
                return QString();
            }
            return filterMemoryCreator(sessionIdNamesList, listAllDateTimeDb, type2, dateTime[0], dateTime[1], TypeFilter::Time);
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
QString TranslatorData::filterMemoryCreator(QList<QString>& sessionIdList, QList<QDateTime>& listAllDateTimeDb, const QString& type2, QDateTime& dateTime0, QDateTime& dateTime1, TypeFilter typeFilter)
{
    QString req;
    sqlRequestCreator(sessionIdList, req, listAllDateTimeDb, dateTime0, dateTime1, type2, typeFilter);
    return req;
}

void TranslatorData::sqlRequestCreator(QList<QString>& sessionIdList, QString& req/*строка может быть очень большой, чтобы избежать лишнего копирования*/, QList <QDateTime>& list2, QDateTime& dateTime0, QDateTime& dateTime1, const QString& type2, TypeFilter typeFilter)
{
    req = "(`Session ID` IN (0";// такого ID нет??????
    switch(typeFilter)
    {
    case TypeFilter::DateTime:
    {
        // Дата и время
        if(type2 == "<" || type2 == ">")
        {
            for (int i=0; i < list2.size(); ++i)
            {
                if(dateTime0 < list2[i] && list2[i] < dateTime1) // работает неправильно, если есть ошибки в дате(проверку не добавил, т к затратно и все равно потом будет проверка при вводе)
                    req += ","+ sessionIdList.at(i); //Строка может быть очень большой
            }
        }
        if(type2 == "<=" || type2 == ">=")
        {
            for (int i=0; i < list2.size(); ++i)
            {
                if(dateTime0 < list2[i] && list2[i] <= dateTime1)
                    req += ","+ sessionIdList.at(i); //Строка может быть очень большой
            }
        }

        req += "))";
        break;
    }
    case TypeFilter::Date:
    {
        //Дата  5/1/3377 - 5/1/3877
        if(type2 == "<" || type2 == ">")
        {
            for (int i=0; i < list2.size(); ++i)
            {
                if(dateTime0.date() < list2[i].date() && list2[i].date() < dateTime1.date())
                    req += ","+ sessionIdList.at(i); //Строка может быть очень большой
            }
        }
        if(type2 == "<=" || type2 == ">=")
        {
            for (int i=0; i < list2.size(); ++i)
            {
                if((dateTime0.date() < list2[i].date()) && (list2[i].date() <= dateTime1.date()))
                    req += ","+ sessionIdList.at(i); //Строка может быть очень большой
            }
        }
        req += "))";
        break;
    }
    case TypeFilter::Time:
    {
        //Время
        if(type2 == "<" || type2 == ">")
        {
            for (int i=0; i < list2.size(); ++i)
            {
                if(dateTime0.time() < list2[i].time() && list2[i].time() < dateTime1.time())
                    req += ","+ sessionIdList.at(i); //Строка может быть очень большой
            }
        }
        if(type2 == "<=" || type2 == ">=")
        {
            for (int i=0; i < list2.size(); ++i)
            {
                if(dateTime0.time() < list2[i].time() && list2[i].time() <= dateTime1.time())
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
