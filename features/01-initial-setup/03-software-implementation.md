# Software Implementation Guide

## Table of Contents

1. [Development Environment Setup](#development-environment-setup)
2. [PlatformIO Configuration](#platformio-configuration)
3. [Test Program Implementation](#test-program-implementation)
4. [Code Structure and Documentation](#code-structure-and-documentation)
5. [Expected Serial Output](#expected-serial-output)
6. [Troubleshooting Guide](#troubleshooting-guide)

---

## Development Environment Setup

### Prerequisites

1. **PlatformIO IDE** or **PlatformIO Core (CLI)**
   - Install via VS Code extension or standalone
   - Documentation: https://platformio.org/install

2. **Teensy Platform Support**
   - PlatformIO will auto-install on first build
   - Requires Teensyduino: https://www.pjrc.com/teensy/teensyduino.html

3. **USB Drivers**
   - Windows: Install Teensy drivers from PJRC
   - macOS/Linux: Usually works out of box

### Verify Installation

```bash
# Check PlatformIO version
pio --version

# List available boards (should include teensy41)
pio boards teensy
```

---

## PlatformIO Configuration

### Updated `platformio.ini`

Replace the existing `platformio.ini` with this configuration:

```ini
; PlatformIO Configuration for Mechanical MIDI Piano
; Target: Teensy 4.1 with Adafruit I2C Solenoid Driver

[env:teensy41]
platform = teensy
board = teensy41
framework = arduino

; Serial monitor baud rate
monitor_speed = 115200

; Build flags
build_flags =
    -D ARDUINO_TEENSY41
    -D USB_SERIAL
    -Wno-unused-variable

; Library dependencies
lib_deps =
    adafruit/Adafruit MCP23017 Arduino Library@^2.3.0
    adafruit/Adafruit BusIO@^1.14.0

; Upload settings
upload_protocol = teensy-cli

; Debug build (optional - uncomment for debugging)
; build_type = debug
```

### Library Dependencies Explained

| Library | Version | Purpose |
|---------|---------|---------|
| Adafruit MCP23017 Arduino Library | ^2.3.0 | High-level MCP23017 control |
| Adafruit BusIO | ^1.14.0 | I2C abstraction (auto-dependency) |

### Install Libraries

```bash
# PlatformIO will auto-install on first build, or manually:
pio lib install "adafruit/Adafruit MCP23017 Arduino Library@^2.3.0"
```

---

## Test Program Implementation

### Complete Test Program (`src/main.cpp`)

```cpp
/**
 * @file main.cpp
 * @brief Solenoid Driver Test Program for Mechanical MIDI Piano
 *
 * This program tests the Adafruit I2C to 8 Channel Solenoid Driver
 * with a Teensy 4.1 microcontroller.
 *
 * Hardware:
 *   - Teensy 4.1
 *   - Adafruit I2C Solenoid Driver (Product ID 6318)
 *   - I2C: SDA=Pin 18, SCL=Pin 19 (Wire)
 *   - Default I2C Address: 0x20
 *
 * Test Features:
 *   1. I2C bus scanner
 *   2. MCP23017 initialization and communication verification
 *   3. Sequential channel cycling
 *   4. All-channels simultaneous test
 *   5. Interactive serial commands
 *
 * Serial Commands:
 *   'r' - Re-run all tests
 *   'a' - Activate all channels simultaneously
 *   's' - Run I2C scanner only
 *   '0'-'7' - Toggle individual channel
 *   'h' - Show help menu
 *
 * @author Mechanical MIDI Piano Project
 * @date 2025-01-18
 * @version 1.0.0
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>

// =============================================================================
// CONFIGURATION CONSTANTS
// =============================================================================

/**
 * @defgroup I2CConfig I2C Configuration
 * @{
 */

/** I2C bus speed in Hz (400kHz recommended for MCP23017) */
constexpr uint32_t I2C_CLOCK_SPEED = 400000;

/** Default I2C address for MCP23017 (A0=A1=A2=0) */
constexpr uint8_t MCP23017_DEFAULT_ADDRESS = 0x20;

/** I2C communication timeout in milliseconds */
constexpr uint32_t I2C_TIMEOUT_MS = 100;

/** @} */

/**
 * @defgroup SolenoidConfig Solenoid Control Configuration
 * @{
 */

/** Number of solenoid channels on the driver board */
constexpr uint8_t NUM_CHANNELS = 8;

/**
 * Maximum solenoid on-time in milliseconds
 * Prevents coil overheating - adjust based on solenoid specs
 */
constexpr uint32_t MAX_ON_TIME_MS = 5000;

/**
 * Minimum off-time between activations in milliseconds
 * Allows coil cooling between activations
 */
constexpr uint32_t MIN_OFF_TIME_MS = 50;

/**
 * Test activation duration in milliseconds
 * Short duration for safe testing
 */
constexpr uint32_t TEST_ACTIVATION_MS = 100;

/**
 * Delay between sequential channel tests in milliseconds
 */
constexpr uint32_t TEST_DELAY_MS = 200;

/** @} */

/**
 * @defgroup Pins Pin Definitions
 * @{
 */

/** Built-in LED pin on Teensy 4.1 */
constexpr uint8_t LED_PIN = LED_BUILTIN;

/** @} */

// =============================================================================
// MCP23017 REGISTER DEFINITIONS
// =============================================================================

/**
 * @defgroup MCPRegisters MCP23017 Register Addresses (BANK=0 mode)
 * @{
 */

/** Port A direction register (1=input, 0=output) */
constexpr uint8_t REG_IODIRA = 0x00;

/** Port B direction register */
constexpr uint8_t REG_IODIRB = 0x01;

/** Port A GPIO register */
constexpr uint8_t REG_GPIOA = 0x12;

/** Port B GPIO register */
constexpr uint8_t REG_GPIOB = 0x13;

/** Port A output latch register */
constexpr uint8_t REG_OLATA = 0x14;

/** Port B output latch register */
constexpr uint8_t REG_OLATB = 0x15;

/** @} */

// =============================================================================
// GLOBAL OBJECTS
// =============================================================================

/** MCP23017 GPIO expander instance */
Adafruit_MCP23X17 mcp;

/** Flag indicating if MCP23017 was initialized successfully */
bool mcpInitialized = false;

/** Current channel states (bitmask) */
uint8_t channelStates = 0x00;

/** Timestamp when each channel was last turned on (for timeout protection) */
uint32_t channelOnTime[NUM_CHANNELS] = {0};

/** Timestamp when each channel was last turned off (for cooldown enforcement) */
uint32_t channelOffTime[NUM_CHANNELS] = {0};

// =============================================================================
// FUNCTION PROTOTYPES
// =============================================================================

// Initialization
void initSerial();
void initI2C();
bool initMCP23017();

// I2C Utilities
void scanI2CBus();
uint8_t countI2CDevices();

// Solenoid Control
bool setChannel(uint8_t channel, bool state);
bool setAllChannels(uint8_t states);
bool activateChannel(uint8_t channel, uint32_t duration);
void deactivateAllChannels();
bool isChannelSafe(uint8_t channel, bool turningOn);

// Test Functions
void runAllTests();
void testSequentialChannels();
void testAllChannelsSimultaneous();
void testCommunication();

// Utility Functions
void printSeparator();
void printHelp();
void handleSerialInput();
void blinkLED(uint8_t count, uint32_t duration);

// =============================================================================
// SETUP
// =============================================================================

/**
 * @brief Arduino setup function - runs once at startup
 *
 * Initializes serial communication, I2C bus, and MCP23017.
 * Runs initial diagnostic tests.
 */
void setup() {
    // Initialize LED pin for status indication
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);  // LED on during setup

    // Initialize serial communication
    initSerial();

    // Print startup banner
    printSeparator();
    Serial.println(F("MECHANICAL MIDI PIANO - SOLENOID DRIVER TEST"));
    Serial.println(F("Teensy 4.1 + Adafruit I2C Solenoid Driver"));
    printSeparator();
    Serial.println();

    // Initialize I2C bus
    initI2C();

    // Scan for I2C devices
    scanI2CBus();

    // Initialize MCP23017
    if (initMCP23017()) {
        Serial.println(F("[OK] MCP23017 initialized successfully"));
        mcpInitialized = true;

        // Run initial tests
        Serial.println();
        Serial.println(F("Running initial tests..."));
        runAllTests();
    } else {
        Serial.println(F("[ERROR] Failed to initialize MCP23017!"));
        Serial.println(F("Check wiring and I2C address."));
        mcpInitialized = false;
    }

    // Print help menu
    Serial.println();
    printHelp();

    digitalWrite(LED_PIN, LOW);  // LED off after setup
}

// =============================================================================
// MAIN LOOP
// =============================================================================

/**
 * @brief Arduino main loop - runs continuously
 *
 * Handles serial commands and monitors solenoid safety timeouts.
 */
void loop() {
    // Handle incoming serial commands
    handleSerialInput();

    // Safety: Check for solenoid timeout
    if (mcpInitialized) {
        uint32_t now = millis();
        for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
            // Check if channel is on and has exceeded max on-time
            if ((channelStates & (1 << i)) && (channelOnTime[i] > 0)) {
                if ((now - channelOnTime[i]) >= MAX_ON_TIME_MS) {
                    Serial.print(F("[SAFETY] Channel "));
                    Serial.print(i);
                    Serial.println(F(" auto-shutoff (max on-time exceeded)"));
                    setChannel(i, false);
                }
            }
        }
    }

    // Small delay to prevent tight loop
    delay(1);
}

// =============================================================================
// INITIALIZATION FUNCTIONS
// =============================================================================

/**
 * @brief Initialize serial communication
 *
 * Waits for serial port to be ready (up to 3 seconds).
 * Uses 115200 baud rate as configured in platformio.ini.
 */
void initSerial() {
    Serial.begin(115200);

    // Wait for serial port to connect (with timeout for standalone operation)
    uint32_t startTime = millis();
    while (!Serial && (millis() - startTime < 3000)) {
        delay(10);
    }

    delay(100);  // Additional stabilization delay
}

/**
 * @brief Initialize I2C bus (Wire)
 *
 * Configures the primary I2C bus with the specified clock speed.
 * Uses Teensy 4.1 default I2C pins: SDA=18, SCL=19.
 */
void initI2C() {
    Serial.println(F("Initializing I2C bus..."));
    Serial.print(F("  SDA Pin: 18, SCL Pin: 19"));
    Serial.print(F(", Speed: "));
    Serial.print(I2C_CLOCK_SPEED / 1000);
    Serial.println(F(" kHz"));

    Wire.begin();
    Wire.setClock(I2C_CLOCK_SPEED);

    delay(100);  // Allow bus to stabilize

    Serial.println(F("[OK] I2C bus initialized"));
}

/**
 * @brief Initialize MCP23017 GPIO expander
 *
 * @return true if initialization successful, false otherwise
 *
 * Configures Port A as outputs for solenoid control.
 * Port B is left as inputs (available for future use).
 */
bool initMCP23017() {
    Serial.println();
    Serial.print(F("Initializing MCP23017 at address 0x"));
    Serial.print(MCP23017_DEFAULT_ADDRESS, HEX);
    Serial.println(F("..."));

    // Initialize with Adafruit library
    if (!mcp.begin_I2C(MCP23017_DEFAULT_ADDRESS, &Wire)) {
        Serial.println(F("[ERROR] MCP23017 begin_I2C() failed"));
        return false;
    }

    Serial.println(F("  Configuring Port A as outputs (solenoid channels)..."));

    // Configure all Port A pins as outputs (for solenoids)
    for (uint8_t i = 0; i < 8; i++) {
        mcp.pinMode(i, OUTPUT);
    }

    // Turn all channels off initially
    mcp.writeGPIOA(0x00);
    channelStates = 0x00;

    Serial.println(F("  Port A configured, all channels OFF"));

    // Verify configuration by reading back
    Serial.println(F("  Verifying configuration..."));

    // Test communication
    testCommunication();

    return true;
}

// =============================================================================
// I2C UTILITY FUNCTIONS
// =============================================================================

/**
 * @brief Scan the I2C bus and report all found devices
 *
 * Scans addresses 0x08-0x77 and prints each found device.
 */
void scanI2CBus() {
    Serial.println();
    Serial.println(F("Scanning I2C bus..."));

    uint8_t deviceCount = 0;

    for (uint8_t address = 8; address < 120; address++) {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();

        if (error == 0) {
            Serial.print(F("  [FOUND] Device at address 0x"));
            if (address < 16) Serial.print(F("0"));
            Serial.print(address, HEX);

            // Identify known devices
            if (address >= 0x20 && address <= 0x27) {
                Serial.print(F(" (MCP23017 - Solenoid Driver)"));
            }

            Serial.println();
            deviceCount++;
        }
    }

    Serial.println();
    Serial.print(F("Scan complete. "));
    Serial.print(deviceCount);
    Serial.println(F(" device(s) found."));
}

/**
 * @brief Count the number of I2C devices on the bus
 *
 * @return Number of devices found
 */
uint8_t countI2CDevices() {
    uint8_t count = 0;

    for (uint8_t address = 8; address < 120; address++) {
        Wire.beginTransmission(address);
        if (Wire.endTransmission() == 0) {
            count++;
        }
    }

    return count;
}

// =============================================================================
// SOLENOID CONTROL FUNCTIONS
// =============================================================================

/**
 * @brief Set the state of a single solenoid channel
 *
 * @param channel Channel number (0-7)
 * @param state true=ON, false=OFF
 * @return true if operation successful, false otherwise
 *
 * Includes safety checks for maximum on-time and minimum off-time.
 */
bool setChannel(uint8_t channel, bool state) {
    if (channel >= NUM_CHANNELS) {
        Serial.print(F("[ERROR] Invalid channel: "));
        Serial.println(channel);
        return false;
    }

    if (!mcpInitialized) {
        Serial.println(F("[ERROR] MCP23017 not initialized"));
        return false;
    }

    // Safety check
    if (!isChannelSafe(channel, state)) {
        return false;
    }

    // Update state tracking
    uint32_t now = millis();
    if (state) {
        channelStates |= (1 << channel);
        channelOnTime[channel] = now;
    } else {
        channelStates &= ~(1 << channel);
        channelOffTime[channel] = now;
        channelOnTime[channel] = 0;
    }

    // Write to MCP23017
    mcp.digitalWrite(channel, state ? HIGH : LOW);

    return true;
}

/**
 * @brief Set all channel states at once using a bitmask
 *
 * @param states Bitmask of channel states (bit 0 = channel 0, etc.)
 * @return true if operation successful, false otherwise
 */
bool setAllChannels(uint8_t states) {
    if (!mcpInitialized) {
        Serial.println(F("[ERROR] MCP23017 not initialized"));
        return false;
    }

    // Update state tracking
    uint32_t now = millis();
    for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
        bool newState = (states >> i) & 0x01;
        bool oldState = (channelStates >> i) & 0x01;

        if (newState && !oldState) {
            // Channel turning on
            channelOnTime[i] = now;
        } else if (!newState && oldState) {
            // Channel turning off
            channelOffTime[i] = now;
            channelOnTime[i] = 0;
        }
    }

    channelStates = states;

    // Write all states at once (more efficient)
    mcp.writeGPIOA(states);

    return true;
}

/**
 * @brief Activate a channel for a specified duration, then deactivate
 *
 * @param channel Channel number (0-7)
 * @param duration Activation duration in milliseconds
 * @return true if operation successful, false otherwise
 *
 * This is a blocking function - it waits for the duration to complete.
 */
bool activateChannel(uint8_t channel, uint32_t duration) {
    // Enforce maximum on-time
    if (duration > MAX_ON_TIME_MS) {
        Serial.print(F("[WARNING] Duration clamped to max: "));
        Serial.println(MAX_ON_TIME_MS);
        duration = MAX_ON_TIME_MS;
    }

    if (!setChannel(channel, true)) {
        return false;
    }

    delay(duration);

    setChannel(channel, false);

    return true;
}

/**
 * @brief Immediately deactivate all solenoid channels
 *
 * Emergency stop function - bypasses safety checks.
 */
void deactivateAllChannels() {
    if (mcpInitialized) {
        mcp.writeGPIOA(0x00);
    }

    // Reset all tracking
    channelStates = 0x00;
    for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
        channelOnTime[i] = 0;
        channelOffTime[i] = millis();
    }

    Serial.println(F("[OK] All channels deactivated"));
}

/**
 * @brief Check if a channel state change is safe
 *
 * @param channel Channel number (0-7)
 * @param turningOn true if turning on, false if turning off
 * @return true if operation is safe, false otherwise
 *
 * Checks:
 *   - Minimum off-time has elapsed (when turning on)
 */
bool isChannelSafe(uint8_t channel, bool turningOn) {
    if (!turningOn) {
        return true;  // Always safe to turn off
    }

    // Check minimum off-time
    uint32_t now = millis();
    if (channelOffTime[channel] > 0) {
        uint32_t offDuration = now - channelOffTime[channel];
        if (offDuration < MIN_OFF_TIME_MS) {
            Serial.print(F("[SAFETY] Channel "));
            Serial.print(channel);
            Serial.print(F(" cooldown: wait "));
            Serial.print(MIN_OFF_TIME_MS - offDuration);
            Serial.println(F("ms"));
            return false;
        }
    }

    return true;
}

// =============================================================================
// TEST FUNCTIONS
// =============================================================================

/**
 * @brief Run all diagnostic tests
 */
void runAllTests() {
    if (!mcpInitialized) {
        Serial.println(F("[ERROR] Cannot run tests - MCP23017 not initialized"));
        return;
    }

    printSeparator();
    Serial.println(F("RUNNING ALL TESTS"));
    printSeparator();

    // Test 1: Communication verification
    Serial.println(F("\n--- Test 1: Communication Verification ---"));
    testCommunication();

    // Test 2: Sequential channel test
    Serial.println(F("\n--- Test 2: Sequential Channel Test ---"));
    testSequentialChannels();

    // Test 3: Simultaneous channel test
    Serial.println(F("\n--- Test 3: All Channels Simultaneous ---"));
    testAllChannelsSimultaneous();

    printSeparator();
    Serial.println(F("ALL TESTS COMPLETE"));
    printSeparator();
}

/**
 * @brief Test communication by writing and reading back values
 */
void testCommunication() {
    Serial.println(F("Testing register read/write..."));

    // Read current GPIOA state
    uint8_t gpioState = mcp.readGPIOA();
    Serial.print(F("  GPIOA read: 0x"));
    Serial.println(gpioState, HEX);

    // Write test pattern and read back
    uint8_t testPattern = 0xAA;
    mcp.writeGPIOA(testPattern);
    delay(10);
    uint8_t readBack = mcp.readGPIOA();

    if (readBack == testPattern) {
        Serial.print(F("  [OK] Write/read verified (0x"));
        Serial.print(testPattern, HEX);
        Serial.println(F(")"));
    } else {
        Serial.print(F("  [ERROR] Write/read mismatch! Wrote 0x"));
        Serial.print(testPattern, HEX);
        Serial.print(F(", read 0x"));
        Serial.println(readBack, HEX);
    }

    // Restore all off
    mcp.writeGPIOA(0x00);
    channelStates = 0x00;
}

/**
 * @brief Test each channel sequentially
 *
 * Activates each channel one at a time for TEST_ACTIVATION_MS,
 * then waits TEST_DELAY_MS before the next channel.
 */
void testSequentialChannels() {
    Serial.println(F("Testing channels sequentially..."));
    Serial.print(F("  Activation time: "));
    Serial.print(TEST_ACTIVATION_MS);
    Serial.print(F("ms, Delay: "));
    Serial.print(TEST_DELAY_MS);
    Serial.println(F("ms"));
    Serial.println();

    for (uint8_t channel = 0; channel < NUM_CHANNELS; channel++) {
        Serial.print(F("  Channel "));
        Serial.print(channel);
        Serial.print(F(": ON..."));

        digitalWrite(LED_PIN, HIGH);

        if (activateChannel(channel, TEST_ACTIVATION_MS)) {
            Serial.println(F(" OFF [OK]"));
        } else {
            Serial.println(F(" [FAILED]"));
        }

        digitalWrite(LED_PIN, LOW);

        // Wait between channels (accounts for min off time)
        delay(TEST_DELAY_MS);
    }

    Serial.println(F("  Sequential test complete."));
}

/**
 * @brief Test all channels activated simultaneously
 *
 * Activates all 8 channels at once for TEST_ACTIVATION_MS.
 */
void testAllChannelsSimultaneous() {
    Serial.println(F("Activating all channels simultaneously..."));
    Serial.print(F("  Duration: "));
    Serial.print(TEST_ACTIVATION_MS);
    Serial.println(F("ms"));

    digitalWrite(LED_PIN, HIGH);

    // All channels on
    setAllChannels(0xFF);
    Serial.println(F("  All channels ON"));

    delay(TEST_ACTIVATION_MS);

    // All channels off
    setAllChannels(0x00);
    Serial.println(F("  All channels OFF"));

    digitalWrite(LED_PIN, LOW);

    Serial.println(F("  Simultaneous test complete."));
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * @brief Print a visual separator line
 */
void printSeparator() {
    Serial.println(F("============================================================"));
}

/**
 * @brief Print the help menu
 */
void printHelp() {
    Serial.println(F("SERIAL COMMANDS:"));
    Serial.println(F("  'r' - Re-run all tests"));
    Serial.println(F("  'a' - Activate all channels for 100ms"));
    Serial.println(F("  's' - Run I2C scanner"));
    Serial.println(F("  '0'-'7' - Toggle individual channel"));
    Serial.println(F("  'x' - Emergency stop (all off)"));
    Serial.println(F("  'h' - Show this help menu"));
    Serial.println();
    Serial.println(F("Waiting for commands..."));
}

/**
 * @brief Handle incoming serial commands
 */
void handleSerialInput() {
    if (Serial.available() > 0) {
        char cmd = Serial.read();

        // Flush any remaining characters
        while (Serial.available()) {
            Serial.read();
        }

        Serial.println();

        switch (cmd) {
            case 'r':
            case 'R':
                Serial.println(F("Re-running all tests..."));
                runAllTests();
                break;

            case 'a':
            case 'A':
                Serial.println(F("Activating all channels..."));
                testAllChannelsSimultaneous();
                break;

            case 's':
            case 'S':
                scanI2CBus();
                break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7': {
                uint8_t channel = cmd - '0';
                bool currentState = (channelStates >> channel) & 0x01;
                bool newState = !currentState;

                Serial.print(F("Toggling channel "));
                Serial.print(channel);
                Serial.print(F(" -> "));
                Serial.println(newState ? F("ON") : F("OFF"));

                setChannel(channel, newState);
                break;
            }

            case 'x':
            case 'X':
                Serial.println(F("EMERGENCY STOP"));
                deactivateAllChannels();
                break;

            case 'h':
            case 'H':
            case '?':
                printHelp();
                break;

            case '\n':
            case '\r':
                // Ignore newlines
                break;

            default:
                Serial.print(F("Unknown command: '"));
                Serial.print(cmd);
                Serial.println(F("' (press 'h' for help)"));
                break;
        }
    }
}

/**
 * @brief Blink the built-in LED
 *
 * @param count Number of blinks
 * @param duration Duration of each blink in milliseconds
 */
void blinkLED(uint8_t count, uint32_t duration) {
    for (uint8_t i = 0; i < count; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(duration);
        digitalWrite(LED_PIN, LOW);
        delay(duration);
    }
}
```

---

## Code Structure and Documentation

### File Organization

```
mechanical-midi-piano/
├── platformio.ini          # Build configuration
├── src/
│   └── main.cpp            # Main test program
├── lib/                    # Custom libraries (future)
│   └── SolenoidDriver/     # (Phase 4+)
├── include/                # Header files (future)
└── test/                   # Unit tests (future)
```

### Code Sections Explained

| Section | Purpose |
|---------|---------|
| Configuration Constants | All timing, address, and safety parameters in one place |
| MCP23017 Register Definitions | Hardware register addresses for reference |
| Global Objects | MCP23017 instance and state tracking variables |
| Initialization Functions | Setup serial, I2C, and MCP23017 |
| I2C Utility Functions | Bus scanning and device detection |
| Solenoid Control Functions | Channel activation with safety checks |
| Test Functions | Diagnostic test routines |
| Utility Functions | Helpers for UI and status |

### Key Design Decisions

1. **Safety-First Design**
   - Maximum on-time limit (5 seconds) prevents solenoid overheating
   - Minimum off-time (50ms) ensures cooling between activations
   - Emergency stop (`'x'` command) immediately deactivates all channels

2. **Non-Blocking Architecture (Main Loop)**
   - Main loop monitors safety timeouts without blocking
   - Only test functions use blocking delays
   - Production code should use non-blocking timers

3. **State Tracking**
   - `channelStates` tracks current on/off state for quick access
   - `channelOnTime[]` tracks when each channel was turned on
   - `channelOffTime[]` tracks when each channel was turned off

4. **Error Handling**
   - All functions return success/failure status
   - Error messages include context (which channel, what failed)
   - Graceful degradation if MCP23017 not detected

### Configuration Constants Reference

| Constant | Value | Unit | Purpose |
|----------|-------|------|---------|
| `I2C_CLOCK_SPEED` | 400000 | Hz | I2C bus speed |
| `MCP23017_DEFAULT_ADDRESS` | 0x20 | - | I2C address |
| `I2C_TIMEOUT_MS` | 100 | ms | Communication timeout |
| `NUM_CHANNELS` | 8 | - | Channels per board |
| `MAX_ON_TIME_MS` | 5000 | ms | Safety auto-shutoff |
| `MIN_OFF_TIME_MS` | 50 | ms | Minimum cooldown |
| `TEST_ACTIVATION_MS` | 100 | ms | Test pulse duration |
| `TEST_DELAY_MS` | 200 | ms | Delay between tests |

---

## Expected Serial Output

### Successful Startup

```
============================================================
MECHANICAL MIDI PIANO - SOLENOID DRIVER TEST
Teensy 4.1 + Adafruit I2C Solenoid Driver
============================================================

Initializing I2C bus...
  SDA Pin: 18, SCL Pin: 19, Speed: 400 kHz
[OK] I2C bus initialized

Scanning I2C bus...
  [FOUND] Device at address 0x20 (MCP23017 - Solenoid Driver)

Scan complete. 1 device(s) found.

Initializing MCP23017 at address 0x20...
  Configuring Port A as outputs (solenoid channels)...
  Port A configured, all channels OFF
  Verifying configuration...
Testing register read/write...
  GPIOA read: 0x00
  [OK] Write/read verified (0xAA)
[OK] MCP23017 initialized successfully

Running initial tests...
============================================================
RUNNING ALL TESTS
============================================================

--- Test 1: Communication Verification ---
Testing register read/write...
  GPIOA read: 0x00
  [OK] Write/read verified (0xAA)

--- Test 2: Sequential Channel Test ---
Testing channels sequentially...
  Activation time: 100ms, Delay: 200ms

  Channel 0: ON... OFF [OK]
  Channel 1: ON... OFF [OK]
  Channel 2: ON... OFF [OK]
  Channel 3: ON... OFF [OK]
  Channel 4: ON... OFF [OK]
  Channel 5: ON... OFF [OK]
  Channel 6: ON... OFF [OK]
  Channel 7: ON... OFF [OK]
  Sequential test complete.

--- Test 3: All Channels Simultaneous ---
Activating all channels simultaneously...
  Duration: 100ms
  All channels ON
  All channels OFF
  Simultaneous test complete.
============================================================
ALL TESTS COMPLETE
============================================================

SERIAL COMMANDS:
  'r' - Re-run all tests
  'a' - Activate all channels for 100ms
  's' - Run I2C scanner
  '0'-'7' - Toggle individual channel
  'x' - Emergency stop (all off)
  'h' - Show this help menu

Waiting for commands...
```

### No Device Found Output

```
============================================================
MECHANICAL MIDI PIANO - SOLENOID DRIVER TEST
Teensy 4.1 + Adafruit I2C Solenoid Driver
============================================================

Initializing I2C bus...
  SDA Pin: 18, SCL Pin: 19, Speed: 400 kHz
[OK] I2C bus initialized

Scanning I2C bus...

Scan complete. 0 device(s) found.

Initializing MCP23017 at address 0x20...
[ERROR] MCP23017 begin_I2C() failed
[ERROR] Failed to initialize MCP23017!
Check wiring and I2C address.

SERIAL COMMANDS:
  'r' - Re-run all tests
  ...
```

### Safety Timeout Output

```
[SAFETY] Channel 3 auto-shutoff (max on-time exceeded)
```

### Cooldown Warning Output

```
[SAFETY] Channel 0 cooldown: wait 35ms
```

---

## Troubleshooting Guide

### Issue: "MCP23017 begin_I2C() failed"

**Symptoms**: No device detected, initialization fails

**Diagnostic Steps**:

1. **Check power**:
   ```
   Measure voltage at driver board Vcc pin
   Expected: 3.2V - 3.4V
   ```

2. **Check I2C connections**:
   ```
   Verify SDA (Pin 18) -> Driver SDA
   Verify SCL (Pin 19) -> Driver SCL
   ```

3. **Run I2C scanner** (press 's'):
   ```
   If no devices found -> wiring problem
   If wrong address -> check A0/A1/A2 jumpers
   ```

4. **Check for shorts**:
   ```
   Verify no short between SDA and GND
   Verify no short between SCL and GND
   ```

**Solutions**:

| Finding | Solution |
|---------|----------|
| No voltage at Vcc | Check 3.3V connection from Teensy |
| Voltage at 5V | **STOP** - will damage Teensy! Use 3.3V |
| SDA/SCL swapped | Swap the two wires |
| Different address detected | Update `MCP23017_DEFAULT_ADDRESS` in code |
| Still fails | Try different driver board |

### Issue: Solenoid Doesn't Click

**Symptoms**: Code runs, channel activates (LED blinks), but solenoid doesn't move

**Diagnostic Steps**:

1. **Check DC power supply**:
   ```
   Measure voltage at V+ terminal
   Expected: 12V or 24V (match solenoid rating)
   ```

2. **Check solenoid connection**:
   ```
   Verify solenoid connected to correct channel terminal
   Verify solenoid ground connected to driver GND terminal
   ```

3. **Test solenoid directly**:
   ```
   Disconnect from driver
   Apply DC voltage directly to solenoid
   Should click when powered
   ```

4. **Check channel output**:
   ```
   Measure voltage at channel terminal when activated
   Expected: ~V+ voltage when on, ~0V when off
   ```

**Solutions**:

| Finding | Solution |
|---------|----------|
| No V+ voltage | Connect DC power supply |
| DC supply voltage wrong | Use correct voltage supply |
| Solenoid doesn't work directly | Replace solenoid |
| No voltage at channel output | Check code, try different channel |

### Issue: I2C Errors During Operation

**Symptoms**: Communication works initially, then fails intermittently

**Diagnostic Steps**:

1. **Check cable length**: Keep I2C wires under 30cm
2. **Check for noise**: Solenoid switching can cause EMI
3. **Check pull-ups**: May need additional pull-up resistors

**Solutions**:

| Finding | Solution |
|---------|----------|
| Long cables | Shorten cables or add I2C buffer |
| Errors correlate with solenoid activation | Add 0.1uF capacitors near driver, separate solenoid power |
| Intermittent errors | Add 4.7k pull-ups to SDA/SCL |
| Consistent errors | Reduce I2C speed to 100kHz |

### Issue: Solenoid Gets Hot

**Symptoms**: Solenoid warm/hot to touch after operation

**Diagnostic Steps**:

1. **Check on-time**: Are solenoids staying on too long?
2. **Check duty cycle**: Too many activations per second?
3. **Check voltage**: Is supply voltage too high?

**Solutions**:

| Finding | Solution |
|---------|----------|
| Solenoid always on | Check code for stuck state, verify safety timeout works |
| High duty cycle | Reduce activation frequency |
| Voltage too high | Use lower voltage supply |
| Normal use, still hot | Solenoid may not be rated for continuous use |

### Issue: Multiple Devices Conflict

**Symptoms**: Multiple boards respond erratically

**Diagnostic Steps**:

1. **Run I2C scanner**: Check all detected addresses
2. **Verify unique addresses**: Each board must have different A0/A1/A2 settings

**Solutions**:

| Finding | Solution |
|---------|----------|
| Duplicate addresses | Set different address jumpers on each board |
| Bus overloaded | Split boards across Wire and Wire1 buses |

---

## Build and Upload Instructions

### Build Project

```bash
# Navigate to project directory
cd /path/to/mechanical-midi-piano

# Build project
pio run

# Expected output:
# Processing teensy41 (platform: teensy; board: teensy41; framework: arduino)
# ...
# Building .pio/build/teensy41/firmware.hex
# ===== [SUCCESS] =====
```

### Upload to Teensy

```bash
# Upload firmware
pio run --target upload

# Or use shortcut
pio run -t upload
```

### Open Serial Monitor

```bash
# Open serial monitor at 115200 baud
pio device monitor

# Or specify baud rate explicitly
pio device monitor --baud 115200
```

### Full Build-Upload-Monitor Cycle

```bash
# Build, upload, and open monitor in one command
pio run --target upload && pio device monitor
```
