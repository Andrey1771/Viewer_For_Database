#include "translatordata.h"

#include <QDateTime>
#include <QSqlDriver>
#include <QTableView>
#include "protocolprinteritemmodel.h"


TranslatorData::TranslatorData(const QString &str, const modeDateTime mode)
{
    day=0;
    month=0;
    year=0;
    hours=0;
    minutes=0;
    seconds=0;

    if(!setData(str, mode))
    {
        day=-1;
        month=-1;
        year=-1;
        hours=-1;
        minutes=-1;
        seconds=-1;
    }
}

bool TranslatorData::setData(const QString &str,const modeDateTime mode)
{
    bool ok = false;
    QString buf = str;
    if(mode == modeDateTime::Date)
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
    if(mode == modeDateTime::Time)
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

    if(mode == modeDateTime::TimeDate)
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

QString TranslatorData::repairQString(const QString& str2, const modeDateTime mode)// 11/01/2016 09:11:04-12/01/2016 12:11:26
{
    QString str = "";
    TranslatorData dataQString(str2, mode);
    if(mode == modeDateTime::Time)
    {
        str = QString::number(dataQString.day).rightJustified(2,'0') + ".";
        str += QString::number(dataQString.month).rightJustified(2,'0') + ".";
        str += QString::number(dataQString.year).rightJustified(4,'0');
    }
    if(mode == modeDateTime::Date)
    {
        str = QString::number(dataQString.hours).rightJustified(2,'0') + ":";
        str += QString::number(dataQString.minutes).rightJustified(2,'0') + ":";
        str += QString::number(dataQString.seconds).rightJustified(2,'0');
    }
    if(mode == modeDateTime::TimeDate)
    {
        str = QString::number(dataQString.hours).rightJustified(2,'0') + ":";
        str += QString::number(dataQString.minutes).rightJustified(2,'0') + ":";
        str += QString::number(dataQString.seconds).rightJustified(2,'0');
        str += " ";
        str += QString::number(dataQString.day).rightJustified(2,'0') + ".";
        str += QString::number(dataQString.month).rightJustified(2,'0') + ".";
        str += QString::number(dataQString.year).rightJustified(4,'0');
    }
    return str;
}

QString TranslatorData::check(const QString &list0, const QString &list1, int lineNumber , QAbstractItemModel *model, const QString& type2, int type)
{
    QString req="";
    QDateTime dateTime[2];
    QList <QDateTime> list2;

    dateTime[0] = QDateTime::fromString(TranslatorData::repairQString(list0, modeDateTime::TimeDate), "HH:mm:ss dd.MM.yyyy");
    dateTime[1] = QDateTime::fromString(TranslatorData::repairQString(list1, modeDateTime::TimeDate), "HH:mm:ss dd.MM.yyyy");

    if(type == 1)
        dateTime[1] = QDateTime::fromString("23:59:59 31.12.9999", "HH:mm:ss dd.MM.yyyy");
    if(type == 2)//< 11/1/2016 9:11:04
        dateTime[0] = QDateTime::fromString("00:00:00 01.01.0001", "HH:mm:ss dd.MM.yyyy");

    QSqlTableModel *protocolPrinterItemModel = static_cast<QSqlTableModel*>(model);

    if(dateTime[0].isValid() && dateTime[1].isValid())
        return filterMemoryCreator(list0, list1, lineNumber , model, type2, dateTime[0], dateTime[1], modeDateTime::TimeDate);
    else
    {
        dateTime[0] = QDateTime::fromString(TranslatorData::repairQString(list0, modeDateTime::Time), "dd.MM.yyyy");
        dateTime[1] = QDateTime::fromString(TranslatorData::repairQString(list1, modeDateTime::Time), "dd.MM.yyyy");

        if(type == 1)
            dateTime[1] = QDateTime::fromString("31.12.9999", "dd.MM.yyyy");
        if(type == 2)
            dateTime[0] = QDateTime::fromString("00.00.0000", "dd.MM.yyyy");

        if(dateTime[0].isValid() && dateTime[1].isValid())
        {
            return filterMemoryCreator(list0, list1, lineNumber , model, type2, dateTime[0], dateTime[1], modeDateTime::Time);
        }
        else
        {
            dateTime[0] = QDateTime::fromString(TranslatorData::repairQString(list0, modeDateTime::Date), "HH:mm:ss");
            dateTime[1] = QDateTime::fromString(TranslatorData::repairQString(list1, modeDateTime::Date), "HH:mm:ss");

            if(type == 1)
                dateTime[1] = QDateTime::fromString("23:59:59", "HH:mm:ss");
            if(type == 2)
                dateTime[0] = QDateTime::fromString("00:00:00", "HH:mm:ss");

            if(!dateTime[0].isValid() || !dateTime[1].isValid())
            {
                return QString();
            }
            return filterMemoryCreator(list0, list1, lineNumber , model, type2, dateTime[0], dateTime[1], modeDateTime::Date);
        }
    }
}

QString TranslatorData::filterMemoryCreator(const QString &list0, const QString &list1, int lineNumber , QAbstractItemModel *model, const QString& type2, QDateTime& dateTime0, QDateTime& dateTime1, const modeDateTime type)
{
    ProtocolPrinterItemModel *protocolPrinterItemModel = dynamic_cast<class ProtocolPrinterItemModel*>(model);
    assert(protocolPrinterItemModel != nullptr);

    QString req="(ROWID IN (0";// такого ID нет
    QList <QDateTime> list2;
    int rowCount = protocolPrinterItemModel->allRowCount(); // кол строк

    protocolPrinterItemModel->getQuery().exec("SELECT `"+protocolPrinterItemModel->headerData(lineNumber, Qt::Orientation::Horizontal).toString()+"` FROM `"+protocolPrinterItemModel->tableName())+"`";
    protocolPrinterItemModel->getQuery().first();
    for (int i=0; i<rowCount; ++i)
    {
        list2.push_back(QDateTime::fromString(TranslatorData::repairQString(protocolPrinterItemModel->getQuery().value(lineNumber).toString(), modeDateTime::TimeDate), "HH:mm:ss dd.MM.yyyy"));
        protocolPrinterItemModel->getQuery().next();
    }
    switch(type)
    {
    case modeDateTime::TimeDate:
    {
        // Дата и время
        if(type2 == "<" || type2 == ">")
        {
            for (int i=0; i< list2.size(); ++i)
            {
                if(dateTime0 < list2[i] && list2[i] < dateTime1) // работает неправильно, если есть ошибки в дате(проверку не добавил, т к затратно и все равно потом будет проверка при вводе)
                    req+= ","+ QString::number(i+1); //Строка может быть очень большой
            }
        }
        if(type2 == "<=" || type2 == ">=")
        {
            for (int i=0; i< list2.size(); ++i)
            {
                if(dateTime0 <= list2[i] && list2[i] <= dateTime1)
                    req+= ","+ QString::number(i+1); //Строка может быть очень большой
            }
        }

        req+="))";
        break;
    }
    case modeDateTime::Date:
    {
        //Дата  5/1/3377 - 5/1/3877
        if(type2 == "<" || type2 == ">")
        {
            for (int i=0; i< list2.size(); ++i)
            {
                if(dateTime0.date() < list2[i].date() && list2[i].date() < dateTime1.date())
                    req+= ","+ QString::number(i+1); //Строка может быть очень большой
            }
        }
        if(type2 == "<=" || type2 == ">=")
        {
            for (int i=0; i< list2.size(); ++i)
            {
                if((dateTime0.date() <= list2[i].date()) && (list2[i].date() <= dateTime1.date()))
                    req+= ","+ QString::number(i+1); //Строка может быть очень большой
            }
        }
        req+="))";
        break;
    }
    case modeDateTime::Time:
    {
        //Время
        if(type2 == "<" || type2 == ">")
        {
            for (int i=0; i< list2.size(); ++i)
            {
                if(dateTime0.time() < list2[i].time() && list2[i].time() < dateTime1.time())
                    req+= ","+ QString::number(i+1); //Строка может быть очень большой
            }
        }
        if(type2 == "<=" || type2 == ">=")
        {
            for (int i=0; i< list2.size(); ++i)
            {
                if(dateTime0.time() <= list2[i].time() && list2[i].time() <= dateTime1.time())
                {
                    req+= ","+ QString::number(i+1); //Строка может быть очень большой
                }
            }
        }
        req+="))";//5/1/2544 - 5/1/3877
        break;
    }
    default:
        return "";
    }

    return req;
}


