#pragma once
#include <string>
#include <openssl/ssl.h>

/**
 * @brief TCP client for communicating with the messenger server.
 *
 * Manages a single persistent TCP connection to the server.
 * All network errors are handled internally - the caller receives
 * an empty string or error JSON on failure.
 * 
 * Protocol format:
 *   Request:  JSON + '\n'
 *   Response: JSON + '\n'
 *
 * Blocks on sendRequest(). Designed as a building block for AsyncNetworkClient.
 * Do NOT call from the UI thread directly.
 */
class TcpClient {
public:
	TcpClient(const std::string& server_address, int port);
	~TcpClient();

    /**
     * @brief Opens a blocking TCP connection to the server.
     * @return true on success, false on failure.
     */
	bool connectToServer();

    /**
     * @brief Sends a JSON request and blocks until a line-terminated response is received.
     * @param request_json Single-line JSON string (newline appended automatically).
     * @return Server response as a string, or empty string / error JSON on failure.
     */
	std::string sendRequest(const std::string& request_json);
	bool isConnected() const;
	void setTimeouts(int send_timeout_ms, int recv_timeout_ms);
	void disconnect();

	// Turn on TLS-encryption (Call before connectToServer)
	void enableTls();

private:
	void closeSocket();
	void cleanupSsl();	///< Free sscl resources.

	int m_socket = -1;					///< Socket descriptor, (-1 means not created).
	std::string m_server_address;		///< Server ip or name.
	int m_port;							///< Port.
	bool m_is_server_connected = false;	///< Connection flag.
    std::string m_leftover_buffer;      ///< Buffer for the bytes that left after /n after request	
	bool m_has_timeouts = false;

	// TLS
	bool m_use_tls = false;
	SSL_CTX* m_ssl_ctx = nullptr;
	SSL* m_ssl = nullptr;
};

