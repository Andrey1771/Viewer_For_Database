#ifndef I_COLLECTION_SAVER_H
#define I_COLLECTION_SAVER_H

#include <QVariant>

class ICollectionSaver
{
public:
    ICollectionSaver();
    virtual ~ICollectionSaver();

    virtual bool saveCollection(const QString& collectionName, const QHash<QString, QVariant>& collection) = 0;
};

#endif // I_COLLECTION_SAVER_H
