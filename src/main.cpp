#include "pico/async_context_threadsafe_background.h"
#include <Arduino.h>

bool core1_separate_stack = true;

volatile bool operational = false;
volatile bool serial_ready = false;
static volatile uint64_t timestamp_exit = 0;
static volatile uint64_t timestamp_enter = 0;

static async_context_threadsafe_background_t async_ctx;

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
    delay(10);
  }

  RP2040::enableDoubleResetBootloader();
  serial_ready = true;
  while (!operational) {
    delay(10);
  }

  Serial1.printf("C0 ready...\n");
}

void setup1() {
  while (!serial_ready) {
    delay(10);
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
    const auto rc =
        async_context_execute_sync(&async_ctx.core, do_some_work, &ref_counter);
    assert(rc == ref_counter);
    timestamp_exit = to_us_since_boot(get_absolute_time());
    Serial1.printf("[Counter %d][Enter at: %llu][Exit at: %llu]\n",
                   rc, timestamp_enter, timestamp_exit);
  } else if (c0_counter == 77) {
    timestamp_enter = to_us_since_boot(get_absolute_time());
    const auto rc =
        async_context_execute_sync(&async_ctx.core, do_some_work, &ref_counter);
    assert(rc == ref_counter);
    timestamp_exit = to_us_since_boot(get_absolute_time());
    Serial1.printf("[Counter %d][Enter %llu] Exit %llu from boot\n",
                   rc, timestamp_enter, timestamp_exit);
  } else if (c0_counter == 111) {
    timestamp_enter = to_us_since_boot(get_absolute_time());
    const auto rc =
        async_context_execute_sync(&async_ctx.core, do_some_work, &ref_counter);
    assert(rc == ref_counter);
    timestamp_exit = to_us_since_boot(get_absolute_time());
    Serial1.printf("Counter %d; Enter %llu, Exit %llu from boot\n",
                   rc, timestamp_enter, timestamp_exit);
    c0_counter = 0;
  }
  c0_counter++;
  delay(1);
}

void loop1() { tight_loop_contents(); }
