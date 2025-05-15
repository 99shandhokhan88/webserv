#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../../HTTP/incs/Request.hpp"
#include <string>
#include <poll.h>
#include "../../HTTP/incs/Response.hpp"

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


    bool isRequestComplete() ;

    explicit Client(int client_fd = -1);
    ~Client() {};

    // Request handling
void handleRequest(class Server& server);

    std::string readData();
    void prepare_response(const std::string& content);
    bool send_pending_data();

    // Data management
    void appendRequestData(const char* data, size_t length);
    bool shouldKeepAlive() const;
    void setKeepAlive(bool value);
};

#endif