#ifndef PATHFINDINGPLUGIN_H
#define PATHFINDINGPLUGIN_H

#include <QObject>
#include "PluginInterface.h"
#include "Pathfinding.h"

class PathfindingPlugin : public QObject, public Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PLUGIN_INTERFACE_ID)
    Q_INTERFACES(Plugin)
public:
    PathfindingPlugin(QObject *parent = 0);

    virtual void onLoad(const PluginInterface& interface);
    virtual void onUnload(const PluginInterface& interface);
    virtual void onSceneStarted(const PluginInterface& interface, Scene& scene);
    virtual void onSceneEnded(const PluginInterface& interface, Scene& scene);
    virtual void update(const PluginInterface& interface, Scene& scene, float deltaTime);

private:
    Pathfinding mPathfinding;
    DebugDrawer* mPathfindingDrawer;
};

#endif // PATHFINDINGPLUGIN_H
