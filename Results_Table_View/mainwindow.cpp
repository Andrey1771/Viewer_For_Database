#include "mainwindow.h"
#include "protocolprinterheaderview.h"
#include "protocolprinteritemmodel.h"
#include "newmodel.h"
#include "protocolprinterheaderviewextended.h"
#include "printdialog.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QComboBox>
#include <QFileDialog>

#include <QtSql/QSqlError>
#include <QtSql/QSqlQueryModel>
#include <QtSql/QSqlTableModel>
#include <QtSql/QSqlQuery>
#include <ICollection.h>
#include <CollectionFromJsonLoader.h>
#include <CollectionToJsonSaver.h>

#include <QDebug>
//Test
//#include "randomizeddatabase.h"

#include <QSortFilterProxyModel>
#include <QTextDocument>
#include <QDateTime>
#include <QAction>
#include <QScrollBar>

#include <QtConcurrent/QtConcurrentRun>

const QStringList databaseTypes{"sqlite3"};

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    launchSetSettings();

    connect(ui->menuFile, &QMenu::triggered, this, &MainWindow::onFileMenuActionTriggered);

    QVariantHash c;
    c.insert("path_to_db", "...");
    ICollection coll(c);
    CollectionFromJsonLoader loader("./results_table_view_settings.json");
    CollectionToJsonSaver saver("./results_table_view_settings.json");
    coll.saveCollection(&saver);
    coll.loadCollection(&loader);

}

MainWindow::~MainWindow()
{
    if(this->newModel->model->database().databaseName() != "")
        saveDb(this->newModel->model->database().databaseName());
    delete newModel->model;
    delete newModel;
    delete ui;
}

void MainWindow::launchSetSettings()
{
    //variables
    numberTable = 0;

    //log
    //ui->plainTextEdit->hide();
    //ui->log_Label->setText("");

    //comboBoxs
    ui->DBType_comboBox->addItems(databaseTypes);

    //model
    ProtocolPrinterItemModel *model = new ProtocolPrinterItemModel;
    modelSup = new ProtocolPrinterItemModel;
    newModel = new NewModel(model);

    //view

    //TableView
    ui->tableView->setModel(newModel);
    filter = new ProtocolPrinterHeaderViewExtended(ui->tableView);
    hideColumnsModels();
    ui->tableView->horizontalHeader()->resizeSections(QHeaderView::ResizeMode::Stretch);
    ui->tableView->resizeRowsToContents();
    ui->tableView->setHorizontalHeader(filter);
    newModel->setProtocolFilter(filter);


    //ScrollBar
    QScrollBar *scrollBar = new QScrollBar(Qt::Orientation::Vertical, ui->tableView);
    ui->tableView->setVerticalScrollBar(scrollBar);
    //ScrollBar
    //TableView

    //TableView_Sup
    ui->tableView_Sup->setModel(modelSup);
    filter_Sup = new ProtocolPrinterHeaderView(ui->tableView_Sup);
    filter_Sup->setSecondFilterDatabase(newModel->model, filter);

    ui->tableView_Sup->horizontalHeader()->resizeSections(QHeaderView::ResizeMode::Stretch);
    ui->tableView_Sup->resizeRowsToContents();
    // do it
    QTimer::singleShot(0, this, [=](){ setTable(0); });
    // do it
    ui->tableView_Sup->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tableView_Sup->setHorizontalHeader(filter_Sup);
    ui->tableView_Sup->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_Sup->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //TableView_Sup

    //labels
    ui->Connect_label->setText("Connected to \"" + newModel->model->database().databaseName().remove("."+ui->DBType_comboBox->currentText()) + "\" database");

    //connect database
    QString str = loadDb();
    qDebug() << str;
    if(!createConnection(str))
    {
        QMessageBox::warning(nullptr, QObject::tr("Cannot open database"),
                             QObject::tr("Unable to establish a database connection.\n"
                                         "Click Cancel to exit."), QMessageBox::Cancel);

    }
    else
    {
        addDatabase();
        ///
        ui->groupBoxTable->setTitle("Table " + newModel->model->tableName());///TODO Bug - краш при неудачном запуске из коллекции
        ///
        //lineEdit
        ui->Directory_lineEdit->setText(newModel->model->database().databaseName());
    }


    //connects
    //connect(ui->pushButtonHideShowLog, &QPushButton::clicked, this, &MainWindow::hideShowLog);
    connect(ui->TableName_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setTable(int)));
    connect(ui->Directory_pushButton, &QPushButton::clicked, this, &MainWindow::launchWizard);
    connect(ui->Directory_lineEdit, SIGNAL(returnPressed()), this, SLOT(createConnection()));
    connect(this, &MainWindow::connectionDone, this, &MainWindow::addDatabase);
    connect(ui->Connect_pushButton, &QPushButton::clicked, this, &MainWindow::disconnect);
    connect(ui->pushButtonHideShowSup, &QPushButton::clicked, this, &MainWindow::hideShowSup);
    connect(ui->tableView_Sup, &ProtocolPrinterTableView::selectRow, filter_Sup, &ProtocolPrinterHeaderView::rowSelect);
    connect(ui->tableView_Sup, &ProtocolPrinterTableView::selectionSet, filter_Sup, &ProtocolPrinterHeaderView::updateFilter2);

    connect(ui->tableView->verticalScrollBar(), &QScrollBar::valueChanged, this, &MainWindow::sliderUpdateModel);
    //Logger

}

