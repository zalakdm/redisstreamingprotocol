#include "stream.h"
#include <chrono>
#include <sstream>
#include <algorithm>
#include <stdexcept>

std::string Stream::generateId() {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    return std::to_string(timestamp) + "-0";
}

std::string Stream::parseAndIncrementId(const std::string& id) {
    if (id == "*") {
        return generateId();
    }
    
    // Parse existing ID format: "timestamp-sequence"
    size_t dash_pos = id.find('-');
    if (dash_pos == std::string::npos) {
        throw std::runtime_error("Invalid ID format");
    }
    
    std::string timestamp_str = id.substr(0, dash_pos);
    std::string sequence_str = id.substr(dash_pos + 1);
    
    try {
        int64_t timestamp = std::stoll(timestamp_str);
        int64_t sequence = std::stoll(sequence_str);
        
        // If this ID is less than or equal to last_id, increment sequence
        if (id <= last_id) {
            // Parse last_id to get its sequence
            size_t last_dash = last_id.find('-');
            int64_t last_sequence = std::stoll(last_id.substr(last_dash + 1));
            sequence = last_sequence + 1;
        }
        
        std::string new_id = std::to_string(timestamp) + "-" + std::to_string(sequence);
        last_id = new_id;
        return new_id;
    } catch (const std::exception& e) {
        throw std::runtime_error("Invalid ID format");
    }
}

std::string Stream::addEntry(const std::map<std::string, std::string>& fields, const std::string& id) {
    if (fields.empty()) {
        throw std::runtime_error("ERR wrong number of arguments for 'xadd' command");
    }
    
    std::string entry_id = parseAndIncrementId(id);
    entries.emplace_back(entry_id, fields);
    
    return entry_id;
}

std::vector<StreamEntry> Stream::getRange(const std::string& start, const std::string& end, int count) {
    std::vector<StreamEntry> result;
    
    for (const auto& entry : entries) {
        if (entry.id >= start && entry.id <= end) {
            result.push_back(entry);
            if (count > 0 && result.size() >= static_cast<size_t>(count)) {
                break;
            }
        }
    }
    
    return result;
} 