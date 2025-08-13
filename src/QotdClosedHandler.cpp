/**
 * @file QotdClosedHandler.cpp
 * @brief Implementation of the handler for QOTD client connection closure
 * events.
 *
 * This file implements the QotdClosedHandler class which handles connection
 * closure events for a Quote of the Day (QOTD) client. When the server closes
 * the connection, this handler processes the complete quote received and
 * notifies the user through the serial printer.
 *
 * The implementation demonstrates how to use the EventBridge pattern to ensure
 * that connection closure handling occurs on the correct core with proper
 * thread safety.
 *
 * @author Goran
 * @date 2025-06-22
 * @ingroup AsyncTCPClient
 */

#include "QotdClosedHandler.hpp"

namespace e5 {

    /**
     * @brief Handles the connection closure event.
     *
     * This method is called when the TCP connection is closed by the server.
     * It:
     * 1. Retrieves the complete quote from the buffer
     * 2. Notifies the user that the quote has been received completely
     * 3. Sets a marker in the buffer indicating the transmission is complete
     *
     * In the QOTD protocol, the server closing the connection indicates that
     * the complete quote has been transmitted, so this handler is essential for
     * proper protocol implementation.
     *
     * The method is executed on the core where the ContextManager was
     * initialised, ensuring proper core affinity for non-thread-safe operations
     * like printing.
     */
    void QotdClosedHandler::onWork() {
        m_qotd_in_progress = false;
    }

} // namespace e5
