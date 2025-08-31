# QOTD Integration Testing

This directory contains scripts for integration testing the IoRxBuffer refactoring with a realistic TCP server environment.

## QOTD Server (`qotd_server.bash`)

RFC-865 compliant Quote of the Day server that fetches quotes from quotable.io and formats them according to the RFC standard.

### Features:
- **RFC-865 compliance**: ASCII-only output, 512-character limit, proper line handling
- **Fallback mechanism**: Uses hardcoded quote if API fails
- **Rate limiting aware**: quotable.io limits to 180 requests/minute per IP
- **Character sanitization**: Converts UTF-8 to ASCII, removes control characters
- **Robust error handling**: Graceful degradation when network fails

### Usage:

#### Start QOTD Server:
```bash
# Install ncat if not available (part of nmap package)
sudo apt-get install nmap

# Start QOTD server on port 17 (standard QOTD port)
ncat -l 17 --keep-open --send-only --exec "./scripts/qotd_server.bash"
```

#### Test Server Manually:
```bash
# Connect and receive a quote
nc localhost 17

# Or with telnet
telnet localhost 17
```

### Integration with IoRxBuffer

Your `QotdReceivedHandler` will:
1. Connect to `localhost:17` via `TcpClient`
2. Receive quote data via `IoRxBuffer` through the new async pattern
3. Process data using `peekBuffer()`/`peekConsume()` interface
4. Store quote in `QuoteBuffer` for display

The server perfectly tests your refactoring because:
- **Variable message lengths**: Tests pbuf chain handling
- **ASCII compliance**: Tests character processing
- **Single response pattern**: Tests connection close handling
- **Real network conditions**: Tests async context timing

### Client receive/FIN flow (QOTD)
- The server sends the entire quote, then immediately closes the connection (FIN).
- QotdReceivedHandler::onWork():
  - Resets the quote buffer to start a fresh record
  - Peeks and consumes up to the configured partial-consumption threshold
  - Sets the first chunk in QuoteBuffer
- QotdFinHandler::onWork():
  - Drains remaining bytes from IoRxBuffer in threshold-sized chunks
  - Appends to QuoteBuffer, marks the quote complete, resets the Rx buffer, and shuts down the connection
- The partial-consumption threshold is centralized as `QOTD_PARTIAL_CONSUMPTION_THRESHOLD` (defined in `src/main.cpp`, declared in `include/QotdConfig.hpp`).

### Script Dependencies:
- `curl` - for fetching quotes from API
- `jq` - for JSON parsing
- `iconv` - for UTF-8 to ASCII conversion
- `ncat` - for TCP server functionality

### RFC-865 Compliance Details:
- Maximum 512 characters per response
- ASCII-only character set
- Control character filtering (except CR/LF)
- Proper line ending handling
- Whitespace normalization
