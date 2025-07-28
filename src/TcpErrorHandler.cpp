/**
 * @file TcpErrorHandler.cpp
 * @brief Implementation of TCP error handler
 *
 * This file implements the TcpErrorHandler class which handles TCP error events
 * and manages cleanup of failed write operations.
 */

#include "TcpErrorHandler.hpp"
#include <Arduino.h>

namespace e5 {

    TcpErrorHandler::TcpErrorHandler(const ContextManagerPtr& ctx,
                                    TcpClient& io,
                                    std::unique_ptr<std::string> data,
                                    size_t written,
                                    err_t error)
        : EventBridge(ctx), m_io(io), m_data(std::move(data)),
          m_written(written), m_error(error) {
    }

    void TcpErrorHandler::onWork() {
        // Log the error for debugging
        DEBUGWIRE("TcpErrorHandler: Write failed with error %d, %d bytes written\n",
                 m_error, m_written);

        // Perform cleanup based on error type
        switch (m_error) {
            case ERR_MEM:
                // Memory error - could retry later, but for now just log
                DEBUGWIRE("TcpErrorHandler: Memory error during write\n");
                break;

            case ERR_CONN:
                // Connection error - connection lost
                DEBUGWIRE("TcpErrorHandler: Connection error during write\n");
                break;

            case ERR_TIMEOUT:
                // Timeout error - write took too long
                DEBUGWIRE("TcpErrorHandler: Timeout error during write\n");
                break;

            default:
                // Other errors
                DEBUGWIRE("TcpErrorHandler: Unknown error %d during write\n", m_error);
                break;
        }

        // Cleanup - data will be automatically freed when m_data goes out of scope
        // EventBridge cleanup handles handler destruction automatically
    }

} // namespace e5
