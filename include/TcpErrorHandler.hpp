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
#include "EventBridge.hpp"
#include "TcpClient.hpp"
#include <lwip/err.h>
#include <memory>
#include <string>

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
    class TcpErrorHandler final : public EventBridge {
        private:
            TcpClient& m_io;                              ///< TCP client reference
            std::unique_ptr<std::string> m_data;          ///< Data that failed to write
            size_t m_written;                             ///< Bytes written before error
            err_t m_error;                                ///< Error code

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
             * @param data Data that failed to write
             * @param written Bytes written before error
             * @param error Error code from TCP layer
             */
            TcpErrorHandler(const ContextManagerPtr& ctx,
                           TcpClient& io,
                           std::unique_ptr<std::string> data,
                           size_t written,
                           err_t error);
    };

} // namespace e5
