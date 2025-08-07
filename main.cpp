#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "resp_parser.h"
#include "commands.h"

constexpr int PORT = 6380;
constexpr int BACKLOG = 10;

void handle_client(int client_sock, sockaddr_in client_addr) {
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
    std::cout << "Client connected: " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;
    
    while (true) {
        try {
            std::cout << "DEBUG: Starting to parse command..." << std::endl;
            // Parse the incoming command
            RESPValue command = parseRESP(client_sock);
            std::cout << "DEBUG: Command parsed successfully" << std::endl;
            
            // Check for QUIT command
            if (command.type == RESPType::Array && !command.array.empty() && 
                command.array[0].str == "QUIT") {
                std::cout << "DEBUG: Client requested QUIT" << std::endl;
                break;
            }
            
            // Handle the command and get response
            RESPValue response = handleCommand(command);
            std::cout << "DEBUG: Command handled, sending response..." << std::endl;
            std::string resp_str = serializeRESP(response);
            write(client_sock, resp_str.c_str(), resp_str.size());
            std::cout << "DEBUG: Response sent" << std::endl;
            
        } catch (const std::exception& e) {
            std::cerr << "Error handling client: " << e.what() << std::endl;
            // Send error response
            RESPValue error(RESPType::Error, "Error: " + std::string(e.what()));
            std::string error_str = serializeRESP(error);
            write(client_sock, error_str.c_str(), error_str.size());
            
            // If it's a parsing error, break the loop
            if (std::string(e.what()).find("Failed to read") != std::string::npos) {
                break;
            }
        }
    }
    
    close(client_sock);
    std::cout << "DEBUG: Client connection closed" << std::endl;
}

int main() {
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        std::cerr << "Failed to create socket." << std::endl;
        return 1;
    }

    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt failed." << std::endl;
        close(server_sock);
        return 1;
    }

    sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed." << std::endl;
        close(server_sock);
        return 1;
    }

    if (listen(server_sock, BACKLOG) < 0) {
        std::cerr << "Listen failed." << std::endl;
        close(server_sock);
        return 1;
    }

    std::cout << "Server listening on port " << PORT << std::endl;
    std::vector<std::thread> threads;

    while (true) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_sock = accept(server_sock, (sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) {
            std::cerr << "Accept failed." << std::endl;
            continue;
        }
        threads.emplace_back(handle_client, client_sock, client_addr);
    }

    // Cleanup (unreachable in this infinite loop, but good practice)
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }
    close(server_sock);
    return 0;
} 