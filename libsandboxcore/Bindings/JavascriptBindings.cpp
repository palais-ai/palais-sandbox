#include "JavascriptBindings.h"
#include "../Actor.h"
#include "../Scene.h"
#include "../DebugDrawer.h"
#include "ScriptTimer.h"
#include <QScriptEngine>
#include <QTextStream>
#include <QDebug>
#include <QList>
#include <QVector>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QScriptContextInfo>
#include <OgreQuaternion.h>
#include <OgreVector3.h>

Q_DECLARE_METATYPE(Actor*)
Q_DECLARE_METATYPE(DebugDrawer*)
Q_DECLARE_METATYPE(Ogre::Vector3)
Q_DECLARE_METATYPE(Ogre::Vector3*)
Q_DECLARE_METATYPE(QVector<Ogre::Vector3>)
Q_DECLARE_METATYPE(QVector<Ogre::Vector3*>)
Q_DECLARE_METATYPE(Ogre::Quaternion)
Q_DECLARE_METATYPE(Ogre::Quaternion*)
Q_DECLARE_METATYPE(Ogre::ColourValue)
Q_DECLARE_METATYPE(Ogre::ColourValue*)
Q_DECLARE_METATYPE(RaycastResult)
Q_DECLARE_METATYPE(RaycastResult*)
Q_DECLARE_METATYPE(RangeQueryResult)
Q_DECLARE_METATYPE(RangeQueryResult*)
Q_DECLARE_METATYPE(QVariantMap*)

