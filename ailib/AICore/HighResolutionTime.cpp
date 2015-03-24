#include "HighResolutionTime.h"

BEGIN_NS_AILIB

namespace HighResolutionTime {

    Timestamp milliseconds(double t)
    {
        return Timestamp(t * 1e3);
    }

    Timestamp seconds(double t)
    {
        return Timestamp(t * 1e6);
    }
}

END_NS_AILIB
