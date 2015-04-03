#include "inspectormodel.h"
#include "KnowledgeModel.h"
#include "utility/MetatypeDeclarations.h"
#include <QtQml>
#include <QSharedPointer>
#include <QDebug>
#include "OgreStringConverter.h"

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

QVariantList OgreVector3Model::getTextualRepresentation() const
{
    QVariantList list;

    list << QString("%1, %2, %3").arg(mVector.x).arg(mVector.y).arg(mVector.z);

    return list;
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
    setObjectName(QString("Vector3[%1]").arg(vectors.size()));
}

QVariantList OgreVector3ArrayModel::getTextualRepresentation() const
{
    QVariantList list;

    foreach(Ogre::Vector3 vec, mVectors)
    {
        list += QString("%1, %2, %3").arg(vec.x).arg(vec.y).arg(vec.z);
    }

    return list;
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
                               const QVariantMap& initial)
{
    setModel(name, initial);
}

const QString& InspectorModel::getName() const
{
    return mName;
}

void InspectorModel::setModel(const QString& name,
                              const QVariantMap& initial)
{
    beginResetModel();
    mName = name;
    mKnowledge = initial;
    endResetModel();
    emit nameChanged(name);
}

void InspectorModel::connectTo(const KnowledgeModel* model)
{
    connect(model, &KnowledgeModel::knowledgeAdded,
            this, &InspectorModel::onKnowledgeAdded);

    connect(model, &KnowledgeModel::knowledgeChanged,
            this, &InspectorModel::onKnowledgeChanged);

    connect(model, &KnowledgeModel::knowledgeRemoved,
            this, &InspectorModel::onKnowledgeRemoved);

    connect(model, &QObject::destroyed,
            this, &InspectorModel::onCurrentModelDestroyed);
}

void InspectorModel::onKnowledgeAdded(const QString& key, const QVariant& value)
{
    QVariantMap tmp = mKnowledge;
    tmp[key] = value;
    const int i = tmp.keys().indexOf(key);

    beginInsertRows(QModelIndex(), i, i);
    mKnowledge[key] = value;
    endInsertRows();
}

void InspectorModel::onKnowledgeChanged(const QString& key, const QVariant& value)
{
    QList<QString> keys = mKnowledge.keys();
    const int i = keys.indexOf(key);

    mKnowledge[key] = value;

    emit dataChanged(createIndex(i,i), createIndex(i,i));
}

void InspectorModel::onKnowledgeRemoved(const QString& key)
{
    QList<QString> keys = mKnowledge.keys();
    const int i = keys.indexOf(key);

    beginRemoveRows(QModelIndex(), i, i);
    const int numRemoved = mKnowledge.remove(key);
    assert(numRemoved == 1);
    endRemoveRows();
}

void InspectorModel::onCurrentModelDestroyed()
{
    ;
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

        if(!data.isValid())
        {
            return data;
        }

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

        if(data.canConvert<QObject*>())
        {
            if(data.isNull())
            {
                qWarning() << "NULL qObject encountered in knowledge. Can't display.";
                return QVariant::fromValue(static_cast<QObject*>(0));
            }

            QObject* qobject = data.value<QObject*>();
            if(qobject->thread() != thread())
            {
                // QObjects that are accessed through QML
                // must live in the QMLEngine's thread.
                // Consider adding a wrapper class.
                // The QObject will not be displayed.
                return "QObject (Not on mainthread)";
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
