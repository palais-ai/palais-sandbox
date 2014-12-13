#include "javascriptbindings.h"
#include "scene.h"
#include "actor.h"

#include <QScriptEngine>
#include <QTextStream>
#include <QDebug>

#include <OgreQuaternion.h>
#include <OgreVector3.h>

Q_DECLARE_METATYPE(Ogre::Vector3)
Q_DECLARE_METATYPE(Ogre::Vector3*)
Q_DECLARE_METATYPE(Ogre::Quaternion)
Q_DECLARE_METATYPE(Ogre::Quaternion*)
Q_DECLARE_METATYPE(Actor*)
Q_DECLARE_METATYPE(RaycastResult)
Q_DECLARE_METATYPE(RaycastResult*)

namespace JavaScriptBindings
{

void addBindings(QScriptEngine& engine, Scene* scene)
{
    if(!scene)
    {
        qWarning("Can't install bindings on an uninitialized scene.");
        return;
    }

    Actor_register_prototype(engine);

    QScriptValue sceneVal = engine.newQObject(scene);

    engine.globalObject().setProperty("scene", sceneVal);

    Vector3_register_prototype(engine);
    RaycastResult_register_prototype(engine);

    const QMap<QString, Actor*>& actors = scene->getActors();

    for(QMap<QString, Actor*>::const_iterator it = actors.begin(); it != actors.end(); ++it)
    {
        addActorBinding(it.value(), engine);
    }
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

    obj.setProperty("toString", engine.evaluate("(function() {return 'RaycastResult @value ( hasHit: ' + this.hasHit + ', distance: ' + this.distance + ', actor: ' + this.actor + ')'; })"));

    engine.setDefaultPrototype(qMetaTypeId<RaycastResult>(), obj);
    engine.setDefaultPrototype(qMetaTypeId<RaycastResult*>(), obj);
}

QScriptValue RaycastResult_prototype_distance(QScriptContext *context, QScriptEngine *engine)
{
    // Cast to a pointer to be able to modify the underlying C++ value
    RaycastResult* res = qscriptvalue_cast<RaycastResult*>(context->thisObject());

    if (!res)
    {
        return context->throwError(QScriptContext::TypeError, "RaycastResult.prototype.distance: this object is not a RaycastResult");
    }

    return res->distance;
}

QScriptValue RaycastResult_prototype_actor(QScriptContext *context, QScriptEngine *engine)
{
    // Cast to a pointer to be able to modify the underlying C++ value
    RaycastResult* res = qscriptvalue_cast<RaycastResult*>(context->thisObject());

    if (!res)
    {
        return context->throwError(QScriptContext::TypeError, "RaycastResult.prototype.distance: this object is not a RaycastResult");
    }

    if(!res->actor)
    {
        return engine->undefinedValue();
    }

    return engine->toScriptValue(res->actor);
}

QScriptValue RaycastResult_prototype_hasHit(QScriptContext *context, QScriptEngine *engine)
{
    // Cast to a pointer to be able to modify the underlying C++ value
    RaycastResult* res = qscriptvalue_cast<RaycastResult*>(context->thisObject());

    if (!res)
    {
        return context->throwError(QScriptContext::TypeError, "RaycastResult.prototype.distance: this object is not a RaycastResult");
    }

    return res->actor != NULL;
}


void Actor_register_prototype(QScriptEngine& engine)
{
    const int actorTypeId = qRegisterMetaType<Actor*>("Actor*");

    QScriptValue prototype = engine.newQObject((Actor*)0);
    engine.setDefaultPrototype(actorTypeId, prototype);

    engine.globalObject().setProperty("Actor", engine.newFunction(Vector3_prototype_ctor));
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
    obj.setProperty("normalize", engine.newFunction(Vector3_prototype_normalize));

    engine.setDefaultPrototype(qMetaTypeId<Ogre::Vector3>(), obj);
    engine.setDefaultPrototype(qMetaTypeId<Ogre::Vector3*>(), obj);

    engine.globalObject().setProperty("Vector3", engine.newFunction(Vector3_prototype_ctor));
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
    // Cast to a pointer to be able to modify the underlying C++ value
    Ogre::Vector3* v = qscriptvalue_cast<Ogre::Vector3*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError, "Vector3.prototype.x: this object is not a Ogre::Vector3");
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
    // Cast to a pointer to be able to modify the underlying C++ value
    Ogre::Vector3* v = qscriptvalue_cast<Ogre::Vector3*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError, "Vector3.prototype.y: this object is not a Ogre::Vector3");
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
    // Cast to a pointer to be able to modify the underlying C++ value
    Ogre::Vector3* v = qscriptvalue_cast<Ogre::Vector3*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError, "Vector3.prototype.z: this object is not a Ogre::Vector3");
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
    // Cast to a pointer to be able to modify the underlying C++ value
    Ogre::Vector3* v = qscriptvalue_cast<Ogre::Vector3*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError, "Vector3.prototype.toString: this object is not a Ogre::Vector3");
    }

    QString t;
    return QString("Vector3 @%0 (x: %1, y: %2, z: %3)").arg(t.sprintf("%08p", v)).arg(v->x).arg(v->y).arg(v->z);
}

