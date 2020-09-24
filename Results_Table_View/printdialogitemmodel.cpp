#include "printdialogitemmodel.h"

#include <QComboBox>
#include <protocolprinterheaderview.h>
#include "protocolprinteritemmodel.h"
#include <QDebug>
#include <QDateTime>

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
        QString namesTablesStr = "";
        for(auto var : this->namesTables)
        {
            namesTablesStr += var + "/";
        }
        namesTablesStr += QString::number(comboBoxTablesMap.value(index.row()));
        qDebug() << "namesTablesStr " << namesTablesStr;
        return  namesTablesStr;
    }
    if(role == Qt::UserRole && index.column() == 1)
    {
        QString namesColumnsStr = "";
        auto headerList = model->getHeaderTable(this->namesTables.at(comboBoxTablesMap.value(index.row())));

        for(auto var : headerList)
        {
            namesColumnsStr += var + "/";
        }
        namesColumnsStr += QString::number(comboBoxColumnsMap.value(index.row()));
        qDebug() << "namesColumnsStr " << namesColumnsStr;
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
        qDebug() << "comboBoxTablesMap " << comboBoxTablesMap;
        comboBoxColumnsMap.remove(index.row());
    }

    if(role == Qt::UserRole && index.column() == 1)
    {
        comboBoxColumnsMap.insert(index.row(), value.toInt());
        qDebug() << "comboBoxColumnsMap " << comboBoxColumnsMap;
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

QMap<QString, QString*> PrintDialogItemModel::getFiltersMemory()
{
    QMap<QString, QString*> map;
    int number = 0;
    for (auto var : filtersMemory)
    {
        map.insert(this->namesTables.at(comboBoxTablesMap.value(number)), var);
        ++number;
    }

    return map;
}

void PrintDialogItemModel::setFiltersMemory(QString& filterMemory, const QString& lineEditText, QList<QString>headerList, int lineNumber)
{
    QList<QDateTime> dateTimeList;//namesTables
    qDebug() << "MAGIC: " << nameColumns;
    QList<QString> sessionIdList;

    this->model->getQuery().exec("SELECT `Session ID` FROM `" + model->tableName() + "`;");
    while (this->model->getQuery().next())
    {
        sessionIdList << this->model->getQuery().value(0).toString();
    }
    ///TODO переделать без else if
    if((model->tableName() == tablesNamesList.at(0)) && (lineNumber == int(SpecColumnsNumb::SesRunDateTime)))//"Test Reports"
    {
        ProtocolPrinterHeaderView::checksFilter(sessionIdList, headerList, lineEditText, filterMemory, dateTimeList, filtersMemory, lineNumber, 1);

    }else if((model->tableName() == tablesNamesList.at(0)) && (lineNumber == int(SpecColumnsNumb::SesRunTotalTime)))//"Test Reports"
    {
        ProtocolPrinterHeaderView::checksFilter(sessionIdList, headerList, lineEditText, filterMemory, dateTimeList, filtersMemory, lineNumber, 1);
    }else
    {
        ProtocolPrinterHeaderView::checksFilter(sessionIdList, headerList, lineEditText, filterMemory, dateTimeList, filtersMemory, lineNumber, 0);
    }
    ///
    //ProtocolPrinterHeaderView::checksFilter(model, nullptr, lineEditText, filterMemory, getFiltersMemory(), lineNumber, count, 0);
    QList<QString*> list = filtersMemory;
    for(auto var : list)
    {
        qDebug() << "VAR: "<<  *var;
    }

    qDebug() << "getFiltersMemory: " << filtersMemory;
    qDebug() << "comboBoxColumnsMap: " << comboBoxColumnsMap;
}
