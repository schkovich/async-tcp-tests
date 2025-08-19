/**
 * @file TcpWriterErrorHandler.hpp
 * @brief Handler for TCP writer error events, using the EventBridge pattern.
 *
 * This handler is designed to be registered with TcpClient::setOnWriterErrorCallback().
 * It will be executed in the async context when a writer error occurs.
 *
 * @author Goran
 * @date 2025-08-18
 */

#pragma once

#include "PerpetualBridge.hpp"
#include <functional>

namespace e5 {
    using namespace async_tcp;

class TcpWriterErrorHandler : public PerpetualBridge {
public:
    /**
     * @brief Constructor
     * @param ctx The async context manager
     * @param on_error_callback A callback to invoke on writer error (optional)
     */
    TcpWriterErrorHandler(const AsyncCtx& ctx, std::function<void()> on_error_callback = nullptr);

    /**
     * @brief Called in the async context when a writer error occurs.
     */
    void onWork() override;

private:
    std::function<void()> m_on_error_callback;
};

} // namespace e5
