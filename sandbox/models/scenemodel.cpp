#include "scenemodel.h"
#include <QtQml>
#include <stdint.h>

void SceneModel::declareQML()
{
    qmlRegisterType<SceneModel>("SceneModel", 1, 0,
                                "SceneModel");
}

SceneModel::SceneModel(const QString& name) :
    mName(name),
    mActorSelected(false)
{
}

const QString& SceneModel::getName() const
{
    return mName;
}

QHash<int, QByteArray> SceneModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ModelRoleIndex] = "index";
    roles[ModelRoleName] = "name";
    roles[ModelRoleIsVisible] = "isVisible";
    roles[ModelRoleIsSelected] = "isSelected";
    return roles;
}

void SceneModel::requestCurrentActors()
{
    emit requestEmitCurrentActors();
}

bool SceneModel::getActorSelected() const
{
    return mActorSelected;
}

Qt::ItemFlags SceneModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant SceneModel::data(const QModelIndex &index, int role) const
{
    int idx = index.row();
    switch(role)
    {
    case ModelRoleIndex:
        return idx;
    case ModelRoleName:
        return mActors[idx].getName();
    case ModelRoleIsVisible:
        return mActors[idx].isVisible();
    case ModelRoleIsSelected:
        return mActors[idx].isSelected();
    default:
        return QVariant(QVariant::Invalid);
    }
}

QVariant SceneModel::headerData(int section,
                                Qt::Orientation orientation,
                                int role) const
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);
    return QVariant(QVariant::Invalid);
}

int SceneModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return mActors.size();
}

void SceneModel::onActorAdded(const QString& actorName)
{
    int index = mActors.size();
    beginInsertRows(QModelIndex(), index, index);
    mActors += ActorModel(actorName, true);
    endInsertRows();
}

void SceneModel::onActorRemoved(const QString& actorName)
{
    int i = indexForActorName(actorName);
    if(i != -1)
    {
        beginRemoveRows(QModelIndex(), i, i);
        mActors.remove(i);
        endRemoveRows();
    }
    else
    {
        qWarning("Requested actor removal for %s that wasn't present.",
                 actorName.toLocal8Bit().constData());
    }
}

void SceneModel::onActorChangedVisiblity(const QString& actorName, bool visible)
{
    int i = indexForActorName(actorName);
    if(i != -1)
    {
        mActors[i].setVisible(visible);
        QVector<int> roles;
        roles += ModelRoleIsVisible;
        emit dataChanged(createIndex(i,i), createIndex(i,i), roles);
    }
    else
    {
        qWarning("Requested actor visibility change for %s that wasn't present.",
                 actorName.toLocal8Bit().constData());
    }
}

void SceneModel::onActorChangedSelection(const QString& actorName, bool selected)
{
    int i = indexForActorName(actorName);
    if(i != -1)
    {
        mActorSelected = selected;
        mActors[i].setSelected(selected);
        QVector<int> roles;
        roles += ModelRoleIsSelected;
        emit dataChanged(createIndex(i,i), createIndex(i,i), roles);
        emit actorSelectedChanged(selected);
    }
    else
    {
        qWarning("Requested actor selection change for %s that wasn't present.",
                 actorName.toLocal8Bit().constData());
    }
}

void SceneModel::setVisibleRequested(int index, bool visible)
{
    if(index >= 0 && index < mActors.size())
    {
        emit requestVisiblitiyChange(mActors[index].getName(), visible);
    }
    else
    {
        qWarning("Index out of bounds actor requested. Visibility change was not performed.");
    }
}

void SceneModel::setSelectedRequested(int index, bool selected)
{
    if(index >= 0 && index < mActors.size())
    {
        emit requestSelectionChange(mActors[index].getName(), selected);
    }
    else
    {
        qWarning("Index out of bounds actor requested. Selection was not performed.");
    }
}

int SceneModel::indexForActorName(const QString& actorName) const
{
    for(int32_t i = 0; i < mActors.size(); ++i)
    {
        if(mActors[i].getName() == actorName)
        {
            return i;
        }
    }
    return -1;
}
