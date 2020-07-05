#include "newmodel.h"

#include <QDebug>
#include <QTimer>
#include "protocolprinterheaderview.h"
#include "protocolprinteritemmodel.h"
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
    QString str = filter->filtersRQData();// filtersMemory

    if(!str.isEmpty())
        model->getQuery().exec("SELECT COUNT(*) FROM `" + model->tableName() + "` WHERE " + str + ";");// AS Count
    else
        model->getQuery().exec("SELECT COUNT(*) FROM `" + model->tableName() + "`;");

    model->getQuery().next();
    modelAllRowCount = model->getQuery().value(0).toInt();

    return;
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

    QString str = filter->filtersRQData();// filtersMemory
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

    if(role == Qt::DisplayRole || role == Qt::EditRole)
        return cacheData.at(index.row()).first.at(index.column());//cacheData.at(index.row()).at(index.column())

    return QVariant();
}


void NewModel::fetchMore(const QModelIndex &parent)
{
    int itemsToFetch = modelAllRowCount;
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
void NewModel::removeCacheElements(TypeRemove type, int itemsToFetch)// удаляет лишние элементы, которые вышли за предел cache
{
    switch (type)
    {
    case up:
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
    case down:
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


void NewModel::fetchMoreSecond()
{
    int remainder = seek - cacheSize;
    int itemsToFetch = qMin(cacheMaxSize, remainder);
    if (itemsToFetch <= 0)
        return;

    //Начало добавления строк в cache из БД
    beginInsertRows(QModelIndex(), 0, itemsToFetch - 1);// не уверен в числах
    QString str = filter->filtersRQData();// filtersMemory
    if(!str.isEmpty())
        model->getQuery().exec("SELECT * FROM `"+model->tableName()+"` WHERE ROWID <= " + QString::number(seek - cacheSize) + " AND " + str + ";");
    else
        model->getQuery().exec("SELECT * FROM `"+model->tableName()+"` WHERE ROWID <= " + QString::number(seek - cacheSize) + ";");

    QVector<QString> vector;
    model->getQuery().last();
    for (int i = itemsToFetch - 1; i > -1 ; --i)
    {
        for (int j = 0; j < model->columnCount(); ++j)
            vector.push_back(model->getQuery().value(j).toString());

        cacheData.push_back(QPair<QVector<QString>, int>(vector, i));
        vector.clear();
        model->getQuery().previous();
    }

    model->getQuery().finish();
    model->getQuery().exec();

    seek -= itemsToFetch;
    cacheSize += itemsToFetch;
    endInsertRows();
    //Конец добавления строк в cache из БД

    QPair<QVector<QString>, int> buf;
    if(cacheSize - itemsToFetch*2 == 0)// если количество подгружаемых элементов меньше cacheSize, то останутся элементы, которые будут находится не на своем месте
    {
        removeCacheElements(TypeRemove::down, itemsToFetch);
        for(int i = cacheSize - 1; i >  -1 ; --i)
        {
            buf = cacheData.at(i);
            cacheData.remove(i);
            cacheData.push_back(buf);
        }
    }
    else
    {
        removeCacheElements(TypeRemove::up, itemsToFetch);
        int i = cacheSize - 1;
        for(; i > cacheSize - itemsToFetch -1 ; --i)
        {
            buf = cacheData.at(i);
            cacheData.remove(i);
            cacheData.push_back(buf);
        }

        for (int j = 0; j <= i; ++j)
        {
            buf = cacheData.at(0);
            cacheData.remove(0);
            cacheData.push_back(buf);
        }
    }

    emit rowsAdded((-itemsToFetch));
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


QModelIndex NewModel::index(int row, int column, const QModelIndex &parent) const
{
    if(column > model->columnCount())
        return QModelIndex();

    return createIndex(row,column);
}

QModelIndex NewModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}

void NewModel::magicLoad(int value)
{
    if(cacheData.isEmpty())//Исключение, возникает после reset
    {
        return;
    }

    int magic[2];
    magic[0] = value - cacheMaxSize/2;
    if(magic[0] < 0)
        magic[0] = 0;

    magic[1] = value + cacheMaxSize/2;
    if(magic[1] > modelAllRowCount)
        magic[1] = modelAllRowCount;

    int itemsToFetch = magic[1] - magic[0];
    if (itemsToFetch <= 0)
        return;

    beginInsertRows(QModelIndex(), magic[0], magic[1] - 1);// -1 под вопросом
    QString str = filter->filtersRQData();// filtersMemory
    if(!str.isEmpty())// сделать так везде
        model->getQuery().exec("SELECT * FROM `"+model->tableName()+"` WHERE ROWID > " + QString::number(magic[0]) + " AND " + str + ";");
    else
        model->getQuery().exec("SELECT * FROM `"+model->tableName()+"` WHERE ROWID > " + QString::number(magic[0]) + ";");

    QVector<QString> vector;
    model->getQuery().next();
    for (int i = 0; i < itemsToFetch; ++i)
    {
        for (int j = 0; j < model->columnCount(); ++j)
            vector.push_back(model->getQuery().value(j).toString());

        cacheData.replace(i + magic[0], QPair<QVector<QString>, int>(vector, 1));
        vector.clear();
        model->getQuery().next();
    }
    model->getQuery().finish();
    model->getQuery().exec();
}

