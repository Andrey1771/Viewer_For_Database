#include "CollectionToIniSaver.h"
#include <QSettings>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDataStream>

CollectionToIniSaver::CollectionToIniSaver(const QString& fileName, QSettings* settings)
    : m_fileName(fileName), m_settings(settings)
{
}

CollectionToIniSaver::~CollectionToIniSaver()
{
}

QDataStream* serializeJson(const QVariant& data)
{
    QByteArray bytes;
    QDataStream* stream = new QDataStream(&bytes, QIODevice::WriteOnly);

    *stream << data;
    return stream;
}

QVariant castToSerializeable(const QVariant& variant)
{
    /// TODO: check!!!
    switch (static_cast<QMetaType::Type>(variant.type()))
    {
        case QMetaType::QJsonValue:
        case QMetaType::QJsonObject:
        case QMetaType::QJsonArray:
        case QMetaType::QJsonDocument:
            return QVariant(*serializeJson(variant));
        default:
            break;
    }
    return variant;
}

bool CollectionToIniSaver::saveCollection(const QString& collectionName, const QHash<QString, QVariant>& collection)
{
    QFile file(m_fileName);
    file.remove();
    if (!file.open(QIODevice::WriteOnly))
    {
        //LogCriticalMessage(QString("Not managed to open collection file: %1").arg(m_fileName));
        return false;
    }
    file.close();

    const bool temporary = nullptr == m_settings;
    if (temporary)
    {
        m_settings = new QSettings(m_fileName, QSettings::IniFormat);
    }

    m_settings->beginGroup(collectionName);
    for (auto&& key : collection.keys())
    {
        m_settings->setValue(key, castToSerializeable(collection[key]));
    }
    m_settings->endGroup();

    if (temporary)
    {
        delete m_settings;
        m_settings = nullptr;
    }
    return true;
}
