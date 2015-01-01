#include "scenemodel.h"

SceneModel::SceneModel()
{
}

QHash<int, QByteArray> SceneModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ModelRoleIndex] = "index";
    roles[ModelRoleName] = "name";
    roles[ModelRoleIsVisible] = "visible";
    roles[ModelRoleIsSelected] = "selected";
    return roles;
}

void SceneModel::requestCurrentActors()
{
    emit requestEmitCurrentActors();
}

Qt::ItemFlags SceneModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant SceneModel::data(const QModelIndex &index, int role) const
{
    uint32_t idx = index.row();
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
    for(size_t i = 0; i < mActors.size(); ++i)
    {
        if(mActors[i].getName() == actorName)
        {
            beginRemoveRows(QModelIndex(), i, i);
            mActors.remove(i);
            endRemoveRows();
            return;
        }
    }
}
