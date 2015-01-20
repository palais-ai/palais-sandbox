#include "PlanningPlugin.h"
#include "Planning.h"

#include <OgreSceneManager.h>

PlanningPlugin::PlanningPlugin(QObject *parent) :
    QObject(parent)
{
}

void PlanningPlugin::onLoad(const PluginInterface& interface)
{
    Q_UNUSED(interface);
}

void PlanningPlugin::onUnload(const PluginInterface& interface)
{
    Q_UNUSED(interface);
}

void PlanningPlugin::onSceneStarted(const PluginInterface& interface, Scene& scene)
{
    Q_UNUSED(interface);
    QScriptEngine& engine = scene.getScriptEngine();

    Planner_register_prototype(engine);
}

void PlanningPlugin::onSceneEnded(const PluginInterface& interface, Scene& scene)
{
    Q_UNUSED(interface);
    Q_UNUSED(scene);
}

void PlanningPlugin::update(const PluginInterface& interface, Scene& scene, float deltaTime)
{
    Q_UNUSED(interface);
    Planning::update(scene, deltaTime);
}
