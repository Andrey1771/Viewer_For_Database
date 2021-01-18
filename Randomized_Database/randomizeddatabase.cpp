#include "randomizeddatabase.h"

#include <QtSql>

const int MaxLenghtWord = 40;//(realMax=Max-1)
const int RangeValue = 1000000;//[0:RangeValue-1]

RandomizedDatabase::RandomizedDatabase()
{
}

bool RandomizedDatabase::Random(const QString& nameDatabase, int ammountHundreds)
{
    srand(static_cast<unsigned int>(time(nullptr)));

    QSqlDatabase db = QSqlDatabase :: addDatabase("QSQLITE"); // В зависимости от типа базы данных нужно будет дополнить и изменить
    db.setDatabaseName(nameDatabase);

    if(!db.open())
    {
        QMessageBox::critical(nullptr, QObject::tr("Cannot open database"),
            QObject::tr("Unable to establish a database connection.\n"
                        "Click Cancel to exit."), QMessageBox::Cancel);
        return false;
    }


    db.exec("CREATE TABLE \"Visual Inspections \" (\"Session ID \" INTEGER NOT NULL,\"List ID \" TEXT NOT NULL,\"Name \" TEXT NOT NULL,\"Requirements \" TEXT NOT NULL,\"Test Status \" TEXT NOT NULL)");
    db.exec("CREATE TABLE \"Suspect lists of SRUs \" (\"Session ID \" INTEGER NOT NULL,\"List ID \" TEXT NOT NULL,\"Name \" TEXT NOT NULL,\"S/N \" TEXT NOT NULL,\"Suspected \" TEXT NOT NULL,\"Description \" TEXT NOT NULL)");
    db.exec("CREATE TABLE \"Acceptance Test Reports\" (\"Session ID\" INTEGER PRIMARY KEY AUTOINCREMENT,\"Project Name\" TEXT NOT NULL,\"LRU Name\" TEXT NOT NULL,\"LRU S/N\" TEXT NOT NULL,\"ATE P/N\" TEXT NOT NULL,\"ATE S/N\" TEXT NOT NULL,\"ATE S/W Ver\" TEXT NOT NULL,\"Test Status\" TEXT NOT NULL,\"Run Mode\" TEXT NOT NULL,\"Session Run Date&Time\" TEXT NOT NULL,\"Session Run Total Time (hh:mm:ss)\" TEXT NOT NULL,\"Environment\" TEXT NOT NULL,\"TPS Checksum\" TEXT NOT NULL,\"Operator's Name\" TEXT NOT NULL)");
    db.exec("CREATE TABLE \"Scenario Test Results \" (\"Session ID \" INTEGER NOT NULL, \"Product name \" TEXT NOT NULL, \"Scenario name \" TEXT NOT NULL,\"Caption \" TEXT NOT NULL, \"Test name \" TEXT NOT NULL, \"Low \" TEXT NOT NULL,\"Result \" TEXT NOT NULL, \"High \" TEXT NOT NULL, \"Units \" TEXT NOT NULL, \"Status \" TEXT NOT NULL)");
    QSqlQuery query;
    for (int i=0; i<ammountHundreds; ++i)// можно было сделать через prepare, тогда строки снизу были бы понятнее
    {
        sendRandReq(1,50,&query);//
        sendRandReq(1,50,&query);//
        sendRandReq(2,50,&query);//
        sendRandReq(2,50,&query);//
        sendRandReq(3,50,&query);//Не вмещается в строку queryString
        sendRandReq(3,50,&query);//
        sendRandReq(4,50,&query);//
        sendRandReq(4,50,&query);//
    }
    return true;
}

void RandomizedDatabase::sendRandReq(int type, int size, QSqlQuery *query)
{

    switch (type)
    {
    case(1):
    {
        //Запросы отправляются блоками по size элементов!
        QString queryString3 = "INSERT INTO `Visual Inspections` "
                              "VALUES(?,?,?,?,?),";
        queryString3.append(QString("(?,?,?,?,?),").repeated((size-2)));
        queryString3.append(QString("(?,?,?,?,?);"));
        query->prepare(queryString3);
        //
        for (int y=0; y<size; ++y)
        {
            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randWord())+"");
        }
        query->exec();
        return;
    }
    case(2):
    {
        //Запросы отправляются блоками по size элементов!
        QString queryString3 = "INSERT INTO `Suspect lists of SRUs` "
                              "VALUES(?,?,?,?,?,?),";
        queryString3.append(QString("(?,?,?,?,?,?),").repeated((size-2)));
        queryString3.append(QString("(?,?,?,?,?,?);"));
        query->prepare(queryString3);
        //
        for (int y=0; y<size; ++y)
        {
            query->addBindValue(QString(""+randWord())+"");

            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randWord())+"");
        }
        query->exec();
        return;
    }
    case(3):
    {
        //Запросы отправляются блоками по size элементов!
        QString queryString3 = "INSERT INTO `Acceptance Test Reports` "
                              "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?),";
        queryString3.append(QString("(?,?,?,?,?,?,?,?,?,?,?,?,?,?),").repeated((size-2)));
        queryString3.append(QString("(?,?,?,?,?,?,?,?,?,?,?,?,?,?);"));
        query->prepare(queryString3);
        //
        for (int y=0; y<size; ++y)
        {
            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randDateTime()+""));
            query->addBindValue(QString(""+randTime()+""));
            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randWord())+"");
        }
        query->exec();
        return;
    }
    case(4):
    {
        //Запросы отправляются блоками по size элементов!
        QString queryString4 = "INSERT INTO `Scenario Test Results` "
                              "VALUES(?,?,?,?,?,?,?,?,?,?),";
        queryString4.append(QString("(?,?,?,?,?,?,?,?,?,?),").repeated((size-2)));
        queryString4.append(QString("(?,?,?,?,?,?,?,?,?,?);"));
        query->prepare(queryString4);
        //
        for (int y=0; y<size; ++y)
        {

            query->addBindValue(QString(""+randWord())+"");

            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randWord())+"");

            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randWord())+"");
            query->addBindValue(QString(""+randWord())+"");
        }
        query->exec();
        return;
    }
    default:
        return;
    }
}

QString RandomizedDatabase::randWord()
{
    int lenght = rand()%MaxLenghtWord;
    QString word;
    for (int i=0; i<lenght; ++i)
    {
        word+=QString(char(rand()%75 + 48));// [48:122]
    }
    return word;
}

int RandomizedDatabase::randValue()
{
    return rand()%RangeValue;
}

QString RandomizedDatabase::randDateTime()
{   //28 - чтобы не было ошибок в дате
    return (QString::number(rand()%28 + 1)+"/"+QString::number(rand()%12 + 1)+"/"+QString::number(rand()%10000)+" "+QString::number(rand()%24)+":"+QString::number(rand()%60)+":"+QString::number(rand()%60));
}

QString RandomizedDatabase::randTime()
{
    return (QString::number(rand()%24)+":"+QString::number(rand()%60)+":"+QString::number(rand()%60));
}
