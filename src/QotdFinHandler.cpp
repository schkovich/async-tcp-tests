/**
 * @file QotdFinHandler.cpp
 * @brief Implementation of the handler for QOTD client FIN events.
 *
 * This file implements the QotdFinHandler class which handles FIN
 * events for a Quote of the Day (QOTD) client.
 *
 * The implementation demonstrates how to use the EventBridge pattern to ensure
 * that event handling occurs on the correct core with proper thread
 * safety.
 *
 * @author Goran
 * @date 2025-08-26
 * @ingroup AsyncTCPClient
 */

#include "QotdFinHandler.hpp"
#include <Arduino.h>
#include "QotdConfig.hpp"

namespace e5 {

    /**
     * @brief Handles the FIN event.
     *
     * This method is called when a FIN packet is received.
     *
     * The method is executed on the core where the ContextManager was
     * initialized, ensuring proper core affinity for non-thread-safe
     * operations.
     */
    void QotdFinHandler::onWork() {
        auto available = m_rx_buffer->peekAvailable();
        if (available == 0) {
            // FIN with no data means all data was consumed by receive callback
            // Quote is complete, just mark it and stop connection.
            m_quote_buffer.setComplete();
            // Reset the buffer to free any pbuf resources
            m_rx_buffer->reset();
            m_io.shutdown();
            DEBUGWIRE(
                "[QOTD][FIN] no data, quote complete, connection stopped.");
            return;
        }

        // drain any remaining data
        DEBUGWIRE("[QOTD][FIN] draining %zu bytes\n", available);
        while (available > 0) {
            const size_t consume_size =
                std::min(available, QOTD_PARTIAL_CONSUMPTION_THRESHOLD);
            const char *peek_buffer = m_rx_buffer->peekBuffer();
            // Create string from the chunk to be consumed
            std::string quote_chunk(peek_buffer, consume_size);

            m_quote_buffer.append(quote_chunk);
            // ReSharper disable once CppDFANullDereference
            m_rx_buffer->peekConsume(consume_size);
            available = available - consume_size;
        }

        // Quote is complete after draining all remaining data
        m_quote_buffer.setComplete();
        // Reset the buffer. Data drained.
        // ReSharper disable once CppDFANullDereference
        m_rx_buffer->reset();
        m_io.shutdown();
        DEBUGWIRE("[QOTD] drained, quote complete, connection stopped: %d\n", m_io.status());
    }

} // namespace e5
