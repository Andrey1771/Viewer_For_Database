#include "pdfprintsupport.h"
#include <QDateTime>
#include <QTextDocument>
#include <QSqlQuery>
#include <QSqlError>

PDFPrintSupport::PDFPrintSupport()
{

}


QString PDFPrintSupport::HTMLTableHead(const QString& tableName)
{
    QString html = ("\n<p>" + PDFStyles::HTMLEndTab);
    html += QString("\n<caption align=\"center\">%1</caption>").arg(ShieldingToXML(tableName));
    return html;
}

QString PDFPrintSupport::PrintIndividualTestResults_HTML(QHash<QString, ScenarioData>& s_scenariosData, QList<QString> &tablesInFile, QList<TableHTML>& tablesHTML)
{

    QString html;

    QList<QString> names = s_scenariosData.keys();
    sorterNames(names);

    for (const auto& scenarioName : names)
    {
        QString bHTMLItem = "";
        if(s_scenariosData[scenarioName].testData.isEmpty())
        {
            continue;
        }
        tablesInFile.push_back(scenarioName);
        bHTMLItem += HTMLTableHead(scenarioName);

        bHTMLItem += "\n\t<tr>";
        for (const auto& name : TableColumnNames(NASKTableType::IndividualScenarioResults))
        {
            bHTMLItem += QString("\n\t\t<th align=\"center\">%1</th>").arg(name);
        }
        bHTMLItem += "\n\t</tr>";

        for (const auto& d : s_scenariosData[scenarioName].testData)
        {
            QString htmlItem = "";
            htmlItem += "\n\t<tr>";
            int numberCell = 1 + tableOffset;///TODO возможно FIX
            for (const auto& txtD : d.txtTestData)
            {
                if (EnumColumnIndividualSpecStyle(numberCell - (1 + tableOffset)) == EnumColumnIndividualSpecStyle::TestName)
                {
                    htmlItem += QString("\n\t\t<td align=\"left\">%1</td>").arg(txtD);
                }
                else
                {
                    htmlItem += QString("\n\t\t<td align=\"center\">%1</td>").arg(txtD);
                }
                numberCell++;
            }
            if (d.passState)
            {
                htmlItem += QString("\n\t\t<td align=\"center\" bgcolor=\"green\">%1</td>").arg("Passed");
            }
            else
            {
                htmlItem += QString("\n\t\t<td align=\"center\" bgcolor=\"red\">%1</td>").arg("Failed");
            }
            htmlItem += "\n\t</tr>";
            bHTMLItem += htmlItem;
            //HTMLAdderCheck(html, htmlItem, SeparateType::Head, QPageSize::size(QPageSize::A4, QPageSize::Unit::Point), scenarioName);
        }

        bHTMLItem += PDFStyles::HTMLTableFoot;
        TableHTML tableHTML;
        tableHTML.setHTML(bHTMLItem);
        tablesHTML.push_back(tableHTML);
        html += bHTMLItem;
    }

    return html;
}


QString PDFPrintSupport::PrintScenarioData_HTML(QHash<QString, ScenarioData>& s_scenariosData, QList<QString> &tablesInFile, QList<TableHTML>& tablesHTML)
{

    QString html;
    if(s_scenariosData.keys().isEmpty())
        return html;

    tablesInFile.push_back(TableCaption(NASKTableType::SummaryTestResults));
    html += HTMLTableHead(TableCaption(NASKTableType::SummaryTestResults));

    html += "\n\t<tr>";
    for (const auto& name : TableColumnNames(NASKTableType::SummaryTestResults))
    {
        html += QString("\n\t\t<th align=\"center\">%1</th>").arg(name);
    }
    html += "\n\t</tr>";

    QList<QString> names = s_scenariosData.keys();
    sorterNames(names);

    int i = 1;
    for (const auto& scenarioName : names)
    {
        QString htmlItem = "";
        htmlItem += "\n\t<tr>";

        htmlItem += QString("\n\t\t<td align=\"center\" width=\"10%\">{%" + QString::number(i) + "}</td>");
        ++i;
        htmlItem += QString("\n\t\t<td align=\"left\">%1</td>").arg(scenarioName);

        if (s_scenariosData[scenarioName].scenarioPassState)
        {
            htmlItem += QString("\n\t\t<td align=\"center\" bgcolor=\"green\" width=\"10%\">%1</td>").arg("Passed");
        }
        else
        {
            htmlItem += QString("\n\t\t<td align=\"center\" bgcolor=\"red\" width=\"10%\">%1</td>").arg("Failed");
        }

        htmlItem += "\n\t</tr>";
        html += htmlItem;
    }
    html += PDFStyles::HTMLTableFoot;
    TableHTML tableHTML;
    tableHTML.setHTML(html);
    tablesHTML.push_back(tableHTML);
    return html;
}


