#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#pragma once

#include "ai_global.h"
#include "Any.h"
#include "btHashMap.h"

BEGIN_NS_AILIB

// A generic class to store knowledge by unique keys.
template <typename KEY>
class Blackboard
{
public:
    typedef KEY key_type;

    template <typename T>
    FORCE_INLINE T get(const KEY& key) const
    {
        return ailib::any_cast<T>(mKnowledge[key]);
    }

    FORCE_INLINE bool has(const KEY& key) const
    {
        return mKnowledge.find(key) != NULL;
    }

    template <typename T>
    FORCE_INLINE void set(const KEY& key, const T& value)
    {
        mKnowledge.insert(key, value);
    }

    FORCE_INLINE void remove(const KEY& key)
    {
        mKnowledge.remove(key);
    }

    uint32_t getHashCode() const
    {
        // CREDITS: FNV-1a algorithm primes from http://www.isthe.com/chongo/tech/comp/fnv/
        // (public domain)
        static const uint32_t fnv_prime    =   16777619;
        static const uint32_t offset_basis = 2166136261;

        uint32_t hash = offset_basis;

        // Combine the hashes of the individual keys.
        for(int i = 0; i < mKnowledge.size(); ++i)
        {
            hash *= fnv_prime;
            hash += mKnowledge.getKeyAtIndex(i).getHash();
        }
    }

    int size() const
    {
        return mKnowledge.size();
    }

    bool operator==(const Blackboard& other) const
    {
        if(size() != other.size())
        {
            return false;
        }

        for(int i = 0; i < mKnowledge.size(); ++i)
        {
            if(*other.mKnowledge.find(mKnowledge.getKeyAtIndex(i)) !=
               *mKnowledge.getAtIndex(i))
            {
                return false;
            }
        }

        return true;
    }

private:
    btHashMap<KEY, ailib::hold_any> mKnowledge;
};

// Adapter class to enable the use of Blackboards as keys in btHashMap.
class HashBlackboard
{
private:
    Blackboard*  mBlackboard;
    unsigned int mHash;
public:
    HashBlackboard(Blackboard* board) :
        mBlackboard(board),
        mHash(blackboard->getHashCode())
    {
        ;
    }

    FORCE_INLINE unsigned int getHash()const
    {
        return mHash;
    }

    FORCE_INLINE bool equals(const HashBlackboard& other) const
    {
        return mBlackboard == other.mBlackboard;
    }
};

END_NS_AILIB

#endif // BLACKBOARD_H
