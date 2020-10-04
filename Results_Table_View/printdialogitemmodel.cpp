#include "printdialogitemmodel.h"
#include "protocolprinteritemmodel.h"
#include "sqlquerybuilder.h"
#include "translatordata.h"//возможно, уберу

#include <QComboBox>
#include <QDateTime>
#include <QDebug>

PrintDialogItemModel::PrintDialogItemModel(const QList<QString>& namesTables, ProtocolPrinterItemModel* model, ProtocolPrinterHeaderView* header)
{
    this->namesTables = namesTables;
    this->model = model;
    insertRows(0, 1, QModelIndex());
}

QModelIndex PrintDialogItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    if(modelData.size() <= row || column > nameColumns.size())
        return QModelIndex();

    return createIndex(row,column);
}

QModelIndex PrintDialogItemModel::parent(const QModelIndex &/*child*/) const
{
    return {};
}

int PrintDialogItemModel::rowCount(const QModelIndex &/*parent*/) const
{
    return modelData.size();
}

int PrintDialogItemModel::columnCount(const QModelIndex &/*parent*/) const
{
    return nameColumns.size();
}

QVariant PrintDialogItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if(index.row() > modelData.size())
        return QVariant();
    if(index.row() > filtersMemoryMap.size())
        return QVariant();
    if(role == Qt::DisplayRole && index.column() == 2)
    {
        return filtersMemoryMap[index.row()];
    }

    if(role == Qt::UserRole && index.column() == 0)
    {
        QStringList namesTablesStr;
        for(auto var : this->namesTables)
        {
            namesTablesStr.push_back(var);
        }
        namesTablesStr.push_back(QString::number(comboBoxTablesMap.value(index.row())));
        //qDebug() << "namesTablesStr " << namesTablesStr;
        return  namesTablesStr;
    }
    if(role == Qt::UserRole && index.column() == 1)
    {
        QStringList namesColumnsStr;
        auto headerList = model->getHeaderTable(this->namesTables.at(comboBoxTablesMap.value(index.row())));

        for(auto var : headerList)
        {
            namesColumnsStr.push_back(var);
        }
        namesColumnsStr.push_back(QString::number(comboBoxColumnsMap.value(index.row())));
        //qDebug() << "namesColumnsStr " << namesColumnsStr;
        return  namesColumnsStr;
    }

    if(role == Qt::TextAlignmentRole && index.column() == 0)
    {
        return comboBoxTablesMap.value(index.row());
    }
    if(role == Qt::TextAlignmentRole && index.column() == 1)
    {
        return comboBoxColumnsMap.value(index.row());
    }

    return QVariant();
}


QVariant PrintDialogItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return  nameColumns.at(section);

    if(orientation == Qt::Vertical && role == Qt::DisplayRole)
        return QString("№ %1").arg(section+1);

    return QVariant();
}


bool PrintDialogItemModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent, row, row + count - 1);
    modelData.insert(row, count, modelDataItem());
    for (int i = row; i < row + count; ++i)
    {
        comboBoxTablesMap.insert(i, 0);
        comboBoxColumnsMap.insert(i, model->getHeaderTable(namesTables.at(comboBoxTablesMap.value(0))).at(0).toInt());
        filtersMemoryMap.insert(i, "");
        filtersMemory.insert(i, new QString());
    }

    endInsertRows();
    return true;
}

bool PrintDialogItemModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if(modelData.size() < count)
        return false;
    beginRemoveRows(parent, row, row + count - 1);
    modelData.remove(row, count);
    for (int i = row; i < row + count; ++i)
    {
        comboBoxTablesMap.remove(i);
        comboBoxColumnsMap.remove(i);
        filtersMemoryMap.remove(i);
        delete filtersMemory.at(i);
        filtersMemory.removeAt(i);
    }
    endRemoveRows();
    return true;
}


