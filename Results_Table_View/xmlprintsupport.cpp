#include "xmlprintsupport.h"
#include "printdialog.h"

#include <QDebug>
#include <QSqlQuery>
#include <QPrinter>
#include <QFile>
#include <QTextDocument>
#include <QSqlError>

#include <QDateTime>
#include <QtConcurrent/QtConcurrentRun>
#include <QPainter>

#include <QTextCursor>
XMLPrintSupport::XMLPrintSupport()
{

}

XMLPrintSupport::~XMLPrintSupport()
{
}

void XMLPrintSupport::widthColumnsSet(QString& html, QList<float> sizeList)
{
    for (auto var : sizeList)
    {
        html += "\n<Column ss:AutoFitWidth=\"0\" ss:Width=\"" + QString::number(double(var)) + "\"/>";
    }
}
void XMLPrintSupport::offsetRowsSet(QString& html, unsigned int count)
{
    for (unsigned int i = 0; i < count; ++i)
    {
        html += XMLRowHead + XMLRowFoot;
    }
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
        QList<float> sizeColumns = {14.25, /*55.5,*/ 39.75, 231.75, 24, 62.25, 595.5};
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
        QList<float> sizeColumns = {14.25, /*55.25,*/ 119.25, 150.75, 140.25};
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


QString XMLPrintSupport::PrintTable_XML(NASKTableType type, qlonglong sessionID, QSqlDatabase& db, QMap<QString, QList<QString*>>& filtersMemory)//Suspect list And Visual Inspections
{
    QString xml;

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
        qDebug() << "DB Query Error: " << query.lastError().text();
        return xml;
    }

    if(!query.next())//Query.size() не поддерживается QSQLite, поэтому делаю такое действие для проверки на пустоту таблицы
        return xml;
    else
        query.previous();

    xml += XMLTableHead(TableCaption(type));


    int numColumnSpecStyle = 0;


    xml += XMLRowHead;
    {
        int numberCell = 1 + tableOffset;
        for (const auto& name : TableColumnNames(type))
        {
            xml += XMLRowCellData[4].arg(numberCell).arg(name);
            numberCell++;

            if (name.toLower() == "name")///TODO FIX THAT
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

            bool columnRemoved = false;
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

                const auto val = ShieldingToXML(query.value(index).toString());
                if (type == NASKTableType::SuspectListOfSRUs && EnumColumnSpecStyle(numberCell - (1 + tableOffset)) == EnumColumnSpecStyle::Description)
                {
                    xml += XMLRowCellData[2].arg(numberCell).arg(val);
                }
                else if (type == NASKTableType::SuspectListOfSRUs && EnumColumnSpecStyle(numberCell - (1 + tableOffset)) == EnumColumnSpecStyle::RequirementsOrName)
                {
                    xml += XMLRowCellData[2].arg(numberCell).arg(val);
                }
                else if (type == NASKTableType::SuspectListOfSRUs && EnumColumnSpecStyle(numberCell - (1 + tableOffset)) == EnumColumnSpecStyle::RequirementsOrName)
                {
                    xml += XMLRowCellData[2].arg(numberCell).arg(val);
                }
                else if ((numColumnSpecStyle & 1) && EnumColumnSpecStyle(numberCell - (1 + tableOffset)) == EnumColumnSpecStyle::Name)
                {
                    xml += XMLRowCellData[2].arg(numberCell).arg(val);
                }
                else if ((numColumnSpecStyle & 2)
                         && EnumColumnSpecStyle(numberCell - (1 + tableOffset)) == EnumColumnSpecStyle::RequirementsOrName )
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



QString XMLPrintSupport::PrintScenarioData_XML(int count, QHash<QString, ScenarioData>& s_scenariosData)
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
        if(scenarioName.toLower() == QString("Session ID").toLower())
        {
            continue;
        }

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
        xml += XMLTableFoot.arg("<TabColorIndex>10</TabColorIndex>");
    }
    else
    {
        xml += XMLTableFoot.arg("");
    }
    return xml;
}



QString XMLPrintSupport::PrintIndividualTestResults_XML(QHash<QString, ScenarioData>& s_scenariosData)
{
    QString xml;

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
                    if (EnumColumnIndividualSpecStyle(numberCell - (1 + tableOffset)) == EnumColumnIndividualSpecStyle::TestName)
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
            xml += XMLTableFoot.arg("<TabColorIndex>10</TabColorIndex>");
        }
        else
        {
            xml += XMLTableFoot.arg("");
        }
    }
    return xml;
}
