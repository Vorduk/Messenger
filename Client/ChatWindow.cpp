#include "pch.h"
#include "ChatWindow.h"
#include "imgui.h"
#include <cstring>
#include <chrono>
#include <ctime>

ChatWindow::ChatWindow(const ChatWindowStyle& current_style) 
    : m_current_style(current_style)
{
    std::memset(m_input_buffer, 0, sizeof(m_input_buffer)); // Clear the input buffer.
}

const char* ChatWindow::getName() const
{
    return "Chat";
}

void ChatWindow::render() {

    // Declare local variables to hold a snapshot of shared state under the mutex.
    std::vector<DisplayMessage> messages;   // Local copy of the message list.
    std::string receiver_name;              // Local copy of the chat partner's display name.
    std::string sender_id;                  // Local copy of the current user's ID.
    std::string receiver_id;                // Local copy of the chat partner's ID.
    bool has_handler = false;               // Flag indicating if a send message handler is set.
    GetMessagesUseCase* loader = nullptr;   // Local pointer to the message loader use case.
    {
        std::lock_guard<std::mutex> lock(m_mutex);  // Lock the mutex to safely read shared state.
        messages = m_messages;                      // Copy the message list.
        receiver_name = m_receiver_name;            // Copy the receiver's display name.
        sender_id = m_sender_id;                    // Copy the sender's ID.
        receiver_id = m_receiver_id;                // Copy the receiver's ID.
        has_handler = (m_handler != nullptr);       // Check if a message handler is assigned.
        loader = m_message_loader;                  // Copy the message loader pointer.
    } // Mutex is automatically unlocked when the lock_guard goes out of scope.

    // Non-blocking periodic refresh of message history from the server
    if (loader && !receiver_id.empty()) { // Only refresh if a loader is set and a receiver is selected.
        double now = ImGui::GetTime(); // Get the current time in seconds.
        if (now - m_last_message_refresh > REFRESH_INTERVAL) { // Check if the refresh interval has passed.
            m_last_message_refresh = now; // Update the last refresh timestamp.
            loader->execute(sender_id, receiver_id, 50, 0, // Execute the use case to get messages asynchronously.
                [this](std::vector<Message> newMessages) { // Define a callback for when the server responds.
                    SetHistory(newMessages); // Replace the current message history with the new data.
                });
        }
    }

    renderHeader();
    ImGui::PushStyleColor(ImGuiCol_Separator, m_current_style.separator_color);
    ImGui::Separator();
    ImGui::PopStyleColor();
    renderMessages(messages);
    renderInputArea(has_handler, sender_id, receiver_id);
}

void ChatWindow::renderMessages(const std::vector<DisplayMessage>& messages) {
    const float footer_height = ImGui::GetFrameHeightWithSpacing(); // Calculate the height to reserve for the input area below the message list.
    ImGui::BeginChild("ChatHistory", ImVec2(0, -footer_height), true);  // Start a child window that takes up all remaining space above the input area.
    ImGui::PushStyleColor(ImGuiCol_ChildBg, m_current_style.chat_background_color); // Set the background color of the message area.

    float available_width = ImGui::GetContentRegionAvail().x; // Get the width available for the message list.
    float max_width = available_width * m_current_style.message_style.max_text_width_ratio; // Calculate the maximum width for a single message bubble

    // Render message bubbles.
    for (const DisplayMessage& dm : messages) {
        renderMessageBubble(dm, max_width, available_width);
        ImGui::Spacing();
    }

    // Auto-scroll to the bottom if the user is already at or near the bottom of the chat history.
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 5.0f) { // Check if the current scroll position is close to the maximum.
        ImGui::SetScrollHereY(1.0f); // Set the scroll position to the bottom.
    }

    ImGui::PopStyleColor();
    ImGui::EndChild();
}

