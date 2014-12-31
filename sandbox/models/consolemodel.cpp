#include "consolemodel.h"

ConsoleModel::LogEntry::LogEntry(const QString& message,
                                 ConsoleModel::LogLevel level) :
    message(message),
    level(level)
{
    ;
}

void ConsoleModel::declareQML()
{
    qmlRegisterType<ConsoleModel>("Console", 1, 0,
                                  "ConsoleModel");
}

ConsoleModel::ConsoleModel() :
    mPassedTime(0,0,0,0),
    mIsLogging(false)
{
}

void ConsoleModel::onMessageReceived(LogLevel level, const QString& msg)
{
    // Guard from endless chains of logs when a log triggers another log.
    if(!mIsLogging)
    {
        mIsLogging = true;
        log(msg, level);
        mIsLogging = false;
    }
}

void ConsoleModel::onTimePassed(const QTime& passedTime)
{
    mPassedTime = mPassedTime.addMSecs(passedTime.msec());

    emit onPassedTimeStringChanged(getPassedTimeString());
}

QString ConsoleModel::getPassedTimeString() const
{
    return mPassedTime.toString("mm:ss.zzz");
}

void ConsoleModel::log(const QString& message, LogLevel level)
{
    int index = mLog.size();
    beginInsertRows(QModelIndex(), index, index);
    mLog += LogEntry(message, level);
    endInsertRows();

    emit onFinishedMessage();
}

QHash<int, QByteArray> ConsoleModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ModelRoleIndex] = "index";
    roles[ModelRoleLevel] = "level";
    roles[ModelRoleMessage] = "message";
    return roles;
}

Qt::ItemFlags ConsoleModel::flags(const QModelIndex &index) const
{
    return 0;
}

QVariant ConsoleModel::data(const QModelIndex &index, int role) const
{
    switch(role)
    {
    case ModelRoleIndex:
        return index.row();
    case ModelRoleLevel:
        return static_cast<int>(mLog[index.row()].level);
    case ModelRoleMessage:
        return mLog[index.row()].message;
    default:
        return QVariant(QVariant::Invalid);
    }
}

QVariant ConsoleModel::headerData(int section,
                                    Qt::Orientation orientation,
                                    int role) const
{
    return QVariant(QVariant::Invalid);
}

int ConsoleModel::rowCount(const QModelIndex& parent) const
{
    return mLog.size();
}
