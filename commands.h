#pragma once
#include "resp_parser.h"
#include "stream.h"
#include <map>
#include <memory>

// Global streams storage
extern std::map<std::string, std::shared_ptr<Stream>> streams;

// Command handlers
RESPValue handleXADD(const std::vector<RESPValue>& args);
RESPValue handleXLEN(const std::vector<RESPValue>& args);
RESPValue handleXREAD(const std::vector<RESPValue>& args);
RESPValue handleXRANGE(const std::vector<RESPValue>& args);
RESPValue handleXDEL(const std::vector<RESPValue>& args);
RESPValue handleXTRIM(const std::vector<RESPValue>& args);
RESPValue handlePING(const std::vector<RESPValue>& args);
RESPValue handleECHO(const std::vector<RESPValue>& args);
RESPValue handleQUIT(const std::vector<RESPValue>& args);

// Main command dispatcher
RESPValue handleCommand(const RESPValue& command); 