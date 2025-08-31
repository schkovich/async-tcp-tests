/**
 * @file QotdReceivedHandler.cpp
 * @brief Implementation of the handler for Quote of the Day (QOTD) data
 * received events.
 *
 * This file implements the QotdReceivedHandler class which handles data
 * received events for a QOTD client. When quote data is received, this handler
 * processes the incoming data and stores it in a thread-safe buffer.
 *
 * The implementation demonstrates how to use the EventBridge pattern to ensure
 * that data handling occurs on the correct core with proper thread safety.
 *
 * @author Goran
 * @date 2025-02-18
 * @ingroup AsyncTCPClient
 */

#include "QotdReceivedHandler.hpp"
#include <Arduino.h>
#include "QotdConfig.hpp"

namespace e5 {

    /**
     * @brief Handles the initial data received event for QOTD.
     *
     * QOTD servers send the full quote and then immediately close the
     * connection (FIN). This callback performs only the first-step
     * processing; the remainder is drained on FIN.
     *
     * Specifically, this handler:
     * 1. Resets the quote buffer and completion flag to start a new quote
     * 2. Peeks up to QOTD_PARTIAL_CONSUMPTION_THRESHOLD bytes, copies them
     *    into the buffer via QuoteBuffer::set(), and
     * 3. Consumes exactly the processed bytes via IoRxBuffer::peekConsume()
     * 4. Defers draining of any remaining bytes to QotdFinHandler::onWork()
     *
     * Notes:
     * - The partial consumption threshold is configured by
     *   QOTD_PARTIAL_CONSUMPTION_THRESHOLD (see QotdConfig.hpp / main.cpp)
     * - This handler overwrites any previous content; no append/looping occurs
     *   here. Continuation is handled exclusively by the FIN handler.
     * - Executed on the context/core associated with this handler to maintain
     *   proper affinity.
     */
    void QotdReceivedHandler::onWork() {
        // ReSharper disable once CppDFANullDereference
        const size_t available = m_rx_buffer->peekAvailable();
        if (available == 0) {
            return;
        }

        // A new quote arriving: reset buffer and completion flag.
        m_quote_buffer.resetBuffer();

        // Consume up to threshold, or all available data if less
        const size_t consume_size = std::min(available, QOTD_PARTIAL_CONSUMPTION_THRESHOLD);


        // ReSharper disable once CppDFANullDereference
        const char *peek_buffer = m_rx_buffer->peekBuffer();
        // Create string from the chunk to be consumed
        const std::string quote_chunk(peek_buffer, consume_size);

        // Always set the first chunk; remaining data will be drained on FIN
        m_quote_buffer.set(quote_chunk);
        DEBUGWIRE("[QOTD] Consumed %zu/%zu bytes\n", consume_size, available);
        // ReSharper disable once CppDFANullDereference
        m_rx_buffer->peekConsume(consume_size);

        DEBUGWIRE("[QOTD] First chunk (%zu bytes): '%.*s...'\n",
                 consume_size,
                 static_cast<int>(std::min(quote_chunk.size(), static_cast<size_t>(20))),
                 quote_chunk.c_str());
    }

} // namespace e5