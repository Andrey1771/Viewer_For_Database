#ifndef PDFPRINTSUPPORT_H
#define PDFPRINTSUPPORT_H
#include "printfilestools.h"
#include <QSqlDatabase>



class PDFPrintSupport
{
    struct HTMLFilesProperties
    {
        QList<QString> htmlList;
        QList<QString> fileNameList;
    };
public:
    PDFPrintSupport();

    QString HTMLTableHead(const QString& tableName);
    QString PrintIndividualTestResults_HTML(QHash<QString, ScenarioData> &s_scenariosData);
    QString PrintScenarioData_HTML(QHash<QString, ScenarioData> &s_scenariosData);
    QString PrintTable_HTML(NASKTableType type, qlonglong sessionID, QSqlDatabase &db, QMap<QString, QList<QString *> > &filtersMemory);


    const QString HTMLCSSFooterString = "div.footer {"
                                        "display: block;"
                                        "text-align: center;"
                                        "position: fixed;"
                                        "bottom: 0;"
                                        "margin-bottom: -100px;"
                                        "width: 100%;"
                                        "}";
    const QString HTMLCSSPagesNumbers = "contents {"
                                        "display: block;"
                                        "page: table-of-contents;"
                                        "counter-reset: PAGE 1"
                                        "}"
                                        "@page {"
                                        "@top { content: \"Table of Contents\" }"
                                        "@bottom {"
                                        "content: counter(page)"
                                        "}"
                                        "}";
    const QString HTMLTableFoot = "\n</table>\n<br>\n<br>\n<br>\n</div></p>";//"\n</table>\n<br>\n<br>\n<br>\n</div> <div class=\"footer\">FOOTER</div></p>"
    const QString HTMLHead = "<html>"
                             "<head>"
                             "<style type=\"text/css\">"
            ///"table {"
            ///"margin-bottom: 100px;"
            ///"}"
            //                         "table {"
            //                         "counter-reset: section;"
            //                         "}"
            //        "tr {"
            //        "counter-reset: counterPages 0;"
            //        "}"
            //        "td::before {"
            //        "counter-increment: counterPages 1;"
            //        "content: \"Page \"counter(counterPages, \".\")\":\";"
            //        "}"

            //+ HTMLCSSPagesNumbers
            //+ HTMLCSSFooterString +

            "</style>"
            "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />"
            "</head>"
            "<body>";
    const QString HTMLEndTabStyle = "<div style=\"page-break-after:always\">";//<div style=\"page-break-before:always\"></div>
    const QString HTMLEndTab = HTMLEndTabStyle + "<table bordercolor=\"black\" border=\"1\" bgcolor=\"white\" width=\"100%\">";

};

#endif // PDFPRINTSUPPORT_H