namespace JavaScriptBindings
{
static QScopedPointer<ScriptTimerFactory> gTimerFactory;

static void installModuleSystem(QScriptEngine& engine, Scene* scene)
{
    QUrl url = QUrl::fromLocalFile(scene->getLogicFile());
    const QString currentWorkingDirectory = url.adjusted(QUrl::RemoveFilename).toLocalFile();
    QStringList path;
    path << currentWorkingDirectory;
    engine.globalObject().setProperty("__PATH__", engine.toScriptValue(path));
    engine.globalObject().setProperty("__MODULES__", engine.toScriptValue(QStringList()));
}

void addBindings(QScriptEngine& engine, Scene* scene)
{
    if(!scene)
    {
        qWarning("Can't install bindings on an uninitialized scene.");
        return;
    }

    installModuleSystem(engine, scene);
    timers_register(engine);

    Random_register(engine);
    Actor_register_prototype(engine);
    qRegisterMetaType<DebugDrawer*>("DebugDrawer*");

    QScriptValue sceneVal = engine.newQObject(scene);

    engine.globalObject().setProperty("Scene", sceneVal);
    engine.globalObject().setProperty("require", engine.newFunction(script_require));

    Vector3_register_prototype(engine);
    Quaternion_register_prototype(engine);
    RangeQueryResult_register_prototype(engine);
    RaycastResult_register_prototype(engine);
    Color_register_prototype(engine);
}

static bool loadScript(QScriptEngine *engine, const QString& filename)
{
    QFile file(filename);

    if(!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open include file [" << filename << "].";
        return false;
    }

    QByteArray ba = file.readAll();
    file.close();

    // Set context to parent context
    // See http://www.qtcentre.org/threads/20432-Can-I-include-a-script-from-script
    QScriptContext *context = engine->currentContext();
    QScriptContext *parent = context->parentContext();

    if(parent != 0)
    {
        context->setActivationObject(context->parentContext()->activationObject());
        context->setThisObject(context->parentContext()->thisObject());
    }

    engine->evaluate(ba, filename);
    checkScriptEngineException(*engine, "loading [" + filename + "]");
    return true;
}

static void loadScriptIfNotLoaded(QScriptEngine *engine, const QString& filename)
{
    QScriptValue modulesValue = engine->globalObject().property("__MODULES__");
    QStringList modules = engine->fromScriptValue<QStringList>(modulesValue);

    if(!modules.contains(filename))
    {
        loadScript(engine, filename);
        modules << filename;
        engine->globalObject().setProperty("__MODULES__", engine->toScriptValue(modules));
        qDebug() << "Loaded " << "[" << filename << "].";
    }
    else
    {
        qDebug() << "[" << filename << "] was already loaded.";
    }
}

QScriptValue script_require(QScriptContext *context, QScriptEngine *engine)
{
    if(context->argumentCount() > 0)
    {
        QScriptValue arg0 = context->argument(0);

        if(arg0.isString())
        {
            const QString filename = arg0.toString();
            const QFileInfo fInfo(filename);
            QScriptValue pathValue = engine->globalObject().property("__PATH__");
            const QStringList paths = engine->fromScriptValue<QStringList>(pathValue);
            bool loaded = false;
            foreach(QString dir, paths)
            {
                QFileInfo info(dir);

                if(info.exists())
                {
                    QDir includeDir = info.dir();
                    includeDir.cd(fInfo.dir().path());
                    QString filepath = includeDir.absoluteFilePath(fInfo.fileName());
                    if(QFileInfo(filepath).exists())
                    {
                        loadScriptIfNotLoaded(engine, filepath);
                        loaded = true;
                        break;
                    }
                }
                else
                {
                    qWarning() << "__PATH__ directory [" << dir << "] doesn't exist."
                               << "Your __PATH__ is misconfigured.";
                }
            }

            if(!loaded)
            {
                qWarning() << "The module [" << filename << "] could not be loaded. "
                           << "It wasn't found in your __PATH__.";
            }
        }
        else
        {
            qWarning() << "The argument to require() must be a string. The command was ignored.";
        }
    }
    else
    {
        qWarning() << "require() needs to have an argument.";
    }
    return engine->undefinedValue();
}

void timers_register(QScriptEngine& engine)
{
    gTimerFactory.reset(new ScriptTimerFactory);

    engine.globalObject().setProperty("setTimeout",
                                      engine.newFunction(script_setTimeout));
    engine.globalObject().setProperty("setInterval",
                                      engine.newFunction(script_setInterval));
    engine.globalObject().setProperty("clearTimeout",
                                      engine.newFunction(script_clearTimeout));
    engine.globalObject().setProperty("clearInterval",
                                      engine.newFunction(script_clearInterval));
}

void timers_update(float deltaTime)
{
    gTimerFactory->updateTimers(deltaTime);
}

void Random_register(QScriptEngine& engine)
{
    // Reset the random seed.
    qsrand(777);

    QScriptValue random = engine.newObject();

    random.setProperty("uniform", engine.newFunction(Random_uniform));
    random.setProperty("uniformInt", engine.newFunction(Random_uniformInt));
    engine.globalObject().setProperty("Random", random);
}

QScriptValue Random_uniform(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(engine);

    float low = 0, high = 1;
    if(context->argumentCount() > 0)
    {
        low = context->argument(0).toNumber();

        if(context->argumentCount() > 1)
        {
            high = context->argument(1).toNumber();
        }
    }

    return low + static_cast<float>(qrand()) / static_cast<float>(RAND_MAX / (high - low));
}

QScriptValue Random_uniformInt(QScriptContext *context, QScriptEngine *engine)
{
    return floorf(Random_uniform(context, engine).toNumber() + 0.5);
}

void checkScriptEngineException(QScriptEngine& engine, const QString& context)
{
    if(engine.hasUncaughtException())
    {
        engine.uncaughtExceptionLineNumber();
        if(context.isEmpty())
        {
            qCritical() << "Exception in loaded logic file "
                        << ", ERROR:"
                        << engine.uncaughtException().toString()
                        << ", on line "
                        << engine.uncaughtExceptionLineNumber()
                        << ", backtrace: "
                        << engine.uncaughtExceptionBacktrace().join("\n");
        }
        else
        {
            qCritical() << "Exception in "
                        << context
                        << ", ERROR:"
                        << engine.uncaughtException().toString()
                        << ", on line "
                        << engine.uncaughtExceptionLineNumber()
                        << ", backtrace: "
                        << engine.uncaughtExceptionBacktrace().join("\n");
        }
        engine.clearExceptions();
    }
}

QScriptValue script_addTimer_private(QScriptContext *context, QScriptEngine *engine, bool oneShot)
{
    if(context->argumentCount() == 2)
    {
        if(context->argument(0).isNumber())
        {
            int interval = context->argument(0).toInteger();

            if(context->argument(1).isFunction())
            {
                QScriptValue function = context->argument(1).toObject();

                return gTimerFactory->newTimer(interval, oneShot, function);
            }
            else
            {
                qWarning() << "The addTimer function requires a function as its second argument.";
                return engine->undefinedValue();
            }
        }
        else
        {
            qWarning() << "The addTimer function requires an integer as its first argument.";
            return engine->undefinedValue();
        }
    }
    else
    {
        qWarning() << "The addTimer function requires 2 arguments.";
        return engine->undefinedValue();
    }
}


QScriptValue script_removeTimer_private(QScriptContext *context, QScriptEngine *engine)
{
    if(context->argumentCount() == 1)
    {
        if(context->argument(0).isNumber())
        {
            int handle = context->argument(0).toInteger();

            bool didRemove = gTimerFactory->removeTimer(handle);

            if(!didRemove)
            {
                qWarning() << "clearTimeout / clearInterval : Failed to remove timer by handle [ " << handle << " ].";
            }
            return didRemove;
        }
        else
        {
            qWarning() << "clearTimeout / clearInterval : This function requires an integer as its first argument.";
            return engine->undefinedValue();
        }
    }
    else
    {
        qWarning() << "clearTimeout / clearInterval : This function requires 1 argument.";
        return engine->undefinedValue();
    }
}

QScriptValue script_setTimeout(QScriptContext *context, QScriptEngine *engine)
{
    return script_addTimer_private(context, engine, true);
}

QScriptValue script_setInterval(QScriptContext *context, QScriptEngine *engine)
{
    return script_addTimer_private(context, engine, false);
}

QScriptValue script_clearTimeout(QScriptContext *context, QScriptEngine *engine)
{
    return script_removeTimer_private(context, engine);
}

QScriptValue script_clearInterval(QScriptContext *context, QScriptEngine *engine)
{
    return script_removeTimer_private(context, engine);
}

//class VariantMapPrototype : public QScriptable, public QObject
// FIXME: Add dynamic wrapper for qvariantmap* - using QScriptClass maybe?
void QVariantMapPtr_register_prototype(QScriptEngine& engine)
{
    QScriptValue obj = engine.newObject();

    ;

    engine.setDefaultPrototype(qMetaTypeId<QVariantMap*>(), obj);
}

void RangeQueryResult_register_prototype(QScriptEngine& engine)
{
    QScriptValue obj = engine.newObject();

    obj.setProperty("actors", engine.newFunction(RangeQueryResult_prototype_actors),
                    QScriptValue::PropertyGetter | QScriptValue::ReadOnly);

    engine.setDefaultPrototype(qMetaTypeId<RangeQueryResult>(), obj);
    engine.setDefaultPrototype(qMetaTypeId<RangeQueryResult*>(), obj);
}

QScriptValue RangeQueryResult_prototype_actors(QScriptContext *context, QScriptEngine *engine)
{
    RangeQueryResult* res = qscriptvalue_cast<RangeQueryResult*>(context->thisObject());

    if (!res)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "RangeQueryResult.prototype.actors: \
                                    this object is not a RangeQueryResult");
    }

    QScriptValue retVal = engine->newArray();
    quint32 i = 0;
    foreach(QWeakPointer<Actor> actor, res->actors)
    {
        QSharedPointer<Actor> strong = actor.toStrongRef();
        if(!strong)
        {
            qWarning("RangeQueryResult.prototype.actors: Encountered NULL Actor. Skipping.");
            continue;
        }

        retVal.setProperty(i++, engine->newQObject(strong.data(),
                                                   QScriptEngine::QtOwnership,
                                                   QScriptEngine::PreferExistingWrapperObject));
    }
    return retVal;
}

