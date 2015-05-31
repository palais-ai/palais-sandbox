#include "PlanningPlugin.h"
#include "Planning.h"

#include <QList>
#include <QVariant>
#include <QScriptEngine>
#include <OgreSceneManager.h>

Q_DECLARE_METATYPE(QList<QVariantMap>)

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

    qScriptRegisterSequenceMetaType<QList<QVariantMap> >(&engine);

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
