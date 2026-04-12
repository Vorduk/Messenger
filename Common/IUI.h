#pragma once
#include <string>

class IUI {
public:
    virtual ~IUI() = default;
    virtual void run() = 0;     ///< Run main loop
    virtual void stop() = 0;    ///< Stop main loop

    virtual void showSendMessageConfirmation(const std::string& confirmation) = 0;
    virtual void showSendMessageError(const std::string& error) = 0;
};