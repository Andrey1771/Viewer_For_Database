#include "CollectionToJsonSaver.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <memory>
CollectionToJsonSaver::CollectionToJsonSaver(const QString& fileName, bool plainSave, QFile* file)
    : m_fileName(fileName), m_plainSave(plainSave), m_file(file)
{
}

QJsonObject collectionValueToObject(const QString& key, const QVariant& value)
{
    switch (static_cast<QMetaType::Type>(value.type()))
    {
        case QMetaType::Bool:
            return QJsonObject{ {key, QJsonValue(value.toBool())} };
        case QMetaType::Double:
            return QJsonObject{ {key, QJsonValue(value.toDouble())} };
        case QMetaType::Int:
            return QJsonObject{ {key, QJsonValue(value.toInt())} };
        case QMetaType::LongLong:
            return QJsonObject{ {key, QJsonValue(value.toLongLong())} };
        case QMetaType::QString:
            return QJsonObject{ {key, QJsonValue(value.toString())} };
        case QMetaType::QVariantHash:
            return QJsonObject{ {key, QJsonObject::fromVariantHash(value.toHash())} };
        case QMetaType::QVariantMap:
            return QJsonObject{ {key, QJsonObject::fromVariantMap(value.toMap())} };
        case QMetaType::QVariantList:
            return QJsonObject{ {key, QJsonValue::fromVariant(value)} };
        case QMetaType::QJsonArray:
        case QMetaType::QStringList:
            return QJsonObject{ {key, QJsonValue(value.toJsonArray())} };
        case QMetaType::QJsonObject:
            return QJsonObject{ {key, QJsonValue(value.toJsonObject())} };
        case QMetaType::QJsonValue:
            return QJsonObject{ {key, QJsonValue(value.toJsonValue())} };
        default:
            break;
    }
    return QJsonObject();
}

QJsonObject collectionToObject(const QString& collectionName, const QHash<QString, QVariant>& collection,
                               bool plainSave)
{
    if (plainSave || collectionName.isEmpty())
    {
        return QJsonObject::fromVariantHash(collection);
    }
    else
    {
        QJsonArray array;
        for (const auto& key : collection.keys())
        {
            array << collectionValueToObject(key, collection[key]);
        }
        return QJsonObject{ {collectionName, array} };
    }
}

bool CollectionToJsonSaver::saveCollection(const QString& collectionName, const QHash<QString, QVariant>& collection)
{
    QJsonDocument doc(collectionToObject(collectionName, collection, m_plainSave));
    if (m_file)
    {
        m_file->write(doc.toJson());
    }
    else
    {

        auto file = std::make_unique<QFile>(m_fileName);
        file->remove();
        if (!file->open(QIODevice::WriteOnly))
        {
            //LogCriticalMessage(QString("Not managed to open collection file: %1").arg(m_fileName));
            return false;
        }
        file->write(doc.toJson());
    }

    return true;
}


