#include "ICollection.h"
#include "ICollectionSaver.h"
#include "ICollectionLoader.h"
//#include "InstrumentUtils.h"

ICollection::ICollection(QObject* parent) noexcept : QObject(parent)
{
}

ICollection::ICollection(const ICollectionHash& collection, QObject* parent) noexcept
    : QObject(parent), m_collection(collection)
{
}

ICollection::ICollection(ICollectionHash&& collection, QObject* parent) noexcept
    : QObject(parent), m_collection(std::move(collection))
{
}

ICollection::~ICollection()
{
}

void ICollection::setCollectionName(const QString& name)
{
    m_collectionName = name;
}

QString ICollection::collectionName() const
{
    return m_collectionName;
}

bool ICollection::saveCollection(ICollectionSaver* saver)
{
    if (!saver)
    {
        return false;
    }
    return saver->saveCollection(m_collectionName, m_collection);
}

bool ICollection::loadCollection(ICollectionLoader* loader)
{
    if (!loader)
    {
        return false;
    }
    return loader->loadCollection(m_collectionName, m_collection);
}

void ICollection::printCollection() const
{
    const auto title = QString("Printing collection: %1").arg(m_collectionName);
   // LogInfoMessage(title);
    for (const auto& name : m_collection.keys())
    {
//        const auto message = QString("m_collection[%1] = %2").arg(name)
//                             .arg(InstrumentUtils::CastVariantToString(m_collection[name]));
        //LogInfoMessage(message);
    }
}

void ICollection::printCollection(const ICollectionHash& collHash, const QString& collectionName)
{
    ICollection collection(collHash);
    collection.setCollectionName(collectionName);
    collection.printCollection();
}

ICollectionHash ICollection::getCollection() const
{
    return m_collection;
}

bool ICollection::isEmpty() const
{
    return m_collection.isEmpty();
}

QVariant ICollection::getValue(const QString& name, const QVariant& defaultValue) const
{
    return m_collection.value(name, defaultValue);
}

QVariant ICollection::setValue(const QString& name, const QVariant& value, bool emitOnChanged)
{
    auto&& previousValue = m_collection.value(name);

    m_collection.insert(name, value);
    if (emitOnChanged && previousValue != value)
    {
        emit collectionValueChanged(name);
    }

    return previousValue;
}
