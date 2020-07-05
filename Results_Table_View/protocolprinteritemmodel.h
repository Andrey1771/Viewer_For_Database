#ifndef PROTOCOLPRINTERITEMMODEL_H
#define PROTOCOLPRINTERITEMMODEL_H

#include <QSqlTableModel>
#include <QSqlQuery>

class ProtocolPrinterItemModel : public QSqlTableModel
{
    Q_OBJECT
public:
    ProtocolPrinterItemModel(QObject *parent = nullptr);
    int allRowCount();

    QList<QString> getNamesColumns() const;
    QSqlQuery& getQuery();
    // QAbstractItemModel interface
public:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // QSqlTableModel interface
public:
    void setTable(const QString &tableName) override;

private:
    QSqlQuery query;
    QList <QString> namesColumns;
    int countColumns;
    QString filtersMemory;
signals:
    void TableChanged();
    void filterChanged();

    // QSqlTableModel interface
public:
    virtual void setFilter(const QString &filter) override;

};

#endif // PROTOCOLPRINTERITEMMODEL_H
