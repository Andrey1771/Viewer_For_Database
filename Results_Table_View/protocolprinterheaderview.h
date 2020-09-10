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
    virtual void startSetSettings(QTableView *parent);

    QVector<LineEditExtended*> getLineVect();

    ///TODO перенести в отдельный класс
    static QString filtersRQData(QList<QString*> filterMemoryList);//const QString filtersRQData(QString &filterMemory, int count);
    static void checksFilter(ProtocolPrinterItemModel* model, ProtocolPrinterItemModel* model2, const QString& str, QString& filterMemory, QList<QString*> filterMemoryList, int lineNumber, int count, int type);
    static void checksFilter(QList<QString>headerList, const QString &str, QString &filterMemory, QList<QString *> filterMemoryList, int lineNumber, int count, int type);
    static QList<int> makePrimaryFilter(ProtocolPrinterItemModel* model);
    QList<QString*> getFilterMemoryList(/*QVector<LineEditExtended*> lineVect*/);
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

    //inline void checksFilter(const QString& str, LineEditExtended *line, int type);

    void setPrimaryFilter(const QList <int>& list, bool reset = false);


signals:
    void updateFiltersComp();

};

#endif // PROTOCOLPRINTERHEADERVIEW_H
