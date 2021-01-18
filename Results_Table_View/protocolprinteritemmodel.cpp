#include "protocolprinteritemmodel.h"
#include "qsqlfield.h"

#include <QSqlRecord>
#include <QSqlDriver>

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
    namesColumns = getHeaderTable(tableName);
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

QList<QString> ProtocolPrinterItemModel::getHeaderTable(const QString &tableName)
{
    QList<QString> namesColumns;
    query.exec("select * from '"+ tableName+ "';"); // Приходится делать запрос, чтобы узнать структуру таблицы
    int count = query.record().count(); // упростить
    countColumns = count;
    for (int i=0; i < count; ++i)
    {
        namesColumns << query.record().field(i).name();
    }
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

void ProtocolPrinterItemModel::allRowLoad()
{
    while(canFetchMore(QModelIndex()))
    {
        fetchMore(QModelIndex());
    }
}
