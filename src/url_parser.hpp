#pragma once
#include <string>

struct ParsedUrl {
    std::string host;
    std::string path;
};

inline ParsedUrl parseUrl(const std::string& url) {
    const std::string http = "http://";
    size_t pos = url.find(http);
    size_t host_start = pos == std::string::npos ? 0 : http.size();

    size_t path_start = url.find('/', host_start);
    ParsedUrl p;
    p.host = url.substr(host_start, path_start - host_start);
    p.path = path_start == std::string::npos ? "/" : url.substr(path_start);
    return p;
}
