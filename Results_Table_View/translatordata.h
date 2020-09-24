#ifndef TRANSLATORDATA_H
#define TRANSLATORDATA_H

#include "protocolprinteritemmodel.h"

class QAbstractItemModel;

class TranslatorData
{
public:
    enum class TypeFilter
    {
        DateTime = 0,
        Date = 1,
        Time = 2
    };
    enum class MaxMinDateTime
    {
        None = 0,
        Max = 1,
        Min = 2
    };
private:
    static void sqlRequestCreator(QList<QString>& sessionIdList, QString& req, QList <QDateTime>& list2, QDateTime& dateTime0, QDateTime& dateTime1, const QString& type2, TypeFilter typeFilter);
    static void fillListAllDateTimeDb(QList <QDateTime>& list2, QAbstractItemModel *model, int lineNumber);
    static void fillListAllSessionId(QList<QString>& sessionIdList, QAbstractItemModel *model);
    static QString check(QList<QString>& sessionIdList, const QString &list0, const QString &list1, QList<QDateTime>& listAllDateTimeDb, const QString& type2, MaxMinDateTime maxMinType = MaxMinDateTime::None);
    static QDateTime strDateTimeConv(const QString& str);
    static QDateTime strDateConv(const QString& str);
    static QDateTime strTimeConv(const QString& str);
public:


    TranslatorData(const QString &str = "", int number = 0);
    bool setData(const QString& str = "", int number = 0);
    static QString repairQString(const QString& str2, int k);

    static QString filterMemoryCreator(QList<QString>& sessionIdNamesList, QList<QDateTime>& listAllDateTimeDb, const QString& type2, QDateTime &dateTime0, QDateTime &dateTime1, TypeFilter typeFilter);
    static QString checkWithModel(const QString &list0, const QString &list1, int lineNumber, QAbstractItemModel* model, const QString& type2, MaxMinDateTime maxMinType = MaxMinDateTime::None);
    static QString checkWithoutModel(QList<QString>& sessionIdList, const QString &list0, const QString &list1, QList<QDateTime>& listAllDateTimeDb, const QString& type2, MaxMinDateTime maxMinType = MaxMinDateTime::None);
    int day;
    int month;
    int year;
    int hours;
    int minutes;
    int seconds;

};

#endif // TRANSLATORDATA_H
