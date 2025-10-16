#include "ProxyServer.hpp"
#include <iostream>

const int PORT = 8080;
const size_t CACHE_MAX_SIZE_MB = 200;
const size_t CACHE_MAX_ELEMENT_SIZE_MB = 10;

int main(int argc, char* argv[]) {
    int port = PORT;
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch (const std::exception& e) {
            std::cerr << "Invalid port number. Using default " << PORT << std::endl;
        }
    }
    
    // Convert Megabytes to Bytes for the server configuration
    size_t cache_max_size_bytes = CACHE_MAX_SIZE_MB * 1024 * 1024;
    size_t cache_max_element_size_bytes = CACHE_MAX_ELEMENT_SIZE_MB * 1024 * 1024;

    try {
        ProxyServer server(port, cache_max_size_bytes, cache_max_element_size_bytes);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}