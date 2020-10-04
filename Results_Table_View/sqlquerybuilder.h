#ifndef SQLQUERYBUILDER_H
#define SQLQUERYBUILDER_H
#include <QString>
#include <QDateTime>
class ProtocolPrinterItemModel;
class ProtocolPrinterHeaderView;

class SQLQueryBuilder
{
public:
    enum class TypeColumn
    {
        String,
        DateTime,
        Value,
        Time///Todo добавить в builder
    };
    SQLQueryBuilder() = delete;
    static QString filtersRQData(QList<QString*> filterMemoryList);
    static void checksFilter(ProtocolPrinterItemModel* model, const QString& str, QString& filterMemory, QList<QString*> filterMemoryList, int lineNumber, TypeColumn type);
    static void checksFilter(QList<QString>& sessionIdList/*Может быть пустым*/, QList<QString>headerList, const QString &str, QString &filterMemory, QList<QDateTime>& listAllDateTimeDb, QList<QString *> filterMemoryList, int lineNumber, TypeColumn type);
    static QList<int> makePrimaryFilter(ProtocolPrinterItemModel* model);

};

#endif // SQLQUERYBUILDER_H
