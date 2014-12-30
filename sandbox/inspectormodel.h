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
    OgreVector3Model(const Ogre::Vector3& vector);

    float getX() const;
    float getY() const;
    float getZ() const;
private:
    Ogre::Vector3 mVector;
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
