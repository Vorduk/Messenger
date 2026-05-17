#pragma once
#include "imgui.h"
#include "IDockableWindow.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <memory>

/**
 * @brief Describes a single node in the docking layout tree.
 * Can be either a leaf window or a split container with two children.
 */
struct DockLayoutNode {
    enum Type { Window, Split };            ///< Type of the node: leaf window or directional split.
    Type type;                              ///< Node type.
    std::string window_name;                ///< Window title.
    ImGuiDir split_dir = ImGuiDir_Left;     ///< Split direction.
    float split_ratio = 0.5f;               ///< Ratio of the split (0.0 – 1.0).
    std::unique_ptr<DockLayoutNode> child1; ///< First child (top/left).
    std::unique_ptr<DockLayoutNode> child2; ///< Second child (bottom/right).

    /**
     * @brief Factory for a leaf node that holds a single window.
     * @param name Window title string.
     * @return DockLayoutNode configured as a Window leaf.
     */
    static DockLayoutNode makeWindow(const char* name) {
        DockLayoutNode node;
        node.type = Window;
        node.window_name = name;
        return node;
    }

    /**
     * @brief Factory for a split node with two children.
     * @param dir Direction of the split (ImGuiDir_Left, ImGuiDir_Right, etc.).
     * @param ratio Fraction of space given to the first child.
     * @param c1  First child node.
     * @param c2  Second child node.
     * @return DockLayoutNode configured as a Split node.
     */
    static DockLayoutNode makeSplit(ImGuiDir dir, float ratio,
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

/**
 * @brief Manages a full-window docking space with registered windows.
 *
 * Handles the creation of the main dockspace, dynamic addition/removal
 * of windows, per-frame rendering, and automatic save/restore of the
 * layout via the ImGui .ini file.
 */
class DockManager {
public:
    /**
     * @brief Constructor. Configures the ImGui .ini filename if not already set.
     */
    DockManager();

    /**
     * @brief Add a window to be rendered every frame inside the dockspace.
     * @param window Pointer to the dockable window (ownership remains outside).
     */
    void addWindow(IDockableWindow* window);

    /**
     * @brief Remove a previously added window by its unique name.
     * @param name The window's title (as returned by GetName()).
     */
    void removeWindow(const std::string& name);

    /**
     * @brief Set the docking layout to be applied in the current frame.
     *
     * The layout is applied immediately on the next Begin() call,
     * regardless of any previously saved .ini state.
     *
     * @param layout Root node of the desired layout.
     */
    void setCurrentLayout(DockLayoutNode layout);


    //////////// Call every frame in UI ////////////

    /**
     * @brief Must be called at the beginning of the UI frame.
     * Creates the dockspace window and applies the current layout if necessary.
     */
    void begin();

    /**
     * @brief Renders all currently added windows.
     * Call this between begin() and end().
     */
    void renderWindows();

    /**
     * @brief Must be called at the end of the UI frame. Closes the dockspace window.
     */
    void end();

private:
    /**
     * @brief Recursively build the docking node hierarchy.
     * @param dockspace_id The ImGui ID of the root dockspace node.
     * @param layout       The layout tree to apply.
     */
    void applyLayout(ImGuiID dockspace_id, const DockLayoutNode& layout);

    /**
     * @brief Check if a saved .ini file already exists and has content.
     * @return true if a non-empty ini file is present.
     */
    bool hasIniFile() const;

    std::unordered_map<std::string, IDockableWindow*> m_windows; ///< Active windows by name.
    DockLayoutNode m_current_layout;     ///< Layout to apply when needed.
    bool m_layout_valid = false;         ///< Whether the current layout matches the windows.
    bool m_first_frame = true;           ///< True until the first begin() completes.
    bool m_has_layout = false;
};