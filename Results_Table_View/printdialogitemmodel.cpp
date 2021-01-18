#include "printdialogitemmodel.h"
#include "protocolprinteritemmodel.h"
#include "sqlquerybuilder.h"
#include "translatordata.h"

#include <QDateTime>

#include <QDebug>

PrintDialogItemModel::PrintDialogItemModel(const QList<QString>& namesTables, ProtocolPrinterItemModel* model, ProtocolPrinterHeaderView* /*header*/)
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
    if(index.row() > rowsDataVector.size()/*filtersMemoryMap.size()*/)
        return QVariant();
    if(role == Qt::DisplayRole && index.column() == 2)
    {
        return rowsDataVector[index.row()].filterMemory/*filtersMemoryMap[index.row()]*/;
    }

    if(role == Qt::UserRole && index.column() == 0)
    {
        QStringList namesTablesStr;
        for(auto var : namesTables)
        {
            namesTablesStr.push_back(var);
        }
        namesTablesStr.push_back(QString::number(rowsDataVector[index.row()].tableNumber/*comboBoxTablesMap.value(index.row())*/));
        //qDebug() << "namesTablesStr " << namesTablesStr;
        return  namesTablesStr;
    }
    if(role == Qt::UserRole && index.column() == 1)
    {
        QStringList namesColumnsStr;
        auto headerList = model->getHeaderTable(namesTables.at(rowsDataVector[index.row()].tableNumber/*comboBoxTablesMap.value(index.row())*/));

        for(auto var : headerList)
        {
            namesColumnsStr.push_back(var);
        }
        namesColumnsStr.push_back(QString::number(rowsDataVector[index.row()].columnNumber/*comboBoxColumnsMap.value(index.row())*/));
        //qDebug() << "namesColumnsStr " << namesColumnsStr;
        return  namesColumnsStr;
    }

    if(role == Qt::TextAlignmentRole && index.column() == 0)
    {
        return rowsDataVector[index.row()].tableNumber/*comboBoxTablesMap.value(index.row())*/;
    }
    if(role == Qt::TextAlignmentRole && index.column() == 1)
    {
        return rowsDataVector[index.row()].columnNumber/*comboBoxColumnsMap.value(index.row())*/;
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
    modelData.insert(row, count, ModelDataItem());
    for (int i = row; i < row + count; ++i)
    {
        rowsDataVector.insert(i, RowData());
        filtersSQLMemory.insert(i, new QString());
    }

    endInsertRows();
    return true;
}

/**
 * @brief Удаляет только одну строчку пока
 */
bool PrintDialogItemModel::removeRows(int row, int count, const QModelIndex &parent)
{
    qDebug() << "removeRows!!!! row: " << row << "count: " << count;
    if(modelData.size() < row + count)
        return false;
    beginRemoveRows(parent, row, row + count - 1);
    modelData.remove(row, count);

    for (int i = row; i < row + count; ++i)
    {
        rowsDataVector.remove(row);
        qDebug() << "filtersMemory " << filtersSQLMemory << " size " << filtersSQLMemory.size();
        delete filtersSQLMemory.at(row);
        filtersSQLMemory.removeAt(row);
    }
    endRemoveRows();
    qDebug() << "Ok!!!";
    return true;
}


bool PrintDialogItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(modelData.size()<index.row())
        return false;

    if(role == Qt::UserRole && index.column() == 0)
    {
        rowsDataVector[index.row()].tableNumber = value.toInt();
        rowsDataVector[index.row()].columnNumber = 0;
        rowsDataVector[index.row()].filterMemory = "";
    }

    if(role == Qt::UserRole && index.column() == 1)
    {
        rowsDataVector[index.row()].columnNumber = value.toInt();
    }

    if(role == Qt::EditRole && index.column() == 2)
    {
        rowsDataVector[index.row()].filterMemory = value.toString();
        QList<QString>headerList = model->getHeaderTable(namesTables.at(rowsDataVector[index.row()].tableNumber/*comboBoxTablesMap.value(index.row())*/));
        setFiltersMemory(*filtersSQLMemory.at(index.row()), value.toString(), headerList, rowsDataVector[index.row()].columnNumber/*comboBoxColumnsMap.value(index.row())*/);
    }
    return true;
}


