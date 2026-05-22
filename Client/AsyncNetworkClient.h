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
 *        queuing outgoing requests and delivering responses via callbacks executed in the UI thread.
 *
 * This class implements two separate queues:
 * - Request queue (main thread --> worker thread): stores outgoing JSON requests.
 * - Callback queue (worker thread --> main thread): stores response callbacks to be
 *   executed on the main thread.
 *
 * ## Thread safety
 * - sendRequest() can be called from any thread.
 * - pollCallbacks() must be called from the main/UI thread once per frame.
 * - isConnected() is safe to call from any thread.
 *
 * @note If the initial connection fails, the worker thread still starts and will attempt
 *       to send requests (which will fail with an error response). Reconnection logic
 *       should be added externally if needed.
 */
class AsyncNetworkClient {
public:

    /**
     * @brief Callback type for receiving server responses.
     *
     * The callback is invoked on the main thread during pollCallbacks().
     * The string parameter contains the raw JSON response from the server,
     * or an error JSON like `{"status":"error","message":"..."}` on failure.
     */
    using ResponseCallback = std::function<void(const std::string& response)>;

    using ReconnectCallback = std::function<void()>; ///< Called after successful reconnection

    AsyncNetworkClient(const std::string& server_address, int port);
    ~AsyncNetworkClient();

    /**
     * @brief Queues a JSON request for asynchronous processing.
     *
     * Thread-safe. Can be called from any thread.
     * If the client is not connected, the callback is invoked immediately
     * with an error response.
     *
     * @param json       Single-line JSON request string (newline is appended automatically).
     * @param onResponse Callback invoked on the main thread when the response arrives.
     */
    void sendRequest(const std::string& json, ResponseCallback onResponse);

    /**
     * @brief Returns the current connection status.
     *
     * Thread-safe. Can be called from any thread.
     * @return true if connected to the server, false otherwise.
     */
    bool isConnected() const { return m_is_server_connected; }

    /**
     * @brief Executes all pending response callbacks on the calling thread.
     *
     * Must be called from the main/UI thread once per frame in the render loop.
     * This is required because UI frameworks like ImGui are not thread-safe.
     */
    void pollCallbacks();

    /// Set callback to be invoked after automatic reconnection succeeds
    void setReconnectCallback(ReconnectCallback callback);

private:
    /**
     * @brief Main function executed by the worker thread.
     *
     * Waits on the condition variable for new requests, sends them via TcpClient,
     * and queues the responses into m_pending_callbacks.
     * Exits when m_is_running becomes false.
     */
    void workerLoop();

    std::unique_ptr<TcpClient> m_client;    ///< Blocking TCP client.
    std::string m_server_address;           ///< Server address.
    int m_port;                             ///< Port.

    /**
     * @brief A queued request waiting to be sent.
     */
    struct Request {
        std::string json;           ///< Request.
        ResponseCallback callback;  ///< On response callback
    };

    std::queue<Request> m_queue;    ///< Request quqeue.
    std::mutex m_mutex;             ///< Mutex.
    std::condition_variable m_cv;   ///< Conditional variable.
    std::thread m_worker_thread;    ///< Worker thread.
    std::atomic<bool> m_is_server_connected{ false };       ///< Connection flag.
    std::atomic<bool> m_is_running{ true };                 ///< Working flag.
    std::queue<std::function<void()>> m_pending_callbacks;  ///< Queue for callbacks that should be executed in main thread.
    std::mutex m_callback_mutex;                            ///< Mutex protecting m_pending_callbacks.
    ReconnectCallback m_reconnect_callback;                 ///< Called when reconnection succeeds

    /**
     * @brief Processes a single request from the front of the queue.
     */
    void processQueue(Request request);
};