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

    db.exec("CREATE TABLE \"Visual Inspections\" (\r\n\t\"Project name\"\tTEXT NOT NULL,\r\n\t\"LRU Name\"\tTEXT NOT NULL,\r\n\t\"LRU S/N\"\tTEXT NOT NULL,\r\n\t\"ATE P/N\"\tTEXT NOT NULL,\r\n\t\"ATE S/N\"\tTEXT NOT NULL,\r\n\t\"ATE S/W Ver\"\tTEXT NOT NULL,\r\n\t\"Test Status\"\tTEXT NOT NULL,\r\n\t\"Scenario tests run time\"\tTEXT NOT NULL,\r\n\t\"ID\"\tINTEGER NOT NULL,\r\n\t\"Name\"\tTEXT NOT NULL,\r\n\t\"Requirements\"\tTEXT NOT NULL\r\n)");
    db.exec("CREATE TABLE \"Suspect lists of SRUs\" (\r\n\t\"Project name\"\tTEXT NOT NULL,\r\n\t\"LRU Name\"\tTEXT NOT NULL,\r\n\t\"LRU S/N\"\tTEXT NOT NULL,\r\n\t\"ATE P/N\"\tTEXT NOT NULL,\r\n\t\"ATE S/N\"\tTEXT NOT NULL,\r\n\t\"ATE S/W Ver\"\tTEXT NOT NULL,\r\n\t\"Test Status\"\tTEXT NOT NULL,\r\n\t\"Scenario tests run time\"\tTEXT NOT NULL,\r\n\t\"ID\"\tINTEGER NOT NULL,\r\n\t\"Name\"\tTEXT NOT NULL,\r\n\t\"S/N\"\tTEXT,\r\n\t\"Suspected\"\tTEXT,\r\n\t\"Description\"\tTEXT\r\n)");
    db.exec("CREATE TABLE \"Acceptance Test Reports\" (\r\n\t\"Project Name\"\tTEXT NOT NULL,\r\n\t\"LRU Name\"\tTEXT NOT NULL,\r\n\t\"LRU S/N\"\tTEXT NOT NULL,\r\n\t\"ATE P/N\"\tTEXT NOT NULL,\r\n\t\"ATE S/N\"\tTEXT NOT NULL,\r\n\t\"ATE S/W Ver\"\tTEXT NOT NULL,\r\n\t\"Test Status\"\tTEXT NOT NULL,\r\n\t\"Run Mode\"\tTEXT NOT NULL,\r\n\t\"Date & Time\"\tTEXT NOT NULL,\r\n\t\"Total Time (hh:mm:ss)\"\tTEXT NOT NULL,\r\n\t\"Environment\"\tTEXT NOT NULL,\r\n\t\"TPS Checksum\"\tTEXT NOT NULL,\r\n\t\"Operator's Name\"\tTEXT NOT NULL\r\n)");
    db.exec("CREATE TABLE \"Scenario Test Results\" (\r\n\t\"Project name\"\tTEXT NOT NULL,\r\n\t\"LRU Name\"\tTEXT NOT NULL,\r\n\t\"LRU S/N\"\tTEXT NOT NULL,\r\n\t\"ATE P/N\"\tTEXT NOT NULL,\r\n\t\"ATE S/N\"\tTEXT NOT NULL,\r\n\t\"ATE S/W Ver\"\tTEXT NOT NULL,\r\n\t\"Test Status\"\tTEXT NOT NULL,\r\n\t\"Scenario name\"\tTEXT NOT NULL,\r\n\t\"Scenario run time\"\tTEXT NOT NULL,\r\n\t\"Test name\"\tTEXT NOT NULL,\r\n\t\"Low\"\tTEXT NOT NULL,\r\n\t\"Result\"\tTEXT NOT NULL,\r\n\t\"High\"\tTEXT NOT NULL,\r\n\t\"Units\"\tTEXT NOT NULL,\r\n\t\"Status\"\tTEXT NOT NULL\r\n)");
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
                              "VALUES(?,?,?,?,?,?,?,?,?,?,?),";
        queryString3.append(QString("(?,?,?,?,?,?,?,?,?,?,?),").repeated((size-2)));
        queryString3.append(QString("(?,?,?,?,?,?,?,?,?,?,?);"));
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

            query->addBindValue(QString(""+randDateTime())+"");
            query->addBindValue(QString(""+QString::number(randValue()))+"");
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
                              "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?),";
        queryString3.append(QString("(?,?,?,?,?,?,?,?,?,?,?,?,?),").repeated((size-2)));
        queryString3.append(QString("(?,?,?,?,?,?,?,?,?,?,?,?,?);"));
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

            query->addBindValue(QString(""+randDateTime())+"");
            query->addBindValue(QString(""+QString::number(randValue()))+"");
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
                              "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?),";
        queryString3.append(QString("(?,?,?,?,?,?,?,?,?,?,?,?,?),").repeated((size-2)));
        queryString3.append(QString("(?,?,?,?,?,?,?,?,?,?,?,?,?);"));
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
                              "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?),";
        queryString4.append(QString("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?),").repeated((size-2)));
        queryString4.append(QString("(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);"));
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
            query->addBindValue(QString(""+randDateTime())+"");
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
