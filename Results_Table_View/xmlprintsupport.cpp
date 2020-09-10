#include "xmlprintsupport.h"

#include <QDebug>
#include <QSqlQuery>
#include <QPrinter>
#include <QFile>
#include <QTextDocument>
#include <QSqlError>
#include <QPair>
#include <QDateTime>

QStringList XMLPrintSupport::TableColumnNames(NASKTableType type)
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
            "Session ID",
            "List ID",
            "Name",
            "S/N",
            "Suspected",
            "Description"
        };
    case NASKTableType::VisualInspection:
        return
        {
            "Session ID",
            "List ID",
            "Name",
            "Requirements",
            "Test Status"
        };
    case NASKTableType::SummaryScenarioTestResults:
        return
        {
            "Session ID",
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

QString XMLPrintSupport::TableCaption(NASKTableType type)
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
    return {};}

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

/// NOTE: scenario name should be unique!!!
static QHash<QString, ScenarioData> s_scenariosData;     // scenario name - scenario tests data

const QString HTMLTableFoot = "\n</table>\n<br>\n<br>\n<br>\n</p>";
const QString XMLTableFoot = "\n</Table>"
                             "\n<WorksheetOptions xmlns=\"urn:schemas-microsoft-com:office:excel\">"
                             "\n\t<PageSetup>"
                             "\n\t\t<Header x:Margin=\"0.3\"/>"
                             "\n\t\t<Footer x:Margin=\"0.3\"/>"
                             "\n\t<PageMargins x:Bottom=\"0.75\" x:Left=\"0.7\" x:Right=\"0.7\" x:Top=\"0.75\"/>"
                             "\n\t</PageSetup>"
                             "\n\t<Unsynced/>"
                             "\n\t<Print>"
                             "\n\t<ValidPrinterInfo/>"
                             "\n\t\t<PaperSizeIndex>9</PaperSizeIndex>"
                             "\n\t\t<HorizontalResolution>600</HorizontalResolution>"
                             "\n\t\t<VerticalResolution>600</VerticalResolution>"
                             "\n\t</Print>"
                             "\n\t%1"//"<TabColorIndex>9</TabColorIndex>"//9
        "\n\t<Selected/>"
        "\n\t<ProtectObjects>False</ProtectObjects>"
        "\n\t<ProtectScenarios>False</ProtectScenarios>"
        "\n</WorksheetOptions>"
        "</Worksheet>"
        ;
const QString XMLRowHead = "\n\t<Row ss:AutoFitHeight=\"1\">";// Автонастройка высоты строк
const QString XMLRowCellData[] = {"\n\t\t<Cell ss:Index=\"%1\" ss:StyleID=\"s62\"><Data ss:Type=\"String\">%2</Data></Cell>",//Acceptance Test Report Left Column
                                  "\n\t\t<Cell ss:Index=\"%1\" ss:StyleID=\"s63\"><Data ss:Type=\"String\">%2</Data></Cell>",//Acceptance Test Report Right Column
                                  "\n\t\t<Cell ss:Index=\"%1\" ss:StyleID=\"s65\"><Data ss:Type=\"String\">%2</Data></Cell>",//Default style cell Выравниваем по ss:Horizontal = Left
                                  "\n\t\t<Cell ss:Index=\"%1\" ss:StyleID=\"s66\"><Data ss:Type=\"String\">%2</Data></Cell>",//Default style cell Выравниваем по ss:Horizontal = Center
                                  "\n\t\t<Cell ss:Index=\"%1\" ss:StyleID=\"s67\"><Data ss:Type=\"String\">%2</Data></Cell>",//Default style Head
                                  "\n\t\t<Cell ss:Index=\"%1\" ss:StyleID=\"s68\"><Data ss:Type=\"String\">%2</Data></Cell>",//Status Passed
                                  "\n\t\t<Cell ss:Index=\"%1\" ss:StyleID=\"s69\"><Data ss:Type=\"String\">%2</Data></Cell>"};//Status Failed
