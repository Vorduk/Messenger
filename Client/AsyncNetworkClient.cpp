#include "pch.h"
#include "AsyncNetworkClient.h"
#include "TcpClient.h"
#include "LogMacros.h"

AsyncNetworkClient::AsyncNetworkClient(const std::string& server_address, int port)
    : m_server_address(server_address), m_port(port) {

    m_client = std::make_unique<TcpClient>(server_address, port);

    if (m_client->connectToServer()) {
        m_is_server_connected = true;
        LOG_INFO("Connected to server {}:{}", server_address, port);
    }
    else {
        LOG_WARN("Failed to connect to server {}:{}", server_address, port);
    }
    m_worker_thread = std::thread(&AsyncNetworkClient::workerLoop, this); // Create worker thread for this object.
}

AsyncNetworkClient::~AsyncNetworkClient() {
    m_is_running = false;   // Signal the worker thread to stop.
    m_cv.notify_all();      // Wake up the worker thread if it is sleeping on the condition variable.
    if (m_worker_thread.joinable()) {
        m_worker_thread.join(); // Wait for the worker thread to finish execution.
    }
}

void AsyncNetworkClient::sendRequest(const std::string& json, ResponseCallback onResponse) {
    if (!m_is_server_connected) {
        if (onResponse) onResponse(R"({"status":"error","message":"Not connected"})"); // Immediately report error if not connected.
        return;
    }
    std::lock_guard<std::mutex> lock(m_mutex); // Lock the mutex to safely push into the shared queue.
    m_queue.push({ json, std::move(onResponse) }); // Enqueue the request with its callback.
    m_cv.notify_one(); // Wake up the worker thread to process the new request.
}

void AsyncNetworkClient::processQueue(Request request) {
    std::string response = m_client->sendRequest(request.json); // Perform the blocking TCP request.
    if (request.callback) {
        std::lock_guard<std::mutex> cb_lock(m_callback_mutex); // Lock the callback mutex to safely push the pending callback.
        m_pending_callbacks.push([cb = std::move(request.callback), response] { cb(response); }); // Enqueue the callback for later execution on the main thread.
    }
}

void AsyncNetworkClient::workerLoop() {
    while (m_is_running) {
        std::unique_lock<std::mutex> lock(m_mutex); // Acquire the mutex to check the queue safely.
        m_cv.wait(lock, [this] { return !m_queue.empty() || !m_is_running; }); // Sleep until a request arrives or stop is requested.
        if (!m_is_running) break; // Exit the loop if destruction has been requested.
        
        Request request = std::move(m_queue.front());
        m_queue.pop();

        lock.unlock();
        
        processQueue(std::move(request)); // Delegate request processing to a separate method.
    }
}

void AsyncNetworkClient::pollCallbacks() {
    std::lock_guard<std::mutex> lock(m_callback_mutex); // Lock the mutex to safely access the callback queue.
    while (!m_pending_callbacks.empty()) {
        m_pending_callbacks.front()(); // Execute the callback on the calling (main) thread.
        m_pending_callbacks.pop(); // Remove the executed callback from the queue.
    }
}

void AsyncNetworkClient::setReconnectCallback(ReconnectCallback callback) {
    m_reconnect_callback = std::move(callback);
}