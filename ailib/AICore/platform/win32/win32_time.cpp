#include "HighResolutionTime.h"
#include <windows.h>

BEGIN_NS_AILIB

namespace HighResolutionTime {
    static LARGE_INTEGER initQPC()
    {
        LARGE_INTEGER retVal = 0;
        QueryPerformanceFrequency(&retVal);
        return retVal;
    }

    static LARGE_INTEGER sFrequency = initQPC();

    Timestamp now()
    {
        Timestamp retVal;
        QueryPerformanceCounter(static_cast<LARGE_INTEGER*>(&retVal));
        retVal.QuadPart *= 1.f / nanoseconds(1); // multiply first to prevent precision-loss
        retVal.QuadPart /= sFrequency.QuadPart;
        return retVal;
    }
}

END_NS_AILIB
