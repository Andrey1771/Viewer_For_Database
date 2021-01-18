#ifndef TABLEHTML_H
#define TABLEHTML_H

#include <QString>

class TableHTML
{
    QString html;
public :
    TableHTML(const QString& html = "");

    void setTable(const QString& dataHTML);
    const QString& getTable() const;
    void setHTML(const QString& html);

};

#endif // TABLEHTML_H
