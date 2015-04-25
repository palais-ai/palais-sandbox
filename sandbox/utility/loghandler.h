#ifndef LOGHANDLER_H
#define LOGHANDLER_H

#include "../models/consolemodel.h"
#include <QObject>
#include <OgreLog.h>

class LogHandler : public QObject,
                   public Ogre::LogListener
{
    Q_OBJECT
public:
    void broadcastNewMessageReceived(ConsoleModel::LogLevel level,
                                     const QString& message);

    virtual void messageLogged(const Ogre::String& message,
                               Ogre::LogMessageLevel lml,
                               bool maskDebug,
                               const Ogre::String &logName,
                               bool& skipThisMessage);
signals:
    void newMessageReceived(ConsoleModel::LogLevel level,
                            QString message);
};


#endif // LOGHANDLER_H