void RaycastResult_register_prototype(QScriptEngine& engine)
{
    QScriptValue obj = engine.newObject();

    obj.setProperty("distance", engine.newFunction(RaycastResult_prototype_distance),
                    QScriptValue::PropertyGetter | QScriptValue::ReadOnly);
    obj.setProperty("actor", engine.newFunction(RaycastResult_prototype_actor),
                    QScriptValue::PropertyGetter | QScriptValue::ReadOnly);
    obj.setProperty("hasHit", engine.newFunction(RaycastResult_prototype_hasHit),
                    QScriptValue::PropertyGetter | QScriptValue::ReadOnly);
    obj.setProperty("toString",
                    engine.evaluate("(function() {return 'RaycastResult @value ( \
                                     hasHit: ' + this.hasHit + ', distance: ' \
                                    + this.distance + ', actor: ' + this.actor + ')'; })"));

    engine.setDefaultPrototype(qMetaTypeId<RaycastResult>(), obj);
    engine.setDefaultPrototype(qMetaTypeId<RaycastResult*>(), obj);
}

QScriptValue RaycastResult_prototype_distance(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(engine);

    RaycastResult* res = qscriptvalue_cast<RaycastResult*>(context->thisObject());
    if (!res)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "RaycastResult.prototype.distance: \
                                    this object is not a RaycastResult");
    }
    return res->distance;
}

QScriptValue RaycastResult_prototype_actor(QScriptContext *context, QScriptEngine *engine)
{
    RaycastResult* res = qscriptvalue_cast<RaycastResult*>(context->thisObject());

    if (!res)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "RaycastResult.prototype.distance: \
                                    this object is not a RaycastResult");
    }

    QSharedPointer<Actor> strong = res->actor.toStrongRef();
    if(!strong)
    {
        return engine->undefinedValue();
    }

    return engine->newQObject(strong.data(),
                              QScriptEngine::QtOwnership,
                              QScriptEngine::PreferExistingWrapperObject);
}

QScriptValue RaycastResult_prototype_hasHit(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(engine);

    RaycastResult* res = qscriptvalue_cast<RaycastResult*>(context->thisObject());
    if (!res)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "RaycastResult.prototype.distance: \
                                    this object is not a RaycastResult");
    }
    return res->actor != NULL;
}


void Actor_register_prototype(QScriptEngine& engine)
{
    Q_UNUSED(engine);
    qRegisterMetaType<Actor*>("Actor*");
}

void Vector3_register_prototype(QScriptEngine& engine)
{
    engine.setDefaultPrototype(qMetaTypeId<Ogre::Vector3*>(), QScriptValue());
    QScriptValue obj = engine.newVariant(QVariant::fromValue((Ogre::Vector3*)0));

    obj.setProperty("x", engine.newFunction(Vector3_prototype_x),
                    QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    obj.setProperty("y", engine.newFunction(Vector3_prototype_y),
                    QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    obj.setProperty("z", engine.newFunction(Vector3_prototype_z),
                    QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    obj.setProperty("toString", engine.newFunction(Vector3_prototype_toString));
    obj.setProperty("add", engine.newFunction(Vector3_prototype_add));
    obj.setProperty("subtract", engine.newFunction(Vector3_prototype_subtract));
    obj.setProperty("multiply", engine.newFunction(Vector3_prototype_multiply));
    obj.setProperty("divide", engine.newFunction(Vector3_prototype_divide));
    obj.setProperty("normalize", engine.newFunction(Vector3_prototype_normalize));
    obj.setProperty("distanceTo", engine.newFunction(Vector3_prototype_distance));
    obj.setProperty("dot", engine.newFunction(Vector3_prototype_dot));
    obj.setProperty("cross", engine.newFunction(Vector3_prototype_cross));
    obj.setProperty("rotateBy", engine.newFunction(Vector3_prototype_rotateBy));
    obj.setProperty("length", engine.newFunction(Vector3_prototype_length));
    obj.setProperty("equals", engine.newFunction(Vector3_prototype_equals));

    engine.setDefaultPrototype(qMetaTypeId<Ogre::Vector3>(), obj);
    engine.setDefaultPrototype(qMetaTypeId<Ogre::Vector3*>(), obj);

    engine.globalObject().setProperty("Vector3",
                                      engine.newFunction(Vector3_prototype_ctor));

    qScriptRegisterSequenceMetaType<QVector<Ogre::Vector3> >(&engine);
    qScriptRegisterSequenceMetaType<QVector<Ogre::Vector3*> >(&engine);

    engine.globalObject().setProperty("Vectors",
                                      engine.evaluate("({'UNIT_X': new Vector3(0,0,1),"
                                                      " 'UNIT_Y': new Vector3(0,1,0),"
                                                      " 'UNIT_Z'  : new Vector3(0,0,1)})"));
}

QScriptValue Vector3_prototype_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->isCalledAsConstructor())
    {
        float x = 0, y = 0, z = 0;

        if (context->argumentCount() > 0)
        {
            x = context->argument(0).toNumber();

            if (context->argumentCount() > 1)
            {
                y = context->argument(1).toNumber();

                if (context->argumentCount() > 2)
                {
                    z = context->argument(2).toNumber();
                }
            }
        }

        Ogre::Vector3 v(x,y,z);

        return engine->toScriptValue(v);
    }
    else
    {
        qWarning("Vector3.ctor: Please use the 'new' operator.");
        return engine->undefinedValue();
    }
}

QScriptValue Vector3_prototype_x(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::Vector3* v = qscriptvalue_cast<Ogre::Vector3*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Vector3.prototype.x: \
                                    this object is not a Ogre::Vector3");
    }

    QScriptValue obj = context->thisObject();
    QScriptValue data = obj.data();
    if (!data.isValid())
    {
        data = engine->newObject();
        obj.setData(data);
    }

    QScriptValue result;
    if (context->argumentCount() >= 1)
    {
        float val = context->argument(0).toNumber();
        result = val;
        v->x = val;
    }
    else
    {
        result = v->x;
    }

    return result;
}

