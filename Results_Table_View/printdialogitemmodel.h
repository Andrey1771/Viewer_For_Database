#ifndef PRINTDIALOGITEMMODEL_H
#define PRINTDIALOGITEMMODEL_H
#include <QAbstractItemModel>

class QComboBox;
class ProtocolPrinterHeaderView;
class ProtocolPrinterItemModel;

class PrintDialogItemModel : public QAbstractItemModel
{
private:
    struct modelDataItem{
        QComboBox *DbTable;
        QComboBox *Column;
        ProtocolPrinterHeaderView *filter;
    };
    QList<QString>nameColumns = {"DB table", "Column", "Filter"};
    QVector<modelDataItem>modelData;


    QList<QString> namesTables;
    QMap<int, int> comboBoxTablesMap;
    QMap<int, int> comboBoxColumnsMap;
    ProtocolPrinterItemModel* model;
    QMap<int, QString> filtersMemoryMap;
public:
    PrintDialogItemModel(const QList<QString> &namesTables, ProtocolPrinterItemModel *model, ProtocolPrinterHeaderView *header);

    // QAbstractItemModel interface
public:
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;

    // QAbstractItemModel interface
public:
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    // QAbstractItemModel interface
public:
    virtual bool insertRows(int row, int count, const QModelIndex &parent) override;
    virtual bool removeRows(int row, int count, const QModelIndex &parent) override;

    // QAbstractItemModel interface
public:
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    // QAbstractItemModel interface
public:
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    QList<QString *> getFiltersMemory();
    void setFiltersMemory(QString &filterMemory, const QString &lineEditText, int lineNumber);
};

#endif // PRINTDIALOGITEMMODEL_H
