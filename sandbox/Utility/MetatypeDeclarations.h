#ifndef METATYPEDECLARATIONS_H
#define METATYPEDECLARATIONS_H

#pragma once

#include <QObject>
#include <QVector>
#include <OgreVector3.h>
#include <OgreQuaternion.h>
#include "Actor.h"

Q_DECLARE_METATYPE(Actor*)
Q_DECLARE_METATYPE(Ogre::Vector3)
Q_DECLARE_METATYPE(Ogre::Vector3*)
Q_DECLARE_METATYPE(QVector<Ogre::Vector3>)
Q_DECLARE_METATYPE(QVector<Ogre::Vector3*>)
Q_DECLARE_METATYPE(Ogre::Quaternion)
Q_DECLARE_METATYPE(Ogre::Quaternion*)

#endif // METATYPEDECLARATIONS_H
