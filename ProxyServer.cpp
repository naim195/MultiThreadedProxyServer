#include "ProxyServer.hpp"
#include "HTTPRequest.hpp"
#include <iostream>
#include <vector>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <cstring>

ProxyServer::ProxyServer(int port, size_t cache_max_size, size_t cache_max_element_size)
    : port(port), server_socket_fd(-1) {
    // Use a smart pointer for automatic memory management of the cache
    cache = std::make_unique<CacheManager>(cache_max_size, cache_max_element_size);
}

ProxyServer::~ProxyServer() {
    if (server_socket_fd != -1) {
        close(server_socket_fd);
    }
}

void ProxyServer::run() {
    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd < 0) {
        perror("Failed to create socket");
        return;
    }

    int reuse = 1;
    setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    sockaddr_in server_addr{}; //holds all pieces of IPv4 address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return;
    }

    if (listen(server_socket_fd, SOMAXCONN) < 0) {
        perror("Listen failed");
        return;
    }

    std::cout << "Proxy server listening on port " << port << "..." << std::endl;

    while (true) {
        int client_socket = accept(server_socket_fd, nullptr, nullptr);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        // Create a new thread for each client. The thread will clean itself up when done.
        std::thread client_thread(&ProxyServer::handle_client, this, client_socket);
        client_thread.detach();
    }
}



void ProxyServer::handle_client(int client_socket) {
    std::vector<char> buffer(8192); // Use a larger buffer
    std::string raw_request;

    int bytes_received = recv(client_socket, buffer.data(), buffer.size(), 0);
    if (bytes_received <= 0) {
        close(client_socket);
        return;
    }
    raw_request.assign(buffer.data(), bytes_received);

    // Use the URL as the cache key, which is more reliable
    HTTPRequest request_for_key;
    if (!request_for_key.parse(raw_request)) {
        close(client_socket);
        return;
    }
    std::string cache_key = request_for_key.getHost() + request_for_key.getPath();
    
    std::string cached_response = cache->find(cache_key);
    if (!cached_response.empty()) {
        size_t total_size = cached_response.length();
        size_t bytes_sent = 0;
        const char* data_ptr = cached_response.data();
        const size_t CHUNK_SIZE = 4096;

        while (bytes_sent < total_size) {
            // Calculate remaining bytes and determine how much to send in this chunk.
            size_t remaining = total_size - bytes_sent;
            size_t size_to_send = std::min(CHUNK_SIZE, remaining);

            int sent_this_call = send(client_socket, data_ptr + bytes_sent, size_to_send, 0);

            if (sent_this_call < 0) {
                perror("send failed during cache hit");
                break; 
            }
            bytes_sent += sent_this_call;
        }

    } 
    else {
        // Cache miss
        HTTPRequest request;
        if (request.parse(raw_request)) {
            hostent* host_entry = gethostbyname(request.getHost().c_str());
            if (host_entry) {
                int remote_socket = socket(AF_INET, SOCK_STREAM, 0);
                if (remote_socket >= 0) {
                    sockaddr_in remote_addr{};
                    remote_addr.sin_family = AF_INET;
                    remote_addr.sin_port = htons(std::stoi(request.getPort()));
                    memcpy(&remote_addr.sin_addr.s_addr, host_entry->h_addr, host_entry->h_length);

                    if (connect(remote_socket, (struct sockaddr*)&remote_addr, sizeof(remote_addr)) >= 0) {
                        std::string req_to_send = request.reconstruct();
                        send(remote_socket, req_to_send.c_str(), req_to_send.length(), 0);
                        
                        std::string full_response;
                        while ((bytes_received = recv(remote_socket, buffer.data(), buffer.size(), 0)) > 0) {
                            send(client_socket, buffer.data(), bytes_received, 0);
                            full_response.append(buffer.data(), bytes_received);
                        }
                        
                        cache->add(cache_key, full_response);
                    }
                    close(remote_socket);
                }
            }
        }
    }

    close(client_socket);
}