QScriptValue Vector3_prototype_y(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::Vector3* v = qscriptvalue_cast<Ogre::Vector3*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Vector3.prototype.y: \
                                   this object is not a Ogre::Vector3");
    }

    QScriptValue obj = context->thisObject();
    QScriptValue data = obj.data();
    if (!data.isValid())
    {
        data = engine->newObject();
        obj.setData(data);
    }

    QScriptValue result;
    if (context->argumentCount() >= 1)
    {
        float val = context->argument(0).toNumber();
        result = val;
        v->y = val;
    }
    else
    {
        result = v->y;
    }

    return result;
}

QScriptValue Vector3_prototype_z(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::Vector3* v = qscriptvalue_cast<Ogre::Vector3*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Vector3.prototype.z: \
                                    this object is not a Ogre::Vector3");
    }

    QScriptValue obj = context->thisObject();
    QScriptValue data = obj.data();
    if (!data.isValid())
    {
        data = engine->newObject();
        obj.setData(data);
    }

    QScriptValue result;
    if (context->argumentCount() >= 1)
    {
        float val = context->argument(0).toNumber();
        result = val;
        v->z = val;
    }
    else
    {
        result = v->z;
    }

    return result;
}

QScriptValue Vector3_prototype_toString(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(engine);
    Ogre::Vector3* v = qscriptvalue_cast<Ogre::Vector3*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Vector3.prototype.toString: \
                                    this object is not a Ogre::Vector3");
    }

    return QString("Vector3 (x: %1, y: %2, z: %3)")
                  .arg(v->x)
                  .arg(v->y)
                  .arg(v->z);
}

QScriptValue Vector3_prototype_add(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::Vector3* v = qscriptvalue_cast<Ogre::Vector3*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Vector3.prototype.add: \
                                    this object is not a Ogre::Vector3");
    }

    if (context->argumentCount() >= 1)
    {
        Ogre::Vector3* v2 = qscriptvalue_cast<Ogre::Vector3*>(context->argument(0));
        if (!v2)
        {
            if(context->argument(0).isNumber())
            {
                *v += context->argument(0).toNumber();

                return engine->toScriptValue(v);
            }

            return context->throwError(QScriptContext::TypeError,
                                       "Vector3.prototype.add: \
                                        Argument #0 object is not a Ogre::Vector3 or number.");
        }

        *v += *v2;

        return engine->toScriptValue(v);
    }

    return engine->undefinedValue();
}

QScriptValue Vector3_prototype_subtract(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::Vector3* v = qscriptvalue_cast<Ogre::Vector3*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Vector3.prototype.subtract: \
                                    this object is not a Ogre::Vector3 or number.");
    }

    if (context->argumentCount() >= 1)
    {
        if(context->argument(0).isNumber())
        {
            *v -= context->argument(0).toNumber();

            return engine->toScriptValue(v);
        }

        Ogre::Vector3* v2 = qscriptvalue_cast<Ogre::Vector3*>(context->argument(0));
        if (!v2)
        {
            return context->throwError(QScriptContext::TypeError,
                                       "Vector3.prototype.subtract: \
                                        Argument #0 object is not a Ogre::Vector3 or number.");
        }

        *v -= *v2;

        return engine->toScriptValue(v);
    }

    return engine->undefinedValue();
}

QScriptValue Vector3_prototype_multiply(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::Vector3* v = qscriptvalue_cast<Ogre::Vector3*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Vector3.prototype.multiply: \
                                    this object is not a Ogre::Vector3 or number.");
    }

    if (context->argumentCount() >= 1)
    {
        if(context->argument(0).isNumber())
        {
            *v *= context->argument(0).toNumber();

            return engine->toScriptValue(v);
        }

        Ogre::Vector3* v2 = qscriptvalue_cast<Ogre::Vector3*>(context->argument(0));
        if (!v2)
        {
            return context->throwError(QScriptContext::TypeError,
                                       "Vector3.prototype.multiply:\
                                        Argument #0 object is not a Ogre::Vector3 or a number.");
        }

        *v *= *v2;

        return engine->toScriptValue(v);
    }

    return engine->undefinedValue();
}

