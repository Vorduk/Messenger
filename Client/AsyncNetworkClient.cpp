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
    m_worker_thread = std::thread(&AsyncNetworkClient::workerLoop, this);
}

AsyncNetworkClient::~AsyncNetworkClient() {
    m_is_running = false;   // Stop
    m_cv.notify_all();      // Wake up worker thread.
    if (m_worker_thread.joinable()) m_worker_thread.join(); // Stop worker thread if its ready.
}

void AsyncNetworkClient::sendRequest(const std::string& json, ResponseCallback onResponse) {
    if (!m_is_server_connected) {
        if (onResponse) onResponse(R"({"status":"error","message":"Not connected"})");
        return;
    }
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push({ json, std::move(onResponse) });
    m_cv.notify_one();
}

void AsyncNetworkClient::workerLoop() {
    while (m_is_running) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this] { return !m_queue.empty() || !m_is_running; });
        if (!m_is_running) break;

        Request request = std::move(m_queue.front());
        m_queue.pop();
        lock.unlock();

        std::string response = m_client->sendRequest(request.json);
        if (request.callback) {
            std::lock_guard<std::mutex> lock(m_callback_mutex);
            m_pending_callbacks.push([cb = std::move(request.callback), response] { cb(response); });
        }
    }
}

void AsyncNetworkClient::pollCallbacks() {
    std::lock_guard<std::mutex> lock(m_callback_mutex);
    while (!m_pending_callbacks.empty()) {
        m_pending_callbacks.front()();
        m_pending_callbacks.pop();
    }
}