#include "xmlprintprogress.h"
#include "xmlprintsupport.h"

XMLPrintProgress::XMLPrintProgress(QObject* parent) : QObject(parent)
{
    stop = false;
}

void XMLPrintProgress::setProgressCount(float progressCount)
{
    this->progressCount = progressCount;
    emit progressChanged(this->progressCount);
}

float XMLPrintProgress::getProgressCount()
{
    return progressCount;
}

void XMLPrintProgress::setPartProgress(float partProgress)
{
    this->partProgress = partProgress;
}

float XMLPrintProgress::getPartProgress()
{
    return partProgress;
}

XMLPrintSupport *XMLPrintProgress::getPrintSup() const
{
    return printSup;
}

void XMLPrintProgress::setPrintSup(XMLPrintSupport *value)
{
    printSup = value;
};

void XMLPrintProgress::sendCountFiles(int countFiles)
{
    emit countedFiles(countFiles);
}
