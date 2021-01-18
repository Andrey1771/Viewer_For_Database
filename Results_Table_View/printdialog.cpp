#include "printdialog.h"
#include "ui_printdialog.h"

#include "protocolprinteritemmodel.h"
#include "printcombobox.h"
#include "printdialogitemmodel.h"
#include "protocolprinterheaderview.h"
#include "printcontroller.h"
#include "protocolprinteritemmodel.h"
#include "protocolprintertableview.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrentRun>
#include <QFutureWatcher>

#include <ICollection.h>
#include <CollectionFromJsonLoader.h>
#include <CollectionToJsonSaver.h>

#include <QDebug>

PrintDialog::PrintDialog(QList<QString> namesTables, ProtocolPrinterItemModel* model,
                         ProtocolPrinterHeaderView* header, QSqlDatabase& db, const QString& pathFile, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PrintDialog)
{
    ui->setupUi(this);

    QPixmap map("://new//prefix1//databaseExport.png");
    this->setWindowIcon(QIcon(map));

    printDialogItemModel = new PrintDialogItemModel(namesTables, model, header);

    ui->tableView_Table->setModel(printDialogItemModel);
    PrintComboBox* comboBox = new PrintComboBox(this);
    ui->tableView_Table->setItemDelegateForColumn(0, comboBox);

    PrintComboBox* comboBox2 = new PrintComboBox(this);
    ui->tableView_Table->setItemDelegateForColumn(1, comboBox2);

    ui->progressBar_Progress->hide();
    ui->label_TypePrintFileProgress->hide();
    this->db = &db;

    ui->comboBox_ExportChanger->addItems(exportTypes);

    ui->tableView_Table->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(ui->pushButton_AddRow, &QPushButton::clicked, this, &PrintDialog::onAddRow);
    connect(ui->pushButton_RemoveRow, &QPushButton::clicked, this, &PrintDialog::onRemoveRow);
    connect(ui->pushButton_Cancel, &QPushButton::clicked, this, &PrintDialog::onCancel);

    connect(ui->pushButton_Export, &QPushButton::clicked, this, &PrintDialog::onExport);
    connect(ui->pushButtonSetPath_Directory, &QPushButton::clicked, this, &PrintDialog::launchFilePathWizard);

    lastPath = pathFile;
    ui->lineEdit_Directory->setText(lastPath);
}

PrintDialog::~PrintDialog()
{
    qDebug() << "lastPath " << lastPath;
    if(progress != nullptr)
        progress->watcher.waitForFinished();
    delete ui;
}

PrintDialog::DataForSaver PrintDialog::getPathForSaver()
{
    return DataForSaver("path_to_export", lastPath);
}

void PrintDialog::onAddRow()
{
    printDialogItemModel->insertRows(printDialogItemModel->rowCount(QModelIndex()), 1, QModelIndex());
}

void PrintDialog::onRemoveRow()
{
    QModelIndexList modelIndexList = ui->tableView_Table->getSelectedIndex();
    qDebug() << "QModelInd " << ui->tableView_Table->getSelectedIndex();
    if(!modelIndexList.isEmpty())
        printDialogItemModel->removeRows(modelIndexList.first().row(), modelIndexList.size() / 3 /*делим на 3, тк, возможно, делегаты дублируют выделенные строки или колонки, TODO FIX*/, QModelIndex());
}
void PrintDialog::progressBarSetVal(float val)
{
    ui->progressBar_Progress->setValue(int (val));
}

void PrintDialog::finishedWork()
{
    ui->pushButton_Export->setEnabled(true);
    ui->progressBar_Progress->setVisible(false);
    ui->label_TypePrintFileProgress->setVisible(false);
}

void PrintDialog::startWork()
{
    ui->pushButton_Export->setEnabled(false);
    ui->progressBar_Progress->setVisible(true);
    ui->label_TypePrintFileProgress->setVisible(true);
}

void PrintDialog::dialogWarning(int countFiles)
{
    if(!countFiles)
    {
        QMessageBox::warning(nullptr, QObject::tr("Warning"),
                             QObject::tr("The list of files to print is empty\n"
                                         "Click Cancel to exit."), QMessageBox::Cancel);
        qDebug() << "Файлов нет!";
        return;
    }
    qDebug() << "Количество файлов: " << countFiles;
}