QScriptValue Vector3_prototype_divide(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::Vector3* v = qscriptvalue_cast<Ogre::Vector3*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Vector3.prototype.divide: \
                                    this object is not a Ogre::Vector3 or number.");
    }

    if (context->argumentCount() >= 1)
    {
        if(context->argument(0).isNumber())
        {
            *v /= context->argument(0).toNumber();

            return engine->toScriptValue(v);
        }

        Ogre::Vector3* v2 = qscriptvalue_cast<Ogre::Vector3*>(context->argument(0));
        if (!v2)
        {
            return context->throwError(QScriptContext::TypeError,
                                       "Vector3.prototype.divide:\
                                        Argument #0 object is not a Ogre::Vector3 or a number.");
        }

        *v /= *v2;

        return engine->toScriptValue(v);
    }

    return engine->undefinedValue();
}

QScriptValue Vector3_prototype_normalize(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::Vector3* v = qscriptvalue_cast<Ogre::Vector3*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Vector3.prototype.normalize: \
                                    this object is not a Ogre::Vector3");
    }

    v->normalise();

    return engine->toScriptValue(v);
}


QScriptValue Vector3_prototype_distance(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::Vector3* v = qscriptvalue_cast<Ogre::Vector3*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Vector3.prototype.distance: \
                                    this object is not a Ogre::Vector3");
    }

    if (context->argumentCount() >= 1)
    {
        Ogre::Vector3* v2 = qscriptvalue_cast<Ogre::Vector3*>(context->argument(0));
        if (!v2)
        {
            return context->throwError(QScriptContext::TypeError,
                                       "Vector3.prototype.distance:\
                                        Argument #0 object is not a Ogre::Vector3");
        }

        return v->distance(*v2);
    }

    return engine->undefinedValue();
}

QScriptValue Vector3_prototype_dot(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::Vector3* v = qscriptvalue_cast<Ogre::Vector3*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Vector3.prototype.dot: \
                                    this object is not a Ogre::Vector3");
    }

    if (context->argumentCount() >= 1)
    {
        Ogre::Vector3* v2 = qscriptvalue_cast<Ogre::Vector3*>(context->argument(0));
        if (!v2)
        {
            return context->throwError(QScriptContext::TypeError,
                                       "Vector3.prototype.dot:\
                                        Argument #0 object is not a Ogre::Vector3");
        }

        return engine->toScriptValue(v->dotProduct(*v2));
    }

    return engine->undefinedValue();
}

QScriptValue Vector3_prototype_cross(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::Vector3* v = qscriptvalue_cast<Ogre::Vector3*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Vector3.prototype.cross: \
                                    this object is not a Ogre::Vector3");
    }

    if (context->argumentCount() >= 1)
    {
        Ogre::Vector3* v2 = qscriptvalue_cast<Ogre::Vector3*>(context->argument(0));
        if (!v2)
        {
            return context->throwError(QScriptContext::TypeError,
                                       "Vector3.prototype.cross:\
                                        Argument #0 object is not a Ogre::Vector3");
        }

        return engine->toScriptValue(v->crossProduct(*v2));
    }

    return engine->undefinedValue();
}

QScriptValue Vector3_prototype_rotateBy(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::Vector3* v = qscriptvalue_cast<Ogre::Vector3*>(context->thisObject());
    if (!v)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Vector3.prototype.rotateBy: \
                                    this object is not a Ogre::Vector3");
    }

    if (context->argumentCount() >= 1)
    {
        Ogre::Quaternion* q = qscriptvalue_cast<Ogre::Quaternion*>(context->argument(0));
        if (!q)
        {
            return context->throwError(QScriptContext::TypeError,
                                       "Vector3.prototype.rotateBy:\
                                        Argument #0 object is not a Ogre::Quaternion");
        }
        return engine->toScriptValue(*q * *v);
    }

    return engine->undefinedValue();
}

QScriptValue Vector3_prototype_length(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(engine);
    Ogre::Vector3* v = qscriptvalue_cast<Ogre::Vector3*>(context->thisObject());
    if (!v)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Vector3.prototype.length: \
                                    this object is not a Ogre::Vector3");
    }

    return v->length();
}

QScriptValue Vector3_prototype_equals(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::Vector3* v = qscriptvalue_cast<Ogre::Vector3*>(context->thisObject());
    if (!v)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Vector3.prototype.equals: \
                                    this object is not a Ogre::Vector3");
    }

    if (context->argumentCount() >= 1)
    {
        Ogre::Vector3* v2 = qscriptvalue_cast<Ogre::Vector3*>(context->argument(0));
        if (!v2)
        {
            return context->throwError(QScriptContext::TypeError,
                                       "Vector3.prototype.equals:\
                                        Argument #0 object is not a Ogre::Vector3");
        }

        return v->distance(*v2) < 0.001f;
    }
    return engine->undefinedValue();
}

