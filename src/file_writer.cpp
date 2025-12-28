#include "file_writer.hpp"
#include <stdexcept>

FileWriter::FileWriter(const std::string& path, size_t size) {
    file.open(path, std::ios::binary | std::ios::out | std::ios::in | std::ios::trunc);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open output file");
    }

    // Pre-allocate file size safely
    if (size > 0) {
        file.seekp(static_cast<std::streamoff>(size - 1));
        char zero = 0;
        file.write(&zero, 1);
        file.flush();
    }
}

void FileWriter::write(size_t offset, const char* data, size_t size) {
    if (!data || size == 0) return;

    std::lock_guard<std::mutex> lock(mtx);
    file.seekp(static_cast<std::streamoff>(offset));
    file.write(data, static_cast<std::streamsize>(size));

    if (!file) {
        throw std::runtime_error("File write failed");
    }
}
