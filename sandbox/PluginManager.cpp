#include "PluginManager.h"
#include <QPluginLoader>
#include <QGuiApplication>
#include <QDir>
#include <QDebug>

PluginManager::~PluginManager()
{
    foreach(Plugin* plugin, mPlugins)
    {
        plugin->onUnload(*this);
    }
}

void PluginManager::loadPlugins()
{
    QDir pluginsDir(QCoreApplication::applicationDirPath());
#if defined(Q_OS_MAC)
    pluginsDir.cdUp();
#endif
    pluginsDir.cd("Plugins");

    foreach(QString fileName, pluginsDir.entryList(QDir::Files))
    {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = pluginLoader.instance();
        if (plugin)
        {
            Plugin* interface = qobject_cast<Plugin *>(plugin);
            if (interface)
            {
                mPlugins += interface;
                interface->onLoad(*this);
                qDebug() << "Loaded plugin [ " << fileName << " ].";
            }
        }
    }
}

PluginInterface::PluginInterfaceVersion PluginManager::getVersion() const
{
    PluginInterface::PluginInterfaceVersion piv;
    piv.major = 1;
    piv.minor = 0;
    return piv;
}

void PluginManager::sceneStarted(Scene& scene)
{
    foreach(Plugin* plugin, mPlugins)
    {
        plugin->onSceneStarted(*this, scene);
    }
}

void PluginManager::sceneEnded(Scene& scene)
{
    foreach(Plugin* plugin, mPlugins)
    {
        plugin->onSceneEnded(*this, scene);
    }
}

void PluginManager::update(Scene& scene, float deltaTime)
{
    foreach(Plugin* plugin, mPlugins)
    {
        plugin->update(*this, scene, deltaTime / 1000.f);
    }
}

