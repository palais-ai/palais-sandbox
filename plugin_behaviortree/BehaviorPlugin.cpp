#include "BehaviorPlugin.h"
#include "Behavior.h"
#include <OgreSceneManager.h>

using namespace ailib;

Q_DECLARE_METATYPE(Scheduler*)

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

    engine.setDefaultPrototype(qMetaTypeId<Scheduler*>(), engine.newQObject(&mSchedulerWrapper));
    engine.globalObject().setProperty("Scheduler",
                                      engine.toScriptValue(&mScheduler));

    behavior_tree_register_prototypes(engine);
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
    Q_UNUSED(scene);
    mScheduler.update(HighResolutionTime::milliseconds(8), deltaTime);
}
