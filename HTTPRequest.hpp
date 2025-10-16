#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <map>
#include <vector>

// A class to represent and parse an HTTP request
class HTTPRequest {
public:
    // Tries to parse a raw request buffer. Returns true on success.
    bool parse(const std::string& raw_request);

    // Getters for the parsed request components
    const std::string& getMethod() const { return method; }
    const std::string& getHost() const { return host; }
    const std::string& getPort() const { return port; }
    const std::string& getPath() const { return path; }
    const std::string& getVersion() const { return version; }

    // Reconstructs the request into a string format to be sent to a server
    std::string reconstruct() const;
    
    // Sets a header value (or adds it if it doesn't exist)
    void setHeader(const std::string& key, const std::string& value);

private:
    std::string method;
    std::string host;
    std::string port;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
};

#endif // HTTP_REQUEST_HPP