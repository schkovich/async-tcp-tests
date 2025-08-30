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

namespace e5 {

    /**
     * @brief Handles the data received event with partial consumption testing.
     *
     * This method implements partial consumption testing by:
     * 1. Checking if this is the first chunk or continuation of a quote
     * 2. For quotes longer than PARTIAL_CONSUMPTION_THRESHOLD, consuming only
     *    part of the data and leaving the rest for subsequent onWork() calls
     * 3. Using set() for the first chunk and append() for continuations
     * 4. Processing all remaining data when FIN is received (handled by lwIP)
     *
     * This tests the IoRxBuffer::peekConsume() partial consumption logic across
     * multiple async work cycles and pbuf chain management.
     */
    void QotdReceivedHandler::onWork() {
        const size_t available = m_rx_buffer->peekAvailable();
        if (available == 0) {
            return;
        }

        // Threshold for partial consumption testing
        static constexpr size_t PARTIAL_CONSUMPTION_THRESHOLD = 88;

        // Consume up to threshold, or all available data if less
        const size_t consume_size = std::min(available, PARTIAL_CONSUMPTION_THRESHOLD);


        const char *peek_buffer = m_rx_buffer->peekBuffer();
        // Create string from the chunk to be consumed
        const std::string quote_chunk(peek_buffer, consume_size);

        // Store in quote buffer - set() for first chunk, append() for continuation
        if (m_quote_buffer.empty()) {
            m_quote_buffer.set(quote_chunk);
            if (consume_size <= PARTIAL_CONSUMPTION_THRESHOLD) {
                DEBUGWIRE("[QOTD] Consumed %zu/%zu bytes\n", consume_size, available);
                m_rx_buffer->peekConsume(consume_size);
            }
            DEBUGWIRE("[QOTD] First chunk complete (%zu bytes)\n", consume_size);

            DEBUGWIRE("[QOTD] First chunk (%zu bytes): '%.*s...'\n",
                     consume_size,
                     static_cast<int>(std::min(quote_chunk.size(), static_cast<size_t>(20))),
                     quote_chunk.c_str());
            return;
        }

        m_quote_buffer.append(quote_chunk);
        DEBUGWIRE("[QOTD] Appending chunk (%zu bytes): '%.*s...'\n",
                 consume_size,
                 static_cast<int>(std::min(quote_chunk.size(), static_cast<size_t>(20))),
                 quote_chunk.c_str());

        // Consume the processed chunk from TCP buffer
        m_rx_buffer->peekConsume(consume_size);

        DEBUGWIRE("[QOTD] Consumed %zu/%zu bytes\n", consume_size, available);
    }

} // namespace e5