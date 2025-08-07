/*
 * AsyncTCPClient Example Implementation
 *
 * This file demonstrates the use of the AsyncTCPClient library to connect to a
 * Quote of the Day (QOTD) server and an Echo server asynchronously.
 *
 * It showcases:
 * - Proper thread safety using SyncBridge for shared resources
 * - Event handling with EventBridge derivatives
 * - Core affinity management for non-thread-safe operations
 * - Asynchronous networking on a dual-core Raspberry Pi Pico
 */

#ifndef ESPHOSTSPI
#error This example requires an ESP-Hosted-FG WiFi chip to be defined, see the documentation
// For example, add this to your boards.local.txt:
// rpipico.build.extra_flags=-DESPHOST_RESET=D5 -DESPHOST_HANDSHAKE=D7
// -DESPHOST_DATA_READY=D6 -DESPHOST_CS=D1 -DESPHOSTSPI=SPI
#endif

#include "ContextManager.hpp"
#include "EchoConnectedHandler.hpp"
#include "EchoReceivedHandler.hpp"
#include "QotdClosedHandler.hpp"
#include "QotdConnectedHandler.hpp"
#include "QotdReceivedHandler.hpp"
#include "QuoteBuffer.hpp"
#include "SerialPrinter.hpp"
#include "TcpClient.hpp"
#include "TcpWriter.hpp"
#include "LoopScheduler.hpp"
#include "secrets.h" // Contains STASSID, STAPSK, QOTD_HOST, ECHO_HOST, QOTD_PORT, ECHO_PORT
#include <WiFi.h>
#include <algorithm>

using namespace async_tcp;
/**
 * Allocate separate 8KB stack for core1
 *
 * When false: 8KB stack is split between cores (4KB each)
 * When true:  Each core gets its own 8KB stack
 */
bool core1_separate_stack = true;

volatile bool operational = false; // Global flag for core synchronization
volatile bool ctx1_ready = false;  // For loop() to wait for setup1()
volatile bool qotd_in_progress = false;
volatile bool echo_connected = false;
// WiFi credentials from secrets.h
const auto *ssid = STASSID;
const auto *password = STAPSK;
WiFiMulti multi;

// Server details
const auto *qotd_host = QOTD_HOST;
const auto *echo_host = ECHO_HOST;
constexpr uint16_t qotd_port = QOTD_PORT;
constexpr uint16_t echo_port = ECHO_PORT;

// TCP clients
TcpClient qotd_client;
TcpClient echo_client;

// IP addresses
IPAddress qotd_ip_address;
IPAddress echo_ip_address;

// Global asynchronous context managers for each core
static AsyncCtx ctx0 = {}; // TCP Client Core 0
static AsyncCtx ctx1 = {}; // SerialPrinter and QuoteBuffer on Core 1

// Thread-safe buffer for storing the quote
e5::QuoteBuffer qotd_buffer(ctx1);

// Set up the SerialPrinter for Core 1
e5::SerialPrinter serial_printer(ctx1);

// Timing variables
static e5::LoopScheduler scheduler0; // For Core 0
static e5::LoopScheduler scheduler1; // For Core 1
static const std::string qotd = "qotd";
static const std::string echo = "echo";
static const std::string stack_0 = "stack_0";
static const std::string stack_1 = "stack_1";
static const std::string heap = "heap";

/**
 * @brief Connects to the "quote of the day" server and initiates a connection.
 *
 * Connects to the QOTD server if there isn't another active connection.
 */
void get_quote_of_the_day() {
    // Check if we're already connected first
    if (qotd_in_progress) {
        DEBUGWIRE("QOTD client already connected, skipping new connection\n");
        return;
    }
    DEBUGWIRE(
        "Connecting to QOTD server for new quote\n");
    qotd_in_progress = true;
    if (!qotd_client.connect(qotd_ip_address, qotd_port)) {
        qotd_in_progress = false;
        DEBUGV("Failed to connect to QOTD server.\n");
    }
    qotd_buffer.clear();
}

/**
 * @brief Connects to the echo server and sends data if available.
 *
 * This function checks if the echo client is connected. If not, it attempts to
 * connect. If connected and there is data in the transmission buffer.
 */
void get_echo() {
    const std::string buffer_content = qotd_buffer.get();
    if (!buffer_content.empty()) {
        if (!echo_connected) {
            if (0 == echo_client.connect(echo_ip_address, echo_port)) {
                DEBUGV("Failed to connect to echo server..\n");
                return;
            }
            echo_connected = true;
        }
        DEBUGWIRE("Sending quote to echo server (%d bytes)\n", buffer_content.size());
        if (const size_t sent = echo_client.write(
                reinterpret_cast<const uint8_t *>(buffer_content.c_str()),
                buffer_content.size());
            sent < buffer_content.size()) {
            DEBUGWIRE("[ERROR] echo_client.write sent only %u/%u bytes\n", sent, buffer_content.size());
        }
    }
}

