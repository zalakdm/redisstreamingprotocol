#include "resp_parser.h"
#include <unistd.h>
#include <stdexcept>
#include <sstream>
#include <iostream>

std::string readLine(int sockfd) {
    std::string line;
    char c;
    while (read(sockfd, &c, 1) == 1) {
        if (c == '\r') {
            char next;
            if (read(sockfd, &next, 1) == 1 && next == '\n') break;
            // If we get \r but not \n, treat \r as part of the line
            line += c;
        } else if (c == '\n') {
            // Handle \n without \r (common with nc)
            break;
        } else {
            line += c;
        }
    }
    return line;
}

RESPValue parseRESP(int sockfd) {
    char type;
    ssize_t n = read(sockfd, &type, 1);
    if (n != 1) throw std::runtime_error("Failed to read RESP type");
    
    std::cout << "DEBUG: Received RESP type: '" << type << "' (ASCII: " << (int)type << ")" << std::endl;
    
    // Handle case where we receive a letter instead of RESP type
    if (type >= 'A' && type <= 'Z') {
        // This is likely a simple string command, read the rest
        std::string command = std::string(1, type) + readLine(sockfd);
        std::cout << "DEBUG: Received simple string command: " << command << std::endl;
        
        // Parse the command into an array format
        std::vector<RESPValue> args;
        std::istringstream iss(command);
        std::string token;
        while (iss >> token) {
            args.push_back(RESPValue(RESPType::BulkString, token));
        }
        return RESPValue(args);
    }
    
    switch (type) {
        case '+': // Simple String
            return RESPValue(RESPType::SimpleString, readLine(sockfd));
        case '-': // Error
            return RESPValue(RESPType::Error, readLine(sockfd));
        case ':': { // Integer
            std::string num = readLine(sockfd);
            return RESPValue(std::stoll(num));
        }
        case '$': { // Bulk String
            std::string lenstr = readLine(sockfd);
            int len = std::stoi(lenstr);
            if (len == -1) return RESPValue(RESPType::Null, "");
            std::string str(len, '\0');
            ssize_t total = 0;
            while (total < len) {
                ssize_t r = read(sockfd, &str[total], len - total);
                if (r <= 0) throw std::runtime_error("Failed to read bulk string");
                total += r;
            }
            // Read trailing \r\n
            char crlf[2];
            if (read(sockfd, crlf, 2) != 2 || crlf[0] != '\r' || crlf[1] != '\n')
                throw std::runtime_error("Malformed bulk string");
            return RESPValue(RESPType::BulkString, str);
        }
        case '*': { // Array
            std::string countstr = readLine(sockfd);
            int count = std::stoi(countstr);
            if (count == -1) return RESPValue(RESPType::Null, "");
            std::vector<RESPValue> arr;
            for (int i = 0; i < count; ++i) {
                arr.push_back(parseRESP(sockfd));
            }
            return RESPValue(arr);
        }
        default:
            std::cerr << "DEBUG: Unknown RESP type: '" << type << "' (ASCII: " << (int)type << ")" << std::endl;
            throw std::runtime_error("Unknown RESP type: " + std::string(1, type));
    }
}

std::string serializeRESP(const RESPValue& value) {
    std::ostringstream oss;
    switch (value.type) {
        case RESPType::SimpleString:
            oss << "+" << value.str << "\r\n";
            break;
        case RESPType::Error:
            oss << "-" << value.str << "\r\n";
            break;
        case RESPType::Integer:
            oss << ":" << value.integer << "\r\n";
            break;
        case RESPType::BulkString:
            if (value.str.empty() && value.type == RESPType::Null) {
                oss << "$-1\r\n";
            } else {
                oss << "$" << value.str.size() << "\r\n" << value.str << "\r\n";
            }
            break;
        case RESPType::Array:
            if (value.array.empty() && value.type == RESPType::Null) {
                oss << "*-1\r\n";
            } else {
                oss << "*" << value.array.size() << "\r\n";
                for (const auto& elem : value.array) {
                    oss << serializeRESP(elem);
                }
            }
            break;
        case RESPType::Null:
            oss << "$-1\r\n";
            break;
    }
    return oss.str();
} 