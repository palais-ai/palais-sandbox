#ifndef SCENEMODEL_H
#define SCENEMODEL_H

#include "actormodel.h"

#include <QAbstractListModel>
#include <QVector>

class SceneModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString name READ getName NOTIFY nameChanged)
    Q_PROPERTY(bool actorSelected READ getActorSelected NOTIFY actorSelectedChanged)
public:
    enum ModelRole
    {
        ModelRoleIndex = Qt::UserRole + 1,
        ModelRoleName,
        ModelRoleIsVisible,
        ModelRoleIsSelected
    };

    static void declareQML();
    SceneModel(const QString& name = "");

    const QString& getName() const;
    void requestCurrentActors();
    bool getActorSelected() const;

    // List Model impl
    virtual QHash<int, QByteArray> roleNames() const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual int rowCount(const QModelIndex &parent) const;

    Q_INVOKABLE void setVisibleRequested(int index, bool visible);
    Q_INVOKABLE void setSelectedRequested(int index, bool selected);
signals:
    void nameChanged();
    void requestEmitCurrentActors();
    void requestVisiblitiyChange(const QString& actorName, bool visible);
    void requestSelectionChange(const QString& actorName, bool selected);
    void actorSelectedChanged(bool selected);
public slots:
    void onActorAdded(const QString& actorName);
    void onActorRemoved(const QString& actorName);
    void onActorChangedVisiblity(const QString& actorName, bool visible);
    void onActorChangedSelection(const QString& actorName, bool selected);
private:
    int indexForActorName(const QString& actorName) const;

    QVector<ActorModel> mActors;
    QString mName;
    bool mActorSelected;
};

#endif // SCENEMODEL_H
