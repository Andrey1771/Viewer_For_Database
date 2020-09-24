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
    /// TODO перенести в отдельный файл все констаты связанные с БД и таблицами
    const QStringList tablesNamesList{"Acceptance Test Reports"};// Те таблицы, которые будут поддерживать фильтр с временем
    enum class SpecColumnsNumb{
        SesRunDateTime = 9,
        SesRunTotalTime = 10
    };
    ///
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
    static QString filtersRQData(QList<QString*> filterMemoryList);
    static void checksFilter(ProtocolPrinterItemModel* model, ProtocolPrinterItemModel* model2, ProtocolPrinterHeaderView *headerView, const QString& str, QString& filterMemory, QList<QString*> filterMemoryList, int lineNumber, int type);
    static void checksFilter(QList<QString>& sessionIdList/*Может быть пустым*/, QList<QString>headerList, const QString &str, QString &filterMemory, QList<QDateTime>& listAllDateTimeDb, QList<QString *> filterMemoryList, int lineNumber, int type);
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

    static void setPrimaryFilter(ProtocolPrinterHeaderView *headerView, const QList <int>& list, bool reset = false);


signals:
    void updateFiltersComp();

};

#endif // PROTOCOLPRINTERHEADERVIEW_H
