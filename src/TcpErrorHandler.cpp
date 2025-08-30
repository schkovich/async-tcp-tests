/**
 * @file TcpErrorHandler.cpp
 * @brief Implementation of TCP error handler
 *
 * This file implements the TcpErrorHandler class which handles TCP error events
 * and manages cleanup tasks.
 */

#include "TcpErrorHandler.hpp"
#include <Arduino.h>

namespace e5 {

    void TcpErrorHandler::onWork() {
        // Notify writer about the error if configured
        if (auto *writer = m_io.getWriter()) {
            writer->onError(m_error);
        }
        DEBUGWIRE("[TcpErrorHandler][:i%d] Error %d handled\n",
                  m_io.getClientId(), static_cast<int>(m_error));
    }

} // namespace e5
