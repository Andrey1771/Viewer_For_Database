#include "CollectionFromJsonLoader.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>

CollectionFromJsonLoader::CollectionFromJsonLoader(const QString& fileName, bool plainLoad, QFile* file)
    : m_fileName(fileName), m_plainLoad(plainLoad), m_file(file)
{
}

QVariant collectionValueFromObject(const QVariant& value, const QJsonValue& jsonValue)
{
    switch (static_cast<QMetaType::Type>(value.type()))
    {
        case QMetaType::Bool:
            return jsonValue.toBool();
        case QMetaType::Double:
            return jsonValue.toDouble();
        case QMetaType::Int:
            return jsonValue.toInt();
        case QMetaType::LongLong:
            return jsonValue.toVariant().toLongLong();
        case QMetaType::QString:
            return jsonValue.toString();
        case QMetaType::QJsonArray:
            return jsonValue.toArray();
        case QMetaType::QStringList:
            return jsonValue.toVariant().toStringList();
        case QMetaType::QJsonObject:
            return jsonValue.toObject();
        case QMetaType::QJsonValue:
            return jsonValue;
        case QMetaType::QVariantHash:
            return jsonValue.toVariant().toHash();
        case QMetaType::QVariantMap:
            return jsonValue.toVariant().toMap();
        case QMetaType::QVariantList:
            return jsonValue.toVariant().toList();
        default:
            return jsonValue.toVariant();
    }
}

bool loadFromFile(QFile* file, const QString& collectionName, QHash<QString, QVariant>& collection, bool plainLoad)
{
    QJsonParseError err;
    const auto& doc = QJsonDocument::fromJson(file->readAll(), &err);
    if (!doc.isObject())
    {
//        LogCriticalMessage(QString("Not managed to parse \"%1\" JSON-file, error: %2")
//                           .arg(file->fileName()).arg(err.errorString()));
        return false;
    }

    const auto& docObject = doc.object();
    if (plainLoad || collection.isEmpty() || collectionName.isEmpty())
    {
        for (const auto& key : docObject.keys())
        {
            collection.insert(key, docObject[key].toVariant());
        }
        return true;
    }

    if (!docObject.keys().contains(collectionName))
    {
        return false;
    }

    const auto& array = docObject.value(collectionName).toArray();
    if (array.isEmpty())
    {
        return false;
    }

    const auto& keys = collection.keys();
    for (const auto& key : keys)
    {
        for (const auto& element : array)
        {
            if (element.isObject())
            {
                const auto& elementObject = element.toObject();
                if (elementObject.isEmpty())
                {
                    continue;
                }
                if (!elementObject.contains(key))
                {
                    continue;
                }
                collection.insert(key, collectionValueFromObject(collection[key], elementObject.value(key)));
                break;
            }
        }
    }
    return true;
}

bool CollectionFromJsonLoader::loadCollection(const QString& collectionName, QHash<QString, QVariant>& collection)
{
    const bool temporary = m_file == nullptr;
    if (temporary)
    {
        if (!QFileInfo::exists(m_fileName))
        {
//            LogCriticalMessage(QString("Can't load collection from JSON - the file with name \"%1\" doesn't exist")
//                               .arg(m_fileName));
            return false;
        }

        m_file = new QFile(m_fileName);
        if (!m_file->open(QIODevice::ReadOnly))
        {
            //LogCriticalMessage(QString("Not managed to open collection file: %1").arg(m_fileName));
            delete m_file;
            m_file = nullptr;
            return false;
        }
    }

    auto status = loadFromFile(m_file, collectionName, collection, m_plainLoad);
    if (temporary)
    {
        delete m_file;
        m_file = nullptr;
    }

    return status;
}
