#ifndef PRINTPROGRESS_H
#define PRINTPROGRESS_H

#include <QObject>
#include <QFutureWatcher>
#include <QSharedPointer>

class PrintController;

enum class TypePrint
{
    XML,
    PDF,
    XMLPDF
};

class PrintProgress : public QObject
{
    Q_OBJECT
    float progressCount;
    float partProgress;
    volatile bool stop;

    QSharedPointer<PrintController> printSup;// Он должен будет удалится, когда начнется новая печать или закроется printDialog
    int xmlPdfNumber;

public:
    PrintProgress(QObject* parent = nullptr);
    ~PrintProgress();
    void setProgressCount(float progressCount);
    float getProgressCount();
    void setPartProgress(float partProgress);
    float getPartProgress();

    QFutureWatcher<void> watcher;

    PrintController *getPrintSup() const;
    void startNewPrintSup(PrintController *value);
    void setTypePrint(TypePrint type);
    TypePrint getTypePrint();
    int getXMLPDFNumber();
    void setXMLPDFNumber(int xmlPdfNumber);
private:
    TypePrint type;

signals:
    void progressChanged(float progress);
    void countedFiles(int countFiles);
    void filePrinted(int type);
    void typePrintFileChanged();

};


#endif // PRINTPROGRESS_H
