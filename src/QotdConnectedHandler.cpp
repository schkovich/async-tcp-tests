/**
 * @file QotdConnectedHandler.cpp
 * @brief Implementation of the handler for QOTD client connection events.
 *
 * This file implements the QotdConnectedHandler class which handles connection
 * established events for a Quote of the Day (QOTD) client. When a connection is
 * established, this handler configures the connection parameters and notifies
 * the user through the serial printer.
 *
 * The implementation demonstrates how to use the EventBridge pattern to ensure
 * that connection handling occurs on the correct core with proper thread
 * safety.
 *
 * @author Goran
 * @date 2025-02-19
 * @ingroup AsyncTCPClient
 */

#include "QotdConnectedHandler.hpp"
#include <Arduino.h>

namespace e5 {

    /**
     * @brief Handles the connection established event.
     *
     * This method is called when the TCP connection is established. It:
     * latency
     * 1. Retrieves the local IP address of the connection
     * 2. Formats and prints a message with the local IP address
     *
     * The method is executed on the core where the ContextManager was
     * initialized, ensuring proper core affinity for non-thread-safe operations
     * like printing.
     */
    void QotdConnectedHandler::onWork() {

        m_quote_buffer.resetBuffer();
        auto notify_connect = std::make_unique<std::string>(
            std::string("[INFO] Getting a quote from: ")
            + m_io.remoteIP().toString().c_str()
            + "\n");

        m_serial_printer.print(std::move(notify_connect));
    }

} // namespace e5