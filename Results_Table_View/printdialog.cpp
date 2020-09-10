#include "printdialog.h"
#include "ui_printdialog.h"
#include "protocolprinteritemmodel.h"
#include "printcombobox.h"
#include "printdialogitemmodel.h"
#include "protocolprinterheaderview.h"
#include "XMLPrintSupport.h"
#include "protocolprinteritemmodel.h"

#include <QDebug>

PrintDialog::PrintDialog(const QList<QString>& namesTables, ProtocolPrinterItemModel* model, ProtocolPrinterHeaderView* header, QSqlDatabase& db, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PrintDialog)
{
    ui->setupUi(this);
    printDialogItemModel = new PrintDialogItemModel(namesTables, model, header);

    ui->tableView->setModel(printDialogItemModel);
    PrintComboBox* comboBox = new PrintComboBox(this);
    ui->tableView->setItemDelegateForColumn(0, comboBox);

    PrintComboBox* comboBox2 = new PrintComboBox(this);
    ui->tableView->setItemDelegateForColumn(1, comboBox2);

    this->db = &db;


    ui->comboBox_ExportChanger->addItems(exportTypes);

    connect(ui->pushButton_AddRow, &QPushButton::clicked, this, &PrintDialog::onAddRow);
    connect(ui->pushButton_RemoveRow, &QPushButton::clicked, this, &PrintDialog::onRemoveRow);
    connect(ui->pushButton_Cancel, &QPushButton::clicked, this, &PrintDialog::onCancel);
    connect(ui->pushButton_Export, &QPushButton::clicked, this, &PrintDialog::onExport);
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

void PrintDialog::onExport()
{

    //qDebug() << "Str filters " <<  ;
    switch(ui->comboBox_ExportChanger->currentIndex())
    {
    case(0)://PDF
    {
        XMLPrintSupport::onExportDBAction(*db, "PDF");

        break;
    }
    case(1)://XML
    {
        XMLPrintSupport::onExportDBAction(*db, "XML");
        break;
    }
    default:
    {
        qDebug() << "Такого экспорта пока нет";
        break;
    }
    }

    ProtocolPrinterItemModel* model = new ProtocolPrinterItemModel(this);

    delete model;
}

void PrintDialog::onCancel()
{
    close();
}

QString PrintDialog::getFiltersMemory()
{
    QString filtersMemory;
    QString lineEditText;
    int lineNumber;
    int count = 0;
    ProtocolPrinterItemModel* model = new ProtocolPrinterItemModel;
    ProtocolPrinterHeaderView::checksFilter(model, nullptr, lineEditText, filtersMemory, printDialogItemModel->getFiltersMemory(), lineNumber, count, 0);


}
