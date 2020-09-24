#include "xmlprintsupport.h"
#include "xmlprintprogress.h"

#include <QDebug>
#include <QSqlQuery>
#include <QPrinter>
#include <QFile>
#include <QTextDocument>
#include <QSqlError>
#include <QPair>
#include <QDateTime>
#include <QtConcurrent/QtConcurrentRun>
#include <QtConcurrent/QtConcurrentMap>
#include <QTimer>
#include <QFuture>
#include "printdialog.h"
//#include <QTest>
#include <QFutureWatcher>

XMLPrintSupport::XMLPrintSupport(QObject *parent) : QObject (parent)
{
    stop = false;
}

XMLPrintSupport::~XMLPrintSupport()
{
}

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
    return {};
}


/// NOTE: scenario name should be unique!!!
static QHash<QString, XMLPrintSupport::ScenarioData> s_scenariosData;     // scenario name - scenario tests data

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
        "</Worksheet>";
const QString XMLRowHead = "\n\t<Row ss:AutoFitHeight=\"1\">";// Автонастройка высоты строк
const QString XMLRowCellData[] = {"\n\t\t<Cell ss:Index=\"%1\" ss:StyleID=\"s62\"><Data ss:Type=\"String\">%2</Data></Cell>",//Acceptance Test Report Left Column
                                  "\n\t\t<Cell ss:Index=\"%1\" ss:StyleID=\"s63\"><Data ss:Type=\"String\">%2</Data></Cell>",//Acceptance Test Report Right Column
                                  "\n\t\t<Cell ss:Index=\"%1\" ss:StyleID=\"s65\"><Data ss:Type=\"String\">%2</Data></Cell>",//Default style cell Выравниваем по ss:Horizontal = Left
                                  "\n\t\t<Cell ss:Index=\"%1\" ss:StyleID=\"s66\"><Data ss:Type=\"String\">%2</Data></Cell>",//Default style cell Выравниваем по ss:Horizontal = Center
                                  "\n\t\t<Cell ss:Index=\"%1\" ss:StyleID=\"s67\"><Data ss:Type=\"String\">%2</Data></Cell>",//Default style Head
                                  "\n\t\t<Cell ss:Index=\"%1\" ss:StyleID=\"s68\"><Data ss:Type=\"String\">%2</Data></Cell>",//Status Passed
                                  "\n\t\t<Cell ss:Index=\"%1\" ss:StyleID=\"s69\"><Data ss:Type=\"String\">%2</Data></Cell>"
                                 };//Status Failed
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
    html += QString("\n<caption align=\"center\">%1</caption>").arg(ShieldingToXML(tableName));
    return html;
}

