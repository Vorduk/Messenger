#include "DockManager.h"
#include "imgui_internal.h"
#include <fstream>

DockManager::DockManager() {
    if (ImGui::GetIO().IniFilename == nullptr)
        ImGui::GetIO().IniFilename = "imgui.ini";
}

void DockManager::RegisterWindow(const std::string& name, WindowRenderFunc func) {
    m_windows.push_back({ name, std::move(func) });
}

void DockManager::SetInitialLayout(DockLayoutNode root) {
    m_initial_layout = std::move(root);
    m_initial_layout_set = true;
}

static void BuildDockNode(ImGuiID node_id, const DockLayoutNode& layout) {
    if (layout.type == DockLayoutNode::Window) {
        ImGui::DockBuilderDockWindow(layout.window_name.c_str(), node_id);
    }
    else if (layout.type == DockLayoutNode::Split) {
        ImGuiID child1_id, child2_id;
        ImGui::DockBuilderSplitNode(node_id, layout.split_dir, layout.split_ratio,
            &child1_id, &child2_id);
        BuildDockNode(child1_id, *layout.child1);
        BuildDockNode(child2_id, *layout.child2);
    }
}

void DockManager::ApplyInitialLayout(ImGuiID dockspace_id) {
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);
    BuildDockNode(dockspace_id, m_initial_layout);
    ImGui::DockBuilderFinish(dockspace_id);
}

void DockManager::Begin() {

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags dockspace_flags =
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, dockspace_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspace_id);

    if (m_first_frame) {
        m_first_frame = false;
        bool needInitialLayout = true;

        const char* iniFile = ImGui::GetIO().IniFilename;
        if (iniFile && *iniFile) {
            std::ifstream file(iniFile, std::ios::binary | std::ios::ate);
            if (file.good()) {
                if (file.tellg() > 2)
                    needInitialLayout = false;
            }
        }

        if (needInitialLayout && m_initial_layout_set) {
            ApplyInitialLayout(dockspace_id);
        }
    }
}

void DockManager::RenderWindows() {
    for (auto& win : m_windows) {
        ImGui::Begin(win.name.c_str());
        win.render();
        ImGui::End();
    }
}

void DockManager::End() {
    ImGui::End(); // DockSpace
}