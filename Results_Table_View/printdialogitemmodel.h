#ifndef PRINTDIALOGITEMMODEL_H
#define PRINTDIALOGITEMMODEL_H
#include "protocolprinterheaderview.h"

#include <QAbstractItemModel>
#include <QComboBox>

class QComboBox;
class ProtocolPrinterItemModel;
class PrintDialogItemModel : public QAbstractItemModel
{
private:
    /// TODO перенести в отдельный файл все констаты связанные с БД и таблицами
    const QStringList tablesNamesList{"Acceptance Test Reports"};// Те таблицы, которые будут поддерживать фильтр с временем
    enum class SpecColumnsNumb{
        SesId = 0,
        SesRunDateTime = 9,
        SesRunTotalTime = 10
    };
    ///
    struct ModelDataItem{
        QComboBox *dbTable{nullptr};
        QComboBox *column{nullptr};
        ProtocolPrinterHeaderView *filter{nullptr};
        ~ModelDataItem()
        {
            if(dbTable != nullptr)
                delete dbTable;
            if(column != nullptr)
                delete column;
            if(filter != nullptr)
                delete filter;
        }
    };
    QList<QString> nameColumns = {"DB table", "Column", "Filter"};
    QVector<ModelDataItem> modelData;


    QList<QString> namesTables;
    //QMap<int, int> comboBoxTablesMap;
    //QMap<int, int> comboBoxColumnsMap;
    ProtocolPrinterItemModel* model;
    //QMap<int, QString> filtersMemoryMap;
    QList<QString*> filtersSQLMemory;

    struct RowData
    {
        int tableNumber;
        int columnNumber;
        QString filterMemory;

        RowData(){tableNumber = 0; columnNumber = 0; filterMemory = "";}
    };
    QVector<RowData> rowsDataVector;

    QList<QString*> makeFiltersMemory();

public:
    PrintDialogItemModel(const QList<QString> &namesTables, ProtocolPrinterItemModel *model, ProtocolPrinterHeaderView *header);

    // QAbstractItemModel interface
public:
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    virtual bool insertRows(int row, int count, const QModelIndex &parent) override;
    virtual bool removeRows(int row, int count, const QModelIndex &parent) override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    QMap<QString, QList<QString *> > getFiltersMemory();
    void setFiltersMemory(QString &filterMemory, const QString &lineEditText, QList<QString> headerList, int lineNumber);

};

#endif // PRINTDIALOGITEMMODEL_H
