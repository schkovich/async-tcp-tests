/**
 * @file TcpErrorHandler.hpp
 * @brief Handler for TCP error events using EventBridge pattern
 *
 * This file defines the TcpErrorHandler class which implements the EventBridge
 * pattern to handle TCP error events. It manages cleanup and error reporting
 * for failed write operations.
 *
 * @author Goran
 * @date 2025-02-20
 * @ingroup AsyncTCPClient
 */

#pragma once

#include "ContextManager.hpp"
#include "PerpetualBridge.hpp"
#include "TcpClient.hpp"
#include <lwip/err.h>

namespace e5 {

    using namespace async_tcp;

    /**
     * @class TcpErrorHandler
     * @brief Handles TCP error events using EventBridge pattern
     *
     * This handler processes error notifications from the TCP layer and
     * performs appropriate cleanup. It follows the EventBridge pattern
     * for proper core affinity and thread safety.
     */
    class TcpErrorHandler final : public PerpetualBridge {
            TcpClient &m_io;                     ///< TCP client reference
            err_t m_error = ERR_OK;             ///< Last error code delivered via workload

        protected:
            /**
             * @brief Processes the error event
             *
             * This method handles the error condition, performs cleanup,
             * and optionally schedules retry logic.
             */
            void onWork() override;

        public:
            /**
             * @brief Constructs a TcpErrorHandler
             *
             * @param ctx Context manager for execution
             * @param io TCP client reference
             */
            TcpErrorHandler(const AsyncCtx &ctx, TcpClient &io)
                : PerpetualBridge(ctx), m_io(io) {}

            // Accept error code via EventBridge workload (mirrors EchoReceivedHandler pattern)
            void workload(void *data) override {
                if (data) {
                    const auto *err_ptr = static_cast<err_t*>(data);
                    m_error = *err_ptr;
                    delete err_ptr; // take ownership and free payload
                }
            }
    };

} // namespace e5
