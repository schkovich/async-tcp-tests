/**
 * @file QuoteBuffer.cpp
 * @brief Implementation of the thread-safe buffer for storing and accessing the
 * quote of the day
 *
 * This file implements the QuoteBuffer class which provides thread-safe access
 * to a string buffer using the SyncBridge pattern.
 *
 * @author Goran
 * @date 2025-02-20
 * @ingroup AsyncTCPClient
 */

#include "QuoteBuffer.hpp"
#include <Arduino.h>

namespace e5 {

    /**
     * @brief Constructs a QuoteBuffer with the specified context manager
     *
     * @param ctx Shared context manager for synchronized execution
     */
    QuoteBuffer::QuoteBuffer(const AsyncCtx &ctx) : SyncBridge(ctx) {}

    /**
     * @brief Executes buffer operations in a thread-safe manner
     *
     * This method is called by the SyncBridge to perform modifications to the
     * buffer. It ensures that all modifications happen on the core where
     * the ContextManager was initialized, providing thread safety.
     *
     * @param payload Buffer operation instruction
     * @return PICO_OK on success, or error code on failure
     */
    uint32_t QuoteBuffer::onExecute(const SyncPayloadPtr payload) {

        switch (const auto *buffer_payload =
                    static_cast<BufferPayload *>(payload.get());
                buffer_payload->op) {
        case BufferPayload::SET:
            m_buffer = buffer_payload->data;
            return PICO_OK;

        case BufferPayload::APPEND:
            m_buffer += buffer_payload->data;
            return PICO_OK;

        default:
            if (buffer_payload->result_ptr) {
                *buffer_payload->result_ptr = m_buffer;
                return PICO_OK;
            }
            return PICO_ERROR_NO_DATA;
        }
    }

    /**
     * @brief Sets the buffer content
     *
     * This method replaces the current buffer content with the provided string.
     * Thread-safe through SyncBridge integration, can be called from any core
     * or interrupt context.
     *
     * @param data String to set as the buffer content
     */
    void QuoteBuffer::set(const std::string data) { // NOLINT
        auto payload = std::make_unique<BufferPayload>();
        payload->op = BufferPayload::SET;
        payload->data = data;
        if (const auto result = execute(std::move(payload));
            result != PICO_OK) {
            DEBUGV("[c%d][%llu][ERROR] QuoteBuffer::set() returned error "
                   "%d.\n",
                   rp2040.cpuid(), time_us_64(), result);
        }
    }

    /**
     * @brief Gets the buffer content
     *
     * This method returns a copy of the current buffer content.
     * Thread-safe through SyncBridge integration, can be called from any core
     * or interrupt context.
     *
     * @return Copy of the current buffer content
     */
    std::string QuoteBuffer::get() { // NOLINT
        std::string result_string;
        auto payload = std::make_unique<BufferPayload>();
        payload->op = BufferPayload::GET;
        payload->result_ptr = &result_string;

        if (const auto result = execute(std::move(payload));
            result != PICO_OK) {
            DEBUGV("[c%d][%llu][ERROR] QuoteBuffer::get() returned error "
                   "%d.\n",
                   rp2040.cpuid(), time_us_64(), result);
        }

        return result_string;
    }

    /**
     * @brief Appends data to the buffer content
     *
     * This method adds the provided string to the end of the current buffer
     * content. Thread-safe through SyncBridge integration, can be called from
     * any core or interrupt context.
     *
     * @param data String to append to the buffer content
     */
    void QuoteBuffer::append(const std::string data) { // NOLINT
        auto payload = std::make_unique<BufferPayload>();
        payload->op = BufferPayload::APPEND;
        payload->data = data;
        if (const auto result = execute(std::move(payload));
            result != PICO_OK) {
            DEBUGV("[c%d][%llu][ERROR] QuoteBuffer::append() returned "
                   "error %d.\n",
                   rp2040.cpuid(), time_us_64(), result);
        }
    }

    /**
     * @brief Checks if the buffer is empty
     *
     * This method checks if the current buffer content is empty.
     * Thread-safe through SyncBridge integration, can be called from any core
     * or interrupt context.
     *
     * @return true if the buffer is empty, false otherwise
     */
    bool QuoteBuffer::empty() {
        std::string result_string;
        auto payload = std::make_unique<BufferPayload>();
        payload->op = BufferPayload::GET;
        payload->result_ptr = &result_string;
        if (const auto result = execute(std::move(payload));
            result != PICO_OK) {
            DEBUGV("[c%d][%llu][ERROR] QuoteBuffer::empty() returned error %d.\n",
                   rp2040.cpuid(), time_us_64(), result);
        }
        return result_string.empty();
    }

    /**
     * @brief Clears the buffer content
     *
     * This method removes the current buffer content, making the buffer empty.
     * Thread-safe through SyncBridge integration, can be called from any core
     * or interrupt context.
     */
    void QuoteBuffer::clear() {
        auto payload = std::make_unique<BufferPayload>();
        payload->op = BufferPayload::SET;
        payload->data = "";
        if (const auto result = execute(std::move(payload));
            result != PICO_OK) {
            DEBUGV("[c%d][%llu][ERROR] QuoteBuffer::clear() returned error %d.\n",
                   rp2040.cpuid(), time_us_64(), result);
        }
    }

} // namespace e5
