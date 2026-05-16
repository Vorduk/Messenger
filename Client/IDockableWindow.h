#pragma once
#include <string>

class IDockableWindow {
public:
    virtual ~IDockableWindow() = default;
    virtual const char* getName() const = 0;
    virtual void Render() = 0;
};