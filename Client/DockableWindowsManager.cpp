#include "pch.h"
#include "DockableWindowsManager.h"
#include "imgui_internal.h"
#include <fstream>
#include <functional>

DockableWindowsManager::DockableWindowsManager() {
    // Ensure ImGui saves/loads layout to an .ini file.
    if (ImGui::GetIO().IniFilename == nullptr)
        ImGui::GetIO().IniFilename = "imgui.ini";
}

void DockableWindowsManager::addWindow(DockableWindow* window) {
    // Store the window pointer keyed by its unique name.
    m_windows[window->getName()] = window;
    m_layout_valid = false; // Layout must be rebuilt to include the new window.
}

void DockableWindowsManager::removeWindow(const std::string& name) {
    // Remove the window entry; it will no longer be rendered.
    m_windows.erase(name);
    m_layout_valid = false; // Layout must be rebuilt without this window.
}

void DockableWindowsManager::setCurrentLayout(DockLayoutNode layout) {
    // Save the new layout tree; mark as needing rebuild.
    m_current_layout = std::move(layout);
    m_has_layout = true;
    m_layout_valid = false;
}

void DockableWindowsManager::begin() {
    // Make the dockspace window cover the entire usable viewport area.
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    // Flags: no decorations, just a container for the docking area.
    ImGuiWindowFlags dockspace_flags =
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    // Remove padding and borders so the dockspace fills the window precisely.
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, dockspace_flags);
    ImGui::PopStyleVar(3);

    // Create the actual dockspace node.
    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspace_id);

    // On the very first frame, decide whether to restore saved layout or apply current.
    if (m_first_frame) {
        m_first_frame = false;
        if (!hasIniFile() && m_has_layout) {
            m_layout_valid = false;
        }
    }

    // If the layout is marked invalid and we have a layout + windows, rebuild now.
    if (!m_layout_valid && m_has_layout && !m_windows.empty()) {
        applyLayout(dockspace_id, m_current_layout);
        m_layout_valid = true;
    }
}

void DockableWindowsManager::renderWindows() {
    // Draw each registered window – ImGui will automatically dock them.
    for (const std::pair<const std::string, DockableWindow*>& kv : m_windows) {
        const std::string& name = kv.first;
        DockableWindow* window = kv.second;
        ImGui::Begin(name.c_str());
        window->render();
        ImGui::End();
    }
}

void DockableWindowsManager::end() {
    ImGui::End(); // Close the dockspace container window.
}



void DockableWindowsManager::applyLayout(ImGuiID dockspace_id, const DockLayoutNode& layout) {
    // Wipe any existing layout and create a fresh dockspace node.
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);

    // Recursively build the node tree.
    std::function<void(ImGuiID, const DockLayoutNode&)> build_node;
    build_node = [&](ImGuiID node_id, const DockLayoutNode& node) {
        if (node.type == DockLayoutNode::Window) {
            // Leaf: dock a named window into this node.
            ImGui::DockBuilderDockWindow(node.window_name.c_str(), node_id);
        }
        else if (node.type == DockLayoutNode::Split) {
            // Split the node into two children and recurse.
            ImGuiID child1_id, child2_id;
            ImGui::DockBuilderSplitNode(node_id, node.split_dir, node.split_ratio,
                &child1_id, &child2_id);
            build_node(child1_id, *node.child1);
            build_node(child2_id, *node.child2);
        }
        };
    build_node(dockspace_id, layout);

    // Finalize the docking layout.
    ImGui::DockBuilderFinish(dockspace_id);
}

bool DockableWindowsManager::hasIniFile() const {
    const char* ini_file = ImGui::GetIO().IniFilename;
    if (!ini_file || !*ini_file) return false;
    std::ifstream file(ini_file, std::ios::binary | std::ios::ate);
    // File exists and has more than a minimal size -> saved layout present.
    return file.good() && file.tellg() > 2;
}