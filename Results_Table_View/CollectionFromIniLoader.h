#ifndef COLLECTION_FROM_INI_LOADER_H
#define COLLECTION_FROM_INI_LOADER_H

#include "ICollectionLoader.h"

class QSettings;

class CollectionFromIniLoader final : public ICollectionLoader
{
public:
    explicit CollectionFromIniLoader(const QString& fileName, QSettings* settings = nullptr);
    ~CollectionFromIniLoader();

    // ICollectionLoader interface
    virtual bool loadCollection(const QString& collectionName, QHash<QString, QVariant>& collection) override;

private:
    QString m_fileName;
    QSettings* m_settings {nullptr};
};

#endif // COLLECTION_FROM_INI_LOADER_Hs
