
#include "printfilestools.h"



QStringList TableColumnNames(NASKTableType type)
{
    switch (type)
    {
    case NASKTableType::AcceptanceTestReports:
        return
        {
            "Session ID",
            "Project Name",
            "LRU Name",
            "LRU S/N",
            "ATE P/N",
            "ATE S/N",
            "ATE S/W Ver",
            "Test Status",
            "Run Mode",
            "Session Run Date&Time",
            "Session Run Total Time (hh:mm:ss)",
            "Environment",
            "TPS Checksum",
            "Operator's Name"
        };
    case NASKTableType::SuspectListOfSRUs:
        return
        {
            //"Session ID",
            "List ID",
            "Name",
            "S/N",
            "Suspected",
            "Description"
        };
    case NASKTableType::VisualInspection:
        return
        {
            //"Session ID",
            "List ID",
            "Name",
            "Requirements",
            "Test Status"
        };
    case NASKTableType::SummaryScenarioTestResults:
        return
        {
            //"Session ID",
            "Product name",
            "Scenario name",
            "Caption",
            "Test name",
            "Low",
            "Result",
            "High",
            "Units",
            "Status"
        };
    case NASKTableType::SummaryTestResults:
        return
        {
            "Page",
            "Scenario",
            "Status"
        };
    case NASKTableType::IndividualScenarioResults:
        return
        {
            "Test name",
            "Low",
            "Result",
            "High",
            "Units",
            "Status"
        };

    case NASKTableType::Count:
        break;
    }
    return {};
}

QString TableCaption(NASKTableType type)
{
    switch (type)
    {
    case NASKTableType::AcceptanceTestReports:
        return "Acceptance Test Reports";
    case NASKTableType::SuspectListOfSRUs:
        return "Suspect lists of SRUs";
    case NASKTableType::VisualInspection:
        return "Visual Inspections";
    case NASKTableType::SummaryScenarioTestResults:
        return "Scenario Test Results";
    case NASKTableType::SummaryTestResults:
        return "Test Result";
    case NASKTableType::IndividualScenarioResults:
        return "";      // Scenario name will be used as a caption

    case NASKTableType::Count:
        break;
    }
    return {};
}

QList<QString> TablesCaption()
{
    QList<QString> list;
    for(int i = 0; i < 6; ++i)
    {
        list.push_back(TableCaption(NASKTableType(i)));
    }
    return list;
}

void sorterNames(QList<QString>& names)
{
    struct MagicCPP
    {
        QList<int> numbList;
        QString name;
        MagicCPP(const QString& name, QList<int>& numbList)
        {
            this->numbList = numbList;
            this->name = name;
        }
        bool operator<(const MagicCPP& right)// используется неявно в std::sort!!!
        {
            int i = 0;
            if(right.numbList.size() != 0)
            {
                for (auto var : numbList)
                {

                    if(var < right.numbList.at(i))
                    {
                        return true;
                    }
                    if(var > right.numbList.at(i))
                    {
                        return false;
                    }

                    ++i;
                    if(right.numbList.size() <= i)
                    {
                        if(numbList.size() <= i)
                        {
                            return name < right.name;
                        }
                        return false;
                    }
                }
            }
            return false;
        }
    };

    std::vector<MagicCPP> magicCPPVect;
    QRegExp regExp("[0-9]{1,}.[0-9]{1,}.[0-9]{1,}");
    for (auto name : names)
    {
        int pos = regExp.indexIn(name);
        QList<int> numbList;
        if(pos > -1)
        {
            QString value = regExp.capturedTexts().at(0);
            QList<QString> list = value.split(".");

            for (auto number : list)
            {
                numbList.push_back(number.toInt());
            }
            magicCPPVect.push_back(MagicCPP(name, numbList));
        }
        else
        {
            magicCPPVect.push_back(MagicCPP(name, numbList));
        }
    }
    std::sort(magicCPPVect.begin(), magicCPPVect. end());
    names.clear();
    for (auto var : magicCPPVect)
    {
        names.push_back(var.name);
    }
}

QString getFiltersMemories(QList<QString*> filter)
{
    QString filterMemory = "(";
    bool first = true;
    for(auto var : filter)
    {
        if(*var == "")
            continue;
        if(first)
        {
            first = false;
            filterMemory += "(" + *var + ")";
        }
        else
        {
            filterMemory += " AND " + *var;
        }
    }
    filterMemory += ")";
    qDebug() << "FILTERMEMORY: " << filterMemory;
    return filterMemory;
}

QString ShieldingToXML(QString xmlData)
{
    xmlData.replace("&", "&amp;");
    xmlData.replace("<", "&lt;");
    xmlData.replace(">", "&gt;");
    xmlData.replace(" ", "&ensp;");//&apos;&ensp;
    xmlData.replace("\"", "&quot;");
    return xmlData;
}

bool ParseTestResults(qlonglong sessionID, QHash<QString, ScenarioData>& s_scenariosData, QSqlDatabase& db, QMap<QString, QList<QString*>>& filtersMemory)
{
    QSqlQuery query(db);

    QList<QString*> filter = filtersMemory.value(TableCaption(NASKTableType::SummaryScenarioTestResults));
    bool status;
    if(filter.size())
    {
        QString filterMemory = getFiltersMemories(filter);
        if(filterMemory == "()")
        {
            status = query.exec(QString("SELECT * FROM '%1' WHERE `Session ID` = %2;")
                                .arg(TableCaption(NASKTableType::SummaryScenarioTestResults)).arg(sessionID));
        }
        else
        {
            status = query.exec(QString("SELECT * FROM '%1' WHERE `Session ID` = %2")
                                .arg(TableCaption(NASKTableType::SummaryScenarioTestResults)).arg(sessionID) + " AND " + filterMemory + ";");

        }
    }
    else
    {
        status = query.exec(QString("SELECT * FROM '%1' WHERE `Session ID` = %2;")
                            .arg(TableCaption(NASKTableType::SummaryScenarioTestResults)).arg(sessionID));
    }

    if (!status)
    {
        qDebug() << "DB Query Error: " << query.lastError().text();
        return false;
    }

    while (query.next())
    {
        QString scenarioName;
        QStringList testData;
        auto index = 0;

        while (!query.value(index).isNull())
        {
            const auto val = query.value(index).toString();

            if (index == 2)             // Scenario name column
            {
                scenarioName = val;
                if (!s_scenariosData.contains(scenarioName))
                {
                    s_scenariosData.insert(scenarioName, {});
                }
            }
            else if (index >= 4 && index < 9)        // Test name column
            {
                testData.append(val);
            }
            else if (index == 9)        // Status column
            {
                if (scenarioName.isEmpty())
                {
                    qDebug() << "Scenario name is empty!!!!!!!!!!!!";
                    return false;
                }
                if (!s_scenariosData[scenarioName].appendTestData(
                            testData,  !QString::compare(val, "PASS", Qt::CaseInsensitive)))
                {
                    qDebug() << "Not managed to append test data!!!!!";
                    return false;
                }
            }
            index++;
        }
    }

    return true;
}

