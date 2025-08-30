# The Application Workflow

## Overview

This document describes the workflow and concurrency model of the stress-test application for LwIP integration on the Raspberry Pi Pico platform. The application demonstrates asynchronous TCP client usage, thread-safe resource sharing, and event-driven design across dual cores.

The test application’s **core idea** is to validate the async-tcp library’s LwIP integration with async_context, and to showcase how to customise responses to various LwIP events. It is intentionally structured to reflect lwIP’s preferred single-threaded execution model while exercising cross-core event handling and thread-safe communication.

Design Rationale: lwIP and async_context

lwIP likes single-threaded environments: Its TCP/IP core is designed to run in one main context—whether that’s a polling loop, event loop, or dedicated task. async_context provides this clean, serialized execution model without OS-level thread synchronisation.

Why it works well: Using async_context ensures all lwIP callbacks (including TCP RX/TX events) run in the same logical thread. A separate component or protocol handler can run in a different async_context process on another core, avoiding reentrancy issues.

Core principle: The lwIP raw TCP API is not thread-safe and has no built-in cross-thread marshalling. In a multi-core system, async_context is used to make lwIP thread-safe by ensuring that all API calls happen from the context that owns lwIP. The async-tcp library builds on this by associating PerpetualBridge-based handlers with various lwIP callbacks and by providing a thread-safe mechanism for initiating writes. The integration does not expose the raw TCP API to the consuming application; all TCP operations are marshalled onto the lwIP async_context.

Bonus: This approach naturally supports a “reactor” style architecture:

Core 0: async_context_lwip → TCP/IP and lwIP timers.

Core 1: async_context_app → application logic, business rules, etc.

Lightweight message passing between cores.

## Initial Assessment

### Architecture
- Global context managers are created for each core: `ctx0` (TCP client operations) and `ctx1` (serial printing and quote buffer).
- Thread-safe buffer (`e5::QuoteBuffer qotd_buffer(ctx1)`) is used for storing the quote of the day, utilizing the SyncBridge pattern for safe cross-core access.
- TCP clients are instantiated for both the QOTD and Echo servers, with their associated IP/port configuration.
- Utility functions handle board temperature reading, heap/stack statistics, and WiFi/server connections.

### Flow and Concurrency Patterns

#### Sequence of Operations
1. WiFi and server setup: The application connects to WiFi and resolves server addresses.
2. QOTD retrieval: The function `get_quote_of_the_day()` initiates a connection to the QOTD server if not already in progress.
3. Buffer usage: When a quote is received, it is stored in `qotd_buffer` using its thread-safe `set()` method.
4. Echo operation: The function `get_echo()` reads the current quote from `qotd_buffer` (using `get()`) and, if not empty, sends it to the echo server.
5. Statistics and monitoring: Functions print heap, stack, and temperature stats using the serial printer, which also operates on `ctx1`.

#### Expected Concurrency Patterns
- Reads vs Writes: The buffer is written to only when a new quote is received (infrequent), but read every time an echo operation is attempted (frequent).
- Cross-core access: Core 0 (TCP client) may write to the buffer, while Core 1 (serial printer, echo logic) may read from it, potentially at the same time.
- Synchronization: All access to `qotd_buffer` is funneled through SyncBridge, ensuring thread safety but serializing all operations (no concurrent reads).

## QOTD Protocol and Application Beat

The application leverages the QOTD (Quote of the Day) protocol, where the server sends a quote and closes the connection immediately upon client connection. This server-driven behavior defines the application's operational cycle ("beat"):

- Connection Initiation: The application connects to the QOTD server.
- Quote Reception: Upon connection, the server sends a quote and closes the connection (TCP FIN).
- Buffer Update: The received quote is written to the thread-safe buffer (`qotd_buffer`).
- Echo Trigger: The application then reads the buffer and sends the quote to the echo server.
- Cycle Repeat: The process repeats, with each QOTD server response driving the next cycle.

This protocol-driven flow ensures that each round of buffer update and echo operation is synchronized with the QOTD server's response, providing a natural rhythm for stress-testing and concurrency analysis.

## Critical Integration Note: Cross-Core Buffer Update and Real-Time Constraints

The application updates the quote buffer on each QOTD receive event using:

```cpp
m_quote_buffer.set(data);
```

This is a **blocking, cross-core call** from an interrupt context (network event on core 0) to core 1, where the buffer resides. This design is intentional and reflects real-world requirements:

- **Core 0**: Handles network operations and receives data from the QOTD server.
- **Core 1**: Runs the main application logic and owns the quote buffer.
- The buffer update is performed synchronously and blocks until the operation completes on core 1, ensuring data consistency.

Partial consumption is implemented: the consumer on core 1 processes only a portion of the available data and returns the number of bytes consumed. The RX handler then advances the IoRxBuffer by exactly that amount, leaving any remainder for subsequent processing. Practically, `QotdReceivedHandler` peeks from `IoRxBuffer`, appends processed bytes to `QuoteBuffer`, and calls `consume(n)` to drop just the processed segment. This enables backpressure-friendly flow control and avoids over-consuming unprocessed data.

This approach deliberately stress-tests:
- Cross-core synchronization and blocking calls from interrupt context
- The SyncBridge pattern���s ability to marshal data and execution between cores
- Real-time responsiveness and integration boundaries

