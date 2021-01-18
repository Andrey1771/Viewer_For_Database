#ifndef COLLECTION_TO_INI_SAVER_H
#define COLLECTION_TO_INI_SAVER_H

#include "ICollectionSaver.h"

class QSettings;

class CollectionToIniSaver final : public ICollectionSaver
{
public:
    explicit CollectionToIniSaver(const QString& fileName, QSettings* settings = nullptr);
    ~CollectionToIniSaver();

    // ICollectionSaver interface
    virtual bool saveCollection(const QString& collectionName, const QHash<QString, QVariant>& collection) override;

private:
    QString m_fileName;
    QSettings* m_settings {nullptr};
};

#endif // COLLECTION_TO_INI_SAVER_H
