#ifndef PRINTCONTROLLER_H
#define PRINTCONTROLLER_H
#include "pdfprintsupport.h"
#include "xmlprintsupport.h"
#include <QSqlDatabase>
#include <QObject>


class PrintProgress;
class PrintController : public QObject
{
    Q_OBJECT
    QSqlDatabase db;
    QString typeFile;
    QString directory;
    PrintProgress* progress;
    QMap<QString, QList<QString*>> filtersMemory;

    volatile bool stop;
    struct  FileNameElements
    {
        QString projectName, lruSN, date;
    }/*fileNameElements*/;

    struct FilesCount
    {
        int htmlCount;
        int pdfCount;
    }filesCount;

    static QString CheckRepairfileName(const QString& projectName, const QString& lruSN, const QString& date,
                                       const QString& fileExtension, const QString& directory, QList<QString> &namesFilesExp);
    void PrintHTMLToFile(const QString& html, const QString& fileName);
    void PrintHTMLToFiles(QList<QString> htmlList, QList<QString> htmlFileNameList);
    void PrintHTMLToFiles(QList<QString> htmlList, QList<FileNameElements> htmlFileNameList, const QString& direction);

    void CreateXMLFiles();
    void CreateHTMLFile();
    void CreateXMLAndHTML();

    void WorkAtHTMLFile(const QString& fileName);
    void WorkAtXMLFile(const QString& fileName);
    void WorkAtXMLAndHTMLFile(const QString& fileName);

    void PrintHTMLToPdf(const QString& html, const QString& fileName);
    void PrintHTMLToPdfFiles(QList<QString> htmlList, QList<QString> htmlFileNameList, QList<QHash<QString, ScenarioData>> scenariosHashList);

public:

    PrintController(QSqlDatabase& db, const QString& typeFile, const QString& directory, PrintProgress *progress, QMap<QString, QList<QString*>> filtersMemory, QObject *parent = nullptr);
    static void onExportDBAction(QSqlDatabase& db/*, const QString& typeFile*/, const QString& typeFile,
                                 QString directory, PrintProgress *progress, QMap<QString, QList<QString *> > filtersMemory);

    void setStop(bool stop);


private:
    /// NOTE: scenario name should be unique!!!
    QHash<QString, ScenarioData> s_scenariosData;     // scenario name - scenario tests data

    QList<QString> namesFilesExp;

public slots:
    void close();

};

#endif // PRINTCONTROLLER_H
