#include "Server.hpp"
"""
    Socket Creation: A socket is created using the socket() function.
    Binding: The server binds the socket to a specific address and port using bind().
    Listening: The server listens for incoming client connections using listen().
    Accepting: Once a client connects, the server accepts the connection using accept()
"""
std::vector<Server*> servers;
std::vector<struct pollfd> poll_fds;

// Server constructor
Server::Server(int port) {
    // Create the socket (IPv4, TCP)
    //This creates a socket using the IPv4 protocol (AF_INET) and the TCP protocol (SOCK_STREAM). It returns a file descriptor (server_fd) which will be used for communication.
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure socket options (Reuse address)
    int opt = 1;
    //sets the socket option SO_REUSEADDR so the server can reuse the address and port.
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Set up address struct
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind the socket
    //Binds the socket to a specific IP address and port, defined by the sockaddr_in structure. This tells the operating system which network interface and port the server should listen on.
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    //Sets the server socket into listen mode. It specifies how many connections the server can queue while waiting to accept them. Here, SOMAXCONN is used to allow as many connections as the OS allows.
    if (listen(server_fd, SOMAXCONN) == -1) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    // Set non-blocking mode
    //ets the server socket into non-blocking mode. This means the server will not block on accept() or recv() calls; instead, it will return immediately if there’s no data or no incoming connections. This is useful for multiplexing I/O (handling multiple connections simultaneously).
    if (fcntl(server_fd, F_SETFL, O_NONBLOCK) == -1) {
        perror("Failed to set non-blocking");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server started on port " << port << std::endl;
}

// Server destructor
Server::~Server() {
    close(server_fd);
    std::cout << "Server on port " << ntohs(address.sin_port) << " closed." << std::endl;
}

// Setup multiple servers
void setupServers(const std::vector<int>& ports) {
    for (size_t i = 0; i < ports.size(); ++i) {
        Server* server = new Server(ports[i]); //create q server on each port
        servers.push_back(server); //store the server in a vector

        // Add server socket to poll_fds
        struct pollfd pfd;
        pfd.fd = server->server_fd; // File descriptor of server socket
        pfd.events = POLLIN; // We want to know when it's ready for reading
        pfd.revents = 0;
        poll_fds.push_back(pfd); //adds the server socket’s file descriptor to the poll_fds vector, which will be used for polling with the poll() system call. We want to monitor these sockets for new incoming connections
    }
}

// Accept connections and monitor sockets
void acceptConnections() {
    while (true) {
        int poll_count = poll(&poll_fds[0], poll_fds.size(), -1); //The poll() function monitors a set of file descriptors and waits for one of them to become ready for I/O operations.
        if (poll_count == -1) {
            perror("Poll failed");
            break;
        }

        for (size_t i = 0; i < poll_fds.size(); ++i) {
            if (poll_fds[i].revents & POLLIN) {
                if (i < servers.size()) {
                    // Accept new connection
                    struct sockaddr_in client_address;
                    socklen_t client_len = sizeof(client_address);
                    int client_fd = accept(poll_fds[i].fd, (struct sockaddr*)&client_address, &client_len);

                    if (client_fd == -1) {
                        perror("Accept failed");
                        continue;
                    }

                    // Set client socket to non-blocking
                    fcntl(client_fd, F_SETFL, O_NONBLOCK);

                    // Add client to poll_fds
                    struct pollfd client_pfd;
                    client_pfd.fd = client_fd;
                    client_pfd.events = POLLIN | POLLOUT;
                    client_pfd.revents = 0;
                    poll_fds.push_back(client_pfd);

                    std::cout << "New connection accepted from "
                              << inet_ntoa(client_address.sin_addr) 
                              << ":" << ntohs(client_address.sin_port) << std::endl;
                }
            }
        }
    }
}

// Cleanup resources
void cleanup() {
    for (size_t i = 0; i < servers.size(); ++i) {
        delete servers[i];
    }
    std::cout << "Servers shut down." << std::endl;
}
