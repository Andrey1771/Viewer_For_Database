#include "newmodel.h"
#include "protocolprinterheaderview.h"
#include "protocolprinteritemmodel.h"
#include "sqlquerybuilder.h"

#include <QDebug>

NewModel::NewModel(ProtocolPrinterItemModel *amodel, QObject *parent) : QAbstractItemModel(parent)
{
    this->model = amodel;
}

QList<QString> NewModel::getNamesColumns() const
{
    return model->getNamesColumns();
}

QSqlQuery &NewModel::getQuery()
{
    return model->getQuery();
}

void NewModel::setTable(const QString &tableName)
{
    modelAllRowCount = model->allRowCount();
    reset();
    model->setTable(tableName);

    QString str = SQLQueryBuilder::filtersRQData(filter->getFilterMemoryList());// filtersMemory
    if(!str.isEmpty())
        model->getQuery().exec("SELECT COUNT(*) FROM `" + model->tableName() + "` WHERE " + str + ";");// AS Count
    else
        model->getQuery().exec("SELECT COUNT(*) FROM `" + model->tableName() + "`;");

    model->getQuery().next();
    modelAllRowCount = model->getQuery().value(0).toInt();

    return;
}

void NewModel::setFilter(const QString &filter)
{
    qDebug() << filter;
    reset();
}

void NewModel::setProtocolFilter(ProtocolPrinterHeaderView *filter)
{
    this->filter = filter;
}

void NewModel::setSignals()
{
    connect(this->model, &ProtocolPrinterItemModel::filterChanged, this, &NewModel::updateRowsFilters);
}

void NewModel::updateRowsFilters()
{
    reset();
    qDebug() << SQLQueryBuilder::filtersRQData(filter->getFilterMemoryList());
    QString str = SQLQueryBuilder::filtersRQData(filter->getFilterMemoryList());// filtersMemory
    if(!str.isEmpty())
        model->getQuery().exec("SELECT COUNT(*) FROM `" + model->tableName() + "` WHERE " + str + ";");// AS Count
    else
        model->getQuery().exec("SELECT COUNT(*) FROM `" + model->tableName() + "`;");

    model->getQuery().next();
    modelAllRowCount = model->getQuery().value(0).toInt();

    if(model->getQuery().value(0).toInt() != 0)
    {
        fetchMore(QModelIndex());
        magicLoad(0);
    }

}

bool NewModel::select()
{
    bool ok = model->select();
    updateRowsFilters();
    return ok;
}

void NewModel::reset()
{
    beginResetModel();
    removeRows(0, cacheData.size() - 1);
    cacheData.clear();
    seek = 0;
    cacheSize = 0;
    endResetModel();
}


int NewModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : cacheData.size();
}

int NewModel::columnCount(const QModelIndex &parent) const
{
    return model->columnCount(parent);
}

QVariant NewModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    if((role == Qt::DisplayRole || role == Qt::EditRole) && index.column() == 0)//0 колонка - это Session ID
        return cacheData.at(index.row()).first.at(index.column()).toInt();

    if(role == Qt::DisplayRole || role == Qt::EditRole)
        return cacheData.at(index.row()).first.at(index.column());//cacheData.at(index.row()).at(index.column())

    return QVariant();
}

void NewModel::fetchMore(const QModelIndex &/*parent*/)// Идея такова, он выз один раз, заполняет пустыми элементами, все остальное делает magicLoad
{
    int itemsToFetch = 0;
    itemsToFetch = modelAllRowCount;
    beginInsertRows(QModelIndex(), 0, itemsToFetch - 1);
    QVector<QString> vector;

    for (int j = 0; j < model->columnCount(); ++j)
        vector.push_back("");
    cacheData.fill(QPair<QVector<QString>, int>(vector, 0), itemsToFetch);
    seek += itemsToFetch;
    cacheSize += itemsToFetch;
    endInsertRows();
    emit rowsAdded(itemsToFetch);
}

/**
 * type: 1 - up, 2 - down
 */
