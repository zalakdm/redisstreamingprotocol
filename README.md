# Redis Streams Implementation

A C++ implementation of Redis Streams protocol and API, supporting the core stream operations.

## Features

### Implemented Commands

- **XADD** - Add entries to streams with auto-generated or manual IDs
- **XLEN** - Get the number of entries in a stream
- **XREAD** - Read new entries from streams
- **XRANGE** - Read specific ranges of entries with COUNT support
- **XDEL** - Delete specific entries by ID
- **XTRIM** - Trim streams to a maximum length
- **PING** - Basic connectivity test
- **ECHO** - Echo back messages
- **QUIT** - Gracefully close connection

### Technical Features

- **Multi-threaded server** handling multiple clients
- **RESP protocol parser** for Redis Serialization Protocol
- **In-memory stream storage** with efficient data structures
- **Error handling** with proper RESP error responses
- **Comprehensive testbench** for validation

## Building

### Build Commands

```bash
# Build everything
make all

# Build only the server
make server

# Run testbench
make test

# Clean build artifacts
make clean
```

## Running

### Start the Server

```bash
./redis_server
```

The server will start listening on port 6380.

### Manual Testing

Connect using `nc`:

```bash

# Using netcat
nc localhost 6380
```

### Example Commands

```redis
# Add entries to a stream
XADD mystream * field1 value1 field2 value2
XADD mystream * name alex age 40 city sfo

# Get stream length
XLEN mystream

# Read all entries
XRANGE mystream - +

# Read new entries
XREAD STREAMS mystream 0

# Delete specific entries
XDEL mystream 1234567890-0

# Trim stream to 5 entries
XTRIM mystream MAXLEN 5

# Basic commands
PING
ECHO hello
QUIT
```

### Test Coverage

The testbench covers:

1. **Basic Commands**
   - PING/PONG
   - ECHO functionality

2. **XADD Operations**
   - Auto-generated IDs
   - Manual IDs
   - Multiple field-value pairs

3. **XLEN Operations**
   - Existing streams
   - Non-existent streams

4. **XRANGE Operations**
   - Full range queries
   - COUNT limits
   - Non-existent streams

5. **XREAD Operations**
   - Reading from beginning
   - Reading new entries

6. **XDEL Operations**
   - Single entry deletion
   - Multiple entry deletion
   - Non-existent streams

7. **XTRIM Operations**
   - Trimming to specific length
   - Verification of remaining entries

8. **Edge Cases**
   - Invalid commands
   - Missing arguments
   - Unknown commands

## Architecture

### Core Components

- **main.cpp** - Server entry point and networking
- **resp_parser.h/cpp** - RESP protocol parsing and serialization
- **stream.h/cpp** - Stream data structure and operations
- **commands.h/cpp** - Command handlers and dispatcher
- **testbench.cpp** - Comprehensive tests

### Data Structures

- **StreamEntry** - Individual stream entry with ID and field-value pairs
- **Stream** - Collection of entries with operations
- **RESPValue** - RESP protocol value representation

## Protocol Support

### RESP Types

- Simple Strings (`+`)
- Errors (`-`)
- Integers (`:`)
- Bulk Strings (`$`)
- Arrays (`*`)
- Null values

## Future Enhancements

- **Persistence** - Save streams to disk