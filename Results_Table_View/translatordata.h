#ifndef TRANSLATORDATA_H
#define TRANSLATORDATA_H

#include "protocolprinteritemmodel.h"
#include <QDateTime>

class QAbstractItemModel;

class TranslatorData
{
public:
    enum class TypeDateTimeFilter
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
    static void sqlDateTimeRequestCreator(QList<QString>& sessionIdList, QString& req, QList <QDateTime>& list2, QDateTime& dateTime0, QDateTime& dateTime1, const QString& type2, TypeDateTimeFilter typeFilter);
    static void sqlValueRequestCreator(QList<QString>& sessionIdList, QString& req, QList <int>& list2, const int value0, const int value1, const QString& type2);

    static QString checkDateTime(QList<QString>& sessionIdList, const QString &list0, const QString &list1, QList<QDateTime>& listAllDateTimeDb, const QString& type2, MaxMinDateTime maxMinType = MaxMinDateTime::None);
    static QString checkValue(QList<QString>& sessionIdNamesList, const int value0, const int value1, QList<int>& listAllValueDb, const QString& type2, MaxMinDateTime maxMinType);
    static QDateTime strDateTimeConv(const QString& str);
    static QDateTime strDateConv(const QString& str);
    static QDateTime strTimeConv(const QString& str);
public:

    static void fillListAllDateTimeDb(QList <QDateTime>& list2, QAbstractItemModel *model, int lineNumber);
    static void fillListAllValueDb(QList<int> &list2, QAbstractItemModel *model, int lineNumber);
    static void fillListAllSessionId(QList<QString>& sessionIdList, QAbstractItemModel *model);

    TranslatorData(const QString &str = "");
    void setData(const QString& str = "");
    static QString repairQString(const QString& str2, int k);

    static QString filterMemoryCreator(QList<QString>& sessionIdNamesList, QList<QDateTime>& listAllDateTimeDb, const QString& type2, QDateTime &dateTime0, QDateTime &dateTime1, TypeDateTimeFilter typeFilter);
    static QString filterMemoryCreator(QList<QString>& sessionIdList, QList<int>& listAllValueDb, const QString& type2, const int value0, const int value1);
    static QString checkDateTimeWithModel(const QString &list0, const QString &list1, int lineNumber, QAbstractItemModel* model, const QString& type2, MaxMinDateTime maxMinType = MaxMinDateTime::None);
    static QString checkDateTimeWithoutModel(QList<QString>& sessionIdList, const QString &list0, const QString &list1, QList<QDateTime>& listAllDateTimeDb, const QString& type2, MaxMinDateTime maxMinType = MaxMinDateTime::None);
    static QString checkValueWithModel(const int value0, const int value1, int lineNumber, QAbstractItemModel* model, const QString& type2, MaxMinDateTime maxMinType);
    static QString checkValueWithoutModel(QList<QString>& sessionIdList, const int value0, const int value1, QList<int>& listAllValueDb, const QString& type2, MaxMinDateTime maxMinType);
    QDateTime dateTime;

};

#endif // TRANSLATORDATA_H
