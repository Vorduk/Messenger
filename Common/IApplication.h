#pragma once
#include <string>

/**
 * @brief App control interface.
 */
class IApplication {
public:
    virtual ~IApplication() = default;
    virtual void run() = 0;     ///< Run main loop
    virtual void stop() = 0;    ///< Stop main loop
};