void Quaternion_register_prototype(QScriptEngine& engine)
{
    engine.setDefaultPrototype(qMetaTypeId<Ogre::Quaternion*>(), QScriptValue());
    QScriptValue obj = engine.newVariant(QVariant::fromValue((Ogre::Quaternion*)0));

    obj.setProperty("w", engine.newFunction(Quaternion_prototype_w),
                    QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    obj.setProperty("x", engine.newFunction(Quaternion_prototype_x),
                    QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    obj.setProperty("y", engine.newFunction(Quaternion_prototype_y),
                    QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    obj.setProperty("z", engine.newFunction(Quaternion_prototype_z),
                    QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    obj.setProperty("normalize", engine.newFunction(Quaternion_prototype_normalize));
    obj.setProperty("inverse", engine.newFunction(Quaternion_prototype_inverse));
    obj.setProperty("multiply", engine.newFunction(Quaternion_prototype_multiply));
    obj.setProperty("slerp", engine.newFunction(Quaternion_prototype_slerp));
    obj.setProperty("nlerp", engine.newFunction(Quaternion_prototype_nlerp));
    obj.setProperty("toString", engine.newFunction(Quaternion_prototype_toString));
    obj.setProperty("equals", engine.newFunction(Quaternion_prototype_equals));

    engine.setDefaultPrototype(qMetaTypeId<Ogre::Quaternion>(), obj);
    engine.setDefaultPrototype(qMetaTypeId<Ogre::Quaternion*>(), obj);

    engine.globalObject().setProperty("Quaternion",
                                      engine.newFunction(Quaternion_prototype_ctor));

    qScriptRegisterSequenceMetaType<QVector<Ogre::Quaternion> >(&engine);
    qScriptRegisterSequenceMetaType<QVector<Ogre::Quaternion*> >(&engine);
}

QScriptValue Quaternion_prototype_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->isCalledAsConstructor())
    {
        Ogre::Quaternion q;

        if(context->argumentCount() > 0)
        {
            Ogre::Vector3* axis = qscriptvalue_cast<Ogre::Vector3*>(context->argument(0));
            if(axis)
            {
                float angle = 0;
                if(context->argumentCount() > 1 && context->argument(1).isNumber())
                {
                    angle = context->argument(1).toNumber();
                }
                q = Ogre::Quaternion(Ogre::Degree(angle), *axis);
            }
            else if (context->argument(0).isNumber())
            {
                float x = 0, y = 0, z = 0, w = 0;
                w = context->argument(0).toNumber();

                if (context->argumentCount() > 1 && context->argument(1).isNumber())
                {
                    x = context->argument(1).toNumber();

                    if (context->argumentCount() > 2 && context->argument(2).isNumber())
                    {
                        y = context->argument(2).toNumber();

                        if(context->argumentCount() > 3 && context->argument(3).isNumber())
                        {
                            z = context->argument(3).toNumber();
                        }
                    }
                }

                q = Ogre::Quaternion(w,x,y,z);
            }
        }

        return engine->toScriptValue(q);
    }
    else
    {
        qWarning("Quaternion.ctor: Please use the 'new' operator.");
        return engine->undefinedValue();
    }
}

QScriptValue Quaternion_prototype_x(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::Quaternion* q = qscriptvalue_cast<Ogre::Quaternion*>(context->thisObject());
    if (!q)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Quaternion.prototype.x: \
                                    this object is not a Ogre::Quaternion");
    }

    QScriptValue obj = context->thisObject();
    QScriptValue data = obj.data();
    if (!data.isValid())
    {
        data = engine->newObject();
        obj.setData(data);
    }

    QScriptValue result;
    if (context->argumentCount() >= 1)
    {
        float val = context->argument(0).toNumber();
        result = val;
        q->x = val;
    }
    else
    {
        result = q->x;
    }

    return result;
}

QScriptValue Quaternion_prototype_y(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::Quaternion* q = qscriptvalue_cast<Ogre::Quaternion*>(context->thisObject());
    if (!q)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Quaternion.prototype.x: \
                                    this object is not a Ogre::Quaternion");
    }

    QScriptValue obj = context->thisObject();
    QScriptValue data = obj.data();
    if (!data.isValid())
    {
        data = engine->newObject();
        obj.setData(data);
    }

    QScriptValue result;
    if (context->argumentCount() >= 1)
    {
        float val = context->argument(0).toNumber();
        result = val;
        q->y = val;
    }
    else
    {
        result = q->y;
    }

    return result;
}

QScriptValue Quaternion_prototype_z(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::Quaternion* q = qscriptvalue_cast<Ogre::Quaternion*>(context->thisObject());
    if (!q)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Quaternion.prototype.x: \
                                    this object is not a Ogre::Quaternion");
    }

    QScriptValue obj = context->thisObject();
    QScriptValue data = obj.data();
    if (!data.isValid())
    {
        data = engine->newObject();
        obj.setData(data);
    }

    QScriptValue result;
    if (context->argumentCount() >= 1)
    {
        float val = context->argument(0).toNumber();
        result = val;
        q->z = val;
    }
    else
    {
        result = q->z;
    }

    return result;
}

QScriptValue Quaternion_prototype_w(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::Quaternion* q = qscriptvalue_cast<Ogre::Quaternion*>(context->thisObject());
    if (!q)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Quaternion.prototype.x: \
                                    this object is not a Ogre::Quaternion");
    }

    QScriptValue obj = context->thisObject();
    QScriptValue data = obj.data();
    if (!data.isValid())
    {
        data = engine->newObject();
        obj.setData(data);
    }

    QScriptValue result;
    if (context->argumentCount() >= 1)
    {
        float val = context->argument(0).toNumber();
        result = val;
        q->w = val;
    }
    else
    {
        result = q->w;
    }

    return result;
}

QScriptValue Quaternion_prototype_normalize(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::Quaternion* q = qscriptvalue_cast<Ogre::Quaternion*>(context->thisObject());
    if (!q)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Quaternion.prototype.normalize: \
                                    this object is not a Ogre::Quaternion");
    }
    q->normalise();
    return engine->toScriptValue(q);
}

QScriptValue Quaternion_prototype_inverse(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::Quaternion* q = qscriptvalue_cast<Ogre::Quaternion*>(context->thisObject());
    if (!q)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Quaternion.prototype.inverse: \
                                    this object is not a Ogre::Quaternion");
    }
    return engine->toScriptValue(q->Inverse());
}