Qt::ItemFlags PrintDialogItemModel::flags(const QModelIndex &index) const
{
    if(!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEditable | QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable ;
}

QMap<QString, QList<QString*>> PrintDialogItemModel::getFiltersMemory()
{
    QMap<QString, QList<QString*>> mapTemp;
    int number = 0;

    for (auto var : filtersSQLMemory)
    {
        auto iter = mapTemp.find(namesTables.at(rowsDataVector[number].tableNumber));
        if(iter != mapTemp.end())
        {
            iter->push_back(var);
        }
        else
        {
            QList<QString*> list;
            list.push_back(var);
            mapTemp.insert(namesTables.at(rowsDataVector[number].tableNumber), list);
        }
        ++number;
    }
    QList<QString*> filtersMemoryCombinedList;
    QMap<QString, QList<QString*>> map;
    for (auto key : mapTemp.keys())
    {
        bool first = true;
        for(auto filterString : mapTemp.value(key))
        {
            if(*filterString == "")
                continue;

            if(first)
            {
                first = false;
                filtersMemoryCombinedList.push_back(filterString);
            }
            else
            {
                *filtersMemoryCombinedList.last() += (" OR " + *filterString);
                qDebug() << "*filtersMemoryCombinedList.last();!!! " << *filtersMemoryCombinedList.last();
            }
        }
        map.insert(key, filtersMemoryCombinedList);
        filtersMemoryCombinedList.clear();
    }
    return map;
}

void PrintDialogItemModel::setFiltersMemory(QString& filterMemory, const QString& lineEditText, QList<QString>headerList, int lineNumber)
{
    QList<QDateTime> dateTimeList;//namesTables////
    QList<int> valuesList;

    QList<QString> sessionIdList;

    this->model->getQuery().exec("SELECT `Session ID` FROM `" + model->tableName() + "`;");
    while (this->model->getQuery().next())
    {
        sessionIdList << this->model->getQuery().value(0).toString();
    }
    //TODO переделать без else if
    // 11/1/2000 9:11:04 - 05/2/2033 12:11:26
    if(model->tableName() == tablesNamesList.at(0) && lineNumber == int(SpecColumnsNumb::SesId))
    {
        TranslatorData::fillListAllValueDb(valuesList, model, lineNumber);
        SQLQueryBuilder::checksFilter(sessionIdList, headerList, lineEditText, filterMemory, dateTimeList, valuesList, filtersSQLMemory, lineNumber, SQLQueryBuilder::TypeColumn::Value);
    }
    else if((model->tableName() == tablesNamesList.at(0)) && (lineNumber == int(SpecColumnsNumb::SesRunDateTime)))//"Test Reports"
    {
        TranslatorData::fillListAllDateTimeDb(dateTimeList, model, lineNumber);
        SQLQueryBuilder::checksFilter(sessionIdList, headerList, lineEditText, filterMemory, dateTimeList, valuesList, filtersSQLMemory, lineNumber, SQLQueryBuilder::TypeColumn::DateTime);

    }else if((model->tableName() == tablesNamesList.at(0)) && (lineNumber == int(SpecColumnsNumb::SesRunTotalTime)))//"Test Reports"
    {
        TranslatorData::fillListAllDateTimeDb(dateTimeList, model, lineNumber);
        SQLQueryBuilder::checksFilter(sessionIdList, headerList, lineEditText, filterMemory, dateTimeList, valuesList, filtersSQLMemory, lineNumber, SQLQueryBuilder::TypeColumn::DateTime);
    }else
    {
        SQLQueryBuilder::checksFilter(sessionIdList, headerList, lineEditText, filterMemory, dateTimeList, valuesList, filtersSQLMemory, lineNumber, SQLQueryBuilder::TypeColumn::String);
    }
}


