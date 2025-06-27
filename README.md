# Development Setup Guide

This project uses arduino-pico core for debugging RP2040-based Arduino Pico programs with local modifications.

## Prerequisites

- [Arduino Pico](https://github.com/earlephilhower/arduino-pico) core
- [PlatformIO](https://platformio.org/)
- CMSIS-DAP compatible debug probe

## Setup Process

1. Clone those repositories:

```bash
git clone https://github.com/schkovich/arduino-pico.git 
cd arduino-pico
git checkout execute_sync_2433
git submodule update --init
cd ..
git clone git@github.com:schkovich/pico-sdk-tests.git
cd pico-sdk-tests
```

## Project Structure

```plaintext

├── arduino-pico # Forked arduino-pico core
├── pico-sdk-tests
│   ├── lib
│   ├── .pio # PlatformIO build directory
│   ├── scripts # Debug scripts
│   └── src
```

## Project configuration

The arduino-pico core is configured
in [platformio.ini](https://docs.platformio.org/en/latest/projectconf/index.html#platformio-ini-project-configuration-file)
via [platform_package](https://docs.platformio.org/en/latest/projectconf/sections/env/options/platform/platform_packages.html#platform-packages)
option, pointing to
the [local folder](https://docs.platformio.org/en/latest/core/userguide/pkg/cmd_install.html#local-folder) and “Symbolic
Link” feature:

```ini
[env:debug]
board = nanorp2040connect
framework = arduino
platform = https://github.com/maxgerhardt/platform-raspberrypi.git
board_build.core = earlephilhower
platform_packages =
    framework-arduinopico@symlink://../arduino-pico
```

## Building Core Components

The pico-sdk build process:

1. Compiles core SDK libraries
2. Generates boot stage 2 code for different flash chips
3. Produces required build artifacts for both RP2040 and RP2350

For detailed build steps,
see [make-libpico.sh](https://github.com/schkovich/arduino-pico/blob/execute-sync%407c51742/tools/libpico/make-libpico.sh)
in the arduino-pico core.

### Development Workflow

- Pico SDK and async_context are pre-built statically linked to the project
- The project is built using [PlatformIO](https://docs.platformio.org/en/latest/core/index.html)
- Debug using CMSIS-DAP probe