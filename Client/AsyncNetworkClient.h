#pragma once
#include <string>
#include <functional>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <memory>

class TcpClient;

/**
 * @brief Manages asynchronous TCP communication with the server on a dedicated thread,
 * queuing outgoing requests and delivering responses via callbacks executed in the UI thread.
 */
class AsyncNetworkClient {
public:
    using ResponseCallback = std::function<void(const std::string& response)>;

    AsyncNetworkClient(const std::string& server_address, int port);
    ~AsyncNetworkClient();

    // Can be called from any thread, wil be executed asynchronously.
    void sendRequest(const std::string& json, ResponseCallback onResponse);

    // Check status of connection.
    bool isConnected() const { return m_is_server_connected; }

    // Execute callbacks, call from main loop.
    void pollCallbacks();

private:
    void workerLoop();
    void processQueue();

    std::unique_ptr<TcpClient> m_client;    ///< Tcp client.
    std::string m_server_address;           ///< Server address.
    int m_port;                             ///< Port.

    struct Request {
        std::string json;           ///< Request.
        ResponseCallback callback;  ///< On response callback
    };

    std::queue<Request> m_queue;    ///< Request quqeue.
    std::mutex m_mutex;             ///< Mutex.
    std::condition_variable m_cv;   ///< Conditional variable.
    std::thread m_worker_thread;    ///< Worker thread.

    std::atomic<bool> m_is_server_connected{ false };   ///< Connection flag.
    std::atomic<bool> m_is_running{ true };             ///< Working flag.

    std::queue<std::function<void()>> m_pending_callbacks;  ///< Queue for callbacks that should be executed in main thread.
    std::mutex m_callback_mutex;
};