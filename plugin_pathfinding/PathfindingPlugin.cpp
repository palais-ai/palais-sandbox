#include "PathfindingPlugin.h"
#include "DebugDrawer.h"
#include <OgreSceneManager.h>

PathfindingPlugin::PathfindingPlugin(QObject *parent) :
    QObject(parent)
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

    mPathfindingDrawer = scene.createDebugDrawer("navigation_graph");

    QListIterator<QObject*> it(scene.getActorsArray());
    while(it.hasNext())
    {
        Actor* value = qobject_cast<Actor*>(it.next());
        QString key = value->getName();

        if(key.toLower() == "navmesh")
        {
            mPathfinding.initNavGraphFromQONode(value->getSceneNode());
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
    scene.destroyDebugDrawer(mPathfindingDrawer);
}

void PathfindingPlugin::update(const PluginInterface& interface, Scene& scene, float deltaTime)
{
    Q_UNUSED(interface);
    mPathfinding.update(scene, deltaTime);
}
