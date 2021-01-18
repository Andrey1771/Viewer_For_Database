#include "printprogress.h"

PrintProgress::PrintProgress(QObject* parent) : QObject(parent)
{
    stop = false;
}

PrintProgress::~PrintProgress()
{

}

void PrintProgress::setProgressCount(float progressCount)
{
    this->progressCount = progressCount;
    emit progressChanged(this->progressCount);
}

float PrintProgress::getProgressCount()
{
    return progressCount;
}

void PrintProgress::setPartProgress(float partProgress)
{
    this->partProgress = partProgress;
}

float PrintProgress::getPartProgress()
{
    return partProgress;
}

PrintController *PrintProgress::getPrintSup() const
{
    return printSup.get();
}

void PrintProgress::startNewPrintSup(PrintController* value)
{
    printSup = QSharedPointer<PrintController>(value);
};

void PrintProgress::setTypePrint(TypePrint type)
{
    this->type = type;
}

TypePrint PrintProgress::getTypePrint()
{
    return type;
}

int PrintProgress::getXMLPDFNumber()
{
    return xmlPdfNumber;
}

void PrintProgress::setXMLPDFNumber(int xmlPdfNumber)
{
    this->xmlPdfNumber = xmlPdfNumber;
}


void PrintProgress::nextPartProgress()
{
    setProgressCount(getProgressCount() + getPartProgress());
}


