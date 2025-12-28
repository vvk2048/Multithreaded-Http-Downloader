#pragma once
#include <string>
#include <vector>

class HttpClient {
public:
    std::vector<char> downloadRange(
        const std::string& host,
        const std::string& path,
        size_t start,
        size_t end
    );

    size_t getContentLength(
        const std::string& host,
        const std::string& path
    );
};
