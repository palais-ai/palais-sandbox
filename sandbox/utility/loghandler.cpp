#include "loghandler.h"

void LogHandler::broadcastNewMessageReceived(ConsoleModel::LogLevel level,
                                             const QString& message)
{
    emit newMessageReceived(level, message);
}


void LogHandler::messageLogged(const Ogre::String& message,
                               Ogre::LogMessageLevel lml,
                               bool maskDebug,
                               const Ogre::String &logName,
                               bool& skipThisMessage)
{
    Q_UNUSED(maskDebug);
    Q_UNUSED(logName);
    Q_UNUSED(skipThisMessage);

    ConsoleModel::LogLevel level;
    switch(lml)
    {
    default:
    case Ogre::LML_TRIVIAL:
        level = ConsoleModel::LogLevelDebug;
        break;
    case Ogre::LML_NORMAL:
        level = ConsoleModel::LogLevelInfo;
        break;
    case Ogre::LML_CRITICAL:
        level = ConsoleModel::LogLevelError;
        break;
    }
    broadcastNewMessageReceived(level, QString::fromStdString(message));
}
