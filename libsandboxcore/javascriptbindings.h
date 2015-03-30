#ifndef JAVASCRIPTBINDINGS_H
#define JAVASCRIPTBINDINGS_H

#include "libsandboxcore_global.h"
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

/**
 * @brief The ScriptTimer class
 *
 * @note Timers manage their lifetime automatically.
 * Only interval timers have to be removed explicitly,
 * Otherwise they will only be cleaned up after the corresponding script is destroyed.
 * The update(float) method should be called by the currently active scene to ensure
 * that the script timers are evaluated at the scene's simulation speed.
 */
class ScriptTimer : public QObject
{
    Q_OBJECT

public:
    static int newTimer(int interval,
                        bool oneShot,
                        QScriptEngine& engine,
                        const QScriptValue& function);
    static bool removeTimer(int handle);

    // Updates all active timers. Call this regularily.
    static void updateAll(float deltaTime);

    void update(float deltaTime);

    int getHandle() const;
    QScriptEngine& getEngine();
public slots:
    static void onEngineDestroyed(QObject* engine);
    void timeout();
private:
    static QList<ScriptTimer*> sScriptTimers;
    static int gHandleCounter;

    ScriptTimer(int interval,
                bool oneShot,
                QScriptEngine& engine,
                const QScriptValue& function);

    float mTimeLeft;
    const float mInitialTime;
    const bool mIsOneShot;
    QScriptEngine& mEngine;
    QScriptValue mFunction;
    int mHandle;
};

namespace JavaScriptBindings
{
void DLL_EXPORT addBindings(QScriptEngine& engine, Scene* scene);
void DLL_EXPORT addActorBinding(Actor* actor, QScriptEngine& engine);
void DLL_EXPORT removeActorBinding(Actor* actor, QScriptEngine& engine);
QString DLL_EXPORT cleanIdentifier(const QString& input);
void DLL_EXPORT checkScriptEngineException(QScriptEngine& engine, const QString& context);

void DLL_EXPORT timers_register(QScriptEngine& engine);
void DLL_EXPORT timers_update(float deltaTime); //< To be called by the active scene
QScriptValue script_addTimer_private(QScriptContext *context, QScriptEngine *engine, bool oneShot);
QScriptValue script_removeTimer_private(QScriptContext *context, QScriptEngine *engine);

QScriptValue script_require(QScriptContext *context, QScriptEngine *engine);

QScriptValue script_setTimeout(QScriptContext *context, QScriptEngine *engine);
QScriptValue script_setInterval(QScriptContext *context, QScriptEngine *engine);
QScriptValue script_clearTimeout(QScriptContext *context, QScriptEngine *engine);
QScriptValue script_clearInterval(QScriptContext *context, QScriptEngine *engine);

void QVariantMapPtr_register_prototype(QScriptEngine& engine);

void RangeQueryResult_register_prototype(QScriptEngine& engine);
QScriptValue RangeQueryResultt_prototype_actors(QScriptContext *context, QScriptEngine *engine);

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
QScriptValue Vector3_prototype_normalize(QScriptContext *context, QScriptEngine *engine);
QScriptValue Vector3_prototype_distance(QScriptContext *context, QScriptEngine *engine);
QScriptValue Vector3_prototype_equals(QScriptContext *context, QScriptEngine *engine);
}

#endif // JAVASCRIPTBINDINGS_H
