#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#pragma once

#include "ai_global.h"
#include "Any.h"
#include <map>

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
        return mKnowledge.find(key) != mKnowledge.end();
    }

    template <typename T>
    FORCE_INLINE void set(const KEY& key, const T& value)
    {
        mKnowledge[key] = value;
    }
private:
    std::map<KEY, ailib::hold_any> mKnowledge;
};

END_NS_AILIB

#endif // BLACKBOARD_H
