#include "DockManager.h"
#include "imgui_internal.h"
#include <fstream>

DockManager::DockManager() {
    // Ensure ImGui saves/loads layout to an .ini file.
    if (ImGui::GetIO().IniFilename == nullptr)
        ImGui::GetIO().IniFilename = "imgui.ini";
}

void DockManager::RegisterWindow(const std::string& name, WindowRenderFunc func) {
    // Store the window name and its content-drawing callback.
    m_windows.push_back({ name, std::move(func) });
}

void DockManager::SetInitialLayout(DockLayoutNode root) {
    // Save the default layout tree; applied only on first launch.
    m_initial_layout = std::move(root);
    m_initial_layout_set = true;
}

static void BuildDockNode(ImGuiID node_id, const DockLayoutNode& layout) {
    if (layout.type == DockLayoutNode::Window) {
        // Leaf: dock a named window into this node.
        ImGui::DockBuilderDockWindow(layout.window_name.c_str(), node_id);
    }
    else if (layout.type == DockLayoutNode::Split) {
        // Split the node into two children and recurse.
        ImGuiID child1_id, child2_id;
        ImGui::DockBuilderSplitNode(node_id, layout.split_dir, layout.split_ratio,
            &child1_id, &child2_id);
        BuildDockNode(child1_id, *layout.child1);
        BuildDockNode(child2_id, *layout.child2);
    }
}

void DockManager::ApplyInitialLayout(ImGuiID dockspace_id) {
    // Wipe any existing layout and create a fresh dockspace node.
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);

    // Build the whole window hierarchy inside the dockspace.
    BuildDockNode(dockspace_id, m_initial_layout);

    // Finalize the docking layout.
    ImGui::DockBuilderFinish(dockspace_id);
}

void DockManager::Begin() {

    // Make the dockspace window cover the entire usable viewport area.
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    // Flags.
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

    // On the very first frame, decide whether to apply the default layout
    if (m_first_frame) {
        m_first_frame = false;
        bool needInitialLayout = true;

        // Check if a saved .ini file already exists and has content
        const char* iniFile = ImGui::GetIO().IniFilename;
        if (iniFile && *iniFile) {
            std::ifstream file(iniFile, std::ios::binary | std::ios::ate);
            if (file.good()) {
                // File exists and has more than a minimal size -> saved layout present
                if (file.tellg() > 2)
                    needInitialLayout = false;
            }
        }

        // Apply default layout only when there is no saved state
        if (needInitialLayout && m_initial_layout_set) {
            ApplyInitialLayout(dockspace_id);
        }
    }
}

void DockManager::RenderWindows() {
    // Draw each registered window – ImGui will automatically dock them
    for (WindowInfo& win : m_windows) {
        ImGui::Begin(win.name.c_str());
        win.render(); // Call the user-provided content function
        ImGui::End();
    }
}

void DockManager::End() {
    ImGui::End(); // Close the dockspace container window
}