void NewModel::removeCacheElements(int type, int itemsToFetch)// удаляет лишние элементы, которые вышли за предел cache
{
    switch (type)
    {
    case 1:
    {
        //Если лишние элементы находятся сверху
        if(cacheSize > cacheMaxSize)
        {
            beginRemoveRows(QModelIndex(), cacheSize - itemsToFetch/*seek - cacheSize*/, cacheSize - 1/*seek - cacheSize*/);// можно оптимизировать
            cacheSize -= itemsToFetch;
            cacheData.remove(cacheSize - itemsToFetch, itemsToFetch);//
            endRemoveRows();
        }
        break;
    }
    case 2:
    {
        //Если лишние элементы находятся снизу
        if(cacheSize > cacheMaxSize)
        {
            beginRemoveRows(QModelIndex(), cacheSize - itemsToFetch/*seek - cacheSize*/, cacheSize - 1/*seek - cacheSize*/);// можно оптимизировать
            cacheSize -= itemsToFetch;
            cacheData.remove(0, itemsToFetch);//
            endRemoveRows();
        }
        break;
    }
    default:
    {
        break;
    }
    }
}

bool NewModel::canFetchMore(const QModelIndex &parent) const
{
    if (parent.isValid())
        return false;
    return seek < modelAllRowCount;
}

QVariant NewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Vertical && role == Qt::DisplayRole)
        return QString("%1").arg(seek - cacheData.size() + section + 1);
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return this->getNamesColumns().at(section);
    return QVariant();
}


QModelIndex NewModel::index(int row, int column, const QModelIndex &/*parent*/) const
{
    if(column > model->columnCount())
        return QModelIndex();

    return createIndex(row,column);
}

QModelIndex NewModel::parent(const QModelIndex &/*child*/) const
{
    return QModelIndex();
}


void NewModel::loadItems(const int leftBorder, const int rightBorder, const int itemsToFetch)
{
    beginInsertRows(QModelIndex(), leftBorder, rightBorder - 1);// -1 под вопросом
    QString str = SQLQueryBuilder::filtersRQData(filter->getFilterMemoryList());// filtersMemory
    if(!str.isEmpty())
        model->getQuery().exec("SELECT * FROM `" + model->tableName() + "` WHERE ROWID > " + QString::number(leftBorder) + " AND " + str + ";");
    else
        model->getQuery().exec("SELECT * FROM `" + model->tableName() + "` WHERE ROWID > " + QString::number(leftBorder) + ";");

    QVector<QString> vector;
    model->getQuery().next();
    for (int i = 0; i < itemsToFetch; ++i)
    {
        for (int j = 0; j < model->columnCount(); ++j)
            vector.push_back(model->getQuery().value(j).toString());

        cacheData.replace(i + leftBorder, QPair<QVector<QString>, int>(vector, 1));
        vector.clear();
        model->getQuery().next();
    }
    model->getQuery().finish();
    model->getQuery().exec();
}

void NewModel::magicLoad(int value)
{
    if(cacheData.isEmpty())//Исключение, возникает после reset
    {
        return;
    }

    int leftBorder = value - cacheMaxSize / 2;
    if(leftBorder < 0)
        leftBorder = 0;

    int rightBorder = value + cacheMaxSize / 2;
    if(rightBorder > modelAllRowCount)
        rightBorder = modelAllRowCount;

    int itemsToFetch = rightBorder - leftBorder;
    if (itemsToFetch <= 0)
        return;

    loadItems(leftBorder, rightBorder, itemsToFetch);
}

void NewModel::magicAllLoad()
{
    if(cacheData.isEmpty())//Исключение, возникает после reset
    {
        return;
    }

    int leftBorder = 0;
    int rightBorder = modelAllRowCount;

    int itemsToFetch = rightBorder - leftBorder;
    if (itemsToFetch <= 0)
        return;

    loadItems(leftBorder, rightBorder, itemsToFetch);
}

void NewModel::sort(int column, Qt::SortOrder order)
{
    return QAbstractItemModel::sort(column, order);
}