QString PDFPrintSupport::PrintTable_HTML(NASKTableType type, qlonglong sessionID, QSqlDatabase& db, QMap<QString, QList<QString*>>& filtersMemory, QList<QString> &tablesInFile, QList<TableHTML>& tablesHTML)
{

    QString html;
    QSqlQuery query(db);

    QList<QString*> filter = filtersMemory.value(TableCaption(type));
    bool status;
    if(filter.size())
    {
        QString filterMemory = getFiltersMemories(filter);
        if(filterMemory == "()")
        {
            status = query.exec(QString("SELECT * FROM '%1' WHERE `Session ID` = %2;")
                                .arg(TableCaption(type)).arg(sessionID));
        }
        else
        {
            status = query.exec(QString("SELECT * FROM '%1' WHERE `Session ID` = %2")
                                .arg(TableCaption(type)).arg(sessionID) + " AND " + filterMemory + ";");
        }
    }
    else
    {
        status = query.exec(QString("SELECT * FROM '%1' WHERE `Session ID` = %2;")
                            .arg(TableCaption(type)).arg(sessionID));
    }

    if (!status)
    {
        qDebug() << QString("DB Query '%1' Error: ").arg("queryStr") << query.lastError().text();
        return html;
    }

    if(!query.next())//Query.size() не поддерживается QSQLite, поэтому делаю такое действие для проверки на пустоту таблицы
        return html;
    else
        query.previous();

    tablesInFile.push_back(TableCaption(type));
    html += HTMLTableHead(TableCaption(type));

    html += "\n\t<tr>";
    int numColumnSpecStyle = 0;
    for (const auto& name : TableColumnNames(type))
    {
        html += QString("\n\t\t<th align=\"center\">%1</th>").arg(name);
        if (name.toLower() == "name")///TODO FIX THAT
        {
            ++numColumnSpecStyle;

        }
        if (name.toLower() == "requirements")
        {
            numColumnSpecStyle += 2;
        }
    }
    html += "\n\t</tr>";
    while (query.next())
    {
        QString htmlItem;
        auto index = 0;
        htmlItem += "\n\t<tr>";

        bool columnRemoved = false;
        int numberCell = 1 + tableOffset;
        while (!query.value(index).isNull())
        {
            if(type == NASKTableType::SuspectListOfSRUs || type == NASKTableType::VisualInspection || type == NASKTableType::SummaryScenarioTestResults)
            {
                if(ColumnsLocked(numberCell - (1 + tableOffset)) == ColumnsLocked::SessionId)
                {
                    numberCell++;
                    index++;
                    columnRemoved = true;
                    continue;
                }
                if(columnRemoved)
                {
                    numberCell--;
                    columnRemoved = false;
                }
            }

            const auto val = query.value(index).toString();

            if (type == NASKTableType::SuspectListOfSRUs && EnumColumnSpecStyle(numberCell - (1 + tableOffset)) == EnumColumnSpecStyle::Description)
            {
                htmlItem += QString("\n\t\t<td align=\"left\">%1</td>").arg(val);
            }
            else if ((numColumnSpecStyle & 1) && EnumColumnSpecStyle(numberCell - (1 + tableOffset)) == EnumColumnSpecStyle::Name)
            {
                htmlItem += QString("\n\t\t<td align=\"left\">%1</td>").arg(val);
            }
            else if ((numColumnSpecStyle & 2)
                     && EnumColumnSpecStyle(numberCell - (1 + tableOffset)) == EnumColumnSpecStyle::RequirementsOrName )
            {
                htmlItem += QString("\n\t\t<td align=\"left\">%1</td>").arg(val);
            }
            else
            {
                htmlItem += QString("\n\t\t<td align=\"center\">%1</td>").arg(val);
            }

            index++;
            numberCell++;
        }
        htmlItem += "\n\t</tr>";

        html+=htmlItem;
    }
    html += PDFStyles::HTMLTableFoot;

    TableHTML tableHTML;
    tableHTML.setHTML(html);
    tablesHTML.push_back(tableHTML);
    return html;
}
