#pragma once
#include <string>

class TcpClient {
public:
	TcpClient(const std::string& server_address, int port);
	~TcpClient();

	bool connectToServer();
	std::string sendRequest(const std::string& request_json);

private:
	int m_socket = -1;					///< Socket descriptor, (-1 means not created).
	std::string m_server_address;		///< Server ip or name.
	int m_port;							///< Port.
	bool m_is_server_connected = false;	///< Connection flag.
};