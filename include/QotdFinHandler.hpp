/**
 * @file QotdFinHandler.hpp
 * @brief Defines a handler for QOTD client FIN events.
 *
 * This file contains the QotdFinHandler class which implements the
 * EventBridge pattern to handle FIN events for a Quote of the Day (QOTD)
 * client. When a FIN packet is received, this handler is triggered.
 *
 * The handler demonstrates how to implement the EventBridge pattern for
 * specific event handling with proper core affinity.
 *
 * @author Goran
 * @date 2025-08-26
 * @ingroup AsyncTCPClient
 */

#pragma once
#include "ContextManager.hpp"
#include "PerpetualBridge.hpp"
#include "QuoteBuffer.hpp"
#include "TcpClient.hpp"

namespace e5 {
    using namespace async_tcp;

    /**
     * @class QotdFinHandler
     * @brief Handles the FIN event for a QOTD client.
     *
     * This handler is triggered when a FIN packet is received, indicating
     * the graceful termination of a connection by the server. It implements the
     * EventBridge pattern to ensure that the handling occurs on the correct
     * core with proper thread safety.
     */
    class QotdFinHandler final : public PerpetualBridge {
            TcpClient &m_io; /**< Reference to the TCP client handling the
                                     connection. */
        IoRxBuffer *m_rx_buffer = nullptr; /**< Pointer to the IO receive buffer
                                         associated with the TCP
                                         client. */
            QuoteBuffer
                &m_quote_buffer; /**< Buffer for storing the quote data. */

        protected:
            /**
             * @brief Handles the FIN event.
             *
             * This method is called when a FIN packet is received.
             *
             * The method is executed on the core where the ContextManager was
             * initialized, ensuring proper core affinity for non-thread-safe
             * operations.
             */
            void onWork() override;

        public:
            /**
             * @brief Constructs a QotdFinHandler.
             *
             * @param ctx A shared pointer to the context manager that will
             * execute this handler.
             * @param io A reference to the TCP client that established the
             * connection.
             * @param quote_buffer
             */
            explicit QotdFinHandler(const AsyncCtx &ctx,
                                          TcpClient &io,
                                          QuoteBuffer &quote_buffer)
                : PerpetualBridge(ctx), m_io(io),
                  m_quote_buffer(quote_buffer) {}

            // Override the virtual workload for RxBuffer
            void workload(void *data) override {
                m_rx_buffer = static_cast<IoRxBuffer*>(data);
            }

    };

} // namespace e5

