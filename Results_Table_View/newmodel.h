#ifndef NEWMODEL_H
#define NEWMODEL_H

#include <QAbstractItemModel>
#include "protocolprinteritemmodel.h"

///////////////
enum TypeRemove{
    up = 1, down = 2
};
///////////////
///
///
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
    void setProtocolFilter(ProtocolPrinterHeaderView *filter);
    void setSignals();

    void removeCacheElements(TypeRemove type, int itemsToFetch);

public slots:
    bool select();
    void reset();
    void magicLoad(int value);

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
    /**
     * @brief fetchMoreSecond - добавляем строки сверху
     * Требует отдельного соединения с ScrollBar
     */
    void fetchMoreSecond();
    virtual bool canFetchMore(const QModelIndex &parent) const override;

private:
    QVector<QPair<QVector<QString>, int>> cacheData;// каждому вектору присваивается его номер строки из таблицы

    int cacheMaxSize{60};// желательно четный
    int cacheSize{0};
    int seek{0};
    int modelAllRowCount{0};

private slots:
    void updateRowsFilters();

signals:
    void rowsAdded(int count);
};

#endif // NEWMODEL_H
