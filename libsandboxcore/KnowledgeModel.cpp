#include "KnowledgeModel.h"

KnowledgeModel::KnowledgeModel()
{
    ;
}

bool KnowledgeModel::hasKnowledge(const QString &key) const
{
    return mKnowledge.contains(key);
}

QVariant KnowledgeModel::getKnowledge(const QString& key) const
{
    return mKnowledge[key];
}

void KnowledgeModel::removeKnowledge(const QString& key)
{
    int numRemoved = mKnowledge.remove(key);

    if(numRemoved != 0)
    {
        emit knowledgeRemoved(key);
    }
}

void KnowledgeModel::setKnowledge(const QString& key, const QVariant& value)
{
    QVariantMap::iterator it = mKnowledge.find(key);
    const bool isEdited = it != mKnowledge.end();
    if(isEdited)
    {
        if(value != it.value())
        {
            mKnowledge.insert(it, key, value);
            emit knowledgeChanged(key, value);
        }
    }
    else
    {
        mKnowledge.insert(key, value);
        emit knowledgeAdded(key, value);
    }
}

QVariantMap* KnowledgeModel::getKnowledgePtr()
{
    return &mKnowledge;
}

const QVariantMap& KnowledgeModel::getKnowledge() const
{
    return mKnowledge;
}
