#include "BehaviorPlugin.h"
#include "Behavior.h"
#include <OgreSceneManager.h>

using namespace ailib;

BehaviorPlugin::BehaviorPlugin(QObject *parent) :
    QObject(parent)
{
}

void BehaviorPlugin::onLoad(const PluginInterface& interface)
{
    Q_UNUSED(interface);
}

void BehaviorPlugin::onUnload(const PluginInterface& interface)
{
    Q_UNUSED(interface);
}

void BehaviorPlugin::onSceneStarted(const PluginInterface& interface, Scene& scene)
{
    Q_UNUSED(interface);
    QScriptEngine& engine = scene.getScriptEngine();
}

void BehaviorPlugin::onSceneEnded(const PluginInterface& interface, Scene& scene)
{
    Q_UNUSED(interface);
    Q_UNUSED(scene);

    mScheduler.clear();
}

void BehaviorPlugin::update(const PluginInterface& interface, Scene& scene, float deltaTime)
{
    Q_UNUSED(interface);

    mScheduler.update(HighResolutionTime::milliseconds(8), deltaTime);
}
