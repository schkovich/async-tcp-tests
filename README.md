# async-tcp-tests

This repository was initially set as a testbed for debugging and validating fixes for two bugs in the Raspberry Pi Pico SDK. Over time, it evolved into a dedicated environment for developing and testing the async-tcp library.

## Current Purpose

The repository is now focused on testing and demonstrating the async-tcp library, which provides an event-driven, thread-safe TCP client abstraction for the RP2040 platform using async_context and lwIP.

## Quick Start

To evaluate or build this project:

1. **Clone this repository:**

   ```sh
   git clone https://github.com/schkovich/async-tcp-tests.git
   cd async-tcp-tests
   ```

2. **Initialise and update submodules:**

   ```sh
   git submodule update --init --recursive
   ```

3. **Build and monitor with PlatformIO:**

   ```sh
   pio run
   pio device monitor
   ```

   For instructions on using async-tcp in a new project, see the [async-tcp README](lib/async-tcp/README.md).

## Project Structure

```plaintext
├── lib
│   └── async-tcp    # async-tcp library as a submodule
├── scripts          # Debug scripts
├── src              # Application source code
└── docs             # Documentation
```

## Notes
- The repo was formerly named `pico-sdk-tests`.
- The original pico-sdk bugfixes are now upstream and no longer require local patching.
- The async-tcp library is under active development and testing here.

For more details, see The [Application Workflow](docs/workflow.md).
