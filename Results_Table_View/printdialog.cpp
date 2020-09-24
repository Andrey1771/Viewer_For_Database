#include "printdialog.h"
#include "ui_printdialog.h"
#include "protocolprinteritemmodel.h"
#include "printcombobox.h"
#include "printdialogitemmodel.h"
#include "protocolprinterheaderview.h"
#include "XMLPrintSupport.h"
#include "protocolprinteritemmodel.h"
#include "xmlprintprogress.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QtConcurrent/QtConcurrentRun>
#include <QFutureWatcher>

PrintDialog::PrintDialog(const QList<QString>& namesTables, ProtocolPrinterItemModel* model, ProtocolPrinterHeaderView* header, QSqlDatabase& db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PrintDialog)
{
    ui->setupUi(this);

    printDialogItemModel = new PrintDialogItemModel(namesTables, model, header);

    ui->tableView_Table->setModel(printDialogItemModel);
    PrintComboBox* comboBox = new PrintComboBox(this);
    ui->tableView_Table->setItemDelegateForColumn(0, comboBox);

    PrintComboBox* comboBox2 = new PrintComboBox(this);
    ui->tableView_Table->setItemDelegateForColumn(1, comboBox2);

    ui->progressBar_Progress->hide();
    this->db = &db;


    ui->comboBox_ExportChanger->addItems(exportTypes);

    connect(ui->pushButton_AddRow, &QPushButton::clicked, this, &PrintDialog::onAddRow);
    connect(ui->pushButton_RemoveRow, &QPushButton::clicked, this, &PrintDialog::onRemoveRow);
    connect(ui->pushButton_Cancel, &QPushButton::clicked, this, &PrintDialog::onCancel);

    connect(ui->pushButton_Export, &QPushButton::clicked, this, &PrintDialog::onExport);
    connect(ui->pushButtonSetPath_Directory, &QPushButton::clicked, this, &PrintDialog::launchFilePathWizard);
}

PrintDialog::~PrintDialog()
{
    delete ui;
}

void PrintDialog::onAddRow()
{
    printDialogItemModel->insertRows(printDialogItemModel->rowCount(QModelIndex()), 1, QModelIndex());
}

void PrintDialog::onRemoveRow()
{
    printDialogItemModel->removeRows(printDialogItemModel->rowCount(QModelIndex())-1, 1, QModelIndex());
}
void PrintDialog::progressBarSetVal(float val)
{
    ui->progressBar_Progress->setValue(int (val));
}

void PrintDialog::finishedWork()
{
    ui->pushButton_Export->setEnabled(true);
    progress->getPrintSup()->close();
}

void PrintDialog::startWork()
{
    ui->pushButton_Export->setEnabled(false);
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
    progress = new XMLPrintProgress(this);
    ui->progressBar_Progress->setVisible(true);
    ui->progressBar_Progress->setValue(0);
    connect(progress, &XMLPrintProgress::progressChanged, this, &PrintDialog::progressBarSetVal);
    connect(&progress->watcher, &QFutureWatcher<void>::finished, this, &PrintDialog::finishedWork);
    startWork();
    switch(ui->comboBox_ExportChanger->currentIndex())
    {
    case(0)://"PDF + XML"
    {
        XMLPrintSupport::onExportDBAction(*db, "PDF + XML", dir.path(), progress, printDialogItemModel->getFiltersMemory());
        break;
    }
    case(1)://PDF
    {
        XMLPrintSupport::onExportDBAction(*db, "PDF", dir.path(), progress, printDialogItemModel->getFiltersMemory());
        break;
    }
    case(2)://XML
    {
        XMLPrintSupport::onExportDBAction(*db, "XML", dir.path(), progress, printDialogItemModel->getFiltersMemory());
        break;
    }
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
    QString strPath = QFileDialog::getExistingDirectory(nullptr, "Path to file save", QString());
    ui->lineEdit_Directory->setText(strPath);
}

void PrintDialog::closeEvent(QCloseEvent */*event*/)
{
    if(progress != nullptr)
    {
        progress->getPrintSup()->setStop(true);
    }
}
