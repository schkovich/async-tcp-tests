/**
 * @file EchoReceivedHandler.hpp
 * @brief Defines a handler for TCP client data received events.
 *
 * This file contains the EchoReceivedHandler class which implements the
 * EventBridge pattern to handle data received events for an echo client. When
 * data is received, this handler processes the incoming data directly as it
 * arrives in natural chunks due to having Nagle's algorithm disabled.
 *
 * @author Goran
 * @date 2025-02-17
 * @ingroup AsyncTCPClient
 */

#pragma once
#include "PerpetualBridge.hpp"
#include "SerialPrinter.hpp"
#include "TcpClient.hpp"
#include "QuoteBuffer.hpp"

namespace e5 {
    using namespace async_tcp;

    /**
     * @class EchoReceivedHandler
     * @brief Handles the data received event for an echo client.
     *
     * This handler is triggered when data is received on a TCP connection
     * for an echo client. It implements the EventBridge pattern to ensure that
     * the handling occurs on the correct core with proper thread safety.
     *
     * The handler processes naturally chunked data (since Nagle's algorithm is
     * disabled) and then outputs it through the SerialPrinter.
     */
    class EchoReceivedHandler final : public PerpetualBridge {
            TcpClient &m_io; /**< Reference to the TCP client handling the
                                     connection. */
            SerialPrinter &m_serial_printer; /**< Reference to the serial
                                                printer for output. */
            QuoteBuffer &m_qotd_buffer; /**< Reference to the quote buffer for
                                            storing received data. */

        protected:
            /**
             * @brief Handles the data received event.
             *
             * This method is called when data is received on the TCP
             * connection. It processes the available data and outputs
             * it through the SerialPrinter.
             *
             * The method is executed on the core where the ContextManager was
             * initialized, ensuring proper core affinity for non-thread-safe
             * operations.
             */
            void onWork() override;

        public:
            /**
             * @brief Constructs an EchoReceivedHandler.
             *
             * @param ctx Shared pointer to the context manager that will
             * execute this handler
             * @param io Reference to the TCP client that received the data
             * @param serial_printer Reference to the serial printer for output
             * messages
             * @param qotd_buffer Reference to the quote buffer for storing
             * received data
             */
            explicit EchoReceivedHandler(const AsyncCtx &ctx,
                                         TcpClient &io,
                                         SerialPrinter &serial_printer,
                                         QuoteBuffer &qotd_buffer)
                : PerpetualBridge(ctx),
                  m_io(io),
                  m_serial_printer(serial_printer),
                  m_qotd_buffer(qotd_buffer) {
            }
    };

} // namespace e5