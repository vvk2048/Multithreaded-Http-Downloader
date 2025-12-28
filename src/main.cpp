#include "http_client.hpp"
#include "thread_pool.hpp"
#include "file_writer.hpp"
#include "url_parser.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: ./downloader <url> <output>\n";
        return 1;
    }

    auto parsed = parseUrl(argv[1]);
    HttpClient client;

    size_t size = client.getContentLength(parsed.host, parsed.path);
    FileWriter writer(argv[2], size);

    const size_t CHUNK = 1 << 20; // 1MB
    ThreadPool pool(4);

    for (size_t i = 0; i < size; i += CHUNK) {
        size_t start = i;
        size_t end = std::min(i + CHUNK - 1, size - 1);

        pool.submit([&, start, end] {
            auto data = client.downloadRange(parsed.host, parsed.path, start, end);
            writer.write(start, data.data(), data.size());
        });
    }
}
