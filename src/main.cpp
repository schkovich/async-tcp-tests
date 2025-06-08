#include "pico/async_context_threadsafe_background.h"
#include <Arduino.h>
#include <atomic>

bool core1_separate_stack = true;

volatile bool operational = false;
volatile bool serial_ready = false;
static volatile uint64_t timestamp_exit = 0;
static volatile uint64_t timestamp_enter = 0;

static async_context_threadsafe_background_t async_ctx;

struct hello_world_state {
    unsigned long *counter;
    async_at_time_worker_t worker;
    std::atomic_bool in_use;
};

extern "C" {
    void hello_world(async_context_t *context, struct async_work_on_timeout *timeout) { // NOLINT
        (void) context;
        auto *state = static_cast<hello_world_state*>(timeout->user_data);
        Serial1.printf("Hello from cycle %lu!\n", *state->counter);
        state->in_use.store(false);
    }
}

uint32_t do_some_work(void *param) {
    const auto value = static_cast<uint32_t *>(param);
    (*value)++;
    return *value;
}

void setup() {
    Serial1.setRX(PIN_SERIAL1_RX);
    Serial1.setTX(PIN_SERIAL1_TX);
    Serial1.setPollingMode(true);
    Serial1.begin(115200);

    while (!Serial1) {
        tight_loop_contents();
    }

    RP2040::enableDoubleResetBootloader();
    serial_ready = true;
    while (!operational) {
        ;
    }

    Serial1.printf("C0 ready...\n");
}

void setup1() {
    while (!serial_ready) {
        tight_loop_contents();
    }
    async_context_threadsafe_background_config_t cfg =
        async_context_threadsafe_background_default_config();
    operational = async_context_threadsafe_background_init(&async_ctx, &cfg);
    assert(operational);
    Serial1.printf("C1 ready...\n");
}

void loop() {
    static unsigned long c0_counter = 0;
    static uint32_t ref_counter = 0;

    if (c0_counter == 55) {
        timestamp_enter = to_us_since_boot(get_absolute_time());
        const auto rc = async_context_execute_sync(&async_ctx.core,
                                                   do_some_work, &ref_counter);
        assert(rc == ref_counter);
        timestamp_exit = to_us_since_boot(get_absolute_time());
        Serial1.printf("[Counter %d][Enter at: %llu][Exit at: %llu]\n", rc,
                       timestamp_enter, timestamp_exit);
    } else if (c0_counter == 77) {
        timestamp_enter = to_us_since_boot(get_absolute_time());
        const auto rc = async_context_execute_sync(&async_ctx.core,
                                                   do_some_work, &ref_counter);
        assert(rc == ref_counter);
        timestamp_exit = to_us_since_boot(get_absolute_time());
        Serial1.printf("[Counter %d][Enter %llu] Exit %llu from boot\n", rc,
                       timestamp_enter, timestamp_exit);
    } else if (c0_counter == 111) {
        timestamp_enter = to_us_since_boot(get_absolute_time());
        const auto rc = async_context_execute_sync(&async_ctx.core,
                                                   do_some_work, &ref_counter);
        assert(rc == ref_counter);
        timestamp_exit = to_us_since_boot(get_absolute_time());
        Serial1.printf("Counter %d; Enter %llu, Exit %llu from boot\n", rc,
                       timestamp_enter, timestamp_exit);
        c0_counter = 0;
    }
    c0_counter++;
    sleep_us(777);
}

void loop1() {
    static uint32_t atw_counter = 0;
    static unsigned long c1_counter = 0;
    static hello_world_state state = {
        .counter = &atw_counter,
        .worker = {nullptr},
        .in_use = false
    };

    if (c1_counter == 33) {
        atw_counter++;
        if (!state.in_use.exchange(true)) {  // Only proceed if we can acquire the state
            state.worker.do_work = hello_world;
            state.worker.user_data = &state;  // Pass whole state so we can clear in_use
            async_context_add_at_time_worker_in_ms(&async_ctx.core, &state.worker, 7);
        }
        c1_counter = 0;
    }
    c1_counter++;

    sleep_us(77);
}
