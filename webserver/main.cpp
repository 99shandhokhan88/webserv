#include "Server.hpp"

int main() {
    std::vector<int> ports = {8080, 8081}; // Example ports
    setupServers(ports);
    acceptConnections();
    cleanup();
    return 0;
}