QList<QString> XMLStyles = {"\n<Style ss:ID=\"Default\" ss:Name=\"Normal\">"
                            "\n<Alignment ss:Vertical=\"Bottom\"/>"
                            "\n<Borders/>"
                            "\n<Font ss:FontName=\"Calibri\" x:CharSet=\"204\" x:Family=\"Swiss\" ss:Size=\"11\" ss:Color=\"#000000\"/>"// устанавливаем шрифт
                            "\n<Interior/>"
                            "\n<NumberFormat/>"
                            "\n<Protection/>"
                            "\n</Style>",

                            ///Acceptance Test Report Left Column
                            "\n<Style ss:ID=\"s62\">"//"\n<NumberFormat ss:Format=\"#,##0.00&quot;р.&quot;\"/>"

                            "<Font ss:FontName=\"Aria\" x:CharSet=\"204\" x:Family=\"Swiss\" ss:Size=\"12\" ss:Color=\"#000000\" ss:Bold=\"1\"/>"
                            " \n<Alignment ss:Horizontal=\"Left\" ss:Vertical=\"Center\" ss:WrapText=\"1\"/>"// Выравниваем
                            "\n<Borders>"
                            "\n <Border ss:Position=\"Bottom\" ss:LineStyle=\"Continuous\" ss:Weight=\"2\"/>"
                            "\n <Border ss:Position=\"Left\" ss:LineStyle=\"Continuous\" ss:Weight=\"2\"/>"
                            "\n <Border ss:Position=\"Right\" ss:LineStyle=\"Continuous\" ss:Weight=\"2\"/>"
                            "\n <Border ss:Position=\"Top\" ss:LineStyle=\"Continuous\" ss:Weight=\"2\"/>"
                            "\n</Borders>"
                            "<Interior ss:Color=\"#BFBFBF\" ss:Pattern=\"Solid\"/>"// Заливка фона
                            "\n</Style>",

                            ///Acceptance Test Report Right Column
                            "\n<Style ss:ID=\"s63\">"
                            "<Font ss:FontName=\"Aria\" x:CharSet=\"204\" x:Family=\"Swiss\" ss:Size=\"12\" ss:Color=\"#000000\" ss:Bold=\"1\"/>"
                            " <Alignment ss:Horizontal=\"Center\" ss:Vertical=\"Center\"  ss:WrapText=\"1\"/>"// Выравниваем по середине
                            "\n<Borders>"
                            "\n <Border ss:Position=\"Bottom\" ss:LineStyle=\"Continuous\" ss:Weight=\"2\"/>"
                            "\n <Border ss:Position=\"Left\" ss:LineStyle=\"Continuous\" ss:Weight=\"2\"/>"
                            "\n <Border ss:Position=\"Right\" ss:LineStyle=\"Continuous\" ss:Weight=\"2\"/>"
                            "\n <Border ss:Position=\"Top\" ss:LineStyle=\"Continuous\" ss:Weight=\"2\"/>"
                            "\n</Borders>"
                            "\n</Style>",

                            ///Acceptance Test Report Header
                            "\n<Style ss:ID=\"s64\">"
                            "<Font ss:FontName=\"Aria\" x:CharSet=\"204\" x:Family=\"Swiss\" ss:Size=\"16\" ss:Color=\"#000000\" ss:Bold=\"1\" ss:Underline=\"Single\"/>"
                            " <Alignment ss:Horizontal=\"Center\" ss:Vertical=\"Center\"  ss:WrapText=\"1\"/>"
                            "\n<Borders>"
                            "\n <Border ss:Position=\"Bottom\" ss:LineStyle=\"Continuous\" ss:Weight=\"2\"/>"
                            "\n <Border ss:Position=\"Left\" ss:LineStyle=\"Continuous\" ss:Weight=\"2\"/>"
                            "\n <Border ss:Position=\"Right\" ss:LineStyle=\"Continuous\" ss:Weight=\"2\"/>"
                            "\n <Border ss:Position=\"Top\" ss:LineStyle=\"Continuous\" ss:Weight=\"2\"/>"
                            "\n</Borders>"
                            "\n</Style>",

                            ///Default style cell Выравниваем по ss:Horizontal = Left
                            "\n<Style ss:ID=\"s65\">"
                            "<Font ss:FontName=\"Aria\" x:CharSet=\"204\" x:Family=\"Swiss\" ss:Size=\"8\" ss:Color=\"#000000\" ss:Bold=\"0\"/>"
                            " <Alignment ss:Horizontal=\"Left\" ss:Vertical=\"Center\"  ss:WrapText=\"1\"/>"
                            "\n<Borders>"
                            "\n <Border ss:Position=\"Bottom\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>"
                            "\n <Border ss:Position=\"Left\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>"
                            "\n <Border ss:Position=\"Right\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>"
                            "\n <Border ss:Position=\"Top\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>"
                            "\n</Borders>"
                            "\n</Style>",

                            ///Default style cell Выравниваем по ss:Horizontal = Center
                            "\n<Style ss:ID=\"s66\">"
                            "<Font ss:FontName=\"Aria\" x:CharSet=\"204\" x:Family=\"Swiss\" ss:Size=\"8\" ss:Color=\"#000000\" ss:Bold=\"0\"/>"
                            " <Alignment ss:Horizontal=\"Center\" ss:Vertical=\"Center\"  ss:WrapText=\"1\"/>"
                            "\n<Borders>"
                            "\n <Border ss:Position=\"Bottom\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>"
                            "\n <Border ss:Position=\"Left\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>"
                            "\n <Border ss:Position=\"Right\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>"
                            "\n <Border ss:Position=\"Top\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>"
                            "\n</Borders>"
                            "\n</Style>",

                            ///Default style Head (названия колонок)
                            "\n<Style ss:ID=\"s67\">"//"\n<NumberFormat ss:Format=\"#,##0.00&quot;р.&quot;\"/>"
                            "<Font ss:FontName=\"Aria\" x:CharSet=\"204\" x:Family=\"Swiss\" ss:Size=\"11\" ss:Color=\"#000000\" ss:Bold=\"1\"/>"
                            " \n<Alignment ss:Horizontal=\"Left\" ss:Vertical=\"Center\" ss:WrapText=\"1\"/>"
                            "\n<Borders>"
                            "\n <Border ss:Position=\"Bottom\" ss:LineStyle=\"Continuous\" ss:Weight=\"2\"/>"
                            "\n <Border ss:Position=\"Left\" ss:LineStyle=\"Continuous\" ss:Weight=\"2\"/>"
                            "\n <Border ss:Position=\"Right\" ss:LineStyle=\"Continuous\" ss:Weight=\"2\"/>"
                            "\n <Border ss:Position=\"Top\" ss:LineStyle=\"Continuous\" ss:Weight=\"2\"/>"
                            "\n</Borders>"
                            "<Interior ss:Color=\"#BFBFBF\" ss:Pattern=\"Solid\"/>"// Заливка фона
                            "\n</Style>",

                            ///Status Passed
                            "\n<Style ss:ID=\"s68\">"
                            "<Font ss:FontName=\"Aria\" x:CharSet=\"204\" x:Family=\"Swiss\" ss:Size=\"8\" ss:Color=\"#000000\" ss:Bold=\"0\"/>"
                            " <Alignment ss:Horizontal=\"Center\" ss:Vertical=\"Center\"  ss:WrapText=\"1\"/>"
                            "\n<Borders>"
                            "\n <Border ss:Position=\"Bottom\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>"
                            "\n <Border ss:Position=\"Left\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>"
                            "\n <Border ss:Position=\"Right\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>"
                            "\n <Border ss:Position=\"Top\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>"
                            "\n</Borders>"
                            "<Interior ss:Color=\"#00FF00\" ss:Pattern=\"Solid\"/>"
                            "\n</Style>",

                            ///Status Failed
                            "\n<Style ss:ID=\"s69\">"
                            "<Font ss:FontName=\"Aria\" x:CharSet=\"204\" x:Family=\"Swiss\" ss:Size=\"8\" ss:Color=\"#000000\" ss:Bold=\"0\"/>"
                            " <Alignment ss:Horizontal=\"Center\" ss:Vertical=\"Center\"  ss:WrapText=\"1\"/>"
                            "\n<Borders>"
                            "\n <Border ss:Position=\"Bottom\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>"
                            "\n <Border ss:Position=\"Left\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>"
                            "\n <Border ss:Position=\"Right\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>"
                            "\n <Border ss:Position=\"Top\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"/>"
                            "\n</Borders>"
                            "<Interior ss:Color=\"#FF0000\" ss:Pattern=\"Solid\"/>"
                            "\n</Style>",

                            ///Default style HeaderTable (названия таблиц)
                            "\n<Style ss:ID=\"s70\">"
                            "<Font ss:FontName=\"Aria\" x:CharSet=\"204\" x:Family=\"Swiss\" ss:Size=\"12\" ss:Color=\"#000000\" ss:Bold=\"1\"/>"
                            " <Alignment ss:Horizontal=\"Left\" ss:Vertical=\"Center\"  ss:WrapText=\"1\"/>"
                            "\n</Style>"
                           };
