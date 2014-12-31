#include "loghandler.h"

void LogHandler::broadcastNewMessageReceived(ConsoleModel::LogLevel level,
                                             const QString& message)
{
    emit newMessageReceived(level, message);
}
