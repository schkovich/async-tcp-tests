/**
 * @file EchoReceivedHandler.cpp
 * @brief Implementation of the handler for TCP client data received events.
 *
 * This file implements the EchoReceivedHandler class which handles data
 * received events from the echo server. Since Nagle's algorithm is disabled,
 * data naturally arrives in network-determined chunks.
 *
 * @author Goran
 * @date 2025-02-17
 * @ingroup AsyncTCPClient
 */

#include "EchoReceivedHandler.hpp"
#include "QuoteBuffer.hpp"  // for END_OF_QUOTE_MARKER
#include <string>

namespace e5 {

    /**
     * @brief Handles the data received event.
     *
     * This method is called when data is received on the TCP connection. It:
     * 1. Peeks at available data in the TCP buffer without consuming it
     * 2. Creates a string from the peeked data
     * 3. Outputs the data through the SerialPrinter with chunk information
     * 4. Consumes the data from the TCP buffer
     *
     * With Nagle's algorithm disabled, data arrives in multiple TCP
     * segments based on network conditions.
     */
    void EchoReceivedHandler::onWork() {
        const size_t available = m_io.peekAvailable();
        if (available == 0) return;
        const char *data = m_io.peekBuffer();
        // Print any incoming echo data
        std::string chunk(data, available);
        // Accumulate buffer for marker detection
        m_buffer.append(chunk);
        m_io.peekConsume(available);
        // When end-of-quote marker is seen across any segments, clear buffer and quote
        static const std::string marker = ::e5::QuoteBuffer::END_OF_QUOTE_MARKER;
        if (m_buffer.find(marker) != std::string::npos) {
            auto quote = std::make_unique<std::string>(m_buffer);
                m_serial_printer.print(std::move(quote));
            m_qotd_buffer.set("");
            m_buffer.clear();
        }
    }

} // namespace e5
