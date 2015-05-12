#include "SceneModel.h"
#include <QtQml>
#include <stdint.h>
#include <QDebug>

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
        return actorForIndex(idx).getName();
    case ModelRoleIsVisible:
        return actorForIndex(idx).isVisible();
    case ModelRoleIsSelected:
        return actorForIndex(idx).isSelected();
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
    if(mActors.contains(actorName))
    {
        qWarning() << "SceneModel.onActorAdded: Tried to add actor with name [ "
                   << actorName << " ] that already existed.";
        return;
    }

    QMap<QString, ActorModel> copy = mActors;
    copy[actorName] = ActorModel(actorName, true);
    const int index = copy.keys().indexOf(actorName);

    beginInsertRows(QModelIndex(), index, index);
    mActors[actorName] = ActorModel(actorName, true);
    endInsertRows();
}

void SceneModel::onActorRemoved(const QString& actorName)
{
    QMap<QString, ActorModel>::const_iterator it = mActors.find(actorName);
    if(it != mActors.end())
    {
        const int i = indexForName(actorName);
        beginRemoveRows(QModelIndex(), i, i);
        mActors.remove(actorName);
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
    QMap<QString, ActorModel>::iterator it = mActors.find(actorName);
    if(it != mActors.end())
    {
        it->setVisible(visible);

        const int i = indexForName(actorName);
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
    QMap<QString, ActorModel>::iterator it = mActors.find(actorName);
    if(it != mActors.end())
    {
        mActorSelected = selected;
        it->setSelected(selected);

        const int i = indexForName(actorName);
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
        emit requestVisiblitiyChange(actorForIndex(index).getName(), visible);
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
        emit requestSelectionChange(actorForIndex(index).getName(), selected);
    }
    else
    {
        qWarning("Index out of bounds actor requested. Selection was not performed.");
    }
}

ActorModel SceneModel::actorForIndex(int index) const
{
    if(index >= mActors.size())
    {
        qWarning() << "Requested actor at index [ " << index
                   << " ] that is out of bounds [ size=" << mActors.size()
                   << " ].";
        return ActorModel();
    }
    return mActors[mActors.keys().at(index)];
}

int SceneModel::indexForName(const QString& name) const
{
    return mActors.keys().indexOf(name);
}
