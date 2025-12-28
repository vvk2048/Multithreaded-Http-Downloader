#pragma once
#include <fstream>
#include <mutex>
#include <string>

class FileWriter {
public:
    FileWriter(const std::string& path, size_t size);
    void write(size_t offset, const char* data, size_t size);

private:
    std::fstream file;
    std::mutex mtx;
};
