/**
 * @file SerialPrinter.cpp
 * @brief Implementation of asynchronous serial printing utility
 *
 * This file implements the SerialPrinter class which provides asynchronous
 * printing capabilities using the async context for proper core affinity.
 */

#include "SerialPrinter.hpp"
#include "ContextManager.hpp"
#include "MessageBuffer.hpp"
#include "PrintHandler.hpp"

namespace e5 {

    // Constructor implementation
    SerialPrinter::SerialPrinter(const ContextManagerPtr &ctx) : m_ctx(ctx) {}

    // Print method implementation for std::string
    uint32_t SerialPrinter::print(std::unique_ptr<std::string> message) {
        // Create and run handler using factory method
        PrintHandler::create(m_ctx, std::move(message));
        return PICO_OK; // Return success code
    }

} // namespace e5
