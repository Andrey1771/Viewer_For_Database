#include "printcontroller.h"
#include "printprogress.h"
#include "printfilestools.h"
#include <QFile>
#include <QSqlQuery>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>
#include <QDateTime>
#include <QSqlError>
#include <QPrinter>
#include <QTextDocument>
#include <QPainter>

PrintController::PrintController(QSqlDatabase& db, const QString& typeFile,const QString& directory, PrintProgress* progress, QMap<QString, QList<QString*>> filtersMemory, QObject *parent) : QObject (parent)
{
    this->db = db;
    this->typeFile = typeFile;
    this->directory = directory;
    this->progress = progress;
    this->filtersMemory = filtersMemory;
    stop = false;
}



QString PrintController::CheckRepairfileName(const QString& projectName, const QString& lruSN, const QString& date,
                                             const QString& fileExtension, const QString &directory, QList<QString>& namesFilesExp)
{
    QString filePath = QString(directory + "/%1_%2_%3")
            .arg(projectName).arg(lruSN).arg(date);

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

void PrintController::PrintHTMLToFile(const QString& html, const QString& fileName)
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

void PrintController::PrintHTMLToFiles(QList<QString> htmlList, QList<QString> htmlFileNameList)
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

void PrintController::PrintHTMLToFiles(QList<QString> htmlList, QList<PrintController::FileNameElements> htmlFileNameList, const QString& direction)
{
    for(int i = 0; i < htmlList.size(); ++i)//
    {
        if(stop)
        {
            return;
        }
        QString path = CheckRepairfileName(htmlFileNameList.at(i).projectName, htmlFileNameList.at(i).lruSN, htmlFileNameList.at(i).date, "xls", direction, namesFilesExp);
        progress->setProgressCount(progress->getProgressCount() + progress->getPartProgress());
        PrintHTMLToFile(htmlList.at(i), path);
    }
}

void PrintController::CreateHTMLFile()
{
    QString str = db.databaseName();
    progress->watcher.setFuture(QtConcurrent::run(this, &PrintController::WorkAtHTMLFile, str));
    return;
}

void PrintController::CreateXMLFiles()
{
    QString str = db.databaseName();
    progress->watcher.setFuture(QtConcurrent::run(this, &PrintController::WorkAtXMLFile, str));
    return;
}

void PrintController::CreateXMLAndHTML()
{
    QString str = db.databaseName();
    progress->watcher.setFuture(QtConcurrent::run(this, &PrintController::WorkAtXMLAndHTMLFile, str));
    return;
}

void PrintController::WorkAtHTMLFile(const QString& fileName)
{
    QSqlDatabase db;

    db = QSqlDatabase :: addDatabase("QSQLITE", "Second2"); // В зависимости от типа базы данных нужно будет дополнить и изменить
    db.setDatabaseName(fileName);

    if(!db.open())
        return ;

    QSqlQuery query(db);
    const auto atrColumnsNames = TableColumnNames(NASKTableType::AcceptanceTestReports);
    QList<QString*> filter = filtersMemory.value(TableCaption(NASKTableType::AcceptanceTestReports));
    bool status;
    PDFPrintSupport printerPDF;

    if(filter.size())
    {
        QString filterMemory = getFiltersMemories(filter);
        if(filterMemory == "()")
            status = query.exec(QString("SELECT * FROM '%1';").arg(TableCaption(NASKTableType::AcceptanceTestReports)));
        else
            status = query.exec(QString("SELECT * FROM '%1'").arg(TableCaption(NASKTableType::AcceptanceTestReports)) + " WHERE " + filterMemory + ";");
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

    int countFiles = 0;

    while (query.next())
    {
        countFiles++;

        QString html = printerPDF.HTMLHead;
        //QString html = HTMLHead + HTMLEndTab;
        html += "<p>" + printerPDF.HTMLEndTab;
        html += QString("<caption align=\"center\">%1</caption>")
                .arg(TableCaption(NASKTableType::AcceptanceTestReports));
        //<div style=\"page-break-before:always\"></div>
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

            if (index == 1)////////////!!!!!!!!
            {
                elems.projectName = val.toString();
            }
            if (index == 3)
            {
                elems.lruSN = val.toString();
            }

            index++;
        }
        html += "</table><br><br><br></div></p>";



        ///
        /// Вставить footer и проверку на размер
        ///
        int count = 1;
        QString temp = printerPDF.PrintTable_HTML(NASKTableType::SuspectListOfSRUs, sessionID, db, filtersMemory);
        if(temp != "")
            count++;
        html += temp;
        temp = printerPDF.PrintTable_HTML(NASKTableType::VisualInspection, sessionID, db, filtersMemory);
        if(temp != "")
            count++;
        html += temp;
        if (!ParseTestResults(sessionID, s_scenariosData, db, filtersMemory))
        {
            qDebug() << "Not managed to parse test results!!!!!";
            return ;
        }
        count++;

        html += printerPDF.PrintScenarioData_HTML(s_scenariosData);
        html += printerPDF.PrintIndividualTestResults_HTML(s_scenariosData);

        html += "</div></p></body></html>";//<footer>%1</footer>QString().arg(elems.projectName)

        htmlList.push_back(html);
        fileNameList.push_back(CheckRepairfileName(elems.projectName, elems.lruSN, elems.date, "pdf", directory, namesFilesExp));

        scenariosHashList.push_back(s_scenariosData);
        s_scenariosData.clear();

    }
    emit progress->countedFiles(countFiles);
    progress->setProgressCount(0);

    if(htmlList.size() != 0)
        progress->setPartProgress(float(100) / htmlList.size());
    else
        progress->setProgressCount(100);

    PrintHTMLToPdfFiles(htmlList, fileNameList, scenariosHashList);
    progress->setProgressCount(100);
    db.close();
    if(progress->getTypePrint() == TypePrint::PDF)
        emit progress->filePrinted(int(TypePrint::PDF));
}

void PrintController::WorkAtXMLFile(const QString& fileName)
{

    QSqlDatabase db;

    db = QSqlDatabase :: addDatabase("QSQLITE", "Second"); // В зависимости от типа базы данных нужно будет дополнить и изменить
    db.setDatabaseName(fileName);

    if(!db.open())
        return;

    /// TODO: add document properties???
    ///
    ///  <DocumentProperties xmlns="urn:schemas-microsoft-com:office:office">
    ///    <Author>Author name</Author>
    ///    <LastAuthor>Author name</LastAuthor>
    ///    <Created>current dt</Created>
    ///    <Version>some version</Version>
    ///  </DocumentProperties>

    XMLPrintSupport printerXML;
    QList<QString> xmlMainList;

    QString xmlStyles = "";
    foreach (auto var, printerXML.XMLStyles)
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

    QList<PrintController::FileNameElements> fileNameElementsList;
    QSqlQuery query(db);
    const auto atrColumnsNames = TableColumnNames(NASKTableType::AcceptanceTestReports);
    ///////////////////////////
    QList<QString*> filter = filtersMemory.value(TableCaption(NASKTableType::AcceptanceTestReports));
    bool status;
    ///////////////////////////
    if(filter.size())
    {
        //qDebug() << QString("SELECT * FROM '%1'").arg(TableCaption(NASKTableType::AcceptanceTestReports)) + " WHERE " + getFiltersMemories(filter) + ";";
        QString filterMemory = getFiltersMemories(filter);
        if(filterMemory == "()")
            status = query.exec(QString("SELECT * FROM '%1';").arg(TableCaption(NASKTableType::AcceptanceTestReports)));
        else
            status = query.exec(QString("SELECT * FROM '%1'").arg(TableCaption(NASKTableType::AcceptanceTestReports)) + " WHERE " + filterMemory + ";");
    }
    else
    {
        status = query.exec(QString("SELECT * FROM '%1';").arg(TableCaption(NASKTableType::AcceptanceTestReports)));
    }

    if (!status)
    {
        qDebug() << "DB Query Error: " << query.lastError().text();
        return;
    }
    int countFiles = 0;
    while (query.next())
    {
        countFiles++;
        QString xml = xmlHead;
        xml += printerXML.XMLTableHead(TableCaption(NASKTableType::AcceptanceTestReports));

        qlonglong sessionID = 0;
        auto index = 0;

        PrintController::FileNameElements fileNameElements;
        while (!query.value(index).isNull())
        {
            const auto val = query.value(index);//Acceptance Test Reports
            {
                int numberCell = 1 + tableOffset;
                xml += printerXML.XMLRowHead;

                xml += printerXML.XMLRowCellData[0].arg(numberCell).arg(ShieldingToXML(atrColumnsNames.value(index)));
                numberCell++;
                if (atrColumnsNames.value(index).toLower() == "session run date&time")
                {
                    auto dt = QDateTime::fromString(val.toString());
                    if (!dt.isValid())
                    {
                        dt = QDateTime::fromString(val.toString(), "MM/dd/yyyy HH:mm:ss");
                    }
                    fileNameElements.date = dt.toString("MM_dd_yyyy___HH.mm.ss");

                    xml += printerXML.XMLRowCellData[1].arg(numberCell).arg(ShieldingToXML(dt.toString("MM/dd/yyyy HH:mm:ss")));
                    numberCell++;
                }
                else
                {
                    xml += printerXML.XMLRowCellData[1].arg(numberCell).arg(ShieldingToXML(val.toString()));
                    numberCell++;
                }

                xml += printerXML.XMLRowFoot;

                if (!index)
                {
                    sessionID = val.toLongLong();
                }
                if (atrColumnsNames.value(index).toLower() == "project name")
                {
                    fileNameElements.projectName = val.toString();
                }
                if (atrColumnsNames.value(index).toLower() == "lru s/n")
                {
                    fileNameElements.lruSN = val.toString();
                }

                index++;
            }
        }
        xml += printerXML.XMLTableFoot.arg("");
        {
            int count = 1;
            QString temp = printerXML.PrintTable_XML(NASKTableType::SuspectListOfSRUs, sessionID, db, filtersMemory);
            if(temp != "")
                count++;
            xml += temp;
            temp = printerXML.PrintTable_XML(NASKTableType::VisualInspection, sessionID, db, filtersMemory);
            if(temp != "")
                count++;
            xml += temp;

            if (!ParseTestResults(sessionID, s_scenariosData, db, filtersMemory))
            {
                qDebug() << "Not managed to parse test results!!!!!";
                return ;
            }
            count++;
            xml += printerXML.PrintScenarioData_XML(count, s_scenariosData);
        }
        xml += printerXML.PrintIndividualTestResults_XML(s_scenariosData);

        xml += xmlFoot;

        xmlMainList << xml;
        fileNameElementsList << fileNameElements;
        s_scenariosData.clear();
    }
    emit progress->countedFiles(countFiles);
    db.close();
    progress->setProgressCount(0);

    if(xmlMainList.size() != 0)
        progress->setPartProgress(float(100) / xmlMainList.size());
    else
        progress->setProgressCount(100);

    PrintHTMLToFiles(xmlMainList, fileNameElementsList, directory);

    progress->setProgressCount(100);
    if(progress->getTypePrint() == TypePrint::XML)
        emit progress->filePrinted(int(TypePrint::XML));
}

void PrintController::WorkAtXMLAndHTMLFile(const QString& fileName)
{
    WorkAtXMLFile(fileName);
    WorkAtHTMLFile(fileName);
    emit progress->filePrinted(int(TypePrint::XMLPDF));
}


static const int textMargins = 12; // in millimeters
static const int borderMargins = 10; // in millimeters

double mmToPixels(QPrinter& printer, int mm)
{
    return mm * 0.039370147 * printer.resolution();
}
static int temp = 0;
void paintPage(QPrinter& printer, int pageNumber, int pageCount,
                      QPainter* painter, QTextDocument* doc,
                      const QRectF& textRect, qreal footerHeight)
{
    //qDebug() << "Printing page" << pageNumber;
    //const QSizeF pageSize = printer.paperRect().size();
    //qDebug() << "pageSize=" << pageSize;

    //const double bm = mmToPixels(printer, borderMargins);
    //const QRectF borderRect(bm, bm, pageSize.width() - 2 * bm, pageSize.height() - 2 * bm);
    //painter->drawRect(borderRect);

    painter->save();
    // textPageRect is the rectangle in the coordinate system of the QTextDocument, in pixels,
    // and starting at (0,0) for the first page. Second page is at y=doc->pageSize().height().
    const QRectF textPageRect(0, pageNumber * doc->pageSize().height(), doc->pageSize().width(), doc->pageSize().height());
    // Clip the drawing so that the text of the other pages doesn't appear in the margins
    painter->setClipRect(textRect);
    // Translate so that 0,0 is now the page corner
    painter->translate(0, -textPageRect.top());
    // Translate so that 0,0 is the text rect corner
    painter->translate(textRect.left(), textRect.top());
    doc->drawContents(painter);
    painter->restore();

    // Footer: page number or "end"
    QRectF footerRect = textRect;
    footerRect.setTop(textRect.bottom());
    footerRect.setHeight(footerHeight);

    painter->drawText(footerRect, Qt::AlignVCenter | Qt::AlignRight, QObject::tr("rewrewrewrerwer %1").arg(pageNumber+1));
}

void printDocument(QPrinter& printer, QTextDocument* doc)
{
    QPainter painter( &printer );
    QSizeF pageSize = printer.pageRect().size(); // page size in pixels
    // Calculate the rectangle where to lay out the text
    const double tm = mmToPixels(printer, textMargins);
    const qreal footerHeight = painter.fontMetrics().height();
    const QRectF textRect(tm, tm, pageSize.width() - 2 * tm, pageSize.height() - 2 * tm - footerHeight);
    //qDebug() << "textRect=" << textRect;
    doc->setPageSize(textRect.size());

    const int pageCount = doc->pageCount();

    bool firstPage = true;
    for (int pageIndex = 0; pageIndex < pageCount; ++pageIndex) {

        if (!firstPage)
            printer.newPage();

            paintPage( printer, pageIndex, pageCount, &painter, doc, textRect, footerHeight );
        firstPage = false;
    }

}



void PrintController::PrintHTMLToPdf(const QString& html, const QString& fileName)
{
    {
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPrinter::A4);
    printer.setOutputFileName(fileName);
    printer.setPageMargins(QMargins(0, 0, 0, 50));

    //QPainter painter;
    //painter.begin(&printer);

    QTextDocument doc;
    doc.setHtml(html);
    //cur.insertHtml(html);


//    QString pdf = doc.toHtml();
//    QFile f("C:\\Users\\Hohlov\\Desktop\\html97.txt");
//    f.open(QIODevice::WriteOnly);
//    f.write(pdf.toUtf8().constData());
//    f.close();

    //doc.print(&printer);
    printDocument(printer, &doc);
    //doc.print(&printer);

    }
//    {
//        QPrinter printer(QPrinter::PrinterResolution);
//        printer.setOutputFormat(QPrinter::PdfFormat);
//        printer.setPageSize(QPrinter::A4);
//        printer.setOutputFileName(fileName);
//        printer.setPageMargins(QMargins(0, 0, 0, 0));

//        QPainter painter;

//        painter.begin(&printer);

//        painter.setBrush(Qt::transparent);
//        painter.drawPixmap(0, 0, 100, 100, QPixmap("img.png"));
//        painter.drawPixmap(200, 0, 100, 100, QPixmap("img.png"));
//        painter.drawPixmap(0, 100, 100, 100, QPixmap("img.png"));
//        painter.drawPixmap(200, 100, 100, 100, QPixmap("img.png"));
//        painter.end();
//        //    QFile f("C:\\Users\\Hohlov\\Desktop\\html22.txt");
//        //    f.open(QIODevice::WriteOnly);
//        //    f.write(html.toUtf8().constData());
//        //    f.close();
//    }
}



void PrintController::PrintHTMLToPdfFiles(QList<QString> htmlList, QList<QString> htmlFileNameList, QList<QHash<QString, ScenarioData>> scenariosHashList)
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


void PrintController::onExportDBAction(QSqlDatabase& db, const QString& typeFile, QString directory, PrintProgress* progress, QMap<QString, QList<QString*>> filtersMemory)
{
    if (!db.isOpen())
    {
        qDebug() << "DB is not open";
        return;
    }

    progress->startNewPrintSup(new PrintController(db, typeFile, directory, progress, filtersMemory, nullptr));
    PrintController* printer = progress->getPrintSup();
    if (typeFile == "PDF")
    {
        printer->CreateHTMLFile();
    }
    if (typeFile == "XML")
    {
        printer->CreateXMLFiles();
    }
    if(typeFile == "PDF + XML")
    {
        printer->CreateXMLAndHTML();
    }

    qDebug() << "Export finished!";
}

void PrintController::setStop(bool stop)
{
    this->stop = stop;
}


void PrintController::close()
{
    //this->~PrintController();
}









