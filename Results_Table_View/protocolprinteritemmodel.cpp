#include "protocolprinteritemmodel.h"
#include "qsqlfield.h"

#include <QSqlRecord>
#include <QSqlDriver>

#include <QEvent>
#include <QDebug>
#include <QElapsedTimer>
ProtocolPrinterItemModel::ProtocolPrinterItemModel(QObject *parent) : QSqlTableModel (parent)
{
}

QVariant ProtocolPrinterItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QSqlTableModel::headerData(section, orientation, role);
    if(orientation == Qt::Vertical && role == Qt::DisplayRole)
        return QString("%1").arg(section+1);
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return namesColumns[section];
    return QVariant();
}

// QSqlTableModel interface
void ProtocolPrinterItemModel::setTable(const QString &tableName)
{
    namesColumns.clear();
    query.exec("select * from '"+ tableName+ "';"); // Приходится делать запрос, чтобы узнать структуру таблицы
    int count = query.record().count(); // упростить
    countColumns = count;
    for (int i=0; i < count; ++i)
    {
        namesColumns << query.record().field(i).name();
    }

    QSqlTableModel::setTable(tableName);
    emit TableChanged();
}

QSqlQuery &ProtocolPrinterItemModel::getQuery()
{
    return query;
}

QList<QString> ProtocolPrinterItemModel::getNamesColumns() const
{
    return namesColumns;
}

int ProtocolPrinterItemModel::allRowCount()
{
    if ( this->database().driver()->hasFeature(QSqlDriver::QuerySize)) //SQLite не поддерживает size()
    {
        return this->getQuery().size();
    }
    else
    {
        getQuery().exec("ANALYZE");
        getQuery().next();
        getQuery().exec("select * from SQLITE_STAT1;");
        getQuery().next();
        do
        {
//            if(!getQuery().value(0).isValid())
//            {
//                getQuery().exec("SELECT COUNT(*) FROM `" + tableName() + "`;");// Может в системной таблице БД содержится количество строк? Нужно к ней обратиться
//                getQuery().next();
//                qDebug() << getQuery().value(0).toInt();
//                return getQuery().value(0).toInt();
//            }
            if(getQuery().value(0) == tableName())
            {
                int i = getQuery().value(2).toInt();
                getQuery().exec();
                getQuery().exec("DROP TABLE sqlite_stat1;");
                getQuery().next();
                return i;
            }
        }
        while(getQuery().next());

        getQuery().exec("SELECT COUNT(*) FROM `" + tableName() + "`;");// Может в системной таблице БД содержится количество строк? Нужно к ней обратиться
        getQuery().next();
        return getQuery().value(0).toInt();
        // Заменить, если возможно this->query.numRowsAffected()
    }
}


void ProtocolPrinterItemModel::setFilter(const QString &filter)
{
    filtersMemory = filter;
    emit filterChanged();
    QSqlTableModel::setFilter(filter);
    return ;
}
