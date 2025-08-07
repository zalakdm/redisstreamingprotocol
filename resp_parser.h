#pragma once
#include <string>
#include <vector>
#include <cstdint>

enum class RESPType { SimpleString, Error, Integer, BulkString, Array, Null };

struct RESPValue {
    RESPType type;
    std::string str; // For SimpleString, Error, BulkString
    int64_t integer = 0; // For Integer
    std::vector<RESPValue> array; // For Array

    RESPValue() : type(RESPType::Null) {}
    RESPValue(RESPType t, const std::string& s) : type(t), str(s) {}
    RESPValue(int64_t i) : type(RESPType::Integer), integer(i) {}
    RESPValue(const std::vector<RESPValue>& arr) : type(RESPType::Array), array(arr) {}
};

std::string readLine(int sockfd);
RESPValue parseRESP(int sockfd);

// Serialize a RESPValue to a RESP-encoded string
std::string serializeRESP(const RESPValue& value); 