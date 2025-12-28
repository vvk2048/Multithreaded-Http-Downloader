// Replace your existing http_client.cpp body with this improved version.
// NOTE: minimal changes to keep it synchronous and simple.

#include "http_client.hpp"
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <algorithm>

static int connectSocket(const std::string& host, const std::string& port = "80") {
    addrinfo hints{}, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host.c_str(), port.c_str(), &hints, &res) != 0)
        throw std::runtime_error("getaddrinfo failed");

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        freeaddrinfo(res);
        throw std::runtime_error("socket failed");
    }

    if (connect(sock, res->ai_addr, res->ai_addrlen) != 0) {
        close(sock);
        freeaddrinfo(res);
        throw std::runtime_error("connect failed");
    }
    freeaddrinfo(res);
    return sock;
}

static std::string recvAll(int sock) {
    char buffer[4096];
    std::string out;
    ssize_t n;
    while ((n = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        out.append(buffer, buffer + n);
    }
    return out;
}

static std::string getHeaderValue(const std::string& headers, const std::string& name) {
    auto pos = headers.find(name);
    if (pos == std::string::npos) return "";
    auto line_end = headers.find("\r\n", pos);
    if (line_end == std::string::npos) return "";
    auto val_start = pos + name.size();
    // trim
    std::string val = headers.substr(val_start, line_end - val_start);
    while (!val.empty() && (val.front() == ' ' || val.front() == ':')) val.erase(val.begin());
    return val;
}

// helper to parse status code
static int parseStatus(const std::string& header) {
    std::istringstream ss(header);
    std::string http, status;
    ss >> http >> status;
    return std::stoi(status);
}

size_t HttpClient::getContentLength(const std::string& host, const std::string& path) {
    // follow possible redirects (up to 5)
    std::string curHost = host;
    std::string curPath = path;
    for (int redirect = 0; redirect < 5; ++redirect) {
        int sock = connectSocket(curHost);
        std::ostringstream req;
        req << "HEAD " << curPath << " HTTP/1.1\r\n"
            << "Host: " << curHost << "\r\n"
            << "User-Agent: Mozilla/5.0 (X11; Linux x86_64)\r\n"
            << "Accept: */*\r\n"
            << "Connection: close\r\n\r\n";

        std::string request = req.str();
        send(sock, request.c_str(), request.size(), 0);

        std::string response = recvAll(sock);
        close(sock);

        // split headers/body
        auto pos = response.find("\r\n\r\n");
        std::string headers = pos == std::string::npos ? response : response.substr(0, pos);

        int status = parseStatus(headers);
        if (status >= 300 && status < 400) {
            // follow Location
            std::string loc = getHeaderValue(headers, "Location:");
            if (loc.empty()) throw std::runtime_error("redirect without Location");
            // parse new host/path (simple)
            if (loc.rfind("http://", 0) == 0) {
                size_t start = std::string("http://").size();
                size_t slash = loc.find('/', start);
                curHost = loc.substr(start, slash - start);
                curPath = slash == std::string::npos ? "/" : loc.substr(slash);
            } else {
                // relative redirect - use current host
                curPath = loc;
            }
            continue;
        }

        auto clen = getHeaderValue(headers, "Content-Length:");
        if (clen.empty()) throw std::runtime_error("Content-Length not found");
        return std::stoul(clen);
    }
    throw std::runtime_error("Too many redirects");
}

std::vector<char> HttpClient::downloadRange(
    const std::string& host,
    const std::string& path,
    size_t start,
    size_t end
) {
    // Basic redirect handling (up to 5)
    std::string curHost = host;
    std::string curPath = path;

    for (int redirect = 0; redirect < 5; ++redirect) {
        int sock = connectSocket(curHost);
        std::ostringstream req;
        req << "GET " << curPath << " HTTP/1.1\r\n"
            << "Host: " << curHost << "\r\n"
            << "User-Agent: Mozilla/5.0 (X11; Linux x86_64)\r\n"
            << "Accept: */*\r\n"
            << "Range: bytes=" << start << "-" << end << "\r\n"
            << "Connection: close\r\n"
            << "Referer: http://" << host << "\r\n\r\n";

        std::string request = req.str();
        send(sock, request.c_str(), request.size(), 0);

        // Read until close
        std::string response = recvAll(sock);
        close(sock);

        // Separate headers/body
        auto pos = response.find("\r\n\r\n");
        if (pos == std::string::npos) {
            // maybe an HTML error - treat as redirect/error
            int status = parseStatus(response);
            if (status >= 300 && status < 400) {
                std::string headers = response;
                std::string loc = getHeaderValue(headers, "Location:");
                if (loc.empty()) throw std::runtime_error("redirect without Location");
                if (loc.rfind("http://", 0) == 0) {
                    size_t startp = std::string("http://").size();
                    size_t slash = loc.find('/', startp);
                    curHost = loc.substr(startp, slash - startp);
                    curPath = slash == std::string::npos ? "/" : loc.substr(slash);
                } else {
                    curPath = loc;
                }
                continue;
            }
            throw std::runtime_error("Invalid HTTP response");
        }

        std::string header = response.substr(0, pos);
        int status = parseStatus(header);
        if (status >= 300 && status < 400) {
            std::string loc = getHeaderValue(header, "Location:");
            if (loc.empty()) throw std::runtime_error("redirect without Location");
            if (loc.rfind("http://", 0) == 0) {
                size_t startp = std::string("http://").size();
                size_t slash = loc.find('/', startp);
                curHost = loc.substr(startp, slash - startp);
                curPath = slash == std::string::npos ? "/" : loc.substr(slash);
            } else {
                curPath = loc;
            }
            continue;
        }

        // everything ok -> extract body
        std::vector<char> data;
        data.insert(data.end(), response.begin() + pos + 4, response.end());
        return data;
    }

    throw std::runtime_error("Too many redirects while downloading");
}
