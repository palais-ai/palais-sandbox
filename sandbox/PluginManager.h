#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "PluginInterface.h"

class PluginManager : public PluginInterface
{
public:
    ~PluginManager();

    void loadPlugins();
    virtual PluginInterface::PluginInterfaceVersion getVersion() const;

    void sceneStarted(Scene& scene);
    void sceneEnded(Scene& scene);
    void update(Scene& scene, float deltaTime);
private:
    QVector<Plugin*> mPlugins;
};

#endif // PLUGINMANAGER_H
