#include "server.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>

Server* server = nullptr;

void signalHandler(int signal) {
    if (server) {
        std::cout << "Shutting down server..." << std::endl;
        server->stopServer();
    }
    exit(signal);
}

int main() {
    // Set up signal handling for clean shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    std::cout << "Starting NextGig Backend Server" << std::endl;
    server = new Server();
    
    // Start the server on port 8080
    if (!server->startServer(8080)) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }
    
    // Keep the main thread alive
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}