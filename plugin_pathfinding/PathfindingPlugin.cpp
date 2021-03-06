#include "PathfindingPlugin.h"
#include "DebugDrawer.h"
#include <OgreSceneManager.h>

Scene* gCurrentScene = NULL;

PathfindingPlugin::PathfindingPlugin(QObject *parent) :
    QObject(parent),
    mPathfindingDrawer(NULL)
{
}

void PathfindingPlugin::onLoad(const PluginInterface& interface)
{
    Q_UNUSED(interface);
}

void PathfindingPlugin::onUnload(const PluginInterface& interface)
{
    Q_UNUSED(interface);
}

void PathfindingPlugin::onSceneStarted(const PluginInterface& interface, Scene& scene)
{
    Q_UNUSED(interface);

    gCurrentScene = &scene;

    QListIterator<QObject*> it(scene.getActorsArray());
    while(it.hasNext())
    {
        Actor* value = qobject_cast<Actor*>(it.next());
        QString key = value->getName();

        if(key.toLower() == "navmesh")
        {
            mPathfindingDrawer = scene.createDebugDrawer("navigation_graph");
            mPathfinding.initNavGraphFromNode(value->getSceneNode());
            mPathfinding.visualizeNavGraph(mPathfindingDrawer);
            break;
        }
    }

    QScriptEngine& engine = scene.getScriptEngine();
    engine.globalObject().setProperty("Pathfinding",
                                      engine.newQObject(&mPathfinding),
                                      QScriptValue::Undeletable | QScriptValue::ReadOnly);
}

void PathfindingPlugin::onSceneEnded(const PluginInterface& interface, Scene& scene)
{
    Q_UNUSED(interface);
    if(mPathfindingDrawer)
    {
        scene.destroyDebugDrawer(mPathfindingDrawer);
        mPathfindingDrawer = NULL;
    }

    gCurrentScene = NULL;
}

void PathfindingPlugin::update(const PluginInterface& interface, Scene& scene, float deltaTime)
{
    Q_UNUSED(interface);
    mPathfinding.update(scene, deltaTime);
}
