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
    mFrameCount(0),
    mIsLogging(false)
{
    ;
}

void ConsoleModel::onMessageReceived(LogLevel level, QString msg)
{
    // Prevent endless chains of logs when a log event triggers another log event.
    if(!mIsLogging)
    {
        mIsLogging = true;
        log(msg, level);
        mIsLogging = false;
    }
}

void ConsoleModel::onFrameSwapped()
{
    ++mFrameCount;
}

void ConsoleModel::onFPSTimeout()
{
    emit frameCounterStringChanged(getFrameCounterString());
    mFrameCount = 0;
}

void ConsoleModel::onReload()
{
    mPassedTime = QTime(0,0,0,0);
    emit passedTimeStringChanged(getPassedTimeString());
}

void ConsoleModel::onTimePassed(const QTime& passedTime)
{
    mPassedTime = mPassedTime.addMSecs(passedTime.msec());
    emit passedTimeStringChanged(getPassedTimeString());
}

QString ConsoleModel::getPassedTimeString() const
{
    return mPassedTime.toString("mm:ss.zzz");
}

QString ConsoleModel::getFrameCounterString() const
{
    return QString("%1 fps").arg(mFrameCount);
}

void ConsoleModel::clear()
{
    beginRemoveRows(QModelIndex(), 0, mLog.size() - 1);

    mLog.clear();

    endRemoveRows();
}

void ConsoleModel::log(const QString& message, LogLevel level)
{
    int index = mLog.size();
    beginInsertRows(QModelIndex(), index, index);
    mLog += LogEntry(message, level);
    endInsertRows();

    emit finishedMessage();
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
    Q_UNUSED(index);
    return 0;
}

QVariant ConsoleModel::data(const QModelIndex &index, int role) const
{
    if(index.row() >= mLog.size())
    {
        return QVariant(QVariant::Invalid);
    }

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
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);
    return QVariant(QVariant::Invalid);
}

int ConsoleModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return mLog.size();
}
