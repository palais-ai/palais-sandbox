#ifndef CONSOLEMODEL_H
#define CONSOLEMODEL_H

#include <QAbstractListModel>
#include <QTime>
#include <QList>
#include <QtQml>
#include <stdint.h>

class ConsoleModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString passedTimeString READ getPassedTimeString
                                        NOTIFY passedTimeStringChanged)
    Q_PROPERTY(QString fpsString READ getFrameCounterString
                                 NOTIFY frameCounterStringChanged)
public:
    enum ModelRole
    {
        ModelRoleIndex = Qt::UserRole + 1,
        ModelRoleLevel,
        ModelRoleMessage
    };

    enum LogLevel
    {
        LogLevelDebug = 0,
        LogLevelInfo = 1,
        LogLevelWarning = 2,
        LogLevelError = 3
    };
    Q_ENUMS(LogLevel)

    static void declareQML();
    ConsoleModel();

    QString getPassedTimeString() const;
    QString getFrameCounterString() const;

    void log(const QString& message, LogLevel level = LogLevelDebug);

    // List Model impl
    virtual QHash<int, QByteArray> roleNames() const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual int rowCount(const QModelIndex &parent) const;
signals:
    void passedTimeStringChanged(QString passedTime);
    void finishedMessage();
    void frameCounterStringChanged(QString string);
public slots:
    void clear();
    void onTimePassed(const QTime& passedTime);
    void onMessageReceived(LogLevel level, QString msg);
    void onFrameSwapped();
    void onFPSTimeout();
private:
    struct LogEntry
    {
        LogEntry(const QString& message, LogLevel level);

        QString message;
        LogLevel level;
    };

    QTime mPassedTime;
    QList<LogEntry> mLog;
    uint32_t mFrameCount;
    bool mIsLogging;
};

#endif // CONSOLEMODEL_H
