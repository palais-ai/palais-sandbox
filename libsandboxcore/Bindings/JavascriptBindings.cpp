#include "JavascriptBindings.h"
#include "../Actor.h"
#include "../Scene.h"
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
Q_DECLARE_METATYPE(Ogre::Vector3)
Q_DECLARE_METATYPE(Ogre::Vector3*)
Q_DECLARE_METATYPE(QVector<Ogre::Vector3>)
Q_DECLARE_METATYPE(QVector<Ogre::Vector3*>)
Q_DECLARE_METATYPE(Ogre::Quaternion)
Q_DECLARE_METATYPE(Ogre::Quaternion*)

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

    Actor_register_prototype(engine);

    QScriptValue sceneVal = engine.newQObject(scene);

    engine.globalObject().setProperty("Scene", sceneVal);
    engine.globalObject().setProperty("require", engine.newFunction(script_require));

    Vector3_register_prototype(engine);
    RangeQueryResult_register_prototype(engine);
    RaycastResult_register_prototype(engine);
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
    obj.setProperty("equals", engine.newFunction(Vector3_prototype_equals));

    engine.setDefaultPrototype(qMetaTypeId<Ogre::Vector3>(), obj);
    engine.setDefaultPrototype(qMetaTypeId<Ogre::Vector3*>(), obj);

    engine.globalObject().setProperty("Vector3",
                                      engine.newFunction(Vector3_prototype_ctor));

    qScriptRegisterSequenceMetaType<QVector<Ogre::Vector3> >(&engine);
    qScriptRegisterSequenceMetaType<QVector<Ogre::Vector3*> >(&engine);
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
    if (context->argumentCount() == 1)
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
    if (context->argumentCount() == 1)
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
    if (context->argumentCount() == 1)
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

    QString t;
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

    if (context->argumentCount() == 1)
    {
        Ogre::Vector3* v2 = qscriptvalue_cast<Ogre::Vector3*>(context->argument(0));
        if (!v2)
        {
            return context->throwError(QScriptContext::TypeError,
                                       "Vector3.prototype.add: \
                                        Argument #0 object is not a Ogre::Vector3");
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
                                    this object is not a Ogre::Vector3");
    }

    if (context->argumentCount() == 1)
    {
        Ogre::Vector3* v2 = qscriptvalue_cast<Ogre::Vector3*>(context->argument(0));
        if (!v2)
        {
            return context->throwError(QScriptContext::TypeError,
                                       "Vector3.prototype.subtract: \
                                        Argument #0 object is not a Ogre::Vector3");
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
                                    this object is not a Ogre::Vector3");
    }

    if (context->argumentCount() == 1)
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
                                        Argument #0 object is not a Ogre::Vector3 or a number");
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
                                    this object is not a Ogre::Vector3");
    }

    if (context->argumentCount() == 1)
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
                                        Argument #0 object is not a Ogre::Vector3 or a number");
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

    if (context->argumentCount() == 1)
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

QScriptValue Vector3_prototype_equals(QScriptContext *context, QScriptEngine *engine)
{
    Ogre::Vector3* v = qscriptvalue_cast<Ogre::Vector3*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError,
                                   "Vector3.prototype.equals: \
                                    this object is not a Ogre::Vector3");
    }

    if (context->argumentCount() == 1)
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
