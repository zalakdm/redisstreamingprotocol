#include "commands.h"
#include <stdexcept>

// Global streams storage
std::map<std::string, std::shared_ptr<Stream>> streams;

RESPValue handleXADD(const std::vector<RESPValue>& args) {
    if (args.size() < 4) {
        return RESPValue(RESPType::Error, "ERR wrong number of arguments for 'xadd' command");
    }
    
    // XADD key ID field value [field value ...]
    std::string key = args[1].str;
    std::string id = args[2].str;
    
    // Parse field-value pairs
    std::map<std::string, std::string> fields;
    for (size_t i = 3; i < args.size(); i += 2) {
        if (i + 1 >= args.size()) {
            return RESPValue(RESPType::Error, "ERR wrong number of arguments for 'xadd' command");
        }
        fields[args[i].str] = args[i + 1].str;
    }
    
    // Get or create stream
    if (streams.find(key) == streams.end()) {
        streams[key] = std::make_shared<Stream>();
    }
    
    try {
        std::string entry_id = streams[key]->addEntry(fields, id);
        return RESPValue(RESPType::BulkString, entry_id);
    } catch (const std::exception& e) {
        return RESPValue(RESPType::Error, "ERR " + std::string(e.what()));
    }
}

RESPValue handlePING(const std::vector<RESPValue>& args) {
    if (args.size() == 1) {
        return RESPValue(RESPType::SimpleString, "PONG");
    } else if (args.size() == 2) {
        return RESPValue(RESPType::BulkString, args[1].str);
    } else {
        return RESPValue(RESPType::Error, "ERR wrong number of arguments for 'ping' command");
    }
}

RESPValue handleECHO(const std::vector<RESPValue>& args) {
    if (args.size() != 2) {
        return RESPValue(RESPType::Error, "ERR wrong number of arguments for 'echo' command");
    }
    return RESPValue(RESPType::BulkString, args[1].str);
}

RESPValue handleXLEN(const std::vector<RESPValue>& args) {
    if (args.size() != 2) {
        return RESPValue(RESPType::Error, "ERR wrong number of arguments for 'xlen' command");
    }
    
    std::string key = args[1].str;
    
    // Check if stream exists
    if (streams.find(key) == streams.end()) {
        return RESPValue(0); // Return 0 for non-existent streams
    }
    
    size_t length = streams[key]->length();
    return RESPValue(static_cast<int64_t>(length));
}

RESPValue handleQUIT(const std::vector<RESPValue>& args) {
    return RESPValue(RESPType::SimpleString, "OK");
}

RESPValue handleXREAD(const std::vector<RESPValue>& args) {
    if (args.size() < 4) {
        return RESPValue(RESPType::Error, "ERR wrong number of arguments for 'xread' command");
    }
    
    // Parse arguments: XREAD [COUNT count] [BLOCK milliseconds] STREAMS key [key ...] id [id ...]
    std::vector<std::string> keys;
    std::vector<std::string> ids;
    bool found_streams = false;
    
    for (size_t i = 1; i < args.size(); ++i) {
        std::string arg = args[i].str;
        std::transform(arg.begin(), arg.end(), arg.begin(), ::toupper);
        
        if (arg == "STREAMS") {
            found_streams = true;
            // After STREAMS, the remaining arguments are keys and ids
            size_t remaining = args.size() - i - 1;
            if (remaining % 2 != 0) {
                return RESPValue(RESPType::Error, "ERR Unbalanced XREAD list of streams");
            }
            
            size_t num_streams = remaining / 2;
            for (size_t j = 0; j < num_streams; ++j) {
                keys.push_back(args[i + 1 + j].str);
                ids.push_back(args[i + 1 + num_streams + j].str);
            }
            break;
        }
        // For now, skip COUNT and BLOCK options
    }
    
    if (!found_streams || keys.empty()) {
        return RESPValue(RESPType::Error, "ERR wrong number of arguments for 'xread' command");
    }
    
    // Build response array
    std::vector<RESPValue> response_array;
    
    for (size_t i = 0; i < keys.size(); ++i) {
        std::string key = keys[i];
        std::string id = ids[i];
        
        if (streams.find(key) == streams.end()) {
            // Stream doesn't exist, skip it
            continue;
        }
        
        // Get entries newer than the specified ID
        std::vector<StreamEntry> new_entries;
        const auto& stream = streams[key];
        
        // Directly iterate through entries to find newer ones
        for (const auto& entry : stream->getEntries()) {
            if (id == "0" || entry.id > id) {
                new_entries.push_back(entry);
            }
        }
        
        if (!new_entries.empty()) {
            // Create stream entry array: [key, [[id, [field, value, ...]], ...]]
            std::vector<RESPValue> stream_data;
            
            for (const auto& entry : new_entries) {
                std::vector<RESPValue> entry_data;
                entry_data.push_back(RESPValue(RESPType::BulkString, entry.id));
                
                // Add field-value pairs
                std::vector<RESPValue> fields;
                for (const auto& field : entry.fields) {
                    fields.push_back(RESPValue(RESPType::BulkString, field.first));
                    fields.push_back(RESPValue(RESPType::BulkString, field.second));
                }
                entry_data.push_back(RESPValue(fields));
                
                stream_data.push_back(RESPValue(entry_data));
            }
            
            std::vector<RESPValue> stream_entry;
            stream_entry.push_back(RESPValue(RESPType::BulkString, key));
            stream_entry.push_back(RESPValue(stream_data));
            
            response_array.push_back(RESPValue(stream_entry));
        }
    }
    
    if (response_array.empty()) {
        // Return null array if no new entries
        return RESPValue(RESPType::Null, "");
    }
    
    return RESPValue(response_array);
}

RESPValue handleCommand(const RESPValue& command) {
    if (command.type != RESPType::Array || command.array.empty()) {
        return RESPValue(RESPType::Error, "ERR invalid command");
    }
    
    std::string cmd = command.array[0].str;
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
    
    if (cmd == "XADD") {
        return handleXADD(command.array);
    } else if (cmd == "XLEN") {
        return handleXLEN(command.array);
    } else if (cmd == "XREAD") {
        return handleXREAD(command.array);
    } else if (cmd == "PING") {
        return handlePING(command.array);
    } else if (cmd == "ECHO") {
        return handleECHO(command.array);
    } else if (cmd == "QUIT") {
        return handleQUIT(command.array);
    } else {
        return RESPValue(RESPType::Error, "ERR unknown command '" + command.array[0].str + "'");
    }
} 