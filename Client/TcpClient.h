#pragma once
#include <string>

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

private:
	int m_socket = -1;					///< Socket descriptor, (-1 means not created).
	std::string m_server_address;		///< Server ip or name.
	int m_port;							///< Port.
	bool m_is_server_connected = false;	///< Connection flag.
    std::string m_leftover_buffer;      ///< Buffer for the bytes that left after /n after request
};
