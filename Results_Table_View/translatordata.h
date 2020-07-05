#ifndef TRANSLATORDATA_H
#define TRANSLATORDATA_H

#include "protocolprinteritemmodel.h"

enum modeDateTime
{
    Time, Date, TimeDate
};

class QAbstractItemModel;

class TranslatorData
{
public:
    TranslatorData(const QString &str = "", const modeDateTime mode = modeDateTime::TimeDate);
    bool setData(const QString& str = "", const modeDateTime mode  = modeDateTime::TimeDate);
    static QString repairQString(const QString& str2, const modeDateTime mode);
    static QString check(const QString &list0, const QString &list1, int lineNumber, QAbstractItemModel *model, const QString &type2, int type = 0);
    static QString filterMemoryCreator(const QString &list0, const QString &list1, int lineNumber , QAbstractItemModel *model, const QString& type2, QDateTime &dateTime0, QDateTime &dateTime1, const modeDateTime type);

    int day;
    int month;
    int year;
    int hours;
    int minutes;
    int seconds;

};

#endif // TRANSLATORDATA_H
