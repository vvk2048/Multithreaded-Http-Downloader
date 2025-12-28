CXX=g++
CXXFLAGS=-std=c++17 -Wall -pthread

SRC=src/main.cpp src/http_client.cpp src/thread_pool.cpp src/file_writer.cpp
OUT=downloader

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)
