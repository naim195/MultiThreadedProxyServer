#include "HTTPRequest.hpp"
#include <sstream>
#include <iostream>

/**
 * @brief Parses a raw HTTP request string into its constituent parts.
 *
 * This function is the core of the request handling logic. It takes the raw
 * text sent by the browser and deconstructs it into a structured format
 * that the rest of the proxy can easily use.
 *
 * @param raw_request A std::string containing the full, raw HTTP request.
 * @return true if parsing was successful, false for any malformed request.
 */
bool HTTPRequest::parse(const std::string& raw_request) {
    // A stringstream is a powerful tool that allows us to treat a string
    // as if it were a stream, like a file or the console. This lets us "read"
    // from the string line-by-line or word-by-word.
    std::stringstream full_request_stream(raw_request);
    std::string current_line;

    // Parse the Request-Line (the very first line of the request) 
    // Example: "GET http://www.example.com/index.html HTTP/1.1"

    // We use std::getline to read from our stream until we hit the first newline.
    // If we can't even read one line, the request is invalid.
    if (!std::getline(full_request_stream, current_line) || current_line.empty()) {
        return false;
    }

    // Now, we create a *second*, temporary stringstream just for this one line.
    // This allows us to easily extract the three space-separated parts.
    std::stringstream request_line_stream(current_line);
    std::string full_url;

    // The extraction operator (>>) reads from the stream word by word.
    // "GET" -> method
    // "http://www.example.com/index.html" -> full_url
    // "HTTP/1.1" -> version
    request_line_stream >> method >> full_url >> version;

    // For this project, we only handle GET requests. A future improvement
    // would be to handle the CONNECT method for HTTPS here.
    if (method != "GET") {
        std::cerr << "Unsupported method encountered: " << method << std::endl;
        return false;
    }

    // ... Parse the URL into its components (host, port, path) ...
    // Example: from "http://www.example.com:8080/path/to/file"

    size_t protocol_separator_pos = full_url.find("://");
    if (protocol_separator_pos == std::string::npos) {
        return false; // Malformed URL, missing the "://"
    }

    // We extract the part after "://", which is called the URI.
    // Example: "www.example.com:8080/path/to/file"
    std::string uri_part = full_url.substr(protocol_separator_pos + 3);

    size_t path_separator_pos = uri_part.find('/');
    
    // The "authority" is the host and optional port part of the URI.
    // Example: "www.example.com:8080"
    std::string host_and_port = (path_separator_pos == std::string::npos) ? uri_part : uri_part.substr(0, path_separator_pos);

    // The path is everything from the first '/' onwards.
    // Example: "/path/to/file"
    path = (path_separator_pos == std::string::npos) ? "/" : uri_part.substr(path_separator_pos);

    // Now, we check if a port was specified in the authority part.
    size_t port_separator_pos = host_and_port.find(':');
    if (port_separator_pos != std::string::npos) {
        // A port was provided, e.g., in "www.example.com:8080".
        host = host_and_port.substr(0, port_separator_pos);
        port = host_and_port.substr(port_separator_pos + 1);
    } else {
        // No port provided. We default to 80 for standard HTTP.
        host = host_and_port;
        port = "80";
    }

    // ... Parse the HTTP Headers 
    // Now we loop through the rest of the main request stream line by line.
    while (std::getline(full_request_stream, current_line) && !current_line.empty() && current_line != "\r") {
        // Example line: "Host: www.example.com\r"
        size_t colon_pos = current_line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = current_line.substr(0, colon_pos);
            // The value is everything after the colon and the space that follows it.
            std::string value = current_line.substr(colon_pos + 2);

            // HTTP lines end with "\r\n". std::getline handles the "\n", but the "\r" might remain.
            // We need to trim it for a clean value.
            if (!value.empty() && value.back() == '\r') {
                value.pop_back();
            }

            // Store the clean key-value pair in our map.
            headers[key] = value;
        }
    }

    return true; // Success!
}

/**
 * @brief Reconstructs the HTTP request into a valid, sendable string.
 * This is used to forward the client's request to the destination server.
 * @return A std::string containing the full HTTP request.
 */
std::string HTTPRequest::reconstruct() const {
    // We create a stringstream object to act as an in-memory text builder.
    std::stringstream reconstructed_request_ss;

    // For example, let's assume our parsed object has this data:
    // method = "GET"
    // path = "/index.html"
    // version = "HTTP/1.1"
    // headers = { {"Host", "www.example.com"}, {"Connection", "close"} }

    // First, we build the Request-Line using the << operator to stream text into the object.
    reconstructed_request_ss << method << " " << path << " " << version << "\r\n";
    // The stream's internal buffer now contains: "GET /index.html HTTP/1.1\r\n"

    // Next, we iterate through the headers map and stream each one into the buffer.
    for (const auto& header_pair : headers) {
        reconstructed_request_ss << header_pair.first << ": " << header_pair.second << "\r\n";
    }
    // After looping, the stream's buffer now contains:
    // "GET /index.html HTTP/1.1\r\nHost: www.example.com\r\nConnection: close\r\n"
    
    // Finally, we add the crucial empty line that signals the end of the headers.
    reconstructed_request_ss << "\r\n";
    // The final buffer is now:
    // "GET /index.html HTTP/1.1\r\nHost: www.example.com\r\nConnection: close\r\n\r\n"
    
    // The .str() method converts the stream's entire buffer into a single std::string for us to return.
    return reconstructed_request_ss.str();
}


/**
 * @brief Sets or updates a header value.
 * @param key The header key (e.g., "Host").
 * @param value The header value (e.g., "www.example.com").
 */
void HTTPRequest::setHeader(const std::string& key, const std::string& value) {
    headers[key] = value;
}