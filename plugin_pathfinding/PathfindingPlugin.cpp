#include "PathfindingPlugin.h"
#include "DebugDrawer.h"

#include <OgreSceneManager.h>

PathfindingPlugin::PathfindingPlugin(QObject *parent) :
    QObject(parent)
{
}

void PathfindingPlugin::onLoad(const PluginInterface& interface)
{

}

void PathfindingPlugin::onUnload(const PluginInterface& interface)
{

}

void PathfindingPlugin::onSceneStarted(const PluginInterface& interface, Scene& scene)
{
    mPathfindingDrawer = scene.createDebugDrawer("navigation_graph");

    QMapIterator<QString, Actor*> it(scene.getActors());
    while(it.hasNext())
    {
        it.next();

        const QString& key = it.key();
        if(key.toLower() == "navmesh")
        {
            mPathfinding.initNavGraphFromOgreNode(it.value()->getSceneNode());
            mPathfinding.visualizeNavGraph(mPathfindingDrawer);

            Ogre::String name = mPathfindingDrawer->getName();
            Ogre::SceneNode* vis = scene.getOgreSceneManager()->getSceneNode(name);
            scene.addActor(vis);
            break;
        }
    }

    QScriptEngine& engine = scene.getScriptEngine();
    engine.globalObject().setProperty("pathfinding", engine.newQObject(&mPathfinding));
}

void PathfindingPlugin::onSceneEnded(const PluginInterface& interface, Scene& scene)
{
    scene.destroyDebugDrawer(mPathfindingDrawer);
}

void PathfindingPlugin::update(const PluginInterface& interface, Scene& scene, float deltaTime)
{
    mPathfinding.update(scene, deltaTime);
}
