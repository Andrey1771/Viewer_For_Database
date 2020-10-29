#include "mainwindow.h"
#include "protocolprinterheaderview.h"
#include "protocolprinteritemmodel.h"
#include "newmodel.h"
#include "protocolprinterheaderviewextended.h"

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
#include <QTimer>

#include <QtConcurrent/QtConcurrentRun>
#include <QSortFilterProxyModel>

const QStringList databaseTypes{"sqlite3"};

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    launchSetSettings();

    connect(ui->menuFile, &QMenu::triggered, this, &MainWindow::onFileMenuActionTriggered);

    QVariantHash c;
    c.insert("path_to_db", "...");
    c.insert("path_to_export", "");
    ICollection coll(c);
    CollectionFromJsonLoader loader("./results_table_view_settings.json");
    CollectionToJsonSaver saver("./results_table_view_settings.json");
    coll.saveCollection(&saver);
    coll.loadCollection(&loader);
}

MainWindow::~MainWindow()
{   
    if(newModel != nullptr && newModel->model != nullptr && newModel->model->database().databaseName() != "")
        savePathDb(this->newModel->model->database().databaseName());
    else
        savePathDb("...");

    QDir dir;
    dir.setPath(exportDataForSaver.pathFile);

    if(exportDataForSaver.pathFile != "" && dir.exists())
        saveExportPath();
    else
        saveExportPath("");

    saveData(c);

    if(newModel != nullptr && newModel->model != nullptr)
        delete newModel->model;
    if(newModel != nullptr)
        delete newModel;
    if(modelSup != nullptr)
        delete modelSup;
    delete ui;
}

void MainWindow::launchSetSettings()
{
    //variables
    numberTable = 0;

    //comboBoxs
    ui->DBType_comboBox->addItems(databaseTypes);

    //model
    ProtocolPrinterItemModel *model = new ProtocolPrinterItemModel;/// TODO Важно избавиться от него!!!
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
    qDebug() << "PathDb" << str;
    if(!createConnection(str))
    {
    }
    else
    {
        addDatabase();
        ui->groupBoxTable->setTitle("Table: " + newModel->model->tableName());
        //lineEdit
        ui->Directory_lineEdit->setText(newModel->model->database().databaseName());

    }

    //connects
    connect(ui->TableName_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setTable(int)));
    connect(ui->Directory_pushButton, &QPushButton::clicked, this, &MainWindow::launchWizard);
    connect(ui->Directory_lineEdit, SIGNAL(returnPressed()), this, SLOT(createConnection()));
    connect(this, &MainWindow::connectionDone, this, &MainWindow::addDatabase);
    connect(ui->Connect_pushButton, &QPushButton::clicked, this, &MainWindow::disconnect);
    connect(ui->pushButtonHideShowSup, &QPushButton::clicked, this, &MainWindow::hideShowSup);
    connect(ui->tableView_Sup, &ProtocolPrinterTableView::selectRow, filter_Sup, &ProtocolPrinterHeaderView::rowSelect);
    connect(ui->tableView_Sup, &ProtocolPrinterTableView::selectionSet, filter_Sup, &ProtocolPrinterHeaderView::updateFilter2);

    connect(ui->tableView->verticalScrollBar(), &QScrollBar::valueChanged, this, &MainWindow::sliderUpdateModel);
    connect(newModel, &NewModel::dataChanged, this, &MainWindow::updateModel);

    connect(filter, &QHeaderView::sectionClicked, this, &MainWindow::colomnNewModelFilter);
    connect(filter_Sup, &QHeaderView::sectionClicked, this, &MainWindow::colomnModelFilter);
    
    ///TODO FIX BUG
    //connect(newModel->model, &ProtocolPrinterItemModel::TableChanged, );
    ///TODO FIX BUG
    /// 
    exportDataForSaver.pathFile = loadPathExportDialog();
}

void MainWindow::ProxyNewModelLoad()
{
    proxyModelNewModel = new QSortFilterProxyModel(this);
    proxyModelNewModel->setSourceModel(newModel);
    proxyModelNewModel->setDynamicSortFilter(true);
    ui->tableView->setModel(proxyModelNewModel);
}
void MainWindow::ProxyModelSupLoad()
{
    proxyModelModelSup = new QSortFilterProxyModel(this);
    proxyModelModelSup->setSourceModel(modelSup);
    proxyModelModelSup->setDynamicSortFilter(true);
    ui->tableView_Sup->setModel(proxyModelModelSup);
}

void MainWindow::colomnNewModelFilter(int logicalIndex)
{
    newModel->magicAllLoad();
    qDebug() << "sortTypeNewModel " << sortTypeNewModelList << logicalIndex;
    if(sortTypeNewModelList.at(logicalIndex))
    {
        ui->tableView->sortByColumn(logicalIndex, Qt::SortOrder::AscendingOrder);
    }
    else
    {
        ui->tableView->sortByColumn(logicalIndex, Qt::SortOrder::DescendingOrder);
    }
    sortTypeNewModelList[logicalIndex] = !sortTypeNewModelList.at(logicalIndex);
}