/**
 * @brief Prints heap statistics using the SerialPrinter.
 */
void print_heap_stats() {
    // Get heap data
    const int freeHeap = rp2040.getFreeHeap();
    const int usedHeap = rp2040.getUsedHeap();
    const int totalHeap = rp2040.getTotalHeap();

    // Format the string with stats using the same syntax as notify_connect
    auto heap_stats = std::make_unique<std::string>(
        "Free: " + std::to_string(freeHeap) +
        ", Used: " + std::to_string(usedHeap) +
        ", Total: " + std::to_string(totalHeap) + "\n");

    serial_printer.print(std::move(heap_stats));
}

/**
 * @brief Prints stack statistics for the current core.
 *
 * This function retrieves the free stack size and formats it into a string
 * which is then printed using the SerialPrinter.
 */
void print_stack_stats() {
    const auto free_stack = rp2040.getFreeStack();

    // Format the string with stack for calling core stats
    auto stack_stats = std::make_unique<std::string>(
        "Free Stack on core " + std::to_string(get_core_num()) + ": " +
        std::to_string(free_stack) + "\n");
    serial_printer.print(std::move(stack_stats));
}

/**
 * @brief Initializes the Wi-Fi connection and asynchronous context on Core 0.
 */
void setup() {
    Serial1.begin(115200);
    while (!Serial1) {
        tight_loop_contents();
    }

    RP2040::enableDoubleResetBootloader();

    multi.addAP(ssid, password);

    if (multi.run() != WL_CONNECTED) {
        DEBUGV("Unable to connect to network, rebooting in 10 seconds...\n");
        delay(10000);
        rp2040.reboot();
    }

    hostByName(qotd_host, qotd_ip_address, 1000);
    hostByName(echo_host, echo_ip_address, 1000);

    auto config = async_context_threadsafe_background_default_config();
    config.custom_alarm_pool = alarm_pool_create_with_unused_hardware_alarm(16);
    if (!ctx0.initDefaultContext(config)) {
        panic_compact("CTX init failed on Core 0\n");
    }

    // Create TcpWriter locally and transfer ownership to the echo client
    auto echo_writer = std::make_unique<TcpWriter>(ctx0, echo_client);
    echo_client.setWriter(std::move(echo_writer));

    auto echo_connected_handler = std::make_unique<e5::EchoConnectedHandler>(
        ctx0, echo_client, serial_printer);
    echo_connected_handler->initialisePerpetualBridge();
    echo_client.setOnConnectedCallback(std::move(echo_connected_handler));

    auto echo_received_handler = std::make_unique<e5::EchoReceivedHandler>(
        ctx0, echo_client, serial_printer, qotd_buffer);
    echo_received_handler->initialisePerpetualBridge();
    echo_client.setOnReceivedCallback(std::move(echo_received_handler));

    auto qotd_connected_handler = std::make_unique<e5::QotdConnectedHandler>(
        ctx0, qotd_client, serial_printer, qotd_buffer);
        qotd_connected_handler->initialisePerpetualBridge();
    qotd_client.setOnConnectedCallback(std::move(qotd_connected_handler));

    auto qotd_received_handler = std::make_unique<e5::QotdReceivedHandler>(
        ctx0, qotd_buffer, qotd_client);
    qotd_received_handler->initialisePerpetualBridge();
    qotd_client.setOnReceivedCallback(std::move(qotd_received_handler));

    auto qotd_closed_handler = std::make_unique<e5::QotdClosedHandler>(
        ctx0, qotd_buffer, qotd_in_progress);
    qotd_closed_handler->initialisePerpetualBridge();
    qotd_client.setOnClosedCallback(std::move(qotd_closed_handler));

    scheduler0.setEntry(qotd, 1010);
    scheduler0.setEntry(echo, 101);
    scheduler0.setEntry(stack_0, 707070);

    operational = true;
}

/**
 * @brief Initializes the asynchronous context on Core 1.
 */
void setup1() {
    while (!operational) {
        tight_loop_contents();
    }

    if (auto config = async_context_threadsafe_background_default_config();
        !ctx1.initDefaultContext(config)) {
        panic_compact("CTX init failed on Core 1\n");
    }

    scheduler1.setEntry(stack_1, 909090);
    scheduler1.setEntry(heap, 808080);
    ctx1_ready = true;
}

/**
 * @brief Main loop function on Core 0.
 * Handles periodic requests to the QOTD and echo servers.
 */
void loop() {

    if (!ctx1_ready) {
        delay(1);
        return;
    }

    if (scheduler0.timeToRun(qotd)) get_quote_of_the_day();
    if (scheduler0.timeToRun(echo)) get_echo();
    if (scheduler0.timeToRun(stack_0)) print_stack_stats();
}

/**
 * @brief Loop function for Core 1.
 * Currently empties as all work is handled through the async context.
 */
void loop1() {
    if (scheduler1.timeToRun(stack_1)) print_stack_stats();
    if (scheduler1.timeToRun(heap)) print_heap_stats();
}
