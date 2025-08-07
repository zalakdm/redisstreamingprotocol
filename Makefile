CXX = g++
CXXFLAGS = -std=c++11 -pthread -Wall -Wextra
SERVER_SOURCES = main.cpp resp_parser.cpp stream.cpp commands.cpp
TESTBENCH_SOURCES = testbench.cpp
SERVER_TARGET = redis_server
TESTBENCH_TARGET = testbench

.PHONY: all clean server testbench test

all: server testbench

server: $(SERVER_TARGET)

$(SERVER_TARGET): $(SERVER_SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^

testbench: $(TESTBENCH_TARGET)

$(TESTBENCH_TARGET): $(TESTBENCH_SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^

test: server testbench
	@echo "Starting Redis Streams server..."
	@./$(SERVER_TARGET) &
	@sleep 2
	@echo "Running testbench..."
	@./$(TESTBENCH_TARGET)
	@echo "Stopping server..."
	@pkill -f $(SERVER_TARGET) || true

clean:
	rm -f $(SERVER_TARGET) $(TESTBENCH_TARGET)

help:
	@echo "Available targets:"
	@echo "  all       - Build both server and testbench"
	@echo "  server    - Build only the Redis server"
	@echo "  testbench - Build only the testbench"
	@echo "  test      - Build and run the testbench"
	@echo "  clean     - Remove built executables"
	@echo "  help      - Show this help message" 