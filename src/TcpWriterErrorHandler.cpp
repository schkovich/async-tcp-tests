/**
 * @file TcpWriterErrorHandler.cpp
 * @brief Implementation of the handler for TCP writer error events.
 *
 * This handler is designed to be registered with
 * TcpClient::setOnWriterErrorCallback(). It will be executed in the async
 * context when a writer error occurs.
 *
 * @author Goran
 * @date 2025-08-18
 */

#include "TcpWriterErrorHandler.hpp"

#include "EphemeralBridge.hpp"

#include <cstdio>

namespace e5 {

    TcpWriterErrorHandler::TcpWriterErrorHandler(
        const AsyncCtx &ctx, std::function<void()> on_error_callback)
        : PerpetualBridge(ctx),
          m_on_error_callback(std::move(on_error_callback)) {}

    void TcpWriterErrorHandler::onWork() {
        // Example: log the error and invoke the callback if provided
        printf("[TcpWriterErrorHandler] Writer error occurred.\n");
        if (m_on_error_callback) {
            m_on_error_callback();
        }
    }

} // namespace e5