QScriptValue Quaternion_prototype_multiply(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::Quaternion* q = qscriptvalue_cast<Ogre::Quaternion*>(context->thisObject());
    if (!q)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Quaternion.prototype.multiply: \
                                    this object is not a Ogre::Quaternion or number.");
    }

    if (context->argumentCount() >= 1)
    {
        Ogre::Quaternion* q2 = qscriptvalue_cast<Ogre::Quaternion*>(context->argument(0));
        if (!q2)
        {
            return context->throwError(QScriptContext::TypeError,
                                       "Quaternion.prototype.multiply:\
                                        Argument #0 object is not a Ogre::Quaternion or a number or number.");
        }

        *q = *q * *q2;

        return engine->toScriptValue(q);
    }
    return engine->undefinedValue();
}

QScriptValue Quaternion_prototype_slerp(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::Quaternion* q = qscriptvalue_cast<Ogre::Quaternion*>(context->thisObject());
    if (!q)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Quaternion.prototype.slerp: \
                                    this object is not a Ogre::Quaternion or number.");
    }

    if (context->argumentCount() >= 2)
    {
        float t = 0;
        if(context->argument(0).isNumber())
        {
            t = context->argument(0).toNumber();
        }
        else
        {
            return context->throwError(QScriptContext::TypeError,
                                       "Quaternion.prototype.slerp:\
                                        Argument #0 object is not a number.");
        }

        Ogre::Quaternion* q2 = qscriptvalue_cast<Ogre::Quaternion*>(context->argument(1));
        if (!q2)
        {
            return context->throwError(QScriptContext::TypeError,
                                       "Quaternion.prototype.slerp:\
                                        Argument #1 object is not a Ogre::Quaternion.");
        }

        bool shortestPath = true;
        if(context->argumentCount() > 2 && context->argument(2).isBool())
        {
            shortestPath = context->argument(2).toBool();
        }

        return engine->toScriptValue(Ogre::Quaternion::Slerp(t, *q, *q2, shortestPath));
    }
    else
    {
        qWarning() << "Quaternion.prototype.slerp: Must be called with 2 arguments: "
                   << "The interpolation point t on [0,1] and the quaternion to rotate to.";
    }
    return engine->undefinedValue();
}

QScriptValue Quaternion_prototype_nlerp(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::Quaternion* q = qscriptvalue_cast<Ogre::Quaternion*>(context->thisObject());
    if (!q)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Quaternion.prototype.nlerp: \
                                    this object is not a Ogre::Quaternion or number.");
    }

    if (context->argumentCount() >= 2)
    {
        float t = 0;
        if(context->argument(0).isNumber())
        {
            t = context->argument(0).toNumber();
        }
        else
        {
            return context->throwError(QScriptContext::TypeError,
                                       "Quaternion.prototype.nlerp:\
                                        Argument #0 object is not a number.");
        }

        Ogre::Quaternion* q2 = qscriptvalue_cast<Ogre::Quaternion*>(context->argument(1));
        if (!q2)
        {
            return context->throwError(QScriptContext::TypeError,
                                       "Quaternion.prototype.nlerp:\
                                        Argument #1 object is not a Ogre::Quaternion.");
        }

        bool shortestPath = true;
        if(context->argumentCount() > 2 && context->argument(2).isBool())
        {
            shortestPath = context->argument(2).toBool();
        }

        return engine->toScriptValue(Ogre::Quaternion::nlerp(t, *q, *q2, shortestPath));
    }
    else
    {
        qWarning() << "Quaternion.prototype.nlerp: Must be called with 2 arguments: "
                   << "The interpolation point t on [0,1] and the quaternion to rotate to.";
    }
    return engine->undefinedValue();
}

QScriptValue Quaternion_prototype_length(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(engine);
    Ogre::Quaternion* q = qscriptvalue_cast<Ogre::Quaternion*>(context->thisObject());
    if (!q)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Quaternion.prototype.length: \
                                    this object is not a Ogre::Quaternion");
    }
    return q->Norm();
}

QScriptValue Quaternion_prototype_toString(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(engine);
    Ogre::Quaternion* q = qscriptvalue_cast<Ogre::Quaternion*>(context->thisObject());
    if (!q)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Quaternion.prototype.toString: \
                                    this object is not a Ogre::Quaternion");
    }

    return QString("Quaternion (w: %0, x: %1, y: %2, z: %3)")
                  .arg(q->w)
                  .arg(q->x)
                  .arg(q->y)
                  .arg(q->z);
}

QScriptValue Quaternion_prototype_equals(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::Quaternion* q = qscriptvalue_cast<Ogre::Quaternion*>(context->thisObject());
    if (!q)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Quaternion.prototype.equals: \
                                    this object is not a Ogre::Quaternion");
    }

    if (context->argumentCount() == 1)
    {
        Ogre::Quaternion* q2 = qscriptvalue_cast<Ogre::Quaternion*>(context->argument(0));
        if (!q2)
        {
            return context->throwError(QScriptContext::TypeError,
                                       "Quaternion.prototype.equals:\
                                        Argument #0 object is not a Ogre::Quaternion");
        }

        return fabs(q->w - q2->w) < 0.001f &&
               fabs(q->x - q2->x) < 0.001f &&
               fabs(q->y - q2->y) < 0.001f &&
               fabs(q->z - q2->z) < 0.001f;
    }
    return engine->undefinedValue();
}