void MainWindow::colomnModelFilter(int logicalIndex)
{
    modelSup->allRowLoad();
    if(sortTypeModelSupList.at(logicalIndex))
    {
        ui->tableView_Sup->sortByColumn(logicalIndex, Qt::SortOrder::AscendingOrder);
    }
    else
    {
        ui->tableView_Sup->sortByColumn(logicalIndex, Qt::SortOrder::DescendingOrder);
    }
    sortTypeModelSupList[logicalIndex] = !sortTypeModelSupList.at(logicalIndex);
}

void MainWindow::updateModel()
{
    ui->tableView->verticalScrollBar()->valueChanged(ui->tableView->verticalScrollBar()->value());
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

    ui->groupBoxTable->setTitle("Table: " + newModel->model->tableName());
    ui->Directory_lineEdit->setText(newModel->model->database().databaseName());

    newModel->setSignals();
    //TableView

    //TableView_Sup
    filter_Sup->clearPrimaryMemory();

    modelSup = new ProtocolPrinterItemModel(this);
    modelSup->setTable("Acceptance Test Reports");
    ui->tableView_Sup->setModel(modelSup);
    modelSup->select();

    //ui->tableView_Sup->hideColumn(0);// скрываем Session ID
    //for (int i = 8; i < modelSup->columnCount(); ++i)
    //    ui->tableView_Sup->hideColumn(i);

    ui->tableView_Sup->horizontalHeader()->resizeSections(QHeaderView::ResizeMode::Stretch);
    ui->tableView_Sup->resizeRowsToContents();

    filter_Sup->update(modelSup);
    filter_Sup->setSecondFilterDatabase(newModel->model, filter);
    //TableView_Sup

    loadSortModelsList();

    ProxyNewModelLoad();
    ProxyModelSupLoad();
}

void MainWindow::loadSortModelsList()
{
    sortTypeNewModelList.clear();
    sortTypeModelSupList.clear();
    for (int i = 0; i < newModel->model->getNamesColumns().size(); ++i)
    {
        sortTypeNewModelList.push_back(false);
    }
    for (int i = 0; i < modelSup->getNamesColumns().size(); ++i)
    {
        sortTypeModelSupList.push_back(false);
    }
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

void MainWindow::savePathDb(const QString& databaseName)
{
    c.insert("path_to_db", databaseName);
}

void MainWindow::saveExportPath(QString str)
{
    if(str == "")
        c.insert("path_to_export", exportDataForSaver.pathFile);
    else
        c.insert("path_to_export", str);
}

void MainWindow::saveData(const QVariantHash& c)
{
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
        ui->groupBoxTable->setTitle("Table: " + newModel->model->tableName());
    }
}

void MainWindow::hideShowSup()
{
    if(!ui->tableView_Sup->isHidden())
    {
        //ui->pushButtonHideShowSup->setText("Hide/Show Acceptance Tests Reports Table");
        ui->tableView_Sup->hide();
        ui->verticalLayoutViews->setStretch(0, 0);
    }
    else
    {
        //ui->pushButtonHideShowSup->setText("Hide/Show Acceptance Tests Reports Table");
        ui->tableView_Sup->show();
        ui->verticalLayoutViews->setStretch(0, 1);
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

    loadSortModelsList();
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

            delete proxyModelModelSup;
            delete proxyModelNewModel;
            proxyModelModelSup = nullptr;
            proxyModelNewModel = nullptr;

            db.close();
            db = QSqlDatabase();

            //db.removeDatabase(databaseName);
            //db.removeDatabase(connectionName);

            ui->TableName_comboBox->clear();
            Names.clear();
            ui->Connect_label->setText("Connected to \"\" database");
            ui->Directory_lineEdit->setText("");
            ui->groupBoxTable->setTitle("Table:");
            savePathDb(databaseName);

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
    QList<QString> namesTables = getNamesTables();
    qDebug() << "NAMES TABLES: " << namesTables;
    namesTables.removeOne("sqlite_sequence");/// TODO Перенести с глобальными переменными

    PrintDialog* printDialog = new PrintDialog(namesTables, modelSup, filter, db, exportDataForSaver.pathFile, this);
    printDialog->exec();
    PrintDialog::DataForSaver dataForSaver(printDialog->getPathForSaver());
    exportDataForSaver.pathFile = dataForSaver.pathFile;
    exportDataForSaver.pathSave = dataForSaver.pathSave;
    delete printDialog;
}

const QString MainWindow::loadPathExportDialog()
{
    QVariantHash c;
    ICollection coll(c);
    CollectionFromJsonLoader loader("results_table_view_settings.json");
    coll.loadCollection(&loader);
    QString str = coll.getCollection().value("path_to_export").toString();
    return str;
}


void MainWindow::onQuitAction()
{
    close();
}
