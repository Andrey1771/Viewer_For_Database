#include "tablehtml.h"
#include "pdfprintsupport.h"

TableHTML::TableHTML(const QString &html)
{
    setHTML(html);
}

void TableHTML::setTable(const QString& dataHTML)
{
    html = PDFStyles::HTMLHead + "<p>";
    html += dataHTML;
    html += "</div></p></body></html>";
}

const QString& TableHTML::getTable() const
{
    return html;
}

void TableHTML::setHTML(const QString& html)
{
    this->html = html;
}
