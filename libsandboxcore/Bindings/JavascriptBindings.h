#ifndef JAVASCRIPTBINDINGS_H
#define JAVASCRIPTBINDINGS_H

#include "../libsandboxcore_global.h"
#include <QScriptValue>
#include <QList>
#include <QTimer>
#include <QScopedPointer>

class QScriptEngine;
class QScriptContext;
class QTimer;
class Scene;
class Actor;

namespace Ogre
{
class Vector3;
}

namespace JavaScriptBindings
{
void DLL_EXPORT addBindings(QScriptEngine& engine, Scene* scene);
void DLL_EXPORT addActorBinding(Actor* actor, QScriptEngine& engine);
void DLL_EXPORT removeActorBinding(Actor* actor, QScriptEngine& engine);
QString DLL_EXPORT cleanIdentifier(const QString& input);
void DLL_EXPORT checkScriptEngineException(QScriptEngine& engine, const QString& context);

void DLL_EXPORT timers_register(QScriptEngine& engine);
void DLL_EXPORT timers_update(float deltaTime); //< To be called by the active scene

// TODO: Split the bindings into multiple files.
void Random_register(QScriptEngine& engine);
QScriptValue Random_uniform(QScriptContext *context, QScriptEngine *engine);
QScriptValue Random_uniformInt(QScriptContext *context, QScriptEngine *engine);

QScriptValue script_addTimer_private(QScriptContext *context, QScriptEngine *engine, bool oneShot);
QScriptValue script_removeTimer_private(QScriptContext *context, QScriptEngine *engine);

QScriptValue script_require(QScriptContext *context, QScriptEngine *engine);

QScriptValue script_setTimeout(QScriptContext *context, QScriptEngine *engine);
QScriptValue script_setInterval(QScriptContext *context, QScriptEngine *engine);
QScriptValue script_clearTimeout(QScriptContext *context, QScriptEngine *engine);
QScriptValue script_clearInterval(QScriptContext *context, QScriptEngine *engine);

void QVariantMapPtr_register_prototype(QScriptEngine& engine);

void RangeQueryResult_register_prototype(QScriptEngine& engine);
QScriptValue RangeQueryResult_prototype_actors(QScriptContext *context, QScriptEngine *engine);

void RaycastResult_register_prototype(QScriptEngine& engine);
QScriptValue RaycastResult_prototype_distance(QScriptContext *context, QScriptEngine *engine);
QScriptValue RaycastResult_prototype_actor(QScriptContext *context, QScriptEngine *engine);
QScriptValue RaycastResult_prototype_hasHit(QScriptContext *context, QScriptEngine *engine);

void Actor_register_prototype(QScriptEngine& engine);

void Vector3_register_prototype(QScriptEngine& engine);
QScriptValue Vector3_prototype_ctor(QScriptContext *context, QScriptEngine *engine);
QScriptValue Vector3_prototype_x(QScriptContext *context, QScriptEngine *engine);
QScriptValue Vector3_prototype_y(QScriptContext *context, QScriptEngine *engine);
QScriptValue Vector3_prototype_z(QScriptContext *context, QScriptEngine *engine);
QScriptValue Vector3_prototype_toString(QScriptContext *context, QScriptEngine *engine);
QScriptValue Vector3_prototype_add(QScriptContext *context, QScriptEngine *engine);
QScriptValue Vector3_prototype_subtract(QScriptContext *context, QScriptEngine *engine);
QScriptValue Vector3_prototype_multiply(QScriptContext *context, QScriptEngine *engine);
QScriptValue Vector3_prototype_divide(QScriptContext *context, QScriptEngine *engine);
QScriptValue Vector3_prototype_normalize(QScriptContext *context, QScriptEngine *engine);
QScriptValue Vector3_prototype_distance(QScriptContext *context, QScriptEngine *engine);
QScriptValue Vector3_prototype_dot(QScriptContext *context, QScriptEngine *engine);
QScriptValue Vector3_prototype_cross(QScriptContext *context, QScriptEngine *engine);
QScriptValue Vector3_prototype_rotateBy(QScriptContext *context, QScriptEngine *engine);
QScriptValue Vector3_prototype_length(QScriptContext *context, QScriptEngine *engine);
QScriptValue Vector3_prototype_equals(QScriptContext *context, QScriptEngine *engine);

void Quaternion_register_prototype(QScriptEngine& engine);
QScriptValue Quaternion_prototype_ctor(QScriptContext *context, QScriptEngine *engine);
QScriptValue Quaternion_prototype_x(QScriptContext *context, QScriptEngine *engine);
QScriptValue Quaternion_prototype_y(QScriptContext *context, QScriptEngine *engine);
QScriptValue Quaternion_prototype_z(QScriptContext *context, QScriptEngine *engine);
QScriptValue Quaternion_prototype_w(QScriptContext *context, QScriptEngine *engine);
QScriptValue Quaternion_prototype_normalize(QScriptContext *context, QScriptEngine *engine);
QScriptValue Quaternion_prototype_inverse(QScriptContext *context, QScriptEngine *engine);
QScriptValue Quaternion_prototype_multiply(QScriptContext *context, QScriptEngine *engine);
QScriptValue Quaternion_prototype_slerp(QScriptContext *context, QScriptEngine *engine);
QScriptValue Quaternion_prototype_nlerp(QScriptContext *context, QScriptEngine *engine);
QScriptValue Quaternion_prototype_length(QScriptContext *context, QScriptEngine *engine);
QScriptValue Quaternion_prototype_toString(QScriptContext *context, QScriptEngine *engine);
QScriptValue Quaternion_prototype_equals(QScriptContext *context, QScriptEngine *engine);

void Color_register_prototype(QScriptEngine& engine);
QScriptValue Color_prototype_ctor(QScriptContext *context, QScriptEngine *engine);
QScriptValue Color_prototype_r(QScriptContext *context, QScriptEngine *engine);
QScriptValue Color_prototype_g(QScriptContext *context, QScriptEngine *engine);
QScriptValue Color_prototype_b(QScriptContext *context, QScriptEngine *engine);
QScriptValue Color_prototype_a(QScriptContext *context, QScriptEngine *engine);
QScriptValue Color_prototype_toString(QScriptContext *context, QScriptEngine *engine);
}

#endif // JAVASCRIPTBINDINGS_H
