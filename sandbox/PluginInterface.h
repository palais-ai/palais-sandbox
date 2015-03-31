#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#pragma once

#include <stdint.h>
#include <QScriptEngine>
#include "Actor.h"
#include "Scene.h"

class PluginInterface
{
public:
    class PluginInterfaceVersion
    {
    public:
        int32_t major;
        int32_t minor;
    };

    virtual ~PluginInterface() {}

    virtual PluginInterfaceVersion getVersion() const = 0;
};

class Plugin
{
public:
    virtual ~Plugin() {}

    virtual void onLoad(const PluginInterface& interface) = 0;
    virtual void onUnload(const PluginInterface& interface) = 0;
    virtual void onSceneStarted(const PluginInterface& interface, Scene& scene) = 0;
    virtual void onSceneEnded(const PluginInterface& interface, Scene& scene) = 0;
    virtual void update(const PluginInterface& interface, Scene& scene, float deltaTime) = 0;
};

#define PLUGIN_INTERFACE_ID "at.ac.univie.a0927193.aisandbox.plugin/1.0"
Q_DECLARE_INTERFACE(Plugin, PLUGIN_INTERFACE_ID)

#endif // PLUGININTERFACE_H
