#ifndef AILIBPLUGIN_H
#define AILIBPLUGIN_H

#include <QObject>
#include "PluginInterface.h"
#include "Scheduler.h"

class BehaviorPlugin : public QObject, public Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PLUGIN_INTERFACE_ID)
    Q_INTERFACES(Plugin)
public:
    BehaviorPlugin(QObject *parent = 0);

    virtual void onLoad(const PluginInterface& interface);
    virtual void onUnload(const PluginInterface& interface);
    virtual void onSceneStarted(const PluginInterface& interface, Scene& scene);
    virtual void onSceneEnded(const PluginInterface& interface, Scene& scene);
    virtual void update(const PluginInterface& interface, Scene& scene, float deltaTime);

private:
    ailib::Scheduler mScheduler;
};

#endif // AILIBPLUGIN_H