void Color_register_prototype(QScriptEngine& engine)
{
    engine.setDefaultPrototype(qMetaTypeId<Ogre::ColourValue*>(), QScriptValue());
    QScriptValue obj = engine.newVariant(QVariant::fromValue((Ogre::ColourValue*)0));

    obj.setProperty("r", engine.newFunction(Color_prototype_r),
                    QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    obj.setProperty("g", engine.newFunction(Color_prototype_g),
                    QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    obj.setProperty("b", engine.newFunction(Color_prototype_b),
                    QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    obj.setProperty("a", engine.newFunction(Color_prototype_a),
                    QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    obj.setProperty("toString", engine.newFunction(Color_prototype_toString));

    engine.setDefaultPrototype(qMetaTypeId<Ogre::ColourValue>(), obj);
    engine.setDefaultPrototype(qMetaTypeId<Ogre::ColourValue*>(), obj);

    engine.globalObject().setProperty("Color",
                                      engine.newFunction(Color_prototype_ctor));

    engine.globalObject().setProperty("Colors",
                                      engine.evaluate("({'BLACK': new Color(0,0,0,1),"
                                                      " 'WHITE': new Color(1,1,1,1),"
                                                      " 'RED'  : new Color(1,0,0,1),"
                                                      " 'GREEN': new Color(0,1,0,1),"
                                                      " 'BLUE' : new Color(0,0,1,1)})"));
}

QScriptValue Color_prototype_ctor(QScriptContext *context, QScriptEngine *engine)
{
    if (context->isCalledAsConstructor())
    {
        float r = 0, g = 0, b = 0, a = 0;

        if (context->argumentCount() > 0)
        {
            r = context->argument(0).toNumber();

            if (context->argumentCount() > 1)
            {
                g = context->argument(1).toNumber();

                if (context->argumentCount() > 2)
                {
                    b = context->argument(2).toNumber();

                    if (context->argumentCount() > 3)
                    {
                        a = context->argument(3).toNumber();
                    }
                }
            }
        }

        Ogre::ColourValue v(r,g,b,a);
        return engine->toScriptValue(v);
    }
    else
    {
        qWarning("Color.ctor: Please use the 'new' operator.");
        return engine->undefinedValue();
    }
}

QScriptValue Color_prototype_r(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::ColourValue* v = qscriptvalue_cast<Ogre::ColourValue*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Color.prototype.r: \
                                    this object is not a Ogre::ColourValue");
    }

    QScriptValue obj = context->thisObject();
    QScriptValue data = obj.data();
    if (!data.isValid())
    {
        data = engine->newObject();
        obj.setData(data);
    }

    QScriptValue result;
    if (context->argumentCount() >= 1)
    {
        float val = context->argument(0).toNumber();
        result = val;
        v->r = val;
    }
    else
    {
        result = v->r;
    }

    return result;
}

QScriptValue Color_prototype_g(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::ColourValue* v = qscriptvalue_cast<Ogre::ColourValue*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Color.prototype.g: \
                                    this object is not a Ogre::ColourValue");
    }

    QScriptValue obj = context->thisObject();
    QScriptValue data = obj.data();
    if (!data.isValid())
    {
        data = engine->newObject();
        obj.setData(data);
    }

    QScriptValue result;
    if (context->argumentCount() >= 1)
    {
        float val = context->argument(0).toNumber();
        result = val;
        v->g = val;
    }
    else
    {
        result = v->g;
    }

    return result;
}

QScriptValue Color_prototype_b(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::ColourValue* v = qscriptvalue_cast<Ogre::ColourValue*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Color.prototype.b: \
                                    this object is not a Ogre::ColourValue");
    }

    QScriptValue obj = context->thisObject();
    QScriptValue data = obj.data();
    if (!data.isValid())
    {
        data = engine->newObject();
        obj.setData(data);
    }

    QScriptValue result;
    if (context->argumentCount() >= 1)
    {
        float val = context->argument(0).toNumber();
        result = val;
        v->b = val;
    }
    else
    {
        result = v->b;
    }

    return result;
}

QScriptValue Color_prototype_a(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::ColourValue* v = qscriptvalue_cast<Ogre::ColourValue*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Color.prototype.r: \
                                    this object is not a Ogre::ColourValue");
    }

    QScriptValue obj = context->thisObject();
    QScriptValue data = obj.data();
    if (!data.isValid())
    {
        data = engine->newObject();
        obj.setData(data);
    }

    QScriptValue result;
    if (context->argumentCount() >= 1)
    {
        float val = context->argument(0).toNumber();
        result = val;
        v->a = val;
    }
    else
    {
        result = v->a;
    }

    return result;
}

QScriptValue Color_prototype_toString(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(engine);
    Ogre::ColourValue* v = qscriptvalue_cast<Ogre::ColourValue*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Color.prototype.toString: \
                                    this object is not a Ogre::ColourValue");
    }

    return QString("Color (r: %1, g: %2, b: %3, a: %4)")
                  .arg(v->r)
                  .arg(v->g)
                  .arg(v->b)
                  .arg(v->a);
}

void addActorBinding(Actor* actor, QScriptEngine& engine)
{
    if(!actor)
    {
        qWarning() << "Tried to add a null actor to the scripting environment."
                   << " The addition was not performed.";
        return;
    }

    engine.globalObject().setProperty(cleanIdentifier(actor->getName()),
                                      engine.newQObject(actor));
}

void removeActorBinding(Actor* actor, QScriptEngine& engine)
{
    if(!actor)
    {
        qWarning() << "Tried to remove a null actor to the scripting environment."
                   << "The removal was not performed.";
        return;
    }

    QString name = cleanIdentifier(actor->getName());

    // Remove its object from the scripting system.
    engine.globalObject().setProperty(name, QScriptValue());
}

QString cleanIdentifier(const QString& input)
{
    QString output = input;
    for(int i = 0; i < input.size(); ++i)
    {
        if(( i == 0 && !QRegExp("[a-zA-Z_$]").exactMatch(input.mid(i, 1)) ) ||
           ( i != 0 && !QRegExp("[0-9a-zA-Z_$]").exactMatch(input.mid(i, 1)) ) )
        {
            output.replace(i, 1, "_");
        }
    }

    return output;
}
} // END NS JavaScriptBindings
