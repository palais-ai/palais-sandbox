#ifndef JAVASCRIPTBINDINGS_H
#define JAVASCRIPTBINDINGS_H

class QScriptEngine;
class QScriptContext;
class Scene;
class Actor;

namespace Ogre {
class Vector3;
}

#include <QScriptValue>

namespace JavaScriptBindings
{
void addBindings(QScriptEngine& engine, Scene* scene);
void addActorBinding(Actor* actor, QScriptEngine& engine);
QString cleanIdentifier(const QString& input);

void Vector3_register_prototype(QScriptEngine& engine);
QScriptValue Vector3_prototype_ctor(QScriptContext *context, QScriptEngine *engine);
QScriptValue Vector3_prototype_x(QScriptContext *context, QScriptEngine *engine);
QScriptValue Vector3_prototype_y(QScriptContext *context, QScriptEngine *engine);
QScriptValue Vector3_prototype_z(QScriptContext *context, QScriptEngine *engine);
QScriptValue Vector3_prototype_toString(QScriptContext *context, QScriptEngine *engine);
}

#endif // JAVASCRIPTBINDINGS_H
