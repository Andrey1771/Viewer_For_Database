#ifndef PRINTDIALOG_H
#define PRINTDIALOG_H

#include <QDialog>

class ProtocolPrinterItemModel;
class PrintDialogItemModel;
class ProtocolPrinterHeaderView;
class QSqlDatabase;

namespace Ui {
class PrintDialog;
}

class PrintDialog : public QDialog
{
    Q_OBJECT
    PrintDialogItemModel* printDialogItemModel{nullptr};
    QSqlDatabase* db;

    const QList<QString> exportTypes{"PDF", "XML"};
public:
    explicit PrintDialog(const QList<QString>& namesTables, ProtocolPrinterItemModel *model, ProtocolPrinterHeaderView *header, QSqlDatabase& db, QWidget *parent = nullptr);
    ~PrintDialog();
public slots:
    void onAddRow();
    void onRemoveRow();
    void onExport();
    void onCancel();

private:
    Ui::PrintDialog *ui;
    QString getFiltersMemory();
};

#endif // PRINTDIALOG_H
