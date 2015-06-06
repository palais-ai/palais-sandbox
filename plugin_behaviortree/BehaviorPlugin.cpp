#include "BehaviorPlugin.h"
#include "Behavior.h"
#include <OgreSceneManager.h>

Q_DECLARE_METATYPE(ailib::Scheduler*)

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

    engine.setDefaultPrototype(qMetaTypeId<ailib::Scheduler*>(),
                               engine.newQObject(new SchedulerPrototype,
                                                 QScriptEngine::ScriptOwnership));
    engine.globalObject().setProperty("Scheduler",
                                      engine.toScriptValue(&mScheduler));

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
    ailib::HighResolutionTime::Timestamp timeInMs =
            ailib::HighResolutionTime::milliseconds(mScheduler.update(
                                                       ailib::HighResolutionTime::milliseconds(8.f),
                                                       deltaTime));

    if(timeInMs > 16)
    {
        //qDebug() << "Excessive frametime: " << timeInMs;
    }
}
