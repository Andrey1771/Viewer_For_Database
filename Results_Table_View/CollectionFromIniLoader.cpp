#include "CollectionFromIniLoader.h"
#include <QSettings>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>

CollectionFromIniLoader::CollectionFromIniLoader(const QString& fileName, QSettings* settings)
    : m_fileName(fileName), m_settings(settings)
{
}

CollectionFromIniLoader::~CollectionFromIniLoader()
{
}

QVariant castToVariant(const QVariant& variant, const QVariant& castTo)
{
    /// TODO: check!!!
    switch (static_cast<QMetaType::Type>(castTo.type()))
    {
        case QMetaType::Bool:
            return variant.toBool();
        case QMetaType::Float:
            return variant.toFloat();
        case QMetaType::Double:
            return variant.toDouble();
        case QMetaType::Int:
            return variant.toInt();
        case QMetaType::UInt:
            return variant.toUInt();
        case QMetaType::QString:
            return variant.toString();
        case QMetaType::QTime:
            return variant.toTime();
        case QMetaType::QDateTime:
            return variant.toDateTime();
        case QMetaType::LongLong:
            return variant.toLongLong();
        case QMetaType::ULongLong:
            return variant.toULongLong();
        case QMetaType::QJsonValue:
            return variant.toJsonValue();
        case QMetaType::QJsonObject:
            return variant.toJsonObject();
        case QMetaType::QJsonArray:
            return variant.toJsonArray();
        case QMetaType::QJsonDocument:
            return variant.toJsonDocument();
        case QMetaType::QStringList:
            return variant.toStringList();
        case QMetaType::QVariantMap:
            return variant.toMap();
        case QMetaType::QVariantHash:
            return variant.toHash();
        case QMetaType::QByteArray:
            return variant.toByteArray();
        default:
            break;
    }
    return QVariant{};
}

bool CollectionFromIniLoader::loadCollection(const QString& collectionName, QHash<QString, QVariant>& collection)
{
    const bool temporary = nullptr == m_settings;
    if (temporary)
    {
        if (!QFileInfo::exists(m_fileName))
        {
//            LogCriticalMessage(QString("Can't load collection from INI - the file with name \"%1\" doesn't exist")
//                               .arg(m_fileName));
            return false;
        }

        m_settings = new QSettings(m_fileName, QSettings::IniFormat);
    }

    m_settings->beginGroup(collectionName);
    auto&& keys = collection.keys();
    for (auto&& key : keys)
    {
        auto&& _default = collection[key];
        collection.insert(key, castToVariant(m_settings->value(key, _default), _default));
    }
    m_settings->endGroup();

    if (temporary)
    {
        delete m_settings;
        m_settings = nullptr;
    }
    return true;
}
