#pragma once

class IUI {
public:
    virtual ~IUI() = default;
    virtual void run() = 0;     ///< Run main cycle
    virtual void stop() = 0;    ///< Stop main cycle
};