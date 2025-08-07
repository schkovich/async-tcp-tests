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
        m_io.peekConsume(available);
        auto quote = std::make_unique<std::string>(chunk);
        m_serial_printer.print(std::move(quote));
    }

} // namespace e5
