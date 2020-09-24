#ifndef PRINTDIALOG_H
#define PRINTDIALOG_H

#include <QDialog>

class ProtocolPrinterItemModel;
class PrintDialogItemModel;
class ProtocolPrinterHeaderView;
class QSqlDatabase;
class XMLPrintSupport;
class XMLPrintProgress;
namespace Ui {
class PrintDialog;
}

class PrintDialog : public QDialog
{
    Q_OBJECT
    PrintDialogItemModel* printDialogItemModel{nullptr};
    QSqlDatabase* db;
    XMLPrintSupport* printSup{nullptr};
    const QList<QString> exportTypes{"PDF + XML", "PDF", "XML"};
public:
    explicit PrintDialog(const QList<QString>& namesTables, ProtocolPrinterItemModel *model, ProtocolPrinterHeaderView *header, QSqlDatabase& db, QWidget *parent = nullptr);
    ~PrintDialog();
public slots:
    void onAddRow();
    void onRemoveRow();
    void onExport();
    void onCancel();
    void launchFilePathWizard();
    void progressBarSetVal(float val);
    void finishedWork();
    void startWork();

private:
    Ui::PrintDialog *ui;
    QString getFiltersMemory();
    void updateMemoryFilters();
    XMLPrintProgress* progress{nullptr};

    // QWidget interface
protected:
    virtual void closeEvent(QCloseEvent *event) override;
};

#endif // PRINTDIALOG_H
