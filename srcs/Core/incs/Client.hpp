#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../../HTTP/incs/Request.hpp"
#include <string>
#include <poll.h>

class Server;  // Forward declaration

class Client {
private:
    std::string pending_data;
    bool keep_alive;

public:
    int fd;
    Request request;
    std::string request_data;
    void parseRequest();



    void reset() {
        request_data.clear();
        // Reset other request-related state
    }


    bool isRequestComplete() const {
        // Check for both headers and content-length if present
        size_t header_end = request_data.find("\r\n\r\n");
        if (header_end == std::string::npos) return false;
        
        // Check for body completeness if needed
        return true;
    }

    explicit Client(int client_fd = -1);
    ~Client() {};

    // Request handling
    void handleRequest(Server& server);
    std::string readData();
    void prepare_response(const std::string& content);
    bool send_pending_data();

    // Data management
    void appendRequestData(const char* data, size_t length);
    bool shouldKeepAlive() const;
    void setKeepAlive(bool value);
};

#endif