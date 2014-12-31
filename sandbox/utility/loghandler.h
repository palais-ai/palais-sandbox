#ifndef LOGHANDLER_H
#define LOGHANDLER_H

#include <QObject>
#include "models/consolemodel.h"

class LogHandler : public QObject
{
    Q_OBJECT
public:
    void broadcastNewMessageReceived(ConsoleModel::LogLevel level,
                                     const QString& message);
signals:
    void newMessageReceived(ConsoleModel::LogLevel level,
                            const QString& message);
};


#endif // LOGHANDLER_H
