#ifndef PROTOCOLPRINTERHEADERVIEW_H
#define PROTOCOLPRINTERHEADERVIEW_H

#include <QHeaderView>

class QTableView;
class ProtocolPrinterItemModel;
class NewModel;
class LineEditExtended;

class ProtocolPrinterHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    explicit ProtocolPrinterHeaderView(QTableView *parent = nullptr);
    // QWidget interface
    QSize sizeHint() const override;

    void clearPrimaryMemory();
    void launch(QTableView *parent);

    void setSecondFilterDatabase(ProtocolPrinterItemModel *model2, ProtocolPrinterHeaderView *header2);
    const QString filtersRQData();
    virtual void startSetSettings(QTableView *parent);

protected:
     void updateGeometries() override;

public slots:
    void generateFilters(int number, bool showFirst = false);
    void adjustPositions();
    void clearFilters();
    void setFilter(int column, const QString& value);

    void send();
    virtual void update(QAbstractTableModel *model);
    void linesUpdate();
    void rowSelect(QModelIndexList modelIndexListSelected, QModelIndexList modelIndexListDeselected);
    void updateFilter2();

protected:
    QVector<LineEditExtended*> LineVect{nullptr};
    ProtocolPrinterItemModel *model{nullptr};
    ProtocolPrinterItemModel *model2{nullptr};
    ProtocolPrinterHeaderView *header2{nullptr};
    unsigned int numberLine{0};
    int adjPosHint;
    QString primaryMemory;
    QList<int> listSelect;

    inline void checksFilter(const QString& str, LineEditExtended *line, int type);

    void setPrimaryFilter(const QList <int>& list, bool reset = false);
    QList<int> makePrimaryFilter();

signals:
    void updateFiltersComp();

};

#endif // PROTOCOLPRINTERHEADERVIEW_H
