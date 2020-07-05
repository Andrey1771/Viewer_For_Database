#ifndef RANDOMIZEDDATABASE_H
#define RANDOMIZEDDATABASE_H

#include <QMessageBox>

class QSqlQuery;

class RandomizedDatabase
{
public:
    RandomizedDatabase();
    static bool Random(const QString &nameDatabase = "databaseName", int ammountHundreds = 1000);

private:
    /**
     * @brief type 1 - "Visual Inspections", 2 - "Suspect lists of SRUs", 3 - "Acceptance Test Reports", 4 - "Scenario Test Results" size>=2
     */
    static void sendRandReq(int type, int size, QSqlQuery *query);
    static QString randWord();
    static int randValue();
    static QString randDateTime();
    static QString randTime();
};

#endif // RANDOMIZEDDATABASE_H