void ChatWindow::renderMessageBubble(const DisplayMessage& dm, float max_width, float available_width) {
    
    // Align the bubble to the right if it was sent by the local user.
    if (dm.is_local) { // Check if the message is from the local user.
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + available_width - max_width); // Push the cursor to the right edge.
    }

    float padding = m_current_style.message_style.bubble_padding;   // Get the internal padding of the bubble from the style.
    float text_wrap_width = max_width - padding * 2;    // Calculate the available width for text inside the bubble, accounting for padding.
    ImVec2 text_size = ImGui::CalcTextSize(dm.text.c_str(), nullptr, false, text_wrap_width); // Calculate the size the text will occupy.

    // Prepare the status and time strings to be displayed in the bubble's footer.
    std::string status_str = messageStatusToString(dm.status); // Convert the message status enum to a string.
    float status_width = status_str.empty() ? 0 : ImGui::CalcTextSize(status_str.c_str()).x + 4.0f; // Calculate the width of the status text, adding a small margin.
    std::string time_str = formatTime(dm.timestamp); // Format the message timestamp into a "HH:MM" string.
    float time_width = ImGui::CalcTextSize(time_str.c_str()).x; // Calculate the width of the time text.
    float bottom_row_width = time_width + status_width; // Calculate the total width of the bottom row containing time and status.

    float block_height = text_size.y + ImGui::GetTextLineHeightWithSpacing() + padding * 2; // Calculate the total height of the message bubble.

    // Choose the background color based on whether the message is local or from the receiver.
    ImVec4 bg_color = dm.is_local ? m_current_style.message_style.sender_bg_color : m_current_style.message_style.reciever_bg_color;
    ImGui::PushStyleColor(ImGuiCol_ChildBg, bg_color); // Set the background color of the bubble.
    ImGui::PushStyleColor(ImGuiCol_Border, m_current_style.message_style.border_color); // Set the border color of the bubble.
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, m_current_style.message_style.border_width); // Set the border width.
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, m_current_style.message_style.bubble_rounding); // Set the corner rounding of the bubble.

    std::string child_id = "msg_" + dm.id; // Create a unique ID for the child window based on the message ID.
    ImGui::BeginChild(child_id.c_str(), ImVec2(max_width, block_height), false, ImGuiWindowFlags_NoScrollbar); // Begin a child window for the message bubble.

    ImGui::SetCursorPos(ImVec2(padding, padding)); // Set the cursor to the top-left corner with padding applied.

    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + text_wrap_width); // Set the position at which text will wrap to the next line.

    // Render the message text using the configured style.
    ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.message_style.text_style.color); // Set the text color.
    ImGui::TextWrapped("%s", dm.text.c_str()); // Render the message text with wrapping.
    ImGui::PopStyleColor(); // Restore the previous text color.

    ImGui::PopTextWrapPos(); // Restore the previous text wrap position.

    // Position the bottom row containing the time and status, right-aligned.
    float bottom_y = block_height - ImGui::GetTextLineHeightWithSpacing() - padding; // Calculate the Y position for the bottom row.
    ImGui::SetCursorPos(ImVec2(padding, bottom_y)); // Set the cursor to the left side of the bottom row.
    float avail_bottom = max_width - padding * 2; // Calculate the available width for the bottom row content.
    float cursor_x = avail_bottom - bottom_row_width; // Calculate the X offset needed to right-align the content.
    ImGui::SetCursorPosX(padding + cursor_x); // Set the cursor X position to right-align the text.

    ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.message_style.time_text_style.color); // Set the color for the time text.
    ImGui::Text("%s", time_str.c_str()); // Render the formatted time string.
    ImGui::PopStyleColor(); // Restore the previous text color.

    // Render the message status text next to the time if it's not empty.
    if (!status_str.empty()) { // Check if there is a status string to display.
        ImGui::SameLine(); // Draw the status on the same line as the time.
        int status_idx = static_cast<int>(dm.status); // Get the integer value of the status enum.
        if (status_idx < 0 || status_idx > 5) status_idx = 5; // Clamp the status index to the valid range for the color array.
        ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.message_style.status_style[status_idx].color); // Set the color for the status text.
        ImGui::Text("%s", status_str.c_str()); // Render the status string.
        ImGui::PopStyleColor(); // Restore the previous text color.
    }

    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

void ChatWindow::renderInputArea(bool has_handler, const std::string& sender_id, const std::string& receiver_id) {
    ImGui::PushStyleColor(ImGuiCol_FrameBg, m_current_style.input_background_color); // Set the background color of the input text field.
    ImGui::PushItemWidth(-1); // Set the input field to stretch and fill the available width.

    bool send_pressed = ImGui::InputTextWithHint("##MessageInput",  // Create an input text widget.
        m_current_style.input_hint_text.c_str(),                    // Set the hint text that appears when the field is empty.
        m_input_buffer, IM_ARRAYSIZE(m_input_buffer),               // Pass the buffer and its size.
        ImGuiInputTextFlags_EnterReturnsTrue);                      // Make the Enter key trigger a return value of true.

    ImGui::PopItemWidth(); // Restore the previous item width.
    ImGui::PopStyleColor(); // Restore the previous frame background color.

    ImGui::SameLine(); // Place the send button on the same line as the input field.

    // Apply style colors for the send button.
    ImGui::PushStyleColor(ImGuiCol_Button, m_current_style.send_button_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, m_current_style.send_button_hover_color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, m_current_style.send_button_active_color);
    ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.send_button_text_style.color);

    // Check if the send button is clicked or Enter was pressed while the input has focus.
    if (ImGui::Button(m_current_style.send_button_text.c_str()) || send_pressed) {
        if (has_handler && strlen(m_input_buffer) > 0) { // Check if a handler is set and the input is not empty.
            if (!sender_id.empty() && !receiver_id.empty()) { // Verify both sender and receiver IDs are valid.
                std::string text = m_input_buffer; // Copy the text from the input buffer.
                std::string temp_id = "temp_" + // Generate a temporary message ID using the current timestamp.
                    std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
                AddMessage(sender_id, text, true, std::chrono::system_clock::now(), temp_id, MessageStatus::Sending); // Add the message locally with a "Sending" status.
                m_handler->onUserSendMessage(sender_id, receiver_id, text); // Notify the handler to send the message over the network.
                std::memset(m_input_buffer, 0, sizeof(m_input_buffer)); // Clear the input buffer after sending.
            }
        }
    }
    ImGui::PopStyleColor(4);
}

