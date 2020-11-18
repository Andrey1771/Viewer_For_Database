#ifndef PRINTDIALOG_H
#define PRINTDIALOG_H

#include "printprogress.h"
#include <QDialog>

class ProtocolPrinterItemModel;
class PrintDialogItemModel;
class ProtocolPrinterHeaderView;
class QSqlDatabase;
class PrintSupport;
//class XMLPrintProgress;
namespace Ui {
class PrintDialog;
}

class PrintDialog : public QDialog
{
    Q_OBJECT
    PrintDialogItemModel* printDialogItemModel{nullptr};
    QSqlDatabase* db;
    PrintSupport* printSup{nullptr};
    const QList<QString> exportTypes{"PDF + Excel", "PDF", "Excel"};
public:
    explicit PrintDialog(QList<QString> namesTables, ProtocolPrinterItemModel *model, ProtocolPrinterHeaderView *header, QSqlDatabase& db, const QString &pathFile, QWidget *parent = nullptr);
    ~PrintDialog();

    struct DataForSaver
    {
        QString pathSave;
        QString pathFile;

        DataForSaver(){this->pathSave = ""; this->pathFile = "";}
        DataForSaver(QString pathSave, QString pathFile)
        {
            this->pathSave = pathSave;
            this->pathFile = pathFile;
        }
        DataForSaver(const DataForSaver& dataForSave)
        {
            this->pathSave = dataForSave.pathSave;
            this->pathFile = dataForSave.pathFile;
        }
    };
    DataForSaver getPathForSaver();

public slots:
    void onAddRow();
    void onRemoveRow();
    void onExport();
    void onCancel();
    void launchFilePathWizard();
    void progressBarSetVal(float val);
    void finishedWork();
    void startWork();
    void dialogWarning(int countFiles);
    void messageFilePrinted(int type);
    void setLabelTypePrintFilePDF();
private:
    Ui::PrintDialog *ui;
    PrintProgress* progress{nullptr};

    QString lastPath;

    // QWidget interface
protected:
    virtual void closeEvent(QCloseEvent *event) override;
};

#endif // PRINTDIALOG_H
