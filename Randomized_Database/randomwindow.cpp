#include "randomwindow.h"
#include "ui_randomwindow.h"
#include "randomizeddatabase.h"

RandomWindow::RandomWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RandomWindow)
{
    ui->setupUi(this);
    RandomizedDatabase::Random("TestDatabaseProtocolPrinterModel500Mk2"".""sqlite3", 5);// создает БД или дополняет существующую ранд элементами, записывается количество сотен
}

RandomWindow::~RandomWindow()
{
    delete ui;
}
