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

namespace JavaScriptBindings
{

void addBindings(QScriptEngine& engine, Scene* scene)
{
    if(!scene)
    {
        qWarning("Can't install bindings on an uninitialized scene.");
        return;
    }

    QScriptValue sceneVal = engine.newQObject(scene);

    engine.globalObject().setProperty("scene", sceneVal);

    Vector3_register_prototype(engine);
    engine.globalObject().setProperty("Vector3", engine.newFunction(Vector3_prototype_ctor));

    const QMap<QString, Actor*>& actors = scene->getActors();

    for(QMap<QString, Actor*>::const_iterator it = actors.begin(); it != actors.end(); ++it)
    {
        addActorBinding(it.value(), engine);
    }
}

void Vector3_register_prototype(QScriptEngine& engine)
{
    engine.setDefaultPrototype(qMetaTypeId<Ogre::Vector3*>(), QScriptValue());
    QScriptValue obj = engine.newVariant(qVariantFromValue((Ogre::Vector3*)0));

    obj.setProperty("x", engine.newFunction(Vector3_prototype_x),
                    QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    obj.setProperty("y", engine.newFunction(Vector3_prototype_y),
                    QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    obj.setProperty("z", engine.newFunction(Vector3_prototype_z),
                    QScriptValue::PropertyGetter | QScriptValue::PropertySetter);
    obj.setProperty("toString", engine.newFunction(Vector3_prototype_toString));

    engine.setDefaultPrototype(qMetaTypeId<Ogre::Vector3>(), obj);
    engine.setDefaultPrototype(qMetaTypeId<Ogre::Vector3*>(), obj);
}

QScriptValue Vector3_prototype_ctor(QScriptContext *context, QScriptEngine *engine)
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

    qDebug() << v.x << ", " << v.y << ", " << v.z;

    QScriptValue sv = context->engine()->newVariant(context->thisObject(), QVariant::fromValue(v));
    return sv;
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
