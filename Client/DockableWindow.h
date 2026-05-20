#pragma once
#include <string>

class DockableWindow {
public:
    virtual ~DockableWindow() = default;
    virtual const char* getName() const = 0;
    virtual void render() = 0;
};