#pragma once
#include "imgui.h"
#include "IDockableWindow.h"
#include <functional>
#include <string>
#include <vector>
#include <memory>

/**
 * @brief Describes a single node in the initial docking layout tree.
 *
 * Can be either a leaf window or a split container with two children.
 */
struct DockLayoutNode {
    /// Type of the node: leaf window or directional split.
    enum Type { Window, Split };
    Type type;                              ///< Node type.
    std::string window_name;                ///< Window title for leaf nodes.
    ImGuiDir split_dir = ImGuiDir_Left;     ///< Split direction (used when type == Split).
    float split_ratio = 0.5f;               ///< Ratio of the split (0.0 – 1.0).
    std::unique_ptr<DockLayoutNode> child1; ///< First child (top/left).
    std::unique_ptr<DockLayoutNode> child2; ///< Second child (bottom/right).

    /**
     * @brief Factory for a leaf node that holds a single window.
     * @param name Window title string.
     * @return DockLayoutNode configured as a Window leaf.
     */
    static DockLayoutNode MakeWindow(const char* name) {
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
    static DockLayoutNode MakeSplit(ImGuiDir dir, float ratio, DockLayoutNode c1, DockLayoutNode c2) {
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
 * Handles the creation of the main dockspace, optional initial layout,
 * per-frame rendering of registered windows, and automatic save/restore
 * of the layout via the ImGui .ini file.
 */
class DockManager {
public:

    /// Callback type for a window's content rendering function.
    using WindowRenderFunc = std::function<void()>;

    /**
     * @brief Constructor. Configures the ImGui .ini filename if not already set.
     */
    DockManager();

    /**
     * @brief Register a window to be rendered every frame inside the dockspace.
     * @param name  Unique window title (used for docking identification).
     * @param func  Callback that renders the window's contents.
     */
    void RegisterWindow(IDockableWindow* window);

    /**
     * @brief Provide a default layout tree that is applied only on the very
     * first launch (when no .ini file exists yet).
     * @param root Root node of the desired initial layout.
     */
    void SetInitialLayout(DockLayoutNode root);

    //////////// Call every frame in UI ////////////

    /**
     * @brief Must be called at the beginning of the UI frame.
     * Creates the dockspace window and applies the initial layout if necessary.
     */
    void Begin();

    /**
     * @brief Renders all previously registered windows.
     * Call this between Begin() and End().
     */
    void RenderWindows();

    /**
     * @brief Must be called at the end of the UI frame. Closes the dockspace window.
     */
    void End();

private:

    /**
     * @brief Build the full node hierarchy starting from an empty dockspace.
     * @param dockspace_id The ImGui ID of the root dockspace node.
     */
    void ApplyInitialLayout(ImGuiID dockspace_id);

    std::vector<IDockableWindow*> m_windows;  ///< All registered windows.
    DockLayoutNode m_initial_layout;    ///< Default layout (used once).
    bool m_initial_layout_set = false;  ///< Whether a default layout was provided.
    bool m_first_frame = true;          ///< True until the first Begin() completes.
};