bool PrintDialogItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(modelData.size()<index.row())
        return false;

    if(role == Qt::UserRole && index.column() == 0)
    {
        comboBoxTablesMap.insert(index.row(), value.toInt());
        //qDebug() << "comboBoxTablesMap " << comboBoxTablesMap;
        comboBoxColumnsMap.remove(index.row());
    }

    if(role == Qt::UserRole && index.column() == 1)
    {
        comboBoxColumnsMap.insert(index.row(), value.toInt());
        //qDebug() << "comboBoxColumnsMap " << comboBoxColumnsMap;
    }

    if(role == Qt::EditRole && index.column() == 2)
    {
        filtersMemoryMap.insert(index.row(), value.toString());///TODO

        QList<QString>headerList = model->getHeaderTable(this->namesTables.at(comboBoxTablesMap.value(index.row())));
        setFiltersMemory(*filtersMemory.at(index.row()), value.toString(), headerList, comboBoxColumnsMap.value(index.row()));
    }
    return true;
}


Qt::ItemFlags PrintDialogItemModel::flags(const QModelIndex &index) const
{
    if(!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEditable | QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable ;
}
/////////////////////////////////
QMap<QString, QList<QString*>> PrintDialogItemModel::getFiltersMemory()
{
    QMap<QString, QList<QString*>> mapTemp;
    int number = 0;
//    QList<QString*> filtersMemoryCombined;// это список из прямых SQL запросов к конкретному стобцу таблицы
//    for (auto var : filtersMemoryMap.keys())
//    {
//        filtersMemoryMap.value(var);
//        filtersMemoryCombined.push_back();
//    }
    for (auto var : filtersMemory)
    {
        auto iter = mapTemp.find(this->namesTables.at(comboBoxTablesMap.value(number)));
        if(iter != mapTemp.end())
        {
            iter->push_back(var);
        }
        else
        {
            QList<QString*> list;
            list.push_back(var);
            mapTemp.insert(this->namesTables.at(comboBoxTablesMap.value(number)), list);
        }
        //map.insert(this->namesTables.at(comboBoxTablesMap.value(number)), var);
        ++number;
    }
    QList<QString*> filtersMemoryCombinedList;
    QMap<QString, QList<QString*>> map;
    for (auto key : mapTemp.keys())
    {
        bool first = true;
        for(auto filterString : mapTemp.value(key))
        {
            if(first)
            {
                first = false;
                filtersMemoryCombinedList.push_back(filterString);
            }
            else
            {
                *filtersMemoryCombinedList.last() += (" OR " + *filterString);
            }
        }
        map.insert(key, filtersMemoryCombinedList);
        filtersMemoryCombinedList.clear();
    }
    return map;
}
///////////////////////////////////
void PrintDialogItemModel::setFiltersMemory(QString& filterMemory, const QString& lineEditText, QList<QString>headerList, int lineNumber)
{
    QList<QDateTime> dateTimeList;//namesTables////
    TranslatorData::fillListAllDateTimeDb(dateTimeList, model, lineNumber);

    //qDebug() << "MAGIC: " << nameColumns;
    QList<QString> sessionIdList;

    this->model->getQuery().exec("SELECT `Session ID` FROM `" + model->tableName() + "`;");
    while (this->model->getQuery().next())
    {
        sessionIdList << this->model->getQuery().value(0).toString();
    }
    ///TODO переделать без else if
    // 11/1/2000 9:11:04 - 05/2/2033 12:11:26
    if((model->tableName() == tablesNamesList.at(0)) && (lineNumber == int(SpecColumnsNumb::SesRunDateTime)))//"Test Reports"
    {
        SQLQueryBuilder::checksFilter(sessionIdList, headerList, lineEditText, filterMemory, dateTimeList, filtersMemory, lineNumber, SQLQueryBuilder::TypeColumn::DateTime);

    }else if((model->tableName() == tablesNamesList.at(0)) && (lineNumber == int(SpecColumnsNumb::SesRunTotalTime)))//"Test Reports"
    {
        SQLQueryBuilder::checksFilter(sessionIdList, headerList, lineEditText, filterMemory, dateTimeList, filtersMemory, lineNumber, SQLQueryBuilder::TypeColumn::DateTime);
    }else
    {
        SQLQueryBuilder::checksFilter(sessionIdList, headerList, lineEditText, filterMemory, dateTimeList, filtersMemory, lineNumber, SQLQueryBuilder::TypeColumn::String);
    }
    ///

    QList<QString*> list = filtersMemory;
    for(auto var : list)
    {
        //qDebug() << "VAR: "<<  *var;
    }

    //qDebug() << "getFiltersMemory: " << filtersMemory;
    //qDebug() << "comboBoxColumnsMap: " << comboBoxColumnsMap;
}
