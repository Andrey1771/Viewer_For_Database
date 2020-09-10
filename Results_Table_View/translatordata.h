#ifndef TRANSLATORDATA_H
#define TRANSLATORDATA_H

#include "protocolprinteritemmodel.h"

class QAbstractItemModel;

class TranslatorData
{
public:
    TranslatorData(const QString &str = "", int number = 0);
    bool setData(const QString& str = "", int number = 0);
    static QString repairQString(const QString& str2, int k);
    static QString check(const QString &list0, const QString &list1, int lineNumber, QAbstractItemModel *model, const QString &type2, int type = 0);
    static QString filterMemoryCreator(const QString &list0, const QString &list1, int lineNumber , QAbstractItemModel *model, const QString& type2, QDateTime &dateTime0, QDateTime &dateTime1, int type);

    int day;
    int month;
    int year;
    int hours;
    int minutes;
    int seconds;

};

#endif // TRANSLATORDATA_H
