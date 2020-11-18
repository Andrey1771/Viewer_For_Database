#ifndef PRINTCONTROLLER_H
#define PRINTCONTROLLER_H
#include "pdfprintsupport.h"
#include "xmlprintsupport.h"
#include <QSqlDatabase>
#include <QObject>
#include <QPrinter>

class PrintProgress;
class QPainterr;
class QTextDocument;
class PrintController : public QObject
{
    Q_OBJECT

    static const int textMargins = 12; // in millimeters
    static const int borderMargins = 10; // in millimeters

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


    QList<QList<TableHTML>> tablesHTMLInFiles;

    static QString CheckRepairfileName(const QString& projectName, const QString& lruSN, const QString& date,
                                       const QString& fileExtension, const QString& directory, QList<QString> &namesFilesExp);
    void PrintXMLToFile(const QString& xml, const QString& fileName);
    void PrintXMLToFiles(QList<QString> xmlList, QList<QString> htmlFileNameList);
    void PrintXMLToFiles(QList<QString> xmlList, QList<FileNameElements> htmlFileNameList, const QString& direction);

    void CreateXMLFiles();
    void CreateHTMLFile();
    void CreateXMLAndHTML();

    void WorkAtHTMLFile(const QString& fileName);
    void WorkAtXMLFile(const QString& fileName);
    void WorkAtXMLAndHTMLFile(const QString& fileName);

    void PrintHTMLToPdf(const QString& fileName, const QList<QString> &tablesInFile, const QList<TableHTML> &tablesHTML);
    void PrintHTMLToPdfFiles(QList<QString> htmlFileNameList, QList<QHash<QString, ScenarioData>> scenariosHashList, const QMap<int, QList<QString> > &tablesInFile);

public:

    PrintController(QSqlDatabase& db, const QString& typeFile, const QString& directory, PrintProgress *progress, QMap<QString, QList<QString*>> filtersMemory, QObject *parent = nullptr);
    static void onExportDBAction(QSqlDatabase& db/*, const QString& typeFile*/, const QString& typeFile,
                                 QString directory, PrintProgress *progress, QMap<QString, QList<QString *> > filtersMemory);

    void setStop(bool stop);


private:
    /// NOTE: scenario name should be unique!!!
    QHash<QString, ScenarioData> s_scenariosData;     // scenario name - scenario tests data

    QList<QString> namesFilesExp;
    QMap<int, QList<QString>> tablesInFile;
    double mmToPixels(QPrinter& printer, int mm);
    void paintPage(int pageNumber, QPainter* painter, QTextDocument* doc,
                   const QRectF& textRect, qreal footerHeight, const QString &tableName, int numberCurrentPageDoc, int firstVal, const QString &fileNamePath);

    void printDocument(QPrinter& printer, const QList<QString> &tablesInFile, const QList<TableHTML> &tablesHTML, const QString &fileNamePath);


    struct TestResultsDataInFile
    {
        QString html;
        int numberTablesInFile;
        TestResultsDataInFile(const QString& ahtml = "", int anumberTablesInFile = -1){ html = ahtml; numberTablesInFile = anumberTablesInFile;}
    };
    TestResultsDataInFile findTestResultsStrNum(const QList<QString>& tablesInFile, const QList<TableHTML> &tablesHTML);

    struct DocumentPrintParameters
    {
        QList<QString> docList;
        QList<int> docCountPagesList;
        QList<QString> tableNameList;
        int sizeDocument = 0;
        QString newTestHTML;

        DocumentPrintParameters(const QList<QString>& adocList, QList<int>& adocCountPagesList, const QList<QString>& atableNameList,
                                int asizeDocument, const QString& anewTestHTML)
        {
            docList = adocList;
            docCountPagesList = adocCountPagesList;
            tableNameList = atableNameList;
            sizeDocument = asizeDocument;
            newTestHTML = anewTestHTML;
        }
    };

    DocumentPrintParameters countingSettingPages(QTextDocument& doc2, const QList<QString>& tablesInFile, const TestResultsDataInFile& testDataInFile, const QList<TableHTML> &tablesHTML);
    QString settingTestPageForDoc(int sizeDocument, int numTable, QTextDocument& doc2, const QList<QString>& tablesInFile, const QString& testDataHTMLInFile);

public slots:
    void close();

};

#endif // PRINTCONTROLLER_H
