#ifndef KNOWLEDGEMODEL_H
#define KNOWLEDGEMODEL_H

#include "libsandboxcore_global.h"

#include <QObject>
#include <QVariant>

class DLL_EXPORT KnowledgeModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap knowledge READ getKnowledge)
public:
    KnowledgeModel();

    QVariantMap getKnowledge() const;
    Q_INVOKABLE bool hasKnowledge(const QString& key) const;
    Q_INVOKABLE QVariant getKnowledge(const QString& key) const;
    Q_INVOKABLE void setKnowledge(const QString& key, const QVariant& value);
    Q_INVOKABLE void removeKnowledge(const QString& key);
signals:
    void knowledgeAdded(const QString& key, const QVariant& knowledge);
    void knowledgeChanged(const QString& key, const QVariant& knowledge);
    void knowledgeRemoved(const QString& key);
private:
    QVariantMap mKnowledge;
};

#endif // KNOWLEDGEMODEL_H
