# Multi-Threaded HTTP Downloader (C++)

A high-performance, multithreaded file downloader implemented in modern
C++ using\
**raw POSIX sockets**, **HTTP Range requests**, and a **thread pool**.

This project focuses on **networking, concurrency, file I/O correctness,
and real-world performance trade-offs**, similar to systems used in
large-scale data ingestion pipelines (e.g., Spark, ML dataset fetchers,
CI/CD artifact downloads).

------------------------------------------------------------------------

## Motivation

Large datasets such as:

-   Market data snapshots\
-   Machine learning datasets\
-   Web-scale crawls (e.g., Common Crawl)\
-   Build artifacts and binaries

are often **hundreds of MBs to multiple GBs** in size.

Single-threaded downloads underutilize available bandwidth and increase
end-to-end pipeline latency.\
This project demonstrates how **parallel range-based downloads** can
reduce wall-clock time while maintaining correctness.

------------------------------------------------------------------------

## Features

-   HTTP **Range-based parallel downloads**
-   Fixed-size **thread pool** for controlled concurrency
-   **Random-access file writes** using file offsets
-   Efficient handling of long-running downloads (1GB+)
-   Minimal dependencies (no `libcurl`)
-   Designed for clarity and correctness over shortcuts

------------------------------------------------------------------------

## Architecture Overview

    Main Thread
    ├─ Parse URL
    ├─ Fetch Content-Length
    ├─ Split file into byte ranges
    ├─ Submit tasks to ThreadPool
    └─ Wait for completion

    ThreadPool
    ├─ Worker 1 → Download range [0 – N]
    ├─ Worker 2 → Download range [N – 2N]
    ├─ Worker 3 → Download range [2N – 3N]
    └─ Worker 4 → Download range [3N – 4N]

    Workers
    ├─ HTTP GET with Range header
    ├─ Validate response
    └─ Write bytes directly to file offset

------------------------------------------------------------------------

## Build Instructions

### Requirements

-   macOS or Linux\
-   `g++` (C++17 or newer)\
-   POSIX sockets

### Build

``` bash
make
```

------------------------------------------------------------------------

## Usage

``` bash
./downloader <http_url> <output_file>
```

### Example

``` bash
./downloader http://ipv4.download.thinkbroadband.com/1GB.zip dataset_1gb.zip
```

------------------------------------------------------------------------

## Benchmark Results

### Test File

    http://ipv4.download.thinkbroadband.com/1GB.zip

### Baseline (curl)

``` bash
time curl -o baseline_1gb.zip http://ipv4.download.thinkbroadband.com/1GB.zip
```

Result:

    real    2m12.67s
    user    0.49s
    sys     5.86s
    CPU     ~4%

### Multithreaded Downloader

``` bash
time ./downloader http://ipv4.download.thinkbroadband.com/1GB.zip dataset_1gb.zip
```

Result:

    real    1m40.85s
    user    0.20s
    sys     0.98s
    CPU     ~1%

<img width="1600" height="202" alt="image" src="https://github.com/user-attachments/assets/969c894d-0f10-47c8-8116-5699edf31de8" />

------------------------------------------------------------------------

## Correctness Verification

``` bash
shasum -a 256 baseline_1gb.zip dataset_1gb.zip
```

------------------------------------------------------------------------

## Future Improvements

-   HTTPS support via OpenSSL\
-   Resume support\
-   Progress reporting\
-   Configurable chunk sizes

------------------------------------------------------------------------

## Author Notes

Built to explore systems-level networking, concurrency, and file I/O
correctness.
