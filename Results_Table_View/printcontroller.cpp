#include "printcontroller.h"
#include "printprogress.h"
#include "printfilestools.h"
#include <QFile>
#include <QSqlQuery>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>
#include <QDateTime>

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

    return filePath;
}

void PrintController::PrintXMLToFile(const QString& xml, const QString& fileName)
{
    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly))
    {
        qDebug() << "Not managed to open " << fileName << " for write HTML";
    }
    else
    {
        f.write(xml.toUtf8().constData());
        f.close();
    }
}

void PrintController::PrintXMLToFiles(QList<QString> xmlList, QList<QString> htmlFileNameList)
{
    for(int i = 0; i < xmlList.size(); ++i)//
    {
        if(stop)
        {
            return;
        }
        progress->nextPartProgress();
        PrintXMLToFile(xmlList.at(i), htmlFileNameList.at(i));
    }
}

void PrintController::PrintXMLToFiles(QList<QString> xmlList, QList<PrintController::FileNameElements> htmlFileNameList, const QString& direction)
{
    for(int i = 0; i < xmlList.size(); ++i)//
    {
        if(stop)
        {
            return;
        }
        QString path = CheckRepairfileName(htmlFileNameList.at(i).projectName, htmlFileNameList.at(i).lruSN, htmlFileNameList.at(i).date, "xls", direction, namesFilesExp);
        progress->nextPartProgress();
        PrintXMLToFile(xmlList.at(i), path);
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

    QList<QString> fileNameList;
    QList<QHash<QString, ScenarioData>>scenariosHashList;

    int countFiles = 0;

    while (query.next())
    {
        QList<TableHTML>tablesHTML;
        QList<QString> tablesInFileList;
        countFiles++;

        QString html = PDFStyles::HTMLHead;
        html += "<p>" + PDFStyles::HTMLEndTab;
        html += QString("<caption align=\"center\">%1</caption>")
                .arg(TableCaption(NASKTableType::AcceptanceTestReports));
        tablesInFileList.push_back(TableCaption(NASKTableType::AcceptanceTestReports));

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
        tablesHTML.push_back(TableHTML(html + "</div></p></body></html>"));
        int count = 1;
        QString temp = printerPDF.PrintTable_HTML(NASKTableType::SuspectListOfSRUs, sessionID, db, filtersMemory, tablesInFileList, tablesHTML);
        if(temp != "")
            count++;
        html += temp;
        temp = printerPDF.PrintTable_HTML(NASKTableType::VisualInspection, sessionID, db, filtersMemory, tablesInFileList, tablesHTML);
        if(temp != "")
            count++;
        html += temp;
        if (!ParseTestResults(sessionID, s_scenariosData, db, filtersMemory))
        {
            qDebug() << "Not managed to parse test results!!!!!";
            return ;
        }
        count++;

        html += printerPDF.PrintScenarioData_HTML(s_scenariosData, tablesInFileList, tablesHTML);
        html += printerPDF.PrintIndividualTestResults_HTML(s_scenariosData, tablesInFileList, tablesHTML);

        html += "</div></p></body></html>";//<footer>%1</footer>QString().arg(elems.projectName)

        fileNameList.push_back(CheckRepairfileName(elems.projectName, elems.lruSN, elems.date, "pdf", directory, namesFilesExp));

        scenariosHashList.push_back(s_scenariosData);
        s_scenariosData.clear();
        tablesInFile.insert(countFiles - 1, tablesInFileList);
        tablesHTMLInFiles.push_back(tablesHTML);
    }
    emit progress->countedFiles(countFiles);
    progress->setProgressCount(0);

    if(tablesHTMLInFiles.size() != 0)
        progress->setPartProgress(float(100) / tablesHTMLInFiles.size());
    else
        progress->setProgressCount(100);

    PrintHTMLToPdfFiles(fileNameList, scenariosHashList, tablesInFile);
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

    QList<QString*> filter = filtersMemory.value(TableCaption(NASKTableType::AcceptanceTestReports));
    bool status;

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

    PrintXMLToFiles(xmlMainList, fileNameElementsList, directory);

    progress->setProgressCount(100);
    if(progress->getTypePrint() == TypePrint::XML)
        emit progress->filePrinted(int(TypePrint::XML));
}

void PrintController::WorkAtXMLAndHTMLFile(const QString& fileName)
{
    WorkAtXMLFile(fileName);
    emit progress->typePrintFileChanged();
    WorkAtHTMLFile(fileName);
    emit progress->filePrinted(int(TypePrint::XMLPDF));
}

double PrintController::mmToPixels(QPrinter& printer, int mm)
{
    return mm * 0.039370147 * printer.resolution();
}

void PrintController::paintPage(int pageNumber, QPainter* painter, QTextDocument* doc,
                                const QRectF& textRect, qreal footerHeight, const QString& tableName,
                                int numberCurrentPageDoc, int firstVal, const QString& fileNamePath)
{
    painter->save();
    const QRectF textPageRect(0, (firstVal + 1 - numberCurrentPageDoc) * doc->pageSize().height(), doc->pageSize().width(), doc->pageSize().height());
    painter->translate(0, -textPageRect.top());
    painter->translate(textRect.left(), textRect.top());
    doc->drawContents(painter, textPageRect);
    painter->restore();

    QRectF footerRect = textRect;
    footerRect.setTop(textRect.bottom());
    footerRect.setHeight(footerHeight);

    painter->drawText(footerRect, Qt::AlignVCenter | Qt::AlignCenter, QObject::tr("%1").arg(tableName));
    footerRect.setTop(textRect.bottom() + footerHeight);
    footerRect.setHeight(footerHeight);
    painter->drawText(footerRect, Qt::AlignVCenter | Qt::AlignCenter, QObject::tr("%1").arg(fileNamePath));
    footerRect.setTop(textRect.bottom() + 2 * footerHeight);
    footerRect.setHeight(footerHeight);
    painter->drawText(footerRect, Qt::AlignVCenter | Qt::AlignRight, QObject::tr("%1").arg(pageNumber + 1));
}

PrintController::TestResultsDataInFile PrintController::findTestResultsStrNum(const QList<QString>& tablesInFile, const QList<TableHTML>& tablesHTML)
{
    int numberTestResultsTable = -1;
    QString testResultsStr;
    for(int i = 0; i < tablesInFile.size(); ++i)
    {
        if(tablesInFile.at(i) == TableCaption(NASKTableType::SummaryTestResults))
        {
            numberTestResultsTable = i;
            testResultsStr = tablesHTML[i].getTable();
            break;
        }
    }
    return TestResultsDataInFile(testResultsStr, numberTestResultsTable);
}

PrintController::DocumentPrintParameters PrintController::countingSettingPages(QTextDocument& doc2, const QList<QString>& tablesInFile,
                                                                               const TestResultsDataInFile& testDataInFile, const QList<TableHTML>& tablesHTML)
{
    QList<QString> docList;
    QList<int> docCountPagesList;
    QList<QString> tableNameList;
    int sizeDocument = 0;

    QString htmlTestResults = testDataInFile.html;
    for(int i = 0; i < tablesInFile.size(); ++i)
    {
        doc2.setHtml(tablesHTML[i].getTable());

        int size = doc2.pageCount() - 1;//-1, тк у таблиц в head стоит тег breakAllways
        sizeDocument += size;

        for(int k = doc2.pageCount() - 1; k > 0; --k)
            tableNameList.push_back(tablesInFile.at(i));
        docList.push_back(tablesHTML[i].getTable());

        docCountPagesList.push_back(doc2.pageCount() - 1);

        //TODO вызов сделать единичным
        htmlTestResults = settingTestPageForDoc(sizeDocument, i, doc2, tablesInFile, htmlTestResults);
    }
    return DocumentPrintParameters(docList, docCountPagesList, tableNameList, sizeDocument, htmlTestResults);
}

QString PrintController::settingTestPageForDoc(int sizeDocument, int numTable, QTextDocument& doc2,
                                        const QList<QString>& tablesInFile, const QString& htmlTestResults)
{
    QString newTestData = htmlTestResults;
    QRegExp regExp("\\{%[0-9]+\\}");
    if(!TablesCaption().count(tablesInFile.at(numTable)))
    {
        regExp.indexIn(htmlTestResults);
        int pos = regExp.pos();
        if(pos > -1)
        {
            QString str = regExp.cap();
            if(sizeDocument + 2 - (doc2.pageCount()) != sizeDocument)
                newTestData.replace(pos, str.length(), QString::number(sizeDocument + 2 - (doc2.pageCount())) + "-" + QString::number(sizeDocument));
            else
                newTestData.replace(pos, str.length(), QString::number(sizeDocument));
        }
        else
            qDebug() << "Error regExp.indexIn(htmlNew); pos > -1";
    }
    return newTestData;
}

void PrintController::printDocument(QPrinter& printer, const QList<QString>& tablesInFile,
                                    const QList<TableHTML>& tablesHTML, const QString& fileNamePath)
{
    QPainter painter(&printer);
    QSizeF pageSize = printer.pageRect().size(); // page size in pixels
    const qreal footerHeight = painter.fontMetrics().height();
    const double tm = mmToPixels(printer, textMargins);
    const QRectF textRect(tm, tm, pageSize.width() - 2.5 * tm, pageSize.height() - 3 * tm - footerHeight * 3);

    if(tablesHTML.size() > tablesInFile.size())
    {
        qDebug() << "ОШИБКА, tablesHTML.size() > tablesInFile.size(): " << tablesHTML.size() << " " << tablesInFile.size();
        throw(std::length_error("ОШИБКА, tablesHTML.size() > tablesInFile.size()"));
    }

    TestResultsDataInFile testDataInFile = findTestResultsStrNum(tablesInFile, tablesHTML);

    QTextDocument doc2;
    doc2.setPageSize(textRect.size());

    PrintController::DocumentPrintParameters documentPrintPar = countingSettingPages(doc2, tablesInFile, testDataInFile, tablesHTML);

    if(testDataInFile.numberTablesInFile != -1)
        documentPrintPar.docList[testDataInFile.numberTablesInFile] = documentPrintPar.newTestHTML;

    bool firstPage = true;
    int k = 0;
    int firstVal = 0;
    QTextDocument doc3;
    doc3.setPageSize(textRect.size());
    if(documentPrintPar.docCountPagesList.size() > 0)
    {
        firstVal = documentPrintPar.docCountPagesList.at(0);
        doc3.setHtml(tablesHTML[0].getTable());
    }

    for (int pageIndex = 0; pageIndex < documentPrintPar.sizeDocument; ++pageIndex)
    {
        if (!firstPage)
            printer.newPage();

        if(documentPrintPar.docCountPagesList.at(k) == 0)
        {
            ++k;
            firstVal = documentPrintPar.docCountPagesList.at(k);
            doc3.setHtml(documentPrintPar.docList.at(k));
        }

        paintPage(pageIndex, &painter, &doc3, textRect, footerHeight, documentPrintPar.tableNameList.at(pageIndex), documentPrintPar.docCountPagesList.at(k), firstVal, fileNamePath);

        firstPage = false;
        --documentPrintPar.docCountPagesList[k];
    }
}

void PrintController::PrintHTMLToPdf(const QString& fileName, const QList<QString>& tablesInFile, const QList<TableHTML>& tablesHTML)
{
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPrinter::A4);
    printer.setOutputFileName(fileName);
    printer.setPageMargins(QMargins(0, 0, 0, 50));

    QRegExp regExp("(\\/[A-Za-z0-9А-Яа-я ,\\.&\\$#№@':;\\{\\}\\[\\]\\(\\)\\*\\^\\%\\_\\-\\=]+\\.pdf)");
    regExp.indexIn(fileName);
    int pos = regExp.pos();
    QString fileNamePath = "";
    if(pos > -1)
    {
        QString temp = regExp.cap();
        temp = temp.remove("/");
        temp = temp.remove(".pdf");
        fileNamePath = temp;
    }
    else
    {
        qDebug() << "Ошибка, имя файла не опознано";
        throw(std::logic_error("Ошибка, имя файла не опознано, if(pos > -1)"));
    }

    printDocument(printer, tablesInFile, tablesHTML, fileNamePath);
}


void PrintController::PrintHTMLToPdfFiles(QList<QString> htmlFileNameList, QList<QHash<QString, ScenarioData>> scenariosHashList, const QMap<int, QList<QString>>& tablesInFile)
{
    for(int i = 0; i < tablesHTMLInFiles.size(); ++i)//
    {
        if(stop)
        {
            return;
        }
        s_scenariosData = scenariosHashList.at(i);
        progress->nextPartProgress();
        PrintHTMLToPdf(htmlFileNameList.at(i), tablesInFile[i], tablesHTMLInFiles.at(i));
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
