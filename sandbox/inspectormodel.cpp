#include "inspectormodel.h"

#include <QtQml>
#include <QSharedPointer>

Q_DECLARE_METATYPE(Ogre::Vector3)
Q_DECLARE_METATYPE(Ogre::Vector3*)
Q_DECLARE_METATYPE(QVector<Ogre::Vector3>)
Q_DECLARE_METATYPE(QVector<Ogre::Vector3*>)
Q_DECLARE_METATYPE(Ogre::Quaternion)
Q_DECLARE_METATYPE(Ogre::Quaternion*)
Q_DECLARE_METATYPE(OgreVector3Model*)

void OgreVector3Model::declareQML()
{
    qmlRegisterUncreatableType<OgreVector3Model>("Inspector", 1, 0,
                                                 "OgreVector3",
                                                 "Cant instantiate ogrevector3.");
}

OgreVector3Model::OgreVector3Model(const Ogre::Vector3& vector) :
    mVector(vector)
{
    setObjectName("Vector3");
}

float OgreVector3Model::getX() const
{
    return mVector.x;
}

float OgreVector3Model::getY() const
{
    return mVector.y;
}
float OgreVector3Model::getZ() const
{
    return mVector.z;
}

void InspectorModel::declareQML()
{
    OgreVector3Model::declareQML();
}

InspectorModel::InspectorModel(const QString& name,
                               const QVariantMap& knowledge) :
    mName(name),
    mKnowledge(knowledge)
{
    ;
}

const QString& InspectorModel::getName() const
{
    return mName;
}

int InspectorModel::getSize() const
{
    return mKnowledge.size();
}

void InspectorModel::setModel(const QString& name,
                              const QVariantMap& knowledge)
{
    mName = name;
    mKnowledge = knowledge;

    emit(onNameChanged(name));
}

QHash<int, QByteArray> InspectorModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ModelRoleIndex] = "index";
    roles[ModelRoleKnowledgeKey] = "key";
    roles[ModelRoleKnowledgeValue] = "value";
    return roles;
}

Qt::ItemFlags InspectorModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant InspectorModel::data(const QModelIndex &index, int role) const
{
    QList<QString> keys = mKnowledge.keys();

    switch(role)
    {
    case ModelRoleIndex:
        return index.row();
    case ModelRoleKnowledgeKey:
        return keys[index.row()];
    case ModelRoleKnowledgeValue:
    {
        QVariant data = mKnowledge[keys[index.row()]];

        if(data.canConvert<Ogre::Vector3>())
        {
            return QVariant::fromValue(new OgreVector3Model(data.value<Ogre::Vector3>()));
        }

        return data;
    }
    default:
        return QVariant(QVariant::Invalid);
    }
}

QVariant InspectorModel::headerData(int section,
                                    Qt::Orientation orientation,
                                    int role) const
{
    return QVariant(QVariant::Invalid);
}

int InspectorModel::rowCount(const QModelIndex& parent) const
{
    return mKnowledge.size();
}
