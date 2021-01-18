#ifndef COLLECTION_TO_JSON_SAVER_H
#define COLLECTION_TO_JSON_SAVER_H

#include "ICollectionSaver.h"

class QFile;

class CollectionToJsonSaver final : public ICollectionSaver
{
public:
    explicit CollectionToJsonSaver(const QString& fileName, bool plainSave = true, QFile* file = nullptr);

    // ICollectionSaver interface
    virtual bool saveCollection(const QString& collectionName,
                                const QHash<QString, QVariant>& collection) override;

private:
    QString m_fileName;
    bool m_plainSave {true};
    QFile* m_file {nullptr};
};

#endif // COLLECTION_TO_JSON_SAVER_H
