#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <chrono>

class RedisStreamsTestbench {
private:
    int sockfd;
    const int PORT = 6380;
    
    bool connect() {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }
        
        sockaddr_in server_addr;
        std::memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        
        if (::connect(sockfd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Failed to connect to server" << std::endl;
            return false;
        }
        
        return true;
    }
    
    std::string sendCommand(const std::string& command) {
        // Send command
        write(sockfd, command.c_str(), command.length());
        write(sockfd, "\n", 1);
        
        // Read response
        std::string response;
        char buffer[1024];
        int n = read(sockfd, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';
            response = buffer;
        }
        
        return response;
    }
    
    void disconnect() {
        if (sockfd >= 0) {
            close(sockfd);
        }
    }

public:
    RedisStreamsTestbench() : sockfd(-1) {}
    
    ~RedisStreamsTestbench() {
        disconnect();
    }
    
    void runAllTests() {
        std::cout << "=== Redis Streams Implementation Testbench ===" << std::endl;
        
        if (!connect()) {
            std::cerr << "Failed to connect to server. Make sure server is running on port 6380." << std::endl;
            return;
        }
        
        std::cout << "Connected to server successfully!" << std::endl;
        
        testBasicCommands();
        testXADD();
        testXLEN();
        testXRANGE();
        testXREAD();
        testXDEL();
        testXTRIM();
        testEdgeCases();
        
        std::cout << "\n=== All tests completed ===" << std::endl;
    }
    
    void testBasicCommands() {
        std::cout << "\n--- Testing Basic Commands ---" << std::endl;
        
        // Test PING
        std::cout << "Testing PING..." << std::endl;
        std::string ping_response = sendCommand("PING");
        std::cout << "PING response: " << ping_response << std::endl;
        
        // Test ECHO
        std::cout << "Testing ECHO..." << std::endl;
        std::string echo_response = sendCommand("ECHO hello world");
        std::cout << "ECHO response: " << echo_response << std::endl;
    }
    
    void testXADD() {
        std::cout << "\n--- Testing XADD ---" << std::endl;
        
        // Test basic XADD
        std::cout << "Testing XADD with auto-generated ID..." << std::endl;
        std::string xadd_response = sendCommand("XADD mystream * field1 value1 field2 value2");
        std::cout << "XADD response: " << xadd_response << std::endl;
        
        // Test XADD with manual ID
        std::cout << "Testing XADD with manual ID..." << std::endl;
        std::string xadd_manual_response = sendCommand("XADD mystream 1234567890-0 name alex age 40");
        std::cout << "XADD manual ID response: " << xadd_manual_response << std::endl;
        
        // Test XADD with multiple fields
        std::cout << "Testing XADD with multiple fields..." << std::endl;
        std::string xadd_multi_response = sendCommand("XADD mystream * name alice age 25 city sfo country usa");
        std::cout << "XADD multiple fields response: " << xadd_multi_response << std::endl;
    }
    
    void testXLEN() {
        std::cout << "\n--- Testing XLEN ---" << std::endl;
        
        // Test XLEN on existing stream
        std::cout << "Testing XLEN on mystream..." << std::endl;
        std::string xlen_response = sendCommand("XLEN mystream");
        std::cout << "XLEN response: " << xlen_response << std::endl;
        
        // Test XLEN on non-existent stream
        std::cout << "Testing XLEN on non-existent stream..." << std::endl;
        std::string xlen_nonexistent_response = sendCommand("XLEN nonexistentstream");
        std::cout << "XLEN non-existent response: " << xlen_nonexistent_response << std::endl;
    }
    
    void testXRANGE() {
        std::cout << "\n--- Testing XRANGE ---" << std::endl;
        
        // Test XRANGE with all entries
        std::cout << "Testing XRANGE - + (all entries)..." << std::endl;
        std::string xrange_all_response = sendCommand("XRANGE mystream - +");
        std::cout << "XRANGE all response: " << xrange_all_response << std::endl;
        
        // Test XRANGE with COUNT
        std::cout << "Testing XRANGE with COUNT..." << std::endl;
        std::string xrange_count_response = sendCommand("XRANGE mystream - + COUNT 2");
        std::cout << "XRANGE with COUNT response: " << xrange_count_response << std::endl;
        
        // Test XRANGE on non-existent stream
        std::cout << "Testing XRANGE on non-existent stream..." << std::endl;
        std::string xrange_nonexistent_response = sendCommand("XRANGE nonexistentstream - +");
        std::cout << "XRANGE non-existent response: " << xrange_nonexistent_response << std::endl;
    }
    
