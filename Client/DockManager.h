#pragma once
#include "imgui.h"
#include <functional>
#include <string>
#include <vector>
#include <memory>

struct DockLayoutNode {
    // Window or split
    enum Type { Window, Split };
    Type type;
    std::string window_name;
    ImGuiDir split_dir = ImGuiDir_Left;
    float split_ratio = 0.5f;
    std::unique_ptr<DockLayoutNode> child1;
    std::unique_ptr<DockLayoutNode> child2;

    // Create node-window
    static DockLayoutNode MakeWindow(const char* name) {
        DockLayoutNode node;
        node.type = Window;
        node.window_name = name;
        return node;
    }

    // Create node-split
    static DockLayoutNode MakeSplit(ImGuiDir dir, float ratio,
        DockLayoutNode c1, DockLayoutNode c2) {
        DockLayoutNode node;
        node.type = Split;
        node.split_dir = dir;
        node.split_ratio = ratio;
        node.child1 = std::make_unique<DockLayoutNode>(std::move(c1));
        node.child2 = std::make_unique<DockLayoutNode>(std::move(c2));
        return node;
    }
};

class DockManager {
public:
    using WindowRenderFunc = std::function<void()>;

    DockManager();

    void RegisterWindow(const std::string& name, WindowRenderFunc func);

    void SetInitialLayout(DockLayoutNode root);

    // Call every frame in UI
    void Begin();
    void RenderWindows();
    void End();

private:
    void ApplyInitialLayout(ImGuiID dockspace_id);

    struct WindowInfo {
        std::string name;
        WindowRenderFunc render;
    };
    std::vector<WindowInfo> m_windows;
    DockLayoutNode m_initial_layout;
    bool m_initial_layout_set = false;
    bool m_first_frame = true;
};