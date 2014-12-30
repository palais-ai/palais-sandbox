#include "consolemodel.h"

ConsoleModel::ConsoleModel() :
    mPassedTime(0,0,0,0)
{
}

QString ConsoleModel::getPassedTimeString() const
{
    return mPassedTime.toString("hh:mm:ss.zzz");
}

void ConsoleModel::log(const QString& message, LogLevel level)
{
    int index = mLog.size();
    beginInsertRows(QModelIndex(), index, index);
    mLog += LogEntry(message, level);
    endInsertRows();
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
