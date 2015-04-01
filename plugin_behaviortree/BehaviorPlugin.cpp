#include "BehaviorPlugin.h"
#include "Behavior.h"
#include <OgreSceneManager.h>

using namespace ailib;

Q_DECLARE_METATYPE(Scheduler*)

BehaviorPlugin::BehaviorPlugin(QObject *parent) :
    QObject(parent),
    mCurrentEngine(NULL)
{
    ;
}

void BehaviorPlugin::onLoad(const PluginInterface& interface)
{
    Q_UNUSED(interface);
    qRegisterMetaType<Status>("Status");
    qRegisterMetaType<QVariantMap*>("QVariantMap*");
}

void BehaviorPlugin::onUnload(const PluginInterface& interface)
{
    Q_UNUSED(interface);
}

void BehaviorPlugin::onSceneStarted(const PluginInterface& interface, Scene& scene)
{
    Q_UNUSED(interface);
    QScriptEngine& engine = scene.getScriptEngine();
    mCurrentEngine = &engine;

    engine.setDefaultPrototype(qMetaTypeId<Scheduler*>(),
                               engine.newQObject(new SchedulerPrototype,
                                                 QScriptEngine::ScriptOwnership));
    engine.globalObject().setProperty("Scheduler",
                                      engine.toScriptValue(&mScheduler),
                                      QScriptValue::Undeletable | QScriptValue::ReadOnly);

    behavior_tree_register_prototypes(engine);
}

void BehaviorPlugin::onSceneEnded(const PluginInterface& interface, Scene& scene)
{
    Q_UNUSED(interface);
    Q_UNUSED(scene);
    mScheduler.clear();
    mCurrentEngine = NULL;
}

void BehaviorPlugin::update(const PluginInterface& interface, Scene& scene, float deltaTime)
{
    Q_UNUSED(interface);
    Q_UNUSED(scene);
    HighResolutionTime::Timestamp timeInMs =
            HighResolutionTime::milliseconds(mScheduler.update(HighResolutionTime::milliseconds(8.f),
                                                               deltaTime));
    if(timeInMs > 0)
    {
        //qDebug() << "Excessive frametime: " << timeInMs;
    }
}