void PrintDialog::messageFilePrinted(int type)
{
    switch (TypePrint(type))
    {
    case (TypePrint::XML):
    {
        QMessageBox::warning(nullptr, QObject::tr("Warning"),
                             QObject::tr("Excel printing completed\n"
                                         "Click Cancel to exit."), QMessageBox::Cancel);
        break;
    }
    case (TypePrint::PDF):
    {
        QMessageBox::warning(nullptr, QObject::tr("Warning"),
                             QObject::tr("PDF printing completed.\n"
                                         "Click Cancel to exit."), QMessageBox::Cancel);
        break;
    }
    case (TypePrint::XMLPDF):
    {
        QMessageBox::warning(nullptr, QObject::tr("Warning"),
                             QObject::tr("Excel and PDF printing completed.\n"
                                         "Click Cancel to exit."), QMessageBox::Cancel);
        break;
    }
    }

}

void PrintDialog::setLabelTypePrintFilePDF()
{
    ui->label_TypePrintFileProgress->setText("PDF is printed");
}

void PrintDialog::onExport()
{
    QDir dir;
    dir.setPath(ui->lineEdit_Directory->text());
    qDebug() << dir.path();
    if (!dir.exists())
    {
        if(!dir.mkpath(dir.path()))
        {
            QMessageBox::critical(this, QObject::tr("Error path not created"), QObject::tr("Cannot create the directory"), QMessageBox::Cancel);
            return;
        }
    }
    lastPath = dir.path();
    progress = new PrintProgress(this);
    ui->progressBar_Progress->setVisible(true);
    ui->progressBar_Progress->setValue(0);
    connect(progress, &PrintProgress::progressChanged, this, &PrintDialog::progressBarSetVal);
    connect(progress, &PrintProgress::countedFiles, this, &PrintDialog::dialogWarning);
    connect(progress, &PrintProgress::filePrinted, this, &PrintDialog::messageFilePrinted);
    connect(progress, &PrintProgress::typePrintFileChanged, this, &PrintDialog::setLabelTypePrintFilePDF);
    connect(&progress->watcher, &QFutureWatcher<void>::finished, this, &PrintDialog::finishedWork);


    startWork();
    switch(ui->comboBox_ExportChanger->currentIndex())
    {
    case(0)://"PDF + XML"//Excel
    {
        qDebug() << "FiltersMemory: " << printDialogItemModel->getFiltersMemory();
        progress->setTypePrint(TypePrint::XMLPDF);
        ui->label_TypePrintFileProgress->setText("Excel is printed");
        PrintController::onExportDBAction(*db, "PDF + XML", dir.path(), progress, printDialogItemModel->getFiltersMemory());
        break;
    }
    case(1)://PDF
    {
        progress->setTypePrint(TypePrint::PDF);
        ui->label_TypePrintFileProgress->setText("PDF is printed");
        PrintController::onExportDBAction(*db, "PDF", dir.path(), progress, printDialogItemModel->getFiltersMemory());
        break;
    }
    case(2)://XML
    {
        progress->setTypePrint(TypePrint::XML);
        ui->label_TypePrintFileProgress->setText("Excel is printed");
        PrintController::onExportDBAction(*db, "XML", dir.path(), progress, printDialogItemModel->getFiltersMemory());
        break;
    }//11/1/2000 9:11:04 - 05/2/2033 12:11:26 9:11:04
    default:
    {
        qDebug() << "Такого экспорта пока нет";
        break;
    }
    }
}

void PrintDialog::onCancel()
{
    close();
}

void PrintDialog::launchFilePathWizard()
{
    QString strPath = QFileDialog::getExistingDirectory(nullptr, QObject::tr("Path to file save"), QString());
    ui->lineEdit_Directory->setText(strPath);
}

void PrintDialog::closeEvent(QCloseEvent */*event*/)
{
    if(progress != nullptr)
    {
        progress->getPrintSup()->setStop(true);
        progress->getPrintSup()->close();
    }
    QString lineStr = ui->lineEdit_Directory->text();
    if(lineStr != "")
        lastPath = lineStr;
}


