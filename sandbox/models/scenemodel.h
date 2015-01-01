#ifndef SCENEMODEL_H
#define SCENEMODEL_H

#include "actormodel.h"

#include <QAbstractListModel>
#include <QVector>

class SceneModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ModelRole
    {
        ModelRoleIndex = Qt::UserRole + 1,
        ModelRoleName,
        ModelRoleIsVisible,
        ModelRoleIsSelected
    };

    SceneModel();

    void requestCurrentActors();

    // List Model impl
    virtual QHash<int, QByteArray> roleNames() const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual int rowCount(const QModelIndex &parent) const;
signals:
    void requestEmitCurrentActors();
public slots:
    void onActorAdded(const QString& actorName);
    void onActorRemoved(const QString& actorName);
private:
    QVector<ActorModel> mActors;
};

#endif // SCENEMODEL_H