void widthColumnsSet(QString& html, QList<float> sizeList)
{
    for (auto var : sizeList)
    {
        html += "\n<Column ss:AutoFitWidth=\"0\" ss:Width=\"" + QString::number(double(var)) + "\"/>";
    }
}
void offsetRowsSet(QString& html, unsigned int count)
{
    for (unsigned int i = 0; i < count; ++i)
    {
        html += XMLRowHead + XMLRowFoot;
    }
}
QString XMLPrintSupport::ShieldingToXML(QString xmlData)
{
    xmlData.replace("&", "&amp;");
    xmlData.replace("<", "&lt;");
    xmlData.replace(">", "&gt;");
    xmlData.replace(" ", "&nbsp;");//&apos;
    xmlData.replace("\"", "&quot;");
    return xmlData;
}
QString XMLPrintSupport::CheckAndFixSizeName(QString name)/// Todo переделать с QFonts
{
    const int maxLenght = 32;
    if (name.length() > maxLenght)
    {
        int size = name.length() + 1 + 3/*3 точки*/ - maxLenght;
        int pointer;
        if (size % 2)
        {
            pointer = (name.length() - int(size) - 1) / 2;
        }
        else
        {
            pointer = (name.length() - size) / 2;
        }

        if (name.at(pointer - 1) == " ")
        {
            pointer--;
        }
        if (name.at(pointer + size) == " ")
        {
            size++;
        }

        name.remove(pointer, size);
        name.insert(pointer, "...");
    }
    return name;
}
QString XMLPrintSupport::XMLTableHead(const QString& tableName)
{
    auto tName = CheckAndFixSizeName(tableName);

    QString html = QString("\n<Worksheet ss:Name=\"%1\">").arg(ShieldingToXML(tName.replace("\"", "'")));
    html += QString("\n<Table>");

    if (tableName.toLower() == "acceptance test reports")
    {
        QList<float> sizeColumns = {14.25, 213.75, 213.75};
        widthColumnsSet(html, sizeColumns);
        offsetRowsSet(html, 2);

        html += XMLRowHead;
        html += QString("<Cell ss:Index=\"2\" ss:StyleID=\"s64\" ss:MergeAcross=\"1\" ss:MergeDown=\"2\"><Data ss:Type=\"String\">%1</Data></Cell>").arg(
                    ShieldingToXML(tableName));
        html += XMLRowFoot;

        offsetRowsSet(html, 4);
    }
    else if (tableName.toLower() == "suspect lists of srus")
    {
        QList<float> sizeColumns = {14.25, 55.5, 39.75, 231.75, 24, 62.25, 595.5};
        widthColumnsSet(html, sizeColumns);
        offsetRowsSet(html, 1);

        html += XMLRowHead;
        html += QString("<Cell ss:Index=\"2\" ss:StyleID=\"s70\" ss:MergeAcross=\"" + QString::number(
                            sizeColumns.length() -
                            1)/* длина этого списка равна количеству колонок таблицы + 1*/ +
                        "\"><Data ss:Type=\"String\">%1</Data></Cell>").arg(ShieldingToXML(tableName));
        html += XMLRowFoot;

    }
    else if (tableName.toLower() == "visual inspections")
    {
        QList<float> sizeColumns = {14.25, 55.25, 119.25, 150.75, 140.25};
        widthColumnsSet(html, sizeColumns);
        offsetRowsSet(html, 1);

        html += XMLRowHead;
        html += QString("<Cell ss:Index=\"2\" ss:StyleID=\"s70\" ss:MergeAcross=\"" + QString::number(
                            sizeColumns.length() -
                            1)/* длина этого списка равна количеству колонок таблицы + 1*/ +
                        "\"><Data ss:Type=\"String\">%1</Data></Cell>").arg(ShieldingToXML(tableName));
        html += XMLRowFoot;

    }
    else if (tableName.toLower() == "test result")
    {
        QList<float> sizeColumns = {14.25, 55.25, 339.75};
        widthColumnsSet(html, sizeColumns);
        offsetRowsSet(html, 1);

        html += XMLRowHead;
        html += QString("<Cell ss:Index=\"2\" ss:StyleID=\"s70\" ss:MergeAcross=\"" + QString::number(
                            sizeColumns.length() -
                            1)/* длина этого списка равна количеству колонок таблицы + 1*/ +
                        "\"><Data ss:Type=\"String\">%1</Data></Cell>").arg(ShieldingToXML(tableName));
        html += XMLRowFoot;

    }
    else
    {
        QList<float> sizeColumns = {14.25, 182.25, 51, 51, 51, 51};
        widthColumnsSet(html, sizeColumns);
        offsetRowsSet(html, 1);

        html += XMLRowHead;
        html += QString("<Cell ss:Index=\"2\" ss:StyleID=\"s70\" ss:MergeAcross=\"" + QString::number(
                            sizeColumns.length() -
                            1)/* длина этого списка равна количеству колонок таблицы + 1*/ +
                        "\"><Data ss:Type=\"String\">%1</Data></Cell>").arg(ShieldingToXML(tableName));
        html += XMLRowFoot;

    }

    return html;
}

