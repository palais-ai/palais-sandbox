#ifndef SCENE_H
#define SCENE_H

#include "OgreHelper.h"
#include "KnowledgeModel.h"
#include <QVector>
#include <QScriptEngine>
#include <QMap>
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include <OgreFrameListener.h>

class Actor;
class OgreEngine;
class DebugDrawer;

namespace Ogre
{
class SceneNode;
class SceneManager;
}

class DLL_EXPORT RaycastResult
{
public:
    Actor* actor;
    float distance;
};

class DLL_EXPORT RangeQueryResult
{
public:
    QList<Actor*> actors;
};

class DLL_EXPORT Scene : public KnowledgeModel,
                         public Ogre::FrameListener
{
    Q_OBJECT
    Q_PROPERTY(QString name READ getName)
    Q_PROPERTY(QObjectList actors READ getActorsArray)
public:
    Scene(const QString& name,
          const QString& sceneFile,
          const QString& logicFile,
          Ogre::SceneNode* root,
          OgreEngine* engine,
          const QString& sceneManagerName);

    ~Scene();

    Ogre::SceneManager* getOgreSceneManager() const;
    DebugDrawer* createDebugDrawer(const QString& name);
    void destroyDebugDrawer(DebugDrawer* drawer);

    // Frame Listener
    virtual bool frameStarted(const Ogre::FrameEvent& evt);
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
    virtual bool frameEnded(const Ogre::FrameEvent& evt);

    Q_INVOKABLE void toggleHighlight(bool highlighted, int index);

    Q_INVOKABLE Actor* instantiate(const QString& name,
                                   const QString& meshName,
                                   const Ogre::Vector3& position = Ogre::Vector3::ZERO,
                                   const Ogre::Quaternion& rotation = Ogre::Quaternion::IDENTITY,
                                   const Ogre::Vector3& scale = Ogre::Vector3(1,1,1));

    Q_INVOKABLE void destroy(Actor* actor);
    Q_INVOKABLE void destroyLater(Actor* actor);
    Q_INVOKABLE void setCameraFocus(Actor* actor);
    // Attaches the actors scene node to the root scene node.
    Q_INVOKABLE void attach(Actor* actor);

    // Reports the first hit actor in the scene.
    Q_INVOKABLE RaycastResult raycast(const Ogre::Vector3& origin,
                                      const Ogre::Vector3& direction);

    Q_INVOKABLE RangeQueryResult rangeQuery(const Ogre::Vector3& origin, float distance);

    void setup();
    void update(float time);
    void performAction(const QString& actionName, const QVariant& params = QVariant());

    QScriptEngine& getScriptEngine();
    const QString& getName() const;

    Actor* addActor(Ogre::SceneNode* node);
    const QMap<QString, Actor*>& getActors() const;
    Q_INVOKABLE bool hasActor(const QString& actorName) const;
    Q_INVOKABLE Actor* getActorByIndex(unsigned int index);
    Q_INVOKABLE Actor* getActorByName(const QString& actorName);
    Actor* getActor(const QString& actorName);
    Q_INVOKABLE QObjectList getActorsArray() const;

    const QString& getLogicFile() const;
    const QString& getSceneFile() const;
signals:
    void actorAdded(const QString& actorName);
    void actorRemoved(const QString& actorName);
    void actorRemovedObject(Actor* actor);
    void actorChangedVisibility(const QString& actorName, bool visible);
    void actorDestroyLater(Actor* actor);
public slots:
    void onRequestEmitCurrentActors();
    void onActorChangeVisible(const QString& actorName,
                              bool visible);
    void onActorVisibilityChanged(Actor* actor, bool visible);
private slots:
    void onDestroyLater(Actor* actor);
private:
    void getActors(Ogre::SceneNode* root);
    Actor* getActorForNode(Ogre::SceneNode* node) const;
    void parseNavMesh(Actor* navmesh);
    void destroyAllAttachedMovableObjects(Ogre::SceneNode* i_pSceneNode);

    QString mName, mSceneFile, mLogicFile;
    OgreEngine* mEngine;
    QString mSceneManagerName;
    Ogre::SceneNode* mRoot;
    Ogre::RaySceneQuery* mRayQuery;
    Ogre::SphereSceneQuery* mSphereQuery;
    QScriptEngine mLogicScript;
    QMap<QString, Actor*> mActors;
    QVector<DebugDrawer*> mDrawers;
    bool mIsSetup;
};

#endif // SCENE_H