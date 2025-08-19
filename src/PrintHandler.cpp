/**
 * @file PrintHandler.cpp
 * @brief Implementation of the handler for serial printing tasks.
 *
 * This file implements the PrintHandler class which handles one-time
 * printing operations to the serial output. When triggered, this handler
 * prints a message and then removes itself from the SerialPrinter's task
 * registry.
 *
 * The implementation demonstrates how to use the EventBridge pattern for
 * self-cleaning, one-shot operations with proper core affinity.
 *
 * @author Goran
 * @date 2025-02-20
 * @ingroup AsyncTCPClient
 */

#include "PrintHandler.hpp"
#include <iostream>

#include <Arduino.h>

namespace e5 {

    /**
     * @brief Handles the print operation.
     *
     * This method is called when the print_handler is executed. It prints the
     * stored message to the serial output. The message and handler cleanup is
     * handled automatically by the EphemeralBridge's self-ownership mechanism.
     */
    void  PrintHandler::onWork() {
        if (!m_message->empty()) {
            Serial1.print(m_message->c_str());
            digitalWrite(LED_BUILTIN, LOW);
        }
    }

    /**
     * @brief Constructs a PrintHandler with the specified context, message, and
     * printer.
     *
     * @param ctx Context manager for execution
     * @param message Message to print
     */
    PrintHandler::PrintHandler(const AsyncCtx &ctx,
                               std::unique_ptr<std::string> message)
        : EphemeralBridge(ctx), m_message(std::move(message)) {}
} // namespace e5