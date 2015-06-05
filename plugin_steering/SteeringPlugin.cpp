#include "SteeringPlugin.h"
#include "Steering.h"
#include <QDebug>

Scene* gCurrentScene = NULL;

Q_DECLARE_METATYPE(Ogre::Vector3)
Q_DECLARE_METATYPE(Ogre::Vector3*)

static ailib::Steering::RaycastResult steering_raycast(const btVector3& origin,
                                                       const btVector3& dir)
{
    RaycastResult res = gCurrentScene->raycast(Ogre::Vector3(origin.x(), origin.y(), origin.z()),
                                               Ogre::Vector3(dir.x(), dir.y(), dir.z()));


    ailib::Steering::RaycastResult retVal;

    QSharedPointer<Actor> ptr = res.actor.toStrongRef();

    retVal.hasHit = !ptr.isNull();
    retVal.distance = res.distance;

    return retVal;
}

static QScriptValue Steering_avoid_obstacle(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::Vector3* pos = qscriptvalue_cast<Ogre::Vector3*>(context->argument(0));
    Ogre::Vector3* vel = qscriptvalue_cast<Ogre::Vector3*>(context->argument(1));
    float maxVel = qscriptvalue_cast<float>(context->argument(2));
    float lookahead = qscriptvalue_cast<float>(context->argument(3));

    btVector3 result = ailib::Steering::avoidObstacle(btVector3(pos->x,pos->y,pos->z),
                                                      btVector3(vel->x,vel->y,vel->z),
                                                      maxVel,
                                                      lookahead,
                                                      &steering_raycast
                                                      );


    Ogre::Vector3 retVal(result.x(), result.y(), result.z());
    return engine->toScriptValue(retVal);
}

SteeringPlugin::SteeringPlugin(QObject *parent) :
    QObject(parent)
{
}

void SteeringPlugin::onLoad(const PluginInterface& interface)
{
    Q_UNUSED(interface);
}

void SteeringPlugin::onUnload(const PluginInterface& interface)
{
    Q_UNUSED(interface);
}

void SteeringPlugin::onSceneStarted(const PluginInterface& interface, Scene& scene)
{
    Q_UNUSED(interface);

    QScriptEngine& engine = scene.getScriptEngine();

    gCurrentScene = &scene;
    engine.globalObject().setProperty("avoidObstacle",
                                   engine.newFunction(Steering_avoid_obstacle));
}

void SteeringPlugin::onSceneEnded(const PluginInterface& interface, Scene& scene)
{
    Q_UNUSED(interface);
    Q_UNUSED(scene);
}

void SteeringPlugin::update(const PluginInterface& interface, Scene& scene, float deltaTime)
{
    Q_UNUSED(interface);
    Q_UNUSED(scene);
    Q_UNUSED(deltaTime);
}
