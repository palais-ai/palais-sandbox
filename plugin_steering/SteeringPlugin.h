#ifndef STEERINGPLUGIN_H
#define STEERINGPLUGIN_H

#include <QObject>
#include "PluginInterface.h"

class SteeringPlugin : public QObject, public Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PLUGIN_INTERFACE_ID)
    Q_INTERFACES(Plugin)
public:
    SteeringPlugin(QObject *parent = 0);

    virtual void onLoad(const PluginInterface& interface);
    virtual void onUnload(const PluginInterface& interface);
    virtual void onSceneStarted(const PluginInterface& interface, Scene& scene);
    virtual void onSceneEnded(const PluginInterface& interface, Scene& scene);
    virtual void update(const PluginInterface& interface, Scene& scene, float deltaTime);

private:
};

#endif // STEERINGPLUGIN_H