QList<QString> MainWindow::getNamesTables()
{
    QList<QString> list;
    QSqlQuery query;
    query.exec("select * from sqlite_master where type = 'table';");
    while (query.next())
    {
        list << query.value(1).toString();
    }
    return list;
}

void MainWindow::fillingComboBoxTableName()
{
    ui->TableName_comboBox->clear();
    Names.clear();

    Names = getNamesTables();
    Names.removeOne("sqlite_sequence");
    Names.removeOne("Acceptance Test Reports");

    ui->TableName_comboBox->addItems(Names);
    newModel->setTable(Names[0]);
    newModel->select();
}

void MainWindow::addDatabase()
{
    fillingComboBoxTableName();
    //TableView
    ui->TableName_comboBox->setCurrentText(Names[numberTable]);
    if(newModel->model->database().databaseName().lastIndexOf("/") != -1)
        ui->Connect_label->setText("Connected to \"" +  (newModel->model->database().databaseName().remove(0, newModel->model->database().databaseName().lastIndexOf("/")+1)).remove("."+ui->DBType_comboBox->currentText()) + "\" database");
    else
        ui->Connect_label->setText("Connected to \"" + newModel->model->database().databaseName().remove("."+ui->DBType_comboBox->currentText()) + "\" database");
    ui->tableView->setModel(newModel);

    hideColumnsModels();

    ui->tableView->horizontalHeader()->resizeSections(QHeaderView::ResizeMode::Stretch);
    ui->tableView->resizeRowsToContents();

    filter->clearPrimaryMemory();
    filter->update(newModel->model);

    ui->groupBoxTable->setTitle("Table " + newModel->model->tableName());
    ui->Directory_lineEdit->setText(newModel->model->database().databaseName());

    newModel->setSignals();
    //TableView

    //TableView_Sup
    filter_Sup->clearPrimaryMemory();

    modelSup = new ProtocolPrinterItemModel(this);
    modelSup->setTable("Acceptance Test Reports");
    ui->tableView_Sup->setModel(modelSup);
    modelSup->select();

    ui->tableView_Sup->hideColumn(0);// скрываем Session ID
    //for (int i = 8; i < modelSup->columnCount(); ++i)
    //    ui->tableView_Sup->hideColumn(i);

    ui->tableView_Sup->horizontalHeader()->resizeSections(QHeaderView::ResizeMode::Stretch);
    ui->tableView_Sup->resizeRowsToContents();

    filter_Sup->update(modelSup);
    filter_Sup->setSecondFilterDatabase(newModel->model, filter);
    //TableView_Sup
}

void MainWindow::hideColumnsModels()
{

    QStringList  list2 = newModel->getNamesColumns();
    for (int i = 0; i < list2.size(); ++i)
    {
        foreach (QString badName, hidenColNewModelList)
        {
            if(badName == list2[i])
                ui->tableView->hideColumn(i);
        }
    }

    list2 = modelSup->getNamesColumns();
    for (int i = 0; i < list2.size(); ++i)
    {
        foreach (QString badName, hidenColModelSupList)
        {
            if(badName == list2[i])
                ui->tableView_Sup->hideColumn(i);
        }
    }
}

void MainWindow::saveDb(const QString& databaseName)
{
    QVariantHash c;
    c.insert("path_to_db", databaseName);
    ICollection coll(c);
    CollectionToJsonSaver saver("results_table_view_settings.json");//./results_table_view_settings.json
    coll.saveCollection(&saver);
}

const QString MainWindow::loadDb()
{
    QVariantHash c;
    ICollection coll(c);
    CollectionFromJsonLoader loader("results_table_view_settings.json");
    coll.loadCollection(&loader);
    QString str = coll.getCollection().value("path_to_db").toString();
    return str;
    //return "";
}

void MainWindow::updateScrollBar(int count)
{
    ui->tableView->verticalScrollBar()->setMaximum(ui->tableView->verticalScrollBar()->maximum() + count);
}

void MainWindow::sliderUpdateModel(int action)
{
    newModel->magicLoad(action);
}

