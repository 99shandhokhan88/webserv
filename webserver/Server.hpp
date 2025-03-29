#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

// Server class declaration
class Server {
public:
    int server_fd;
    struct sockaddr_in address; //A structure (sockaddr_in) that stores the server’s address information (IP and port).

    Server(int port); //will initialize a server to listen on a specified port
    ~Server();
};

// Function prototypes
void setupServers(const std::vector<int>& ports);
void acceptConnections();
void cleanup();

#endif // SERVER_HPP
