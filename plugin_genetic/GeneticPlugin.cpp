#include "GeneticPlugin.h"
#include "GeneticPrototype.h"

GeneticPlugin::GeneticPlugin(QObject *parent) :
    QObject(parent)
{
}

void GeneticPlugin::onLoad(const PluginInterface& interface)
{
    Q_UNUSED(interface);
}

void GeneticPlugin::onUnload(const PluginInterface& interface)
{
    Q_UNUSED(interface);
}

void GeneticPlugin::onSceneStarted(const PluginInterface& interface, Scene& scene)
{
    Q_UNUSED(interface);
    QScriptEngine& engine = scene.getScriptEngine();
    Genetic_register_prototype(engine);
}

void GeneticPlugin::onSceneEnded(const PluginInterface& interface, Scene& scene)
{
    Q_UNUSED(interface);
    Q_UNUSED(scene);
}

void GeneticPlugin::update(const PluginInterface& interface, Scene& scene, float deltaTime)
{
    Q_UNUSED(interface);
    Q_UNUSED(scene);
    Q_UNUSED(deltaTime);
}
