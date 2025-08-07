#pragma once
#include <string>
#include <map>
#include <vector>
#include <memory>

struct StreamEntry {
    std::string id;  // Format: "timestamp-sequence"
    std::map<std::string, std::string> fields;  // field-value pairs
    
    StreamEntry(const std::string& entry_id, const std::map<std::string, std::string>& f) 
        : id(entry_id), fields(f) {}
};

class Stream {
private:
    std::vector<StreamEntry> entries;
    std::string last_id;  // Last generated ID for auto-incrementing sequence

public:
    Stream() : last_id("0-0") {}
    
    // Add an entry to the stream
    std::string addEntry(const std::map<std::string, std::string>& fields, const std::string& id = "*");
    
    // Get entries in a range
    std::vector<StreamEntry> getRange(const std::string& start, const std::string& end, int count = -1);
    
    // Get stream length
    size_t length() const { return entries.size(); }
    
    // Get all entries (for XREAD)
    const std::vector<StreamEntry>& getEntries() const { return entries; }
    
    // Generate next ID based on current timestamp
    std::string generateId();
    
    // Parse ID and increment sequence if needed
    std::string parseAndIncrementId(const std::string& id);
}; 