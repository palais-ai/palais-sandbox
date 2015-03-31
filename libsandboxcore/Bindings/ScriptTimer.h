#ifndef SCRIPTTIMER_H
#define SCRIPTTIMER_H

#include <QObject>
#include <QScriptEngine>
#include <QHash>

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
    static qint32 newTimer(int interval,
                           bool oneShot,
                           QScriptEngine& engine,
                           const QScriptValue& function);
    static bool removeTimer(qint32 handle);

    // Updates all active timers. Call this regularily.
    static void updateAll(float deltaTime);

    void update(float deltaTime);

    qint32 getHandle() const;
    QScriptEngine& getEngine();
public slots:
    static void onEngineDestroyed(QObject* engine);
    void timeout();
private:
    static QHash<qint32, ScriptTimer*> sScriptTimers;
    static qint32 sHandleCounter;

    // Timers may only be instantiated by the factory methods above.
    ScriptTimer(int interval,
                bool oneShot,
                QScriptEngine& engine,
                const QScriptValue& function);

    float mTimeLeft;
    const float mInitialTime;
    const bool mIsOneShot;
    QScriptEngine& mEngine;
    QScriptValue mFunction;
    qint32 mHandle;
};

#endif // SCRIPTTIMER_H
