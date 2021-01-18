#ifndef I_COLLECTION_LOADER_H
#define I_COLLECTION_LOADER_H

#include <QVariant>

class ICollectionLoader
{
public:
    ICollectionLoader();
    virtual ~ICollectionLoader();

    virtual bool loadCollection(const QString& collectionName, QHash<QString, QVariant>& collection) = 0;
};

#endif // I_COLLECTION_LOADER_H
