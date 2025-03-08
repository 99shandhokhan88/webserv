#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../../HTTP/incs/Request.hpp"
#include <string>
#include <poll.h>
#include <sys/types.h>
#include <cstdio>
#include <sstream>

class Server;  // Forward declaration

class Client {
private:  // Private members first to ensure visibility in initializer list
    std::string pending_data;
    bool        keep_alive;
    

public:
    int         fd;
    Request     request;
    std::string request_data;

     void set_keep_alive(bool value) { keep_alive = value; }

    explicit Client(int client_fd = -1) : 
        keep_alive(false),
        fd(client_fd),
        request()  // Explicitly initialize Request
    {}
    
    ~Client() {}

    void handleRequest(Server& server);
    bool send_pending_data();
    bool should_keep_alive() const { return keep_alive; }
    
    std::string readData();
    void sendData(const std::string& data);
    void prepare_response(const std::string& content);
};

#endif