bool XMLPrintSupport::ParseTestResults(QSqlDatabase& db, qlonglong sessionID, QMap<QString, QString*> filtersMemory)
{
    QSqlQuery query(db);

    QString *filter = filtersMemory.value(TableCaption(NASKTableType::SummaryScenarioTestResults), nullptr);
    bool status;
    if(filter != nullptr  && *filter != "")
    {
        status = query.exec(QString("SELECT * FROM '%1' WHERE `Session ID` = %2")
                            .arg(TableCaption(NASKTableType::SummaryScenarioTestResults)).arg(sessionID) + " AND " + *filter + ";");
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

QString XMLPrintSupport::PrintTable_HTML(QSqlDatabase& db, NASKTableType type, qlonglong sessionID, QMap<QString, QString*> filtersMemory)
{
    QString html;
    QSqlQuery query(db);

    QString *filter = filtersMemory.value(TableCaption(type), nullptr);
    bool status;
    if(filter != nullptr)
    {
        status = query.exec(QString("SELECT * FROM '%1' WHERE `Session ID` = %2")
                            .arg(TableCaption(type)).arg(sessionID) + " AND " + *filter + ";");
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
    ////////////////////////
    if(!query.next())//Query.size() не поддерживается QSQLite, поэтому делаю такое действие для проверки на пустоту таблицы
        return html;
    else
        qDebug() << "query результат предыдущего: " << query.previous();
    ////////////////////////
    html += HTMLTableHead(TableCaption(type));

    html += "\n\t<tr>";
    for (const auto& name : TableColumnNames(type))
    {
        html += QString("\n\t\t<th align=\"center\">%1</th>").arg(name);
    }
    html += "\n\t</tr>";

    while (query.next())
    {
        auto index = 0;
        html += "\n\t<tr>";
        while (!query.value(index).isNull())
        {
            const auto val = query.value(index).toString();
            html += QString("\n\t\t<td align=\"center\">%1</td>").arg(val);
            index++;
        }
        html += "\n\t</tr>";
    }
    html += HTMLTableFoot;

    return html;
}

QString XMLPrintSupport::PrintTable_XML(QSqlDatabase& db, NASKTableType type, qlonglong sessionID, QMap<QString, QString*> filtersMemory)//Suspect list And Visual Inspections
{
    QString xml;
    QSqlQuery query(db);

    QString *filter = filtersMemory.value(TableCaption(type), nullptr);

    bool status;
    if(filter != nullptr && *filter != "")
    {
        status = query.exec(QString("SELECT * FROM '%1' WHERE `Session ID` = %2")
                            .arg(TableCaption(type)).arg(sessionID) + " AND " + *filter + ";");
    }
    else
    {
        status = query.exec(QString("SELECT * FROM '%1' WHERE `Session ID` = %2;")
                            .arg(TableCaption(type)).arg(sessionID));
    }

    if (!status)
    {
        qDebug() << "DB Query Error: " << query.lastError().text();
        return xml;
    }
    ////////////////////////
    if(!query.next())//Query.size() не поддерживается QSQLite, поэтому делаю такое действие для проверки на пустоту таблицы
        return xml;
    else
        qDebug() << "query результат предыдущего: " << query.previous();
    ////////////////////////
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

            if (name.toLower() == "name")
            {
                ++numColumnSpecStyle;
                continue;
            }
            if (name.toLower() == "requirements")
            {
                numColumnSpecStyle += 2;
            }
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
                const auto val = ShieldingToXML(query.value(index).toString());

                if ((numColumnSpecStyle & 1) && EnumColumnSpecStyle(numberCell - (1 + tableOffset)) == EnumColumnSpecStyle::Name)
                {
                    xml += XMLRowCellData[2].arg(numberCell).arg(val);
                }
                else if ((numColumnSpecStyle & 2)
                         && EnumColumnSpecStyle(numberCell - (1 + tableOffset)) == EnumColumnSpecStyle::Requirements )
                {
                    xml += XMLRowCellData[2].arg(numberCell).arg(val);
                }
                else
                {
                    xml += XMLRowCellData[3].arg(numberCell).arg(val);
                }

                numberCell++;
                index++;
            }
        }
        xml += XMLRowFoot;
    }
    xml += XMLTableFoot.arg("");

    return xml;
}

