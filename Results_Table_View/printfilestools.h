#ifndef PRINTFILESTOOLS_H
#define PRINTFILESTOOLS_H
#include <QList>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

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

enum class EnumColumnIndividualSpecStyle///TODO FIX
{
    TestName = 0
};

enum class EnumColumnSpecStyle
{
    Name = 1,
    RequirementsOrName = 2,
    Description = 4
};

QStringList TableColumnNames(NASKTableType type);

QString TableCaption(NASKTableType type);

enum class SeparateType
{
    Head,
    Body,
    Footer
};

void sorterNames(QList<QString>& names);

QString getFiltersMemories(QList<QString*> filter);

const int tableOffset = 1;//1 column

enum ColumnsLocked
{
    SessionId = 0
};

QString ShieldingToXML(QString xmlData);

bool ParseTestResults(qlonglong sessionID, QHash<QString, ScenarioData>& s_scenariosData, QSqlDatabase& db,
                      QMap<QString, QList<QString*>>& filtersMemory);

#endif // PRINTFILESTOOLS_H
