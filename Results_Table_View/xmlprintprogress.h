#ifndef XMLPRINTPROGRESS_H
#define XMLPRINTPROGRESS_H
#include <QObject>
#include <QFutureWatcher>

class XMLPrintSupport;

class XMLPrintProgress : public QObject
{
    Q_OBJECT
    float progressCount;
    float partProgress;
    volatile bool stop;
    XMLPrintSupport* printSup;
public:
    XMLPrintProgress(QObject* parent = nullptr);
    void setProgressCount(float progressCount);
    float getProgressCount();
    void setPartProgress(float partProgress);
    float getPartProgress();

    QFutureWatcher<void> watcher;

    XMLPrintSupport *getPrintSup() const;
    void setPrintSup(XMLPrintSupport *value);
    void sendCountFiles(int countFiles);
signals:
    void progressChanged(float progress);
    void countedFiles(int countFiles);

};


#endif // XMLPRINTPROGRESS_H
