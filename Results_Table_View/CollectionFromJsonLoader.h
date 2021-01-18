#ifndef COLLECTION_FROM_JSON_LOADER_H
#define COLLECTION_FROM_JSON_LOADER_H

#include "ICollectionLoader.h"

class QFile;

class CollectionFromJsonLoader final : public ICollectionLoader
{
public:
    explicit CollectionFromJsonLoader(const QString& fileName, bool plainLoad = true, QFile* file = nullptr);

    // ICollectionLoader interface
    virtual bool loadCollection(const QString& collectionName,
                                QHash<QString, QVariant>& collection) override;

private:
    QString m_fileName;
    bool m_plainLoad {true};
    QFile* m_file {nullptr};
};

#endif // COLLECTION_FROM_JSON_LOADER_H