const QString XMLRowFoot = "\n\t</Row>";

QList<QString> namesFilesExp;

QString XMLPrintSupport::HTMLTableHead(const QString& tableName)
{
    QString html = "\n<p><table bordercolor=\"black\" border=\"1\" bgcolor=\"white\" width=\"100%\">";
    html += QString("\n<caption align=\"center\">%1</caption>").arg(shieldingToXML(tableName));
    return html;
}

void widthColumnsSet(QString& html, QList<float> sizeList)
{
    for (auto var : sizeList) {
        html += "\n<Column ss:AutoFitWidth=\"0\" ss:Width=\"" + QString::number(double(var)) + "\"/>";
    }
}
void offsetRowsSet(QString& html, unsigned int count)
{
    for (unsigned int i = 0; i < count; ++i) {
        html += XMLRowHead + XMLRowFoot;
    }
}
QString XMLPrintSupport::shieldingToXML(QString xmlData)
{
    xmlData.replace("&", "&amp;");
    xmlData.replace("<", "&lt;");
    xmlData.replace(">", "&gt;");
    xmlData.replace(" ", "&nbsp;");//&apos;
    xmlData.replace("\"", "&quot;");
    return xmlData;
}
QString XMLPrintSupport::checkAndFixSizeName(QString name)/// Todo переделать с QFonts
{
    const int maxLenght = 32;
    if(name.length() > maxLenght)
    {
        int size = name.length() + 1 + 3/*3 точки*/ - maxLenght;
        int pointer;
        if(size % 2)
        {
            pointer = (name.length() - int(size) - 1)/2;
        }
        else
        {
            pointer = (name.length() - size)/2;
        }
        int tempP = pointer;
        if(name.at(pointer - 1) == " ")
        {
            pointer--;
        }
        if(name.at(pointer + size) == " ")
            size++;

        name.remove(pointer, size);
        name.insert(pointer, "...");
    }
    return name;
}
QString XMLPrintSupport::XMLTableHead(const QString& tableName)
{
    auto tName = checkAndFixSizeName(tableName);

    QString html = QString("\n<Worksheet ss:Name=\"%1\">").arg(shieldingToXML(tName.replace("\"", "'")));
    html += QString("\n<Table>");
    /*
       AcceptanceTestReports,
       SuspectListOfSRUs,
       VisualInspection,
       SummaryScenarioTestResults,
       SummaryTestResults,
       IndividualScenarioResults,
     */

    if(tableName.toLower() == "acceptance test reports")
    {
        QList<float> sizeColumns = {14.25, 213.75, 213.75};
        widthColumnsSet(html, sizeColumns);
        offsetRowsSet(html, 2);

        html += XMLRowHead;
        html += QString("<Cell ss:Index=\"2\" ss:StyleID=\"s64\" ss:MergeAcross=\"1\" ss:MergeDown=\"2\"><Data ss:Type=\"String\">%1</Data></Cell>").arg(shieldingToXML(tableName));
        html += XMLRowFoot;

        offsetRowsSet(html, 4);
    }else if(tableName.toLower() == "suspect lists of srus")
    {
        QList<float> sizeColumns = {14.25, 55.5, 39.75, 231.75, 24, 62.25, 595.5};
        widthColumnsSet(html, sizeColumns);
        offsetRowsSet(html, 1);

        html += XMLRowHead;
        html += QString("<Cell ss:Index=\"2\" ss:StyleID=\"s70\" ss:MergeAcross=\"" + QString::number(sizeColumns.length() - 1)/* длина этого списка равна количеству колонок таблицы + 1*/ + "\"><Data ss:Type=\"String\">%1</Data></Cell>").arg(shieldingToXML(tableName));
        html += XMLRowFoot;

    }else if(tableName.toLower() == "visual inspections")
    {
        QList<float> sizeColumns = {14.25, 55.25, 119.25, 150.75, 140.25};
        widthColumnsSet(html, sizeColumns);
        offsetRowsSet(html, 1);

        html += XMLRowHead;
        html += QString("<Cell ss:Index=\"2\" ss:StyleID=\"s70\" ss:MergeAcross=\"" + QString::number(sizeColumns.length() - 1)/* длина этого списка равна количеству колонок таблицы + 1*/ + "\"><Data ss:Type=\"String\">%1</Data></Cell>").arg(shieldingToXML(tableName));
        html += XMLRowFoot;

    }else if(tableName.toLower() == "test result")
    {
        QList<float> sizeColumns = {14.25, 55.25, 339.75};
        widthColumnsSet(html, sizeColumns);
        offsetRowsSet(html, 1);

        html += XMLRowHead;
        html += QString("<Cell ss:Index=\"2\" ss:StyleID=\"s70\" ss:MergeAcross=\"" + QString::number(sizeColumns.length() - 1)/* длина этого списка равна количеству колонок таблицы + 1*/ + "\"><Data ss:Type=\"String\">%1</Data></Cell>").arg(shieldingToXML(tableName));
        html += XMLRowFoot;

    }else
    {
        QList<float> sizeColumns = {14.25, 182.25, 51, 51, 51, 51};
        widthColumnsSet(html, sizeColumns);
        offsetRowsSet(html, 1);

        html += XMLRowHead;
        html += QString("<Cell ss:Index=\"2\" ss:StyleID=\"s70\" ss:MergeAcross=\"" + QString::number(sizeColumns.length() - 1)/* длина этого списка равна количеству колонок таблицы + 1*/ + "\"><Data ss:Type=\"String\">%1</Data></Cell>").arg(shieldingToXML(tableName));
        html += XMLRowFoot;

    }

    return html;
}

