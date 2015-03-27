#include "knowledgemodel.h"

KnowledgeModel::KnowledgeModel()
{
}

bool KnowledgeModel::hasKnowledge(const QString &key) const
{
    return mKnowledge.contains(key);
}

QVariant KnowledgeModel::getKnowledge(const QString& key) const
{
    const QVariant& retVal = mKnowledge[key];

    return retVal;
}

void KnowledgeModel::removeKnowledge(const QString& key)
{
    mKnowledge.remove(key);
    emit knowledgeRemoved(key);
}

void KnowledgeModel::setKnowledge(const QString& key, const QVariant& value)
{
    const bool isEdited = mKnowledge.contains(key);
    mKnowledge[key] = value;

    if(isEdited)
    {
        emit knowledgeChanged(key, value);
    }
    else
    {
        emit knowledgeAdded(key, value);
    }
}

const QVariantMap& KnowledgeModel::getKnowledge() const
{
    return mKnowledge;
}
