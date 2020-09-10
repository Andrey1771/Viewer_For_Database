#ifndef XMLPRINTSUPPORT_H
#define XMLPRINTSUPPORT_H

#include <QSqlDatabase>
#include <QStringList>

class XMLPrintSupport
{
private:
    enum class NASKTableType
    {
        AcceptanceTestReports,
        SuspectListOfSRUs,
        VisualInspection,
        SummaryScenarioTestResults,
        SummaryTestResults,
        IndividualScenarioResults,


        // for loops
        Count
    };

    struct  FileNameElements{
        QString lruName, lruSN, date;
    }/*fileNameElements*/;

    static const int tableOffset = 1;//1 column

    static QStringList TableColumnNames(NASKTableType type);
    static QString TableCaption(NASKTableType type);


    static QString HTMLTableHead(const QString& tableName);
    static QString XMLTableHead(const QString& tableName);
    static bool ParseTestResults(qlonglong sessionID);
    static QString PrintTable_HTML(NASKTableType type, qlonglong sessionID);
    static QString PrintTable_XML(NASKTableType type, qlonglong sessionID);
    static QString PrintScenarioData_HTML();
    static QString PrintScenarioData_XML();
    static QString PrintIndividualTestResults_HTML();
    static QString PrintIndividualTestResults_XML();
    static void PrintHTMLToPdf(const QString& html, const QString& fileName);
    static void PrintHTMLToFile(const QString& html, const QString& fileName);
    static void PrintHTMLToFiles(const QString& html, const QString &lruName, const QString &lruSN, const QString &Date);
    static QString checkRepairfileName(const QString& lruName, const QString& lruSN, const QString& date);
    static QPair<QList<QString>, QList<XMLPrintSupport::FileNameElements>> createXmlFiles(QSqlDatabase& db);
    static QString createHTMLFile(FileNameElements& fileNameElements, QSqlDatabase& db);
    static QString shieldingToXML(QString xmlData);
    static QString checkAndFixSizeName(QString name);/// Todo переделать с QFonts
public:

    static void onExportDBAction(QSqlDatabase& db/*, const QString& typeFile*/, const QString &typeFile);
};

#endif // XMLPRINTSUPPORT_H