bool XMLPrintSupport::ParseTestResults(qlonglong sessionID)
{
    QSqlQuery query;
    const auto status = query.exec(QString("select * from '%1' where \"Session ID\" = %2;")
                                   .arg(TableCaption(NASKTableType::SummaryScenarioTestResults)).arg(sessionID));
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

QString XMLPrintSupport::PrintTable_HTML(NASKTableType type, qlonglong sessionID)
{
    QString html;
    QSqlQuery query;

    const auto status = query.exec(QString("select * from '%1' where \"Session ID\" = %2;")
                                   .arg(TableCaption(type)).arg(sessionID));
    if (!status)
    {
        qDebug() << "DB Query Error: " << query.lastError().text();
        return html;
    }

    html += HTMLTableHead(TableCaption(type));

    html += "\n\t<tr>";
    for (const auto& name : TableColumnNames(type))
    {
        html += QString("\n\t\t<th align=\"center\">%1</th>").arg(shieldingToXML(name));
    }
    html += "\n\t</tr>";

    while (query.next())
    {
        auto index = 0;
        html += "\n\t<tr>";
        while (!query.value(index).isNull())
        {
            const auto val = query.value(index).toString();
            html += QString("\n\t\t<td align=\"center\">%1</td>").arg(shieldingToXML(val));
            index++;
        }
        html += "\n\t</tr>";
    }
    html += HTMLTableFoot;

    return html;
}

QString XMLPrintSupport::PrintTable_XML(NASKTableType type, qlonglong sessionID)//Suspect list And Visual Inspections
{
    QString xml;
    QSqlQuery query;

    const auto status = query.exec(QString("select * from '%1' where \"Session ID\" = %2;")
                                   .arg(TableCaption(type)).arg(sessionID));
    if (!status)
    {
        qDebug() << "DB Query Error: " << query.lastError().text();
        return xml;
    }

    xml += XMLTableHead(TableCaption(type));


    int numColumnSpecStyle = 0;
    enum class EnumColumnSpecStyle
    {
        Name = 2,
        Requirements = 3
    };


    xml += XMLRowHead;
    {
        int numberCell = 1 + tableOffset;
        for (const auto& name : TableColumnNames(type))
        {
            xml += XMLRowCellData[4].arg(numberCell).arg(name);
            numberCell++;

            if(name.toLower() == "name")
            {
                ++numColumnSpecStyle;
                continue;
            }
            if(name.toLower() == "requirements")
                numColumnSpecStyle += 2;
        }
    }
    xml += XMLRowFoot;


    while (query.next())
    {
        auto index = 0;
        xml += XMLRowHead;
        {
            int numberCell = 1 + tableOffset;
            while (!query.value(index).isNull())
            {
                const auto val = shieldingToXML(query.value(index).toString());

                if((numColumnSpecStyle & 1) && EnumColumnSpecStyle(numberCell - (1 + tableOffset)) == EnumColumnSpecStyle::Name)
                    xml += XMLRowCellData[2].arg(numberCell).arg(val);
                else if((numColumnSpecStyle & 2) && EnumColumnSpecStyle(numberCell - (1 + tableOffset)) == EnumColumnSpecStyle::Requirements )
                    xml += XMLRowCellData[2].arg(numberCell).arg(val);
                else
                    xml += XMLRowCellData[3].arg(numberCell).arg(val);

                numberCell++;
                index++;
            }
        }
        xml += XMLRowFoot;
    }
    xml += XMLTableFoot.arg("");

    return xml;
}

QString XMLPrintSupport::PrintScenarioData_HTML()
{
    QString html;

    html += HTMLTableHead(TableCaption(NASKTableType::SummaryTestResults));

    html += "\n\t<tr>";
    for (const auto& name : TableColumnNames(NASKTableType::SummaryTestResults))
    {
        html += QString("\n\t\t<th align=\"center\">%1</th>").arg(shieldingToXML(name));
    }
    html += "\n\t</tr>";

    for (const auto& scenarioName : s_scenariosData.keys())
    {
        html += "\n\t<tr>";

        html += QString("\n\t\t<td align=\"center\" width=\"10%\"></td>");          /// TODO: append page number!!!!!
        html += QString("\n\t\t<td align=\"left\">%1</td>").arg(shieldingToXML(scenarioName));

        if (s_scenariosData[scenarioName].scenarioPassState)
        {
            html += QString("\n\t\t<td align=\"center\" bgcolor=\"green\" width=\"10%\">%1</td>").arg("Passed");
        }
        else
        {
            html += QString("\n\t\t<td align=\"center\" bgcolor=\"red\" width=\"10%\">%1</td>").arg("Failed");
        }

        html += "\n\t</tr>";
    }
    html += HTMLTableFoot;

    return html;
}

QString XMLPrintSupport::PrintScenarioData_XML()
{

    QString xml;

    xml += XMLTableHead(TableCaption(NASKTableType::SummaryTestResults));

    xml += XMLRowHead;
    {
        int numberCell = 1 + tableOffset;
        for (const auto& name : TableColumnNames(NASKTableType::SummaryTestResults))
        {
            xml += XMLRowCellData[4].arg(numberCell).arg(shieldingToXML(name));
            numberCell++;
        }
    }
    xml += XMLRowFoot;

    static int s_pageNum = 1;   /// TODO: append proper page number!!!!!
    bool faile = false;
    for (const auto& scenarioName : s_scenariosData.keys())
    {
        xml += XMLRowHead;
        {
            int numberCell = 1 + tableOffset;
            xml += XMLRowCellData[3].arg(numberCell).arg(s_pageNum++);
            numberCell++;
            xml += XMLRowCellData[2].arg(numberCell).arg(shieldingToXML(scenarioName));
            numberCell++;

            if (s_scenariosData[scenarioName].scenarioPassState)
            {
                xml += XMLRowCellData[5].arg(numberCell).arg("Passed");
                numberCell++;
            }
            else
            {
                xml += XMLRowCellData[6].arg(numberCell).arg("Failed");
                numberCell++;
                faile = true;
            }

            xml += XMLRowFoot;
        }
    }
    if(faile)
        xml += XMLTableFoot.arg("<TabColorIndex>10</TabColorIndex>");//"<TabColorIndex>9</TabColorIndex>"//9
    else
        xml += XMLTableFoot.arg("");

    return xml;
}

QString XMLPrintSupport::PrintIndividualTestResults_HTML()
{
    QString html;

    for (const auto& scenarioName : s_scenariosData.keys())
    {
        html += HTMLTableHead(shieldingToXML(scenarioName));

        html += "\n\t<tr>";
        for (const auto& name : TableColumnNames(NASKTableType::IndividualScenarioResults))
        {
            html += QString("\n\t\t<th align=\"center\">%1</th>").arg(shieldingToXML(name));
        }
        html += "\n\t</tr>";

        for (const auto& d : s_scenariosData[scenarioName].testData)
        {
            html += "\n\t<tr>";
            for (const auto& txtD : d.txtTestData)
            {
                html += QString("\n\t\t<td align=\"center\">%1</td>").arg(shieldingToXML(txtD));
            }
            if (d.passState)
            {
                html += QString("\n\t\t<td align=\"center\" bgcolor=\"green\">%1</td>").arg("Passed");
            }
            else
            {
                html += QString("\n\t\t<td align=\"center\" bgcolor=\"red\">%1</td>").arg("Failed");
            }
            html += "\n\t</tr>";
        }
        html += HTMLTableFoot;
    }

    return html;
}

QString XMLPrintSupport::PrintIndividualTestResults_XML()
{
    QString xml;

    enum class EnumColumnSpecStyle
    {
        TestName = 0
    };

    for (const auto& scenarioName : s_scenariosData.keys())
    {
        xml += XMLTableHead(scenarioName);

        xml += XMLRowHead;
        {
            int numberCell = 1 + tableOffset;
            for (const auto& name : TableColumnNames(NASKTableType::IndividualScenarioResults))
            {
                xml += XMLRowCellData[4].arg(numberCell).arg(shieldingToXML(name));
                numberCell++;
            }
        }
        xml += XMLRowFoot;
        bool faile = false;
        for (const auto& d : s_scenariosData[scenarioName].testData)
        {
            xml += XMLRowHead;
            {
                int numberCell = 1 + tableOffset;
                for (const auto& txtD : d.txtTestData)
                {
                    if(EnumColumnSpecStyle(numberCell - (1 + tableOffset)) == EnumColumnSpecStyle::TestName)
                        xml += XMLRowCellData[2].arg(numberCell).arg(shieldingToXML(txtD));
                    else
                        xml += XMLRowCellData[3].arg(numberCell).arg(shieldingToXML(txtD));
                    numberCell++;
                }

                if (d.passState)
                {
                    xml += XMLRowCellData[5].arg(numberCell).arg("Passed");
                    numberCell++;
                }
                else
                {
                    xml += XMLRowCellData[6].arg(numberCell).arg("Failed");
                    numberCell++;
                    faile = true;
                }
            }
            xml += XMLRowFoot;
        }
        if(faile)
            xml += XMLTableFoot.arg("<TabColorIndex>10</TabColorIndex>");//"<TabColorIndex>9</TabColorIndex>"//9
        else
            xml += XMLTableFoot.arg("");

    }

    return xml;
}

QString XMLPrintSupport::checkRepairfileName(const QString& lruName, const QString& lruSN, const QString& date)
{
    QString filePath = QString("d:/TempExportedProtocols/%1_%2_%3").arg(lruName).arg(lruSN).arg(date);
    if(namesFilesExp.count(filePath) != 0)
    {
        QString newFilePath;
        int k = 1;
        do
        {
            newFilePath = filePath + "_" + QString::number(k);
            k++;
        }while(namesFilesExp.count(newFilePath) != 0);
        filePath = newFilePath;
    }
    qDebug() << filePath;
    namesFilesExp.push_back(filePath);
    //filePath += ".xls";
    return filePath;
}

void XMLPrintSupport::PrintHTMLToPdf(const QString& html, const QString& fileName)
{

    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPaperSize(QPrinter::A4);
    printer.setOutputFileName(fileName);

    QTextDocument doc;
    doc.setHtml(html);
    doc.print(&printer);
}

void XMLPrintSupport::PrintHTMLToFile(const QString& html, const QString& fileName)
{
    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly))
    {
        qDebug() << "Not managed to open " << fileName << " for write HTML";
    }
    else
    {

        f.write(html.toLocal8Bit());
        //f.write(html.toUtf8().constData());
        f.close();
    }
}

