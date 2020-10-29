#ifndef NEWMODEL_H
#define NEWMODEL_H

#include "protocolprinteritemmodel.h"
#include <QAbstractItemModel>

class ProtocolPrinterHeaderView;
class NewModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    NewModel(ProtocolPrinterItemModel *amodel, QObject *parent = nullptr);
    ProtocolPrinterItemModel *model{nullptr};
    ProtocolPrinterHeaderView *filter{nullptr};

    QList<QString> getNamesColumns() const;
    QSqlQuery& getQuery();

    void setTable(const QString &tableName);
    void setFilter(const QString &filter);
    void setProtocolFilter(ProtocolPrinterHeaderView *filter);
    void setSignals();

    void removeCacheElements(int type, int itemsToFetch);

public slots:
    bool select();
    void reset();
    void magicLoad(int value);
    void magicAllLoad();
//ProtocolPrinterItemModel

// QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual int columnCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;

    virtual void fetchMore(const QModelIndex &parent) override;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    virtual QModelIndex parent(const QModelIndex &child) const override;

public slots:
    virtual bool canFetchMore(const QModelIndex &parent) const override;

private:
    QVector<QPair<QVector<QString>, int>> cacheData;// каждому вектору присваивается его номер строки из таблицы

    int cacheMaxSize{60};// желательно четный
    int cacheSize{0};
    int seek{0};
    int modelAllRowCount{0};

    void loadItems(const int leftBorder, const int rightBorder, const int itemsToFetch);

private slots:
    void updateRowsFilters();

signals:
    void rowsAdded(int count);


    // QAbstractItemModel interface
public:
    virtual void sort(int column, Qt::SortOrder order) override;
};

#endif // NEWMODEL_H