void MainWindow::setTable(int index)
{
    if(index != -1 && Names.size() != 0)// newModel->model != nullptr можно поменять
    {
        newModel->setTable(Names[index]);
        newModel->select();

        hideColumnsModels();

        ui->tableView->resizeColumnsToContents();
        ui->tableView->horizontalHeader()->resizeSections(QHeaderView::ResizeMode::Stretch);
        ui->tableView->resizeRowsToContents();

        ui->tableView_Sup->resizeColumnsToContents();
        ui->tableView_Sup->horizontalHeader()->resizeSections(QHeaderView::ResizeMode::Stretch);
        ui->tableView_Sup->resizeRowsToContents();

        filter->generateFilters(newModel->model->columnCount(), true);

        filter_Sup->update(modelSup);
        filter->update(newModel->model);

        ui->TableName_comboBox->setCurrentText(Names[index]);
        ui->groupBoxTable->setTitle("Table " + newModel->model->tableName());
    }
}

//void MainWindow::hideShowLog()
//{

////    if(!ui->plainTextEdit->isHidden())
////    {
////        ui->pushButtonHideShowLog->setText("Show log");
////        ui->plainTextEdit->hide();
////        ui->log_Label->setText("");
////    }
////    else
////    {
////        ui->pushButtonHideShowLog->setText("Hide log");
////        ui->plainTextEdit->show();
////        ui->log_Label->setText("");
////    }
//}

void MainWindow::hideShowSup()
{
    if(!ui->tableView_Sup->isHidden())
    {
        ui->pushButtonHideShowSup->setText("Show sup");
        ui->tableView_Sup->hide();
    }
    else
    {
        ui->pushButtonHideShowSup->setText("Hide sup");
        ui->tableView_Sup->show();
    }
}

void MainWindow::launchWizard()
{
    QString strFileName = QFileDialog::getOpenFileName(nullptr, "Open database", QString(), "*." + ui->DBType_comboBox->currentText());

    if(!createConnection(strFileName))
    {
        QMessageBox::critical(nullptr, QObject::tr("Cannot open database"),
                              QObject::tr("Unable to establish a database connection.\n"
                                          "Click Cancel to exit."), QMessageBox::Cancel);
        return;// ошибка открытия БД
    }
    // Вызов make в случае done(true)
}

bool MainWindow::dbIsOpen(QSqlDatabase& db)
{
    if(!db.open())
    {
        QMessageBox::critical(nullptr, QObject::tr("Cannot open database"),
                              QObject::tr("Unable to establish a database connection.\n"
                                          "Click Cancel to exit."), QMessageBox::Cancel);
        return false;
    }
    return true;
}

bool MainWindow::createConnection(const QString& fileName)
{
    QString str = fileName;
    str.replace("\\", "/");
    if(!QFile(fileName).exists()) return false;// Ошибка открытия БД

    disconnect();
    db = QSqlDatabase :: addDatabase("QSQLITE"); // В зависимости от типа базы данных нужно будет дополнить и изменить
    db.setDatabaseName(fileName);

    if(!dbIsOpen(db))
        return false;
    newModel = new NewModel(new ProtocolPrinterItemModel, this);
    newModel->filter = filter;
    modelSup = new ProtocolPrinterItemModel(this);
    emit connectionDone();
    return true;
}

bool MainWindow::createConnection() // Для сигнала LineEdit
{
    QString str = ui->Directory_lineEdit->text().replace("\\", "/");
    return createConnection(str);
}

void MainWindow::disconnect()
{
    if(newModel != nullptr)
        if(newModel->model != nullptr )
        {
            QString databaseName = newModel->model->database().databaseName();
            QString connectionName = db.connectionName();

            delete newModel->model;//Работает неправильно
            delete newModel;
            delete modelSup;
            newModel->model = nullptr;
            newModel = nullptr;
            modelSup = nullptr;

            db.close();
            db = QSqlDatabase();

            //db.removeDatabase(databaseName);
            //db.removeDatabase(connectionName);

            ui->TableName_comboBox->clear();
            Names.clear();
            ui->Connect_label->setText("Connected to \"\" database");
            ui->Directory_lineEdit->setText("");
            ui->groupBoxTable->setTitle("Table");
            saveDb(databaseName);
        }
}

void MainWindow::onFileMenuActionTriggered(QAction *action)
{
    qDebug() << "action triggered: " << action->text();

    /// TODO: make actions property

    if (action->text() == tr("Open DB"))
    {
        onOpenDBAction();
    }
    else if (action->text() == tr("Export Report from DB"))
    {
        onExportDBAction();
    }
    else if (action->text() == tr("Quit"))
    {
        onQuitAction();
    }
}

void MainWindow::onOpenDBAction()
{
    launchWizard();
}

void MainWindow::onExportDBAction()
{
    if(!dbIsOpen(db))
        return;
    if(newModel == nullptr || modelSup == nullptr || filter == nullptr || !db.isOpen())
    {
        qDebug() << "onExportDbAction error";
        return;
    }

    PrintDialog* printDialog = new PrintDialog(getNamesTables(), modelSup, filter, db, this);
    printDialog->exec();

}

void MainWindow::onQuitAction()
{
    close();
}