void XMLPrintSupport::PrintHTMLToFiles(const QString& html, const QString& lruName, const QString& lruSN, const QString& date)
{
    //    PrintHTMLToPdf(html, QString("d:/TempExportedProtocols/Exported_protocol___%1.pdf").arg(sessionID));
    //    PrintHTMLToFile(html, QString("d:/TempExportedProtocols/Exported_protocol___%1.html").arg(sessionID));
    //PrintHTMLToPdf(html, checkRepairfileName(lruName, lruSN, date));
    //PrintHTMLToFile(html, checkRepairfileName(lruName, lruSN, date));
}

QPair<QList<QString>, QList<XMLPrintSupport::FileNameElements>> XMLPrintSupport::createXmlFiles(QSqlDatabase& db)
{

    /// TODO: add document properties???
    ///
    ///  <DocumentProperties xmlns="urn:schemas-microsoft-com:office:office">
    ///    <Author>Author name</Author>
    ///    <LastAuthor>Author name</LastAuthor>
    ///    <Created>current dt</Created>
    ///    <Version>some version</Version>
    ///  </DocumentProperties>
    ///
    QList<QString> xmlMainList;

    QString xmlStyles = "";
    foreach (auto var, XMLStyles)
        xmlStyles += var;

    const QString xmlHead =
            "<?xml version=\"1.0\"  encoding=\"UTF-8\"?>"
            "\n<?mso-application progid=\"Excel.Sheet\"?>"
            "\n<Workbook xmlns=\"urn:schemas-microsoft-com:office:spreadsheet\""
            "\nxmlns:o=\"urn:schemas-microsoft-com:office:office\""
            "\nxmlns:x=\"urn:schemas-microsoft-com:office:excel\""
            "\nxmlns:ss=\"urn:schemas-microsoft-com:office:spreadsheet\""
            "\nxmlns:html=\"http://www.w3.org/TR/REC-html40\">"
            "\n<ExcelWorkbook xmlns=\"urn:schemas-microsoft-com:office:excel\">"
            "\n<WindowHeight>11820</WindowHeight>"
            "\n<WindowWidth>15315</WindowWidth>"
            "\n<WindowTopX>120</WindowTopX>"
            "\n<WindowTopY>45</WindowTopY>"
            "\n<ProtectStructure>False</ProtectStructure>"
            "\n<ProtectWindows>False</ProtectWindows>"
            "\n</ExcelWorkbook>"
            "\n<Styles>\n" + xmlStyles +
            "\n</Styles>";

    QString xmlFoot = "\n</Workbook>";

    QList<XMLPrintSupport::FileNameElements> fileNameElementsList;
    QSqlQuery query;
    const auto atrColumnsNames = TableColumnNames(NASKTableType::AcceptanceTestReports);

    const auto status = query.exec(QString("select * from '%1';").arg(TableCaption(NASKTableType::AcceptanceTestReports)));
    if (!status)
    {
        qDebug() << "DB Query Error: " << query.lastError().text();
        return {};
    }

    while (query.next())
    {
        QString xml = xmlHead;
        xml += XMLTableHead(TableCaption(NASKTableType::AcceptanceTestReports));

        qlonglong sessionID = 0;
        auto index = 0;

        XMLPrintSupport::FileNameElements fileNameElements;
        while (!query.value(index).isNull())
        {
            const auto val = query.value(index);//Acceptance Test Reports
            {
                int numberCell = 1 + tableOffset;
                xml += XMLRowHead;
                xml += XMLRowCellData[0].arg(numberCell).arg(shieldingToXML(atrColumnsNames.value(index)));
                numberCell++;
                xml += XMLRowCellData[1].arg(numberCell).arg(shieldingToXML(val.toString()));
                numberCell++;
                xml += XMLRowFoot;

                if (!index)
                {
                    sessionID = val.toLongLong();
                }

                if(atrColumnsNames.value(index).toLower() == "lru name")
                    fileNameElements.lruName = val.toString();
                if(atrColumnsNames.value(index).toLower() == "lru s/n")
                    fileNameElements.lruSN = val.toString();
                if(atrColumnsNames.value(index).toLower() == "session run date&time")
                {
                    QDateTime dateTime;
                    //dateTime.

                    fileNameElements.date = QString(val.toString().split(" ").at(0)).replace("/", ".");

                }

                index++;
            }
        }
        xml += XMLTableFoot.arg("");

        xml += PrintTable_XML(NASKTableType::SuspectListOfSRUs, sessionID);
        xml += PrintTable_XML(NASKTableType::VisualInspection, sessionID);

        if (!ParseTestResults(sessionID))
        {
            qDebug() << "Not managed to parse test results!!!!!";
            return {};
        }
        xml += PrintScenarioData_XML();
        xml += PrintIndividualTestResults_XML();

        xml += xmlFoot;

        xmlMainList << xml;
        fileNameElementsList << fileNameElements;
        s_scenariosData.clear();
    }

    return qMakePair(xmlMainList, fileNameElementsList);
}

