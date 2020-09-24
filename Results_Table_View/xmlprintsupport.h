#ifndef XMLPRINTSUPPORT_H
#define XMLPRINTSUPPORT_H

#include <QSqlDatabase>
#include <QStringList>
#include <QObject>
#include <QFutureWatcher>

class XMLPrintProgress;
class PrintDialog;
class PrintDialogItemModel;
class XMLPrintSupport : public QObject
{
    Q_OBJECT
public:
    struct ScenarioData
    {
        struct TestData
        {
            QStringList txtTestData;
            bool passState;
        };
        QList<TestData> testData;
        bool scenarioPassState {true};

        bool appendTestData(const QStringList& tData, bool status) {
            if (tData.size() < 5) {
                qDebug() << "Test data is corrupted!!!";
                return false;
            }

            testData.append({tData, status});
            scenarioPassState &= status;
            return true;
        }
    };
    static void onExportDBAction(QSqlDatabase& db/*, const QString& typeFile*/, const QString& typeFile,
                                 QString directory, XMLPrintProgress *progress, QMap<QString, QString*>filtersMemory);

private:

    XMLPrintSupport(QObject *parent = nullptr);
    ~XMLPrintSupport();
    volatile bool stop;
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

    struct  FileNameElements
    {
        QString lruName, lruSN, date;
    }/*fileNameElements*/;
    struct HTMLFilesProperties
    {
        QList<QString> htmlList;
        QList<QString> fileNameList;
    };

    static const int tableOffset = 1;//1 column

    static QStringList TableColumnNames(NASKTableType type);
    static QString TableCaption(NASKTableType type);

    static QString HTMLTableHead(const QString& tableName);
    static QString XMLTableHead(const QString& tableName);
    static bool ParseTestResults(QSqlDatabase &db, qlonglong sessionID, QMap<QString, QString *> filtersMemory);
    static QString PrintTable_HTML(QSqlDatabase& db, NASKTableType type, qlonglong sessionID, QMap<QString, QString *> filtersMemory);
    static QString PrintTable_XML(QSqlDatabase& db, NASKTableType type, qlonglong sessionID, QMap<QString, QString *> filtersMemory);
    static QString PrintScenarioData_HTML();
    static QString PrintScenarioData_XML(int count);
    static QString PrintIndividualTestResults_HTML();
    static QString PrintIndividualTestResults_XML();
    static void PrintHTMLToPdf(const QString& html, const QString& fileName);
    static void PrintHTMLToFile(const QString& html, const QString& fileName);
    void PrintHTMLToFiles(QList<QString> htmlList, QList<QString> htmlFileNameList, XMLPrintProgress *progress);
    void PrintHTMLToFiles(QList<QString> htmlList, QList<XMLPrintSupport::FileNameElements> htmlFileNameList, const QString &direction,  XMLPrintProgress* progress);
    void PrintHTMLToPdfFiles(QList<QString> htmlList, QList<QString> htmlFileNameList,  XMLPrintProgress* progress, QList<QHash<QString, ScenarioData>> scenariosHashList);
    static QString CheckRepairfileName(const QString& lruName, const QString& lruSN, const QString& date,
                                       const QString& fileExtension, const QString &directory);
    void CreateXMLFiles(QSqlDatabase& db, const QString& directory, XMLPrintProgress* progress, QMap<QString, QString*> filtersMemory);
    void CreateHTMLFile(QSqlDatabase &db, const QString& directory, XMLPrintProgress* progress, QMap<QString, QString*> filtersMemory);
    void CreateXMLAndHTML(QSqlDatabase& db, const QString &directory, XMLPrintProgress* progress, QMap<QString, QString*> filtersMemory);
    static QString ShieldingToXML(QString xmlData);
    static QString CheckAndFixSizeName(QString name);/// Todo переделать с QFonts
    void WorkAtHTMLFile(const QString &directory, const QString &fileName, XMLPrintProgress* progress, QMap<QString, QString*> filtersMemory);
    void WorkAtXMLFile(const QString &directory, const QString &fileName, XMLPrintProgress* progress, QMap<QString, QString*> filtersMemory);
    void WorkAtXMLAndHTMLFile(const QString& directory, const QString& fileName, XMLPrintProgress* progress, QMap<QString, QString*> filtersMemory);

    static void sorterNames(QList<QString>& names);

public:
    void setStop(bool stop);
public slots:
    void close();

};

#endif // XMLPRINTSUPPORT_H
