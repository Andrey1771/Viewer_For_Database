#ifndef XMLPRINTSUPPORT_H
#define XMLPRINTSUPPORT_H
#include "printfilestools.h"
#include <QSqlDatabase>

class XMLPrintProgress;
class PrintDialog;
class PrintDialogItemModel;
class XMLPrintSupport
{

public:

    XMLPrintSupport();
    ~XMLPrintSupport();

    void widthColumnsSet(QString& html, QList<float> sizeList);
    void offsetRowsSet(QString& html, unsigned int count);
    QString CheckAndFixSizeName(QString name);/// Todo переделать с QFonts
    QString XMLTableHead(const QString& tableName);
    QString PrintTable_XML(NASKTableType type, qlonglong sessionID, QSqlDatabase &db, QMap<QString, QList<QString *> > &filtersMemory);
    QString PrintScenarioData_XML(int count, QHash<QString, ScenarioData> &s_scenariosData);
    QString PrintIndividualTestResults_XML(QHash<QString, ScenarioData> &s_scenariosData);

    //QStringList TableColumnNames(NASKTableType type);
    //QString TableCaption(NASKTableType type);



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
    const QString XMLRowCellData[7] = {"\n\t\t<Cell ss:Index=\"%1\" ss:StyleID=\"s62\"><Data ss:Type=\"String\">%2</Data></Cell>",//Acceptance Test Report Left Column
                                      "\n\t\t<Cell ss:Index=\"%1\" ss:StyleID=\"s63\"><Data ss:Type=\"String\">%2</Data></Cell>",//Acceptance Test Report Right Column
                                      "\n\t\t<Cell ss:Index=\"%1\" ss:StyleID=\"s65\"><Data ss:Type=\"String\">%2</Data></Cell>",//Default style cell Выравниваем по ss:Horizontal = Left
                                      "\n\t\t<Cell ss:Index=\"%1\" ss:StyleID=\"s66\"><Data ss:Type=\"String\">%2</Data></Cell>",//Default style cell Выравниваем по ss:Horizontal = Center
                                      "\n\t\t<Cell ss:Index=\"%1\" ss:StyleID=\"s67\"><Data ss:Type=\"String\">%2</Data></Cell>",//Default style Head
                                      "\n\t\t<Cell ss:Index=\"%1\" ss:StyleID=\"s68\"><Data ss:Type=\"String\">%2</Data></Cell>",//Status Passed
                                      "\n\t\t<Cell ss:Index=\"%1\" ss:StyleID=\"s69\"><Data ss:Type=\"String\">%2</Data></Cell>"
                                     };//Status Failed
    const QList<QString> XMLStyles = {"\n<Style ss:ID=\"Default\" ss:Name=\"Normal\">"
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

};

#endif // XMLPRINTSUPPORT_H