**Future Direction:**
- Tune partial-consumption thresholds and delimiters per protocol
- Add more protocol examples demonstrating delimiter-based and length-prefixed parsing
- Further validate and optimize cross-core, interrupt-safe integration

This section is critical for anyone reviewing or extending the code, as it highlights both the rationale for the current design and the roadmap for future improvements.

## SerialPrinter: Thread-Safe Output and Non-Reentrant Library Integration

The SerialPrinter component is a key part of the test setup, demonstrating how to safely service third-party libraries (such as Serial) that are not re-entrant, using the pico_async_context pattern.

- **Thread-Safe Output:** All calls to Serial.print() are funneled through SerialPrinter, which schedules print jobs to execute on core 1. This ensures that output from any core or interrupt context is printed sequentially and without overlap.
- **Non-Blocking Cross-Core Calls:** For example, in `EchoReceivedHandler::onWork()`, the call `m_serial_printer.print(std::move(quote));` is a non-blocking, cross-core operation. The print job is queued and executed on core 1, maintaining log integrity and avoiding concurrency issues.
- **Log Consistency:** This approach guarantees that all log messages, status updates, and received data appear in the correct order, with no overlaps or garbage, even under heavy cross-core activity.
- **Showcasing pico_async_context:** SerialPrinter exemplifies how to use async_context to service non-reentrant libraries, providing a practical pattern for integrating similar third-party components in multi-core or interrupt-driven environments.

This design is essential for robust, thread-safe output and serves as a reference for handling non-reentrant resources in concurrent applications.

## Event Handlers: Customizing Application Response to LwIP Events

The application demonstrates how to use the async-tcp library's event-driven architecture to customize responses to various LwIP events through dedicated handler classes, all executed under async-context guarantees via PerpetualBridge:

- **QotdConnectedHandler**: Responds to successful QOTD server connections, logs connection details, and prepares the application for the next receive event.
- **QotdReceivedHandler**: Handles incoming quote data, performing a blocking, cross-core update to the quote buffer to ensure data consistency and test integration boundaries.
- **QotdFinHandler**: Handles TCP FIN event, drains remaining RX data, marks quote completion, and triggers the next cycle.
- **EchoReceivedHandler**: Handles data received from the echo server, using a non-blocking, cross-core call to SerialPrinter to ensure thread-safe, ordered output.
- **TcpPollHandler**: Executes lwIP poll activity on the async context (e.g., writer timeout checks). TcpClient no longer performs internal timeout checks; all poll-driven logic must be implemented in this handler.
- **TcpErrorHandler**: Receives error notifications, copies the error code from the bridge payload, and notifies the writer (if present) via `writer->onError(err)`.
- **TcpAckHandler**: Receives ACK notifications, copies the acknowledged length from the bridge payload, and notifies the writer (if present) via `writer->onAckReceived(len)`.

### Bridge Payload Contract

- **Error payload:** TcpClient allocates `err_t` on the heap and passes its pointer to the error handler via `bridge->workload(void*)`. TcpErrorHandler copies the value into an internal member and deletes the payload.
- **ACK payload:** TcpClient allocates `uint16_t` on the heap and passes its pointer to the ACK handler via `bridge->workload(void*)`. TcpAckHandler copies the value into an internal member and deletes the payload.
- **Ownership rule:** Handlers take ownership of the payload pointer provided to `workload(void*)`, must copy the data immediately, and must free the pointer to avoid leaks. This ensures the data outlives the stack frame where the callback was raised.

## Chunked Asynchronous Writes and ACK-Driven Flow Control

The async-tcp library implements robust, context-aware asynchronous write operations using a chunked state machine and ACK-driven flow control. This is a critical aspect of both the library and the application, ensuring safe, efficient, and correct data transmission across cores and contexts.

### How It Works
- When a write is initiated (e.g., via `echo_client.write()`), the data is managed by a `TcpWriter` instance, which breaks large writes into manageable chunks.
- Each chunk is sent using a `TcpWriteHandler`, which is always executed on the correct context/core as enforced by the async context system.
- The actual TCP write operation (`writeChunk`) is performed by the handler, and user code cannot customize or override this process, ensuring safety and consistency.
- When the TCP stack acknowledges a chunk (via an ACK), the internal callback (`_onAckCallback()`) dispatches the `TcpAckHandler` via the PerpetualBridge. The handler notifies the writer by calling `TcpWriter::onAckReceived(len)`.
- If all data has been acknowledged, the writer resets its state; if more data remains, it continues with the next chunk.

### Thread Safety and Context Guarantees
- All write operations and state transitions are performed on the context/core associated with the connection’s AsyncCtx, enforced by asserts and the handler system.
- Direct writer calls from lwIP callbacks are intentionally avoided. Notifications happen through bridge handlers under async-context guarantees.
- User code initiates writes at a high level (e.g., calling `write()`), but cannot interfere with internal chunking, ACK handling, or state machine.

### Why This Matters
- This approach ensures that large writes are safely and efficiently transmitted, even in a multi-core, interrupt-driven environment.
- It prevents data races, corruption, and protocol violations by strictly controlling where and how write operations occur.
- The application benefits from being able to initiate writes from the correct context, with chunking and flow control handled transparently by the library.

## Next Steps

- Expand the examples section to show handler registration for poll, error, and ACK in `setup()`.
- Summarize the concurrency model and access patterns for handlers and bridges in more detail.
- Optionally, instrument the code to log read/write counts for deeper analysis.
