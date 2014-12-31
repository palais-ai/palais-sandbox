#ifndef INSPECTORMODEL_H
#define INSPECTORMODEL_H

#include <QAbstractListModel>

#include <OgreVector3.h>

class OgreVector3Model : public QObject
{
    Q_OBJECT
    Q_PROPERTY(float x READ getX)
    Q_PROPERTY(float y READ getY)
    Q_PROPERTY(float z READ getZ)

public:
    static void declareQML();

    OgreVector3Model(const Ogre::Vector3& vector = Ogre::Vector3());

    float getX() const;
    float getY() const;
    float getZ() const;
private:
    Ogre::Vector3 mVector;
};

class OgreVector3ArrayModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int length READ length)
public:
    static void declareQML();

    OgreVector3ArrayModel(const QVector<Ogre::Vector3>& vectors = QVector<Ogre::Vector3>());

    Q_INVOKABLE OgreVector3Model* get(size_t idx) const;
    Q_INVOKABLE int length() const;
private:
    QVector<Ogre::Vector3> mVectors;
};

class InspectorModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ getName NOTIFY onNameChanged)
    Q_PROPERTY(int size READ getSize NOTIFY onSizeChanged)
public:
    enum ModelRole {
        ModelRoleIndex = Qt::UserRole + 1,
        ModelRoleKnowledgeKey,
        ModelRoleKnowledgeValue
    };

    static void declareQML();

    InspectorModel(const QString& name,
                   const QVariantMap& knowledge);

    void setModel(const QString& name,
                 const QVariantMap& knowledge);

    const QString& getName() const;
    int getSize() const;

    // List Model impl
    virtual QHash<int, QByteArray> roleNames() const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual int rowCount(const QModelIndex &parent) const;
signals:
    void onNameChanged(const QString& name);
    void onSizeChanged(int size);
private:
    QString mName;
    QVariantMap mKnowledge;
};

#endif // INSPECTORMODEL_H
