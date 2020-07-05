#ifndef PROTOCOLPRINTETABLEVIEW_H
#define PROTOCOLPRINTETABLEVIEW_H

#include <QTableView>

class ProtocolPrinterTableView : public QTableView
{
    Q_OBJECT
public:
    ProtocolPrinterTableView(QWidget *parent = nullptr);

protected slots:
    virtual void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;
signals:
    void selectRow(QModelIndexList modelIndexList, QModelIndexList deselected);
    void selectionSet();
    // QWidget interface
protected:
    virtual void focusInEvent(QFocusEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;

    // QObject interface
public:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

    // QAbstractItemView interface
protected slots:
    virtual void verticalScrollbarAction(int action) override;
};

#endif // PROTOCOLPRINTETABLEVIEW_H