void ChatWindow::SetHandler(ISendMessageHandler* handler) {
    std::lock_guard<std::mutex> lock(m_mutex); // Lock the mutex to safely write shared state.
    m_handler = handler; // Store the pointer to the handler.
}

void ChatWindow::SetUsers(const std::string& sender_id,
    const std::string& receiver_id,
    const std::string& receiver_name) {
    std::lock_guard<std::mutex> lock(m_mutex);  // Lock the mutex for safe access.
    m_sender_id = sender_id;                    // Set the local user's ID.
    m_receiver_id = receiver_id;                // Set the remote user's ID.
    m_receiver_name = receiver_name;            // Set the remote user's display name.
    m_messages.clear();     // Clear all existing messages for the new conversation.
}

void ChatWindow::AddMessage(const std::string& sender,
    const std::string& text,
    bool is_local,
    const std::chrono::system_clock::time_point& timestamp,
    const std::string& msg_id,
    MessageStatus status) {
    bool local = is_local ? is_local : (sender == m_sender_id); // Determine if the message is local: use the provided flag or compare sender IDs.
    std::lock_guard<std::mutex> lock(m_mutex); // Lock the mutex to safely modify the message vector.

    std::string id = msg_id; // Use the provided message ID.
    if (id.empty()) { // If no ID was provided, generate a unique one based on the message count and timestamp.
        id = "msg_" + std::to_string(m_messages.size()) + "_" +
            std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    }

    m_messages.push_back({ id, text, local, timestamp, status }); // Construct a DisplayMessage and add it to the end of the list.
}

void ChatWindow::SetHistory(const std::vector<Message>& messages) {
    std::lock_guard<std::mutex> lock(m_mutex); // Lock the mutex.
    m_messages.clear(); // Remove all existing messages.
    for (size_t i = messages.size(); i > 0; --i) { // Iterate through the server messages in reverse order.
        const Message& msg = messages[i - 1]; // Get the current message from the original list.
        bool local = (msg.sender_id == m_sender_id); // Check if the message was sent by the local user.
        m_messages.push_back({ msg.id, msg.text, local, msg.timestamp, msg.status }); // Convert to DisplayMessage and add to the list.
    }
}

void ChatWindow::SetMessageLoader(GetMessagesUseCase* loader) {
    std::lock_guard<std::mutex> lock(m_mutex);  // Lock the mutex for safe write.
    m_message_loader = loader;  // Store the loader pointer.
}

void ChatWindow::renderHeader() {
    if (!m_receiver_name.empty()) { // Check if a receiver name is set (i.e., a conversation is active).
        ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.chat_header_text_style.color); // Set the text color for the header.
        ImGui::Text("%s%s", m_current_style.chat_header_prefix.c_str(), m_receiver_name.c_str()); // Display the prefix and the receiver's name.
        ImGui::PopStyleColor();
    }
    else { // No conversation is active, show a placeholder message.
        ImGui::PushStyleColor(ImGuiCol_Text, m_current_style.empty_chat_text_style.color); // Set the color for the placeholder text.
        ImGui::Text("%s", m_current_style.empty_chat_text.c_str()); // Display the empty state text.
        ImGui::PopStyleColor();
    }
}

std::string ChatWindow::formatTime(const std::chrono::system_clock::time_point& tp) const {
    using namespace std::chrono; // Use the chrono namespace for convenience.
    time_t t = system_clock::to_time_t(tp); // Convert the time_point to a time_t.
    std::tm tm; // Create a tm struct to hold the broken-down local time.
#ifdef _WIN32
    localtime_s(&tm, &t); // Windows-specific thread-safe function to convert to local time.
#else
    localtime_r(&t, &tm); // POSIX thread-safe function to convert to local time.
#endif
    char buf[16]; // Create a character buffer for the formatted time.
    std::strftime(buf, sizeof(buf), "%H:%M", &tm); // Format the time into the buffer as "HH:MM".
    return std::string(buf); // Return the formatted string.
}