QString XMLPrintSupport::createHTMLFile(FileNameElements& fileNameElements, QSqlDatabase& db)
{
    QString html = "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" /></head><body>";
       QSqlQuery query;
       const auto atrColumnsNames = TableColumnNames(NASKTableType::AcceptanceTestReports);

       const auto status = query.exec(QString("select * from '%1';").arg(TableCaption(NASKTableType::AcceptanceTestReports)));
       if (!status)
       {
           qDebug() << "DB Query Error: " << query.lastError().text();
           return QString();
       }

       while (query.next())
       {
           html += "<p><table bordercolor=\"black\" border=\"1\" bgcolor=\"white\" width=\"100%\">";
           html += QString("<caption align=\"center\">%1</caption>")
                   .arg(TableCaption(NASKTableType::AcceptanceTestReports));

           qlonglong sessionID = 0;
           auto index = 0;
           while (!query.value(index).isNull())
           {
               const auto val = query.value(index);

               html += "<tr>";
               html += QString("<td align=\"left\">%1</td>").arg(shieldingToXML(atrColumnsNames.value(index)));
               html += QString("<td align=\"center\">%1</td>").arg(shieldingToXML(val.toString()));
               html += "</tr>";

               if (!index)
               {
                   sessionID = val.toLongLong();
               }
               index++;
           }
           html += "</table><br><br><br></p>";

           html += PrintTable_HTML(NASKTableType::SuspectListOfSRUs, sessionID);
           html += PrintTable_HTML(NASKTableType::VisualInspection, sessionID);
           if (!ParseTestResults(sessionID))
           {
               qDebug() << "Not managed to parse test results!!!!!";
               return QString();
           }
           html += PrintScenarioData_HTML();
           html += PrintIndividualTestResults_HTML();

           html += "</body></html>";


       }
       s_scenariosData.clear();
       return html;
}

void XMLPrintSupport::onExportDBAction(QSqlDatabase& db, const QString& typeFile)
{
    if (!db.isOpen())
    {
        qDebug() << "DB is not open";
        return;
    }
    namesFilesExp.clear();

    if(typeFile == "PDF")
    {
        FileNameElements fileNameElements;
        QString html = createHTMLFile(fileNameElements, db);

        PrintHTMLToPdf(html, checkRepairfileName(fileNameElements.lruName, fileNameElements.lruSN, fileNameElements.date) + ".pdf");
    }
    if(typeFile == "XML")
    {
        QList<FileNameElements> fileNameElementsList;
        auto xmlFiles = createXmlFiles(db);
        for(int i = 0; i < xmlFiles.first.size(); ++i)
        {
            PrintHTMLToFile(xmlFiles.first.at(i), checkRepairfileName(xmlFiles.second.at(i).lruName, xmlFiles.second.at(i).lruSN, xmlFiles.second.at(i).date) + ".xls");
        }
    }

    qDebug() << "Export finished!";
}