QString XMLPrintSupport::PrintScenarioData_HTML(/*QList<int> pagesNumbersList*/)
{
    QString html;
    if(s_scenariosData.keys().isEmpty())
        return html;

    html += HTMLTableHead(TableCaption(NASKTableType::SummaryTestResults));

    html += "\n\t<tr>";
    for (const auto& name : TableColumnNames(NASKTableType::SummaryTestResults))
    {
        html += QString("\n\t\t<th align=\"center\">%1</th>").arg(name);
    }
    html += "\n\t</tr>";

    QList<QString> names = s_scenariosData.keys();
    sorterNames(names);

    for (const auto& scenarioName : names)
    {

        html += "\n\t<tr>";

        html += QString("\n\t\t<td align=\"center\" width=\"10%\"></td>");          /// TODO: append page number!!!!!
        html += QString("\n\t\t<td align=\"left\">%1</td>").arg(scenarioName);

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


void XMLPrintSupport::sorterNames(QList<QString>& names)
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

QString XMLPrintSupport::PrintScenarioData_XML(int count)
{
    QString xml;
    if(s_scenariosData.keys().isEmpty())
        return xml;

    xml += XMLTableHead(TableCaption(NASKTableType::SummaryTestResults));

    xml += XMLRowHead;
    {
        int numberCell = 1 + tableOffset;
        for (const auto& name : TableColumnNames(NASKTableType::SummaryTestResults))
        {
            xml += XMLRowCellData[4].arg(numberCell).arg(ShieldingToXML(name));
            numberCell++;
        }
    }
    xml += XMLRowFoot;

    int s_pageNum = 1 + count;   /// TODO: append proper page number!!!!!
    bool faile = false;

    QList<QString> names = s_scenariosData.keys();
    sorterNames(names);

    for (const auto& scenarioName : names)
    {
        xml += XMLRowHead;
        {
            int numberCell = 1 + tableOffset;
            xml += XMLRowCellData[3].arg(numberCell).arg(s_pageNum++);
            numberCell++;
            xml += XMLRowCellData[2].arg(numberCell).arg(ShieldingToXML(scenarioName));
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
    if (faile)
    {
        xml += XMLTableFoot.arg("<TabColorIndex>10</TabColorIndex>");    //"<TabColorIndex>9</TabColorIndex>"//9
    }
    else
    {
        xml += XMLTableFoot.arg("");
    }

    return xml;
}

QString XMLPrintSupport::PrintIndividualTestResults_HTML()
{
    QString html;

    QList<QString> names = s_scenariosData.keys();
    sorterNames(names);

    for (const auto& scenarioName : names)
    {
        if(s_scenariosData[scenarioName].testData.isEmpty())
        {
            continue;
        }
        html += HTMLTableHead(scenarioName);

        html += "\n\t<tr>";
        for (const auto& name : TableColumnNames(NASKTableType::IndividualScenarioResults))
        {
            html += QString("\n\t\t<th align=\"center\">%1</th>").arg(name);
        }
        html += "\n\t</tr>";

        for (const auto& d : s_scenariosData[scenarioName].testData)
        {
            html += "\n\t<tr>";
            for (const auto& txtD : d.txtTestData)
            {
                html += QString("\n\t\t<td align=\"center\">%1</td>").arg(txtD);
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

    QList<QString> names = s_scenariosData.keys();
    sorterNames(names);

    for (const auto& scenarioName : names)
    {
        if(s_scenariosData[scenarioName].testData.isEmpty())
        {
            continue;
        }
        xml += XMLTableHead(scenarioName);

        xml += XMLRowHead;
        {
            int numberCell = 1 + tableOffset;
            for (const auto& name : TableColumnNames(NASKTableType::IndividualScenarioResults))
            {
                xml += XMLRowCellData[4].arg(numberCell).arg(ShieldingToXML(name));
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
                    if (EnumColumnSpecStyle(numberCell - (1 + tableOffset)) == EnumColumnSpecStyle::TestName)
                    {
                        xml += XMLRowCellData[2].arg(numberCell).arg(ShieldingToXML(txtD));
                    }
                    else
                    {
                        xml += XMLRowCellData[3].arg(numberCell).arg(ShieldingToXML(txtD));
                    }
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
        if (faile)
        {
            xml += XMLTableFoot.arg("<TabColorIndex>10</TabColorIndex>");    //"<TabColorIndex>9</TabColorIndex>"//9
        }
        else
        {
            xml += XMLTableFoot.arg("");
        }
    }
    return xml;
}

QString XMLPrintSupport::CheckRepairfileName(const QString& lruName, const QString& lruSN, const QString& date,
                                             const QString& fileExtension, const QString& directory)
{
    QString filePath = QString(directory + "/%1_%2_%3")
            .arg(lruName).arg(lruSN).arg(date);

    if (namesFilesExp.count(filePath) != 0)
    {
        QString newFilePath;
        int k = 1;
        do
        {
            newFilePath = filePath + "_" + QString::number(k);
            k++;
        } while (namesFilesExp.count(newFilePath) != 0);
        filePath = newFilePath;
    }
    namesFilesExp.push_back(filePath);

    filePath += QString(".%1").arg(fileExtension);
    qDebug() << filePath;

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

        //f.write(html.toLocal8Bit());
        f.write(html.toUtf8().constData());
        f.close();
    }
}

void XMLPrintSupport::PrintHTMLToFiles(QList<QString> htmlList, QList<QString> htmlFileNameList,  XMLPrintProgress* progress)
{
    for(int i = 0; i < htmlList.size(); ++i)//
    {
        if(stop)
        {
            return;
        }
        progress->setProgressCount(progress->getProgressCount() + progress->getPartProgress());
        PrintHTMLToFile(htmlList.at(i), htmlFileNameList.at(i));
    }
}

void XMLPrintSupport::PrintHTMLToFiles(QList<QString> htmlList, QList<XMLPrintSupport::FileNameElements> htmlFileNameList, const QString& direction,  XMLPrintProgress* progress)
{
    for(int i = 0; i < htmlList.size(); ++i)//
    {
        if(stop)
        {
            return;
        }
        QString path = CheckRepairfileName(htmlFileNameList.at(i).lruName, htmlFileNameList.at(i).lruSN, htmlFileNameList.at(i).date, "xls", direction);
        progress->setProgressCount(progress->getProgressCount() + progress->getPartProgress());
        PrintHTMLToFile(htmlList.at(i), path);
    }
}

void XMLPrintSupport::PrintHTMLToPdfFiles(QList<QString> htmlList, QList<QString> htmlFileNameList,  XMLPrintProgress* progress, QList<QHash<QString, ScenarioData>> scenariosHashList)
{
    for(int i = 0; i < htmlList.size(); ++i)//
    {
        if(stop)
        {
            return;
        }
        s_scenariosData = scenariosHashList.at(i);
        progress->setProgressCount(progress->getProgressCount() + progress->getPartProgress());
        PrintHTMLToPdf(htmlList.at(i), htmlFileNameList.at(i));
    }
}

void XMLPrintSupport::close()
{
    this->~XMLPrintSupport();
}

void XMLPrintSupport::setStop(bool stop)
{
    this->stop = stop;
}

void XMLPrintSupport::CreateHTMLFile(QSqlDatabase& db, const QString& directory, XMLPrintProgress* progress, QMap<QString, QString*> filtersMemory)
{
    QString str = db.databaseName();
    progress->watcher.setFuture(QtConcurrent::run(this, &XMLPrintSupport::WorkAtHTMLFile, directory, str, progress, filtersMemory));
    return;
}

void XMLPrintSupport::CreateXMLFiles(QSqlDatabase& db, const QString& directory, XMLPrintProgress* progress, QMap<QString, QString*> filtersMemory)
{
    QString str = db.databaseName();
    progress->watcher.setFuture(QtConcurrent::run(this, &XMLPrintSupport::WorkAtXMLFile, directory, str, progress, filtersMemory));
    return;
}

void XMLPrintSupport::CreateXMLAndHTML(QSqlDatabase& db, const QString& directory, XMLPrintProgress* progress, QMap<QString, QString*> filtersMemory)
{
    QString str = db.databaseName();
    progress->watcher.setFuture(QtConcurrent::run(this, &XMLPrintSupport::WorkAtXMLAndHTMLFile, directory, str, progress, filtersMemory));
    return;
}

void XMLPrintSupport::WorkAtXMLAndHTMLFile(const QString& directory, const QString& fileName, XMLPrintProgress* progress, QMap<QString, QString*> filtersMemory)
{
    WorkAtXMLFile(directory, fileName, progress, filtersMemory);
    WorkAtHTMLFile(directory, fileName, progress, filtersMemory);
}

void XMLPrintSupport::WorkAtHTMLFile(const QString& directory, const QString& fileName, XMLPrintProgress* progress, QMap<QString, QString*> filtersMemory)
{
    QSqlDatabase db;

    db = QSqlDatabase :: addDatabase("QSQLITE", "Second2"); // В зависимости от типа базы данных нужно будет дополнить и изменить
    db.setDatabaseName(fileName);

    if(!db.open())
        return ;

    QSqlQuery query(db);
    const auto atrColumnsNames = TableColumnNames(NASKTableType::AcceptanceTestReports);
    QString *filter = filtersMemory.value(TableCaption(NASKTableType::AcceptanceTestReports), nullptr);
    bool status;

    if(filter != nullptr  && *filter != "")
    {
        status = query.exec(QString("SELECT * FROM '%1'").arg(TableCaption(NASKTableType::AcceptanceTestReports)) + " WHERE " + *filter + ";");
    }
    else
    {
        status = query.exec(QString("SELECT * FROM '%1';").arg(TableCaption(NASKTableType::AcceptanceTestReports)));
    }
    if (!status)
    {
        qDebug() << "DB Query Error: " << query.lastError().text();
        return ;
    }

    FileNameElements elems;

    QList<QString> htmlList;
    QList<QString> fileNameList;
    QList<QHash<QString, ScenarioData>>scenariosHashList;
    while (query.next())
    {
        QString html = "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" /></head><body>";
        html += "<p><table bordercolor=\"black\" border=\"1\" bgcolor=\"white\" width=\"100%\">";
        html += QString("<caption align=\"center\">%1</caption>")
                .arg(TableCaption(NASKTableType::AcceptanceTestReports));

        qlonglong sessionID = 0;
        auto index = 0;
        while (!query.value(index).isNull())
        {
            const auto val = query.value(index);

            html += "<tr>";
            html += QString("<td align=\"left\">%1</td>").arg(atrColumnsNames.value(index));

            if (index == 9)
            {
                auto dt = QDateTime::fromString(val.toString());
                if (!dt.isValid())
                {
                    dt = QDateTime::fromString(val.toString(), "MM/dd/yyyy HH:mm:ss");
                }
                elems.date = dt.toString("MM_dd_yyyy___HH.mm.ss");
                html += QString("<td align=\"center\">%1</td>").arg(dt.toString("MM/dd/yyyy HH:mm:ss"));
            }
            else
            {
                html += QString("<td align=\"center\">%1</td>").arg(val.toString());
            }
            html += "</tr>";

            if (!index)
            {
                sessionID = val.toLongLong();
            }

            if (index == 2)
            {
                elems.lruName = val.toString();
            }
            if (index == 3)
            {
                elems.lruSN = val.toString();
            }

            index++;
        }
        html += "</table><br><br><br></p>";
        ////////////////////////////
        //        {
        //            int count = 1;
        //            QString temp = PrintTable_HTML(db, NASKTableType::SuspectListOfSRUs, sessionID, filtersMemory);
        //            if(temp != "")
        //                count++;
        //            html += temp;
        //            temp = PrintTable_HTML(db, NASKTableType::VisualInspection, sessionID, filtersMemory);
        //            if(temp != "")
        //                count++;
        //            html += temp;
        //            if (!ParseTestResults(db, sessionID, filtersMemory))
        //            {
        //                qDebug() << "Not managed to parse test results!!!!!";
        //                return ;
        //            }
        //            count++;
        //            html += PrintScenarioData_HTML(count);
        //        }
        /////////////////////////////
        int count = 1;
        QString temp = PrintTable_HTML(db, NASKTableType::SuspectListOfSRUs, sessionID, filtersMemory);
        if(temp != "")
            count++;
        html += temp;
        temp = PrintTable_HTML(db, NASKTableType::VisualInspection, sessionID, filtersMemory);
        if(temp != "")
            count++;
        html += temp;
        if (!ParseTestResults(db, sessionID, filtersMemory))
        {
            qDebug() << "Not managed to parse test results!!!!!";
            return ;
        }
        count++;

        html += PrintScenarioData_HTML();
        html += PrintIndividualTestResults_HTML();

        html += "</body></html>";

        htmlList.push_back(html);
        fileNameList.push_back(CheckRepairfileName(elems.lruName, elems.lruSN, elems.date, "pdf", directory));

        scenariosHashList.push_back(s_scenariosData);
        s_scenariosData.clear();

    }
    progress->setProgressCount(0);

    if(htmlList.size() != 0)
        progress->setPartProgress(float(100) / htmlList.size());
    else
        progress->setProgressCount(100);

    PrintHTMLToPdfFiles(htmlList, fileNameList, progress, scenariosHashList);
    progress->setProgressCount(100);
    db.close();
}

void XMLPrintSupport::WorkAtXMLFile(const QString& directory, const QString& fileName, XMLPrintProgress* progress, QMap<QString, QString*> filtersMemory)
{
    QSqlDatabase db;

    db = QSqlDatabase :: addDatabase("QSQLITE", "Second"); // В зависимости от типа базы данных нужно будет дополнить и изменить
    db.setDatabaseName(fileName);

    if(!db.open())
        return ;

    /// TODO: add document properties???
    ///
    ///  <DocumentProperties xmlns="urn:schemas-microsoft-com:office:office">
    ///    <Author>Author name</Author>
    ///    <LastAuthor>Author name</LastAuthor>
    ///    <Created>current dt</Created>
    ///    <Version>some version</Version>
    ///  </DocumentProperties>

    QList<QString> xmlMainList;

    QString xmlStyles = "";
    foreach (auto var, XMLStyles)
    {
        xmlStyles += var;
    }

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
    QSqlQuery query(db);
    const auto atrColumnsNames = TableColumnNames(NASKTableType::AcceptanceTestReports);

    QString *filter = filtersMemory.value(TableCaption(NASKTableType::AcceptanceTestReports), nullptr);
    bool status;

    if(filter != nullptr  && *filter != "")
    {
        status = query.exec(QString("SELECT * FROM '%1'").arg(TableCaption(NASKTableType::AcceptanceTestReports)) + " WHERE " + *filter + ";");
    }
    else
    {
        status = query.exec(QString("SELECT * FROM '%1';").arg(TableCaption(NASKTableType::AcceptanceTestReports)));
    }

    if (!status)
    {
        qDebug() << "DB Query Error: " << query.lastError().text();
        return ;
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

                xml += XMLRowCellData[0].arg(numberCell).arg(ShieldingToXML(atrColumnsNames.value(index)));
                numberCell++;
                if (atrColumnsNames.value(index).toLower() == "session run date&time")
                {
                    auto dt = QDateTime::fromString(val.toString());
                    if (!dt.isValid())
                    {
                        dt = QDateTime::fromString(val.toString(), "MM/dd/yyyy HH:mm:ss");
                    }
                    fileNameElements.date = dt.toString("MM_dd_yyyy___HH.mm.ss");

                    xml += XMLRowCellData[1].arg(numberCell).arg(ShieldingToXML(dt.toString("MM/dd/yyyy HH:mm:ss")));
                    numberCell++;
                }
                else
                {
                    xml += XMLRowCellData[1].arg(numberCell).arg(ShieldingToXML(val.toString()));
                    numberCell++;
                }

                xml += XMLRowFoot;

                if (!index)
                {
                    sessionID = val.toLongLong();
                }
                if (atrColumnsNames.value(index).toLower() == "lru name")
                {
                    fileNameElements.lruName = val.toString();
                }
                if (atrColumnsNames.value(index).toLower() == "lru s/n")
                {
                    fileNameElements.lruSN = val.toString();
                }

                index++;
            }
        }
        xml += XMLTableFoot.arg("");
        {
            int count = 1;
            QString temp = PrintTable_XML(db, NASKTableType::SuspectListOfSRUs, sessionID, filtersMemory);
            if(temp != "")
                count++;
            xml += temp;
            temp = PrintTable_XML(db, NASKTableType::VisualInspection, sessionID, filtersMemory);
            if(temp != "")
                count++;
            xml += temp;

            if (!ParseTestResults(db, sessionID, filtersMemory))
            {
                qDebug() << "Not managed to parse test results!!!!!";
                return ;
            }
            count++;
            xml += PrintScenarioData_XML(count);
        }
        xml += PrintIndividualTestResults_XML();

        xml += xmlFoot;

        xmlMainList << xml;
        fileNameElementsList << fileNameElements;
        s_scenariosData.clear();
    }
    db.close();
    progress->setProgressCount(0);

    if(xmlMainList.size() != 0)
        progress->setPartProgress(float(100) / xmlMainList.size());
    else
        progress->setProgressCount(100);

    PrintHTMLToFiles(xmlMainList, fileNameElementsList, directory, progress);
    progress->setProgressCount(100);
}

void XMLPrintSupport::onExportDBAction(QSqlDatabase& db, const QString& typeFile, QString directory, XMLPrintProgress* progress, QMap<QString, QString*> filtersMemory)
{
    if (!db.isOpen())
    {
        qDebug() << "DB is not open";
        return;
    }
    namesFilesExp.clear();
    XMLPrintSupport* printer = new XMLPrintSupport();
    if (typeFile == "PDF")
    {
        printer->CreateHTMLFile(db, directory, progress, filtersMemory);
    }
    if (typeFile == "XML")
    {
        printer->CreateXMLFiles(db, directory, progress, filtersMemory);
    }
    if(typeFile == "PDF + XML")
    {
        printer->CreateXMLAndHTML(db, directory, progress, filtersMemory);
    }
    progress->setPrintSup(printer);
    qDebug() << "Export finished!";
}
