#ifndef SCRIPTTIMER_H
#define SCRIPTTIMER_H

#include <QObject>
#include <QScriptEngine>
#include <QHash>

class ScriptTimer;

class ScriptTimerFactory : public QObject
{
    Q_OBJECT
public:
    explicit ScriptTimerFactory(QObject* parent = NULL);

    qint32 newTimer(int interval,
                    bool oneShot,
                    const QScriptValue& function);
    bool removeTimer(qint32 handle);

    void updateTimers(float deltaTime);
private:
    QHash<qint32, ScriptTimer*> mScriptTimers;
    ScriptTimer* mCurrentTimer;
    bool mCurrentIsRemoved;
    qint32 mHandleCounter;
};

/**
 * @brief The ScriptTimer class
 *
 * @note Timers manage their lifetime automatically.
 * Only interval timers have to be removed explicitly,
 * Otherwise they will only be cleaned up after the corresponding script is destroyed.
 * The update(float) method should be called by the currently active scene to ensure
 * that the script timers are evaluated at the scene's simulation speed.
 */
class ScriptTimer : public QObject
{
    Q_OBJECT

public:
    bool update(float deltaTime);
    QScriptEngine* getEngine();
public slots:
    void timeout();
private:
    // Timers may only be instantiated by the ScriptTimerFactory.
    ScriptTimer(int interval,
                bool oneShot,
                const QScriptValue& function);

    float mTimeLeft;
    const float mInitialTime;
    const bool mIsOneShot;
    QScriptValue mFunction;

    friend class ScriptTimerFactory;
};

#endif // SCRIPTTIMER_H
