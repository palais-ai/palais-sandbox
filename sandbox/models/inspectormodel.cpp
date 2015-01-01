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
                                                 "Can't instantiate ogrevector3.");
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

void OgreVector3ArrayModel::declareQML()
{
    qmlRegisterUncreatableType<OgreVector3Model>("Inspector", 1, 0,
                                                 "OgreVector3Array",
                                                 "Can't instantiate ogrevector3array.");
}

OgreVector3ArrayModel::OgreVector3ArrayModel(const QVector<Ogre::Vector3>& vectors) :
    mVectors(vectors)
{
    setObjectName("Array<Vector3>");
}

OgreVector3Model* OgreVector3ArrayModel::get(size_t idx) const
{
    return new OgreVector3Model(mVectors[idx]);
}

int OgreVector3ArrayModel::length() const
{
    return mVectors.size();
}

void InspectorModel::declareQML()
{
    OgreVector3Model::declareQML();
    OgreVector3ArrayModel::declareQML();
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
    Q_UNUSED(index);
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
            OgreVector3Model* model = new OgreVector3Model(data.value<Ogre::Vector3>());
            QQmlEngine::setObjectOwnership(model, QQmlEngine::JavaScriptOwnership);
            return QVariant::fromValue(model);
        }

        if(data.canConvert<QVector<Ogre::Vector3> >())
        {
            QVector<Ogre::Vector3> value = data.value<QVector<Ogre::Vector3> >();
            OgreVector3ArrayModel* model = new OgreVector3ArrayModel(value);
            QQmlEngine::setObjectOwnership(model, QQmlEngine::JavaScriptOwnership);
            return QVariant::fromValue(model);
        }

        if(data.canConvert<QList<QVariant> >())
        {
            QList<QVariant> value = data.toList();

            if(value.size() != 0)
            {
                QVariant& data = value.first();

                if(data.canConvert<Ogre::Vector3>())
                {
                    QVector<Ogre::Vector3> converted;
                    foreach(QVariant variant, value)
                    {
                        converted += variant.value<Ogre::Vector3>();
                    }
                    OgreVector3ArrayModel* model = new OgreVector3ArrayModel(converted);
                    QQmlEngine::setObjectOwnership(model, QQmlEngine::JavaScriptOwnership);
                    return QVariant::fromValue(model);
                }
            }
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
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);
    return QVariant(QVariant::Invalid);
}

int InspectorModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return mKnowledge.size();
}