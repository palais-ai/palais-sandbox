#include "HighResolutionTime.h"

#include <chrono>

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

    Timestamp now()
    {
        // Using <chrono> from c++11 for a cross-platform high-resolution timer.
        // This means the library must be compiled with std=c++11.
        return std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::high_resolution_clock::now()
                    - std::chrono::high_resolution_clock::time_point()
                    ).count();
    }
}

END_NS_AILIB
