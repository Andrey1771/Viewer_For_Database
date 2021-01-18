#ifndef I_COLLECTION_H
#define I_COLLECTION_H

#include <QObject>
#include <QVariant>
#include <QHash>

class ICollectionSaver;
class ICollectionLoader;

typedef QHash<QString, QVariant> ICollectionHash;

class ICollection : public QObject
{
    Q_OBJECT
public:
    explicit ICollection(QObject* parent = nullptr) noexcept;
    explicit ICollection(const ICollectionHash& collection, QObject* parent = nullptr) noexcept;
    explicit ICollection(ICollectionHash&& collection, QObject* parent = nullptr) noexcept;
    virtual ~ICollection();

    void setCollectionName(const QString& name);
    QString collectionName() const;

    virtual bool saveCollection(ICollectionSaver* saver);
    virtual bool loadCollection(ICollectionLoader* loader);

    virtual void printCollection() const;
    static void printCollection(const ICollectionHash& collHash, const QString& collectionName = {});
    ICollectionHash getCollection() const;

    bool isEmpty() const;

signals:
    void collectionValueChanged(QString name);

protected:
    virtual QVariant getValue(const QString& name, const QVariant& defaultValue = {}) const;
    virtual QVariant setValue(const QString& name, const QVariant& value, bool emitOnChanged = false);

    template <typename EnumType>
    inline EnumType GetEnumValue(const QString& name) const {
        return static_cast<EnumType>(getValue(name).toInt());
    }

    template <typename EnumType>
    inline EnumType SetEnumValue(const QString& name, const EnumType value) {
        return static_cast<EnumType>(setValue(name, static_cast<int>(value)).toInt());
    }

    ICollectionHash m_collection;
    QString m_collectionName;
};

#endif // I_COLLECTION_H
