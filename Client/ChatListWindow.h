#pragma once
#include "IDockableWindow.h"

class ChatListWindow : public IDockableWindow {
public:
    const char* getName() const override { return "Chat list"; }
    void Render() override;
};