    void testXREAD() {
        std::cout << "\n--- Testing XREAD ---" << std::endl;
        
        // Test XREAD from beginning
        std::cout << "Testing XREAD from beginning..." << std::endl;
        std::string xread_beginning_response = sendCommand("XREAD STREAMS mystream 0");
        std::cout << "XREAD from beginning response: " << xread_beginning_response << std::endl;
        
        // Add a new entry and test XREAD for new entries
        std::cout << "Adding new entry and testing XREAD for new entries..." << std::endl;
        std::string new_entry_response = sendCommand("XADD mystream * newfield newvalue");
        std::cout << "New entry response: " << new_entry_response << std::endl;
        
        // Extract the ID from the response for testing
        std::string last_id = "1234567890-0"; // Use a known ID for testing
        std::string xread_new_response = sendCommand("XREAD STREAMS mystream " + last_id);
        std::cout << "XREAD for new entries response: " << xread_new_response << std::endl;
    }
    
    void testXDEL() {
        std::cout << "\n--- Testing XDEL ---" << std::endl;
        
        // Add some entries for deletion testing
        std::cout << "Adding entries for deletion testing..." << std::endl;
        std::string entry1_response = sendCommand("XADD deletestream * field1 value1");
        std::string entry2_response = sendCommand("XADD deletestream * field2 value2");
        std::string entry3_response = sendCommand("XADD deletestream * field3 value3");
        
        std::cout << "Entry 1: " << entry1_response << std::endl;
        std::cout << "Entry 2: " << entry2_response << std::endl;
        std::cout << "Entry 3: " << entry3_response << std::endl;
        
        // Test XDEL single entry
        std::cout << "Testing XDEL single entry..." << std::endl;
        std::string xdel_single_response = sendCommand("XDEL deletestream 1234567890-0");
        std::cout << "XDEL single response: " << xdel_single_response << std::endl;
        
        // Test XDEL multiple entries
        std::cout << "Testing XDEL multiple entries..." << std::endl;
        std::string xdel_multiple_response = sendCommand("XDEL deletestream 1234567890-0 1234567890-1");
        std::cout << "XDEL multiple response: " << xdel_multiple_response << std::endl;
        
        // Test XDEL on non-existent stream
        std::cout << "Testing XDEL on non-existent stream..." << std::endl;
        std::string xdel_nonexistent_response = sendCommand("XDEL nonexistentstream 1234567890-0");
        std::cout << "XDEL non-existent response: " << xdel_nonexistent_response << std::endl;
    }
    
    void testXTRIM() {
        std::cout << "\n--- Testing XTRIM ---" << std::endl;
        
        // Create a stream with many entries
        std::cout << "Creating stream with many entries..." << std::endl;
        for (int i = 1; i <= 10; ++i) {
            std::string command = "XADD trimstream * field" + std::to_string(i) + " value" + std::to_string(i);
            sendCommand(command);
        }
        
        // Test XLEN before trim
        std::cout << "Testing XLEN before trim..." << std::endl;
        std::string xlen_before_response = sendCommand("XLEN trimstream");
        std::cout << "XLEN before trim: " << xlen_before_response << std::endl;
        
        // Test XTRIM
        std::cout << "Testing XTRIM MAXLEN 3..." << std::endl;
        std::string xtrim_response = sendCommand("XTRIM trimstream MAXLEN 3");
        std::cout << "XTRIM response: " << xtrim_response << std::endl;
        
        // Test XLEN after trim
        std::cout << "Testing XLEN after trim..." << std::endl;
        std::string xlen_after_response = sendCommand("XLEN trimstream");
        std::cout << "XLEN after trim: " << xlen_after_response << std::endl;
        
        // Test XRANGE to see remaining entries
        std::cout << "Testing XRANGE to see remaining entries..." << std::endl;
        std::string xrange_after_trim_response = sendCommand("XRANGE trimstream - +");
        std::cout << "XRANGE after trim: " << xrange_after_trim_response << std::endl;
    }
    
    void testEdgeCases() {
        std::cout << "\n--- Testing Edge Cases ---" << std::endl;
        
        // Test invalid commands
        std::cout << "Testing invalid XADD..." << std::endl;
        std::string invalid_xadd_response = sendCommand("XADD mystream");
        std::cout << "Invalid XADD response: " << invalid_xadd_response << std::endl;
        
        // Test invalid XLEN
        std::cout << "Testing invalid XLEN..." << std::endl;
        std::string invalid_xlen_response = sendCommand("XLEN");
        std::cout << "Invalid XLEN response: " << invalid_xlen_response << std::endl;
        
        // Test invalid XRANGE
        std::cout << "Testing invalid XRANGE..." << std::endl;
        std::string invalid_xrange_response = sendCommand("XRANGE mystream");
        std::cout << "Invalid XRANGE response: " << invalid_xrange_response << std::endl;
        
        // Test unknown command
        std::cout << "Testing unknown command..." << std::endl;
        std::string unknown_command_response = sendCommand("UNKNOWNCOMMAND");
        std::cout << "Unknown command response: " << unknown_command_response << std::endl;
    }
};

int main() {
    RedisStreamsTestbench testbench;
    testbench.runAllTests();
    return 0;
} 