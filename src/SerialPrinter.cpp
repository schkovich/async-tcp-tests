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
#include <atomic>

namespace e5 {

    std::atomic<bool> SerialPrinter::print_lock{false};

    // Constructor implementation
    SerialPrinter::SerialPrinter(const ContextManagerPtr &ctx) : m_ctx(ctx) {}

    // Print method implementation for std::string
    uint32_t SerialPrinter::print(std::unique_ptr<std::string> message) {
        bool expected = false;
        // Use class static member for serialization and reentrancy guard
        if (!print_lock.compare_exchange_strong(expected, true, std::memory_order_acquire)) {
            // Lock is already held, skip or handle contention (for test, just return error)
            return PICO_ERROR_RESOURCE_IN_USE; // Indicate contention
        }
        PrintHandler::create(m_ctx, std::move(message));
        print_lock.store(false, std::memory_order_release);
        return PICO_OK; // Return success code
    }

} // namespace e5
