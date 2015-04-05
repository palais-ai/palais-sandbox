#ifndef INSPECTORMODEL_H
#define INSPECTORMODEL_H

#include <QAbstractListModel>
#include <QVector>
#include <OgreVector3.h>

class KnowledgeModel;

class TextualModel : public QObject
{
    Q_OBJECT
public:
    virtual QVariantList getTextualRepresentation() const = 0;
};

class OgreVector3Model : public TextualModel
{
    Q_OBJECT
    Q_PROPERTY(float x READ getX)
    Q_PROPERTY(float y READ getY)
    Q_PROPERTY(float z READ getZ)

public:
    static void declareQML();

    OgreVector3Model(const Ogre::Vector3& vector = Ogre::Vector3());

    Q_INVOKABLE QVariantList getTextualRepresentation() const;

    float getX() const;
    float getY() const;
    float getZ() const;
private:
    Ogre::Vector3 mVector;
};

class OgreVector3ArrayModel : public TextualModel
{
    Q_OBJECT
    Q_PROPERTY(int length READ length)
public:
    static void declareQML();

    OgreVector3ArrayModel(const QVector<Ogre::Vector3>& vectors = QVector<Ogre::Vector3>());

    Q_INVOKABLE QVariantList getTextualRepresentation() const;
    Q_INVOKABLE OgreVector3Model* get(size_t idx) const;
    Q_INVOKABLE int length() const;
private:
    QVector<Ogre::Vector3> mVectors;
};

class InspectorModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ getName NOTIFY nameChanged)
public:
    enum ModelRole
    {
        ModelRoleIndex = Qt::UserRole + 1,
        ModelRoleKnowledgeKey,
        ModelRoleKnowledgeValue
    };

    static void declareQML();

    InspectorModel(const QString& name,
                   const QVariantMap& data,
                   const KnowledgeModel* knowledge);

    void setModel(const QString& name,
                  const QVariantMap& data);

    void connectTo(const KnowledgeModel* knowledge);

    const QString& getName() const;

    // List Model impl
    virtual QHash<int, QByteArray> roleNames() const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual int rowCount(const QModelIndex &parent) const;
signals:
    void nameChanged(const QString& name);
public slots:
    void onKnowledgeAdded(const QString& key, const QVariant& value);
    void onKnowledgeChanged(const QString& key, const QVariant& value);
    void onKnowledgeRemoved(const QString& key);
private:
    QString mName;
    QVariantMap mKnowledge;
    const KnowledgeModel* mCurrentModel;
};

#endif // INSPECTORMODEL_H