QScriptValue Vector3_prototype_add(QScriptContext *context, QScriptEngine *engine)
{
    // Cast to a pointer to be able to modify the underlying C++ value
    Ogre::Vector3* v = qscriptvalue_cast<Ogre::Vector3*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError, "Vector3.prototype.toString: this object is not a Ogre::Vector3");
    }

    if (context->argumentCount() == 1)
    {
        Ogre::Vector3* v2 = qscriptvalue_cast<Ogre::Vector3*>(context->argument(0));
        if (!v2)
        {
            return context->throwError(QScriptContext::TypeError, "Vector3.prototype.toString: Argument #0 object is not a Ogre::Vector3");
        }

        *v += *v2;

        return engine->toScriptValue(v);
    }

    return engine->undefinedValue();
}

QScriptValue Vector3_prototype_subtract(QScriptContext *context, QScriptEngine *engine)
{
    // Cast to a pointer to be able to modify the underlying C++ value
    Ogre::Vector3* v = qscriptvalue_cast<Ogre::Vector3*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError, "Vector3.prototype.toString: this object is not a Ogre::Vector3");
    }

    if (context->argumentCount() == 1)
    {
        Ogre::Vector3* v2 = qscriptvalue_cast<Ogre::Vector3*>(context->argument(0));
        if (!v2)
        {
            return context->throwError(QScriptContext::TypeError, "Vector3.prototype.toString: Argument #0 object is not a Ogre::Vector3");
        }

        *v -= *v2;

        return engine->toScriptValue(v);
    }

    return engine->undefinedValue();
}

QScriptValue Vector3_prototype_multiply(QScriptContext *context, QScriptEngine *engine)
{
    // Cast to a pointer to be able to modify the underlying C++ value
    Ogre::Vector3* v = qscriptvalue_cast<Ogre::Vector3*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError, "Vector3.prototype.toString: this object is not a Ogre::Vector3");
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
            return context->throwError(QScriptContext::TypeError, "Vector3.prototype.toString: Argument #0 object is not a Ogre::Vector3 or a number");
        }

        *v *= *v2;

        return engine->toScriptValue(v);
    }

    return engine->undefinedValue();
}

QScriptValue Vector3_prototype_normalize(QScriptContext *context, QScriptEngine *engine)
{
    // Cast to a pointer to be able to modify the underlying C++ value
    Ogre::Vector3* v = qscriptvalue_cast<Ogre::Vector3*>(context->thisObject());

    if (!v)
    {
        return context->throwError(QScriptContext::TypeError, "Vector3.prototype.toString: this object is not a Ogre::Vector3");
    }

    v->normalise();

    return engine->toScriptValue(v);
}

void addActorBinding(Actor* actor, QScriptEngine& engine)
{
    qDebug() << "Adding actor " << cleanIdentifier(actor->getName()) << " to the scripting system.";
    engine.globalObject().setProperty(cleanIdentifier(actor->getName()), engine.newQObject(actor));
}

QString cleanIdentifier(const QString &input)
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
