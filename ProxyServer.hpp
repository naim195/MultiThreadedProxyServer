#ifndef PROXY_SERVER_HPP
#define PROXY_SERVER_HPP

#include "CacheManager.hpp"
#include <memory>

class ProxyServer {
public:
    ProxyServer(int port, size_t cache_max_size, size_t cache_max_element_size);
    ~ProxyServer();

    // Starts the server's main listening loop. This is a blocking function.
    void run();

private:
    // The function that each client thread will execute
    void handle_client(int client_socket);
    
    int port;
    int server_socket_fd;
    std::unique_ptr<CacheManager> cache; // Use a smart pointer to manage the cache's memory
};

#endif // PROXY_SERVER_HPP