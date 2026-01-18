# SolenoidDriver Library API Design

## Table of Contents

1. [Overview](#overview)
2. [Class Architecture](#class-architecture)
3. [Complete Class Interface](#complete-class-interface)
4. [Method Documentation](#method-documentation)
5. [Usage Examples](#usage-examples)
6. [Error Handling](#error-handling)
7. [Configuration Options](#configuration-options)
8. [Multi-Board Management](#multi-board-management)
9. [Performance Considerations](#performance-considerations)
10. [Future MIDI Integration](#future-midi-integration)

---

## Overview

The `SolenoidDriver` library provides a high-level, safety-aware interface for controlling solenoids through the Adafruit I2C to 8 Channel Solenoid Driver boards. The library abstracts the MCP23017 GPIO expander details and implements critical safety features to prevent solenoid damage.

### Design Goals

1. **Safety First**: Built-in protection against solenoid overheating
2. **Simple API**: Easy to use for common operations
3. **Scalable**: Support for multiple boards (88 piano keys)
4. **Non-Blocking**: Suitable for real-time MIDI applications
5. **Debuggable**: Comprehensive error reporting and diagnostics

### Library Files

```
lib/
└── SolenoidDriver/
    ├── SolenoidDriver.h        # Main header file
    ├── SolenoidDriver.cpp      # Implementation
    ├── SolenoidChannel.h       # Individual channel class
    ├── SolenoidChannel.cpp     # Channel implementation
    ├── SolenoidConfig.h        # Configuration constants
    └── library.json            # PlatformIO library manifest
```

---

## Class Architecture

### UML Class Diagram

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              <<enumeration>>                                 │
│                              SolenoidError                                   │
├─────────────────────────────────────────────────────────────────────────────┤
│ SOLENOID_OK = 0                                                             │
│ SOLENOID_ERR_NOT_INITIALIZED = 1                                            │
│ SOLENOID_ERR_INVALID_CHANNEL = 2                                            │
│ SOLENOID_ERR_INVALID_BOARD = 3                                              │
│ SOLENOID_ERR_I2C_COMM = 4                                                   │
│ SOLENOID_ERR_SAFETY_TIMEOUT = 5                                             │
│ SOLENOID_ERR_SAFETY_COOLDOWN = 6                                            │
│ SOLENOID_ERR_DUTY_CYCLE = 7                                                 │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│                              <<struct>>                                      │
│                           SolenoidConfig                                     │
├─────────────────────────────────────────────────────────────────────────────┤
│ + maxOnTimeMs: uint32_t = 5000                                              │
│ + minOffTimeMs: uint32_t = 50                                               │
│ + maxDutyCycle: float = 0.5                                                 │
│ + i2cTimeoutMs: uint32_t = 100                                              │
│ + i2cClockHz: uint32_t = 400000                                             │
└─────────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────────┐
│                           SolenoidChannel                                    │
├─────────────────────────────────────────────────────────────────────────────┤
│ - _boardIndex: uint8_t                                                       │
│ - _channelIndex: uint8_t                                                     │
│ - _globalIndex: uint8_t                                                      │
│ - _isOn: bool                                                               │
│ - _lastOnTime: uint32_t                                                     │
│ - _lastOffTime: uint32_t                                                    │
│ - _totalOnTime: uint32_t                                                    │
├─────────────────────────────────────────────────────────────────────────────┤
│ + SolenoidChannel(boardIdx, channelIdx, globalIdx)                          │
│ + isOn(): bool                                                              │
│ + onDuration(): uint32_t                                                    │
│ + timeSinceOff(): uint32_t                                                  │
│ + boardIndex(): uint8_t                                                     │
│ + channelIndex(): uint8_t                                                   │
│ + globalIndex(): uint8_t                                                    │
│ + updateState(isOn: bool): void                                             │
└─────────────────────────────────────────────────────────────────────────────┘
                                      △
                                      │
                                      │ uses
                                      │
┌─────────────────────────────────────────────────────────────────────────────┐
│                           SolenoidDriver                                     │
├─────────────────────────────────────────────────────────────────────────────┤
│ - _wire: TwoWire*                                                           │
│ - _mcp: Adafruit_MCP23X17[MAX_BOARDS]                                       │
│ - _channels: SolenoidChannel[MAX_CHANNELS]                                  │
│ - _boardCount: uint8_t                                                      │
│ - _channelCount: uint8_t                                                    │
│ - _initialized: bool                                                        │
│ - _config: SolenoidConfig                                                   │
│ - _lastError: SolenoidError                                                 │
│ - _errorCallback: void(*)(SolenoidError, uint8_t)                           │
├─────────────────────────────────────────────────────────────────────────────┤
│ + SolenoidDriver()                                                          │
│ + begin(wire: TwoWire&, addresses: uint8_t[], count: uint8_t): bool         │
│ + begin(wire: TwoWire&, address: uint8_t = 0x20): bool                      │
│ + setConfig(config: SolenoidConfig): void                                   │
│ + getConfig(): SolenoidConfig                                               │
│                                                                              │
│ + on(channel: uint8_t): SolenoidError                                       │
│ + off(channel: uint8_t): SolenoidError                                      │
│ + set(channel: uint8_t, state: bool): SolenoidError                         │
│ + pulse(channel: uint8_t, durationMs: uint32_t): SolenoidError              │
│                                                                              │
│ + allOn(): SolenoidError                                                    │
│ + allOff(): SolenoidError                                                   │
│ + setAll(states: uint8_t[]): SolenoidError                                  │
│ + setBoardChannels(board: uint8_t, states: uint8_t): SolenoidError          │
│                                                                              │
│ + isOn(channel: uint8_t): bool                                              │
│ + getChannelState(channel: uint8_t): SolenoidChannel*                       │
│ + getBoardState(board: uint8_t): uint8_t                                    │
│                                                                              │
│ + update(): void                                                            │
│ + emergencyStop(): void                                                     │
│                                                                              │
│ + isInitialized(): bool                                                     │
│ + getLastError(): SolenoidError                                             │
│ + getErrorString(error: SolenoidError): const char*                         │
│ + setErrorCallback(callback: void(*)(SolenoidError, uint8_t)): void         │
│                                                                              │
│ + getBoardCount(): uint8_t                                                  │
│ + getChannelCount(): uint8_t                                                │
│ + scanI2C(): uint8_t                                                        │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## Complete Class Interface

### SolenoidConfig.h

```cpp
/**
 * @file SolenoidConfig.h
 * @brief Configuration structures and constants for SolenoidDriver
 */

#ifndef SOLENOID_CONFIG_H
#define SOLENOID_CONFIG_H

#include <stdint.h>

// =============================================================================
// LIBRARY LIMITS
// =============================================================================

/** Maximum number of driver boards supported per I2C bus */
constexpr uint8_t SOLENOID_MAX_BOARDS_PER_BUS = 8;

/** Maximum number of channels per board */
constexpr uint8_t SOLENOID_CHANNELS_PER_BOARD = 8;

/** Maximum total channels (for static allocation) */
constexpr uint8_t SOLENOID_MAX_CHANNELS = 128;  // 16 boards x 8 channels

// =============================================================================
// DEFAULT CONFIGURATION VALUES
// =============================================================================

/** Default maximum solenoid on-time (ms) - prevents overheating */
constexpr uint32_t SOLENOID_DEFAULT_MAX_ON_TIME_MS = 5000;

/** Default minimum off-time between activations (ms) - allows cooling */
constexpr uint32_t SOLENOID_DEFAULT_MIN_OFF_TIME_MS = 50;

/** Default maximum duty cycle (0.0 - 1.0) */
constexpr float SOLENOID_DEFAULT_MAX_DUTY_CYCLE = 0.5f;

/** Default I2C communication timeout (ms) */
constexpr uint32_t SOLENOID_DEFAULT_I2C_TIMEOUT_MS = 100;

/** Default I2C clock speed (Hz) */
constexpr uint32_t SOLENOID_DEFAULT_I2C_CLOCK_HZ = 400000;

// =============================================================================
// MCP23017 CONSTANTS
// =============================================================================

/** MCP23017 base I2C address */
constexpr uint8_t MCP23017_BASE_ADDRESS = 0x20;

/** MCP23017 maximum address */
constexpr uint8_t MCP23017_MAX_ADDRESS = 0x27;

// =============================================================================
// ERROR CODES
// =============================================================================

/**
 * @enum SolenoidError
 * @brief Error codes returned by SolenoidDriver methods
 */
enum class SolenoidError : uint8_t {
    /** Operation completed successfully */
    OK = 0,

    /** Driver not initialized (begin() not called or failed) */
    NOT_INITIALIZED = 1,

    /** Invalid channel number specified */
    INVALID_CHANNEL = 2,

    /** Invalid board number specified */
    INVALID_BOARD = 3,

    /** I2C communication error */
    I2C_COMMUNICATION = 4,

    /** Safety: Maximum on-time exceeded, channel auto-disabled */
    SAFETY_TIMEOUT = 5,

    /** Safety: Minimum cooldown time not elapsed */
    SAFETY_COOLDOWN = 6,

    /** Safety: Duty cycle limit exceeded */
    DUTY_CYCLE_EXCEEDED = 7,

    /** Operation in progress (non-blocking mode) */
    BUSY = 8,

    /** Generic/unknown error */
    UNKNOWN = 255
};

// =============================================================================
// CONFIGURATION STRUCTURE
// =============================================================================

/**
 * @struct SolenoidConfig
 * @brief Runtime configuration options for SolenoidDriver
 */
struct SolenoidConfig {
    /**
     * Maximum time a solenoid can stay on continuously (milliseconds)
     * Solenoid will auto-shutoff after this duration to prevent overheating.
     * Set to 0 to disable (NOT RECOMMENDED for typical solenoids).
     * Default: 5000ms
     */
    uint32_t maxOnTimeMs = SOLENOID_DEFAULT_MAX_ON_TIME_MS;

    /**
     * Minimum time solenoid must be off before re-activation (milliseconds)
     * Enforces cooling period between activations.
     * Set to 0 to disable cooldown enforcement.
     * Default: 50ms
     */
    uint32_t minOffTimeMs = SOLENOID_DEFAULT_MIN_OFF_TIME_MS;

    /**
     * Maximum duty cycle (0.0 to 1.0)
     * Calculated over a rolling window. Activation blocked if exceeded.
     * Set to 1.0 to disable duty cycle limiting.
     * Default: 0.5 (50%)
     */
    float maxDutyCycle = SOLENOID_DEFAULT_MAX_DUTY_CYCLE;

    /**
     * I2C communication timeout (milliseconds)
     * Time to wait for I2C response before declaring communication error.
     * Default: 100ms
     */
    uint32_t i2cTimeoutMs = SOLENOID_DEFAULT_I2C_TIMEOUT_MS;

    /**
     * I2C clock frequency (Hz)
     * Standard: 100000 (100kHz), Fast: 400000 (400kHz)
     * Default: 400000 (400kHz)
     */
    uint32_t i2cClockHz = SOLENOID_DEFAULT_I2C_CLOCK_HZ;

    /**
     * Enable safety features
     * When false, maxOnTime and minOffTime checks are bypassed.
     * WARNING: Disabling safety can damage solenoids!
     * Default: true
     */
    bool safetyEnabled = true;

    /**
     * Enable verbose debug output to Serial
     * Default: false
     */
    bool debugEnabled = false;
};

#endif // SOLENOID_CONFIG_H
```

### SolenoidChannel.h

```cpp
/**
 * @file SolenoidChannel.h
 * @brief Individual solenoid channel state tracking
 */

#ifndef SOLENOID_CHANNEL_H
#define SOLENOID_CHANNEL_H

#include <stdint.h>
#include <Arduino.h>

/**
 * @class SolenoidChannel
 * @brief Tracks the state and timing of a single solenoid channel
 *
 * Maintains timing information for safety enforcement and diagnostics.
 */
class SolenoidChannel {
public:
    /**
     * @brief Construct a new Solenoid Channel object
     *
     * @param boardIndex Index of the driver board (0-15)
     * @param channelIndex Index of the channel on the board (0-7)
     * @param globalIndex Global channel index across all boards (0-127)
     */
    SolenoidChannel(uint8_t boardIndex = 0, uint8_t channelIndex = 0, uint8_t globalIndex = 0);

    /**
     * @brief Check if the channel is currently on
     * @return true if solenoid is activated
     */
    bool isOn() const;

    /**
     * @brief Get duration the channel has been on (if currently on)
     * @return Duration in milliseconds, or 0 if off
     */
    uint32_t onDuration() const;

    /**
     * @brief Get time since channel was turned off
     * @return Duration in milliseconds since last off, or UINT32_MAX if never turned off
     */
    uint32_t timeSinceOff() const;

    /**
     * @brief Get the board index
     * @return Board index (0-15)
     */
    uint8_t boardIndex() const;

    /**
     * @brief Get the channel index on the board
     * @return Channel index (0-7)
     */
    uint8_t channelIndex() const;

    /**
     * @brief Get the global channel index
     * @return Global index (0-127)
     */
    uint8_t globalIndex() const;

    /**
     * @brief Get total on-time for duty cycle calculation
     * @return Total milliseconds the channel has been on
     */
    uint32_t totalOnTime() const;

    /**
     * @brief Reset accumulated statistics
     */
    void resetStats();

    /**
     * @brief Update the channel state (called internally by SolenoidDriver)
     * @param isOn New state
     */
    void updateState(bool isOn);

private:
    uint8_t _boardIndex;      ///< Board index (0-15)
    uint8_t _channelIndex;    ///< Channel on board (0-7)
    uint8_t _globalIndex;     ///< Global channel index
    bool _isOn;               ///< Current state
    uint32_t _lastOnTime;     ///< millis() when last turned on
    uint32_t _lastOffTime;    ///< millis() when last turned off
    uint32_t _totalOnTime;    ///< Accumulated on-time for duty cycle
    uint32_t _activationCount;///< Number of activations
};

#endif // SOLENOID_CHANNEL_H
```

### SolenoidDriver.h

```cpp
/**
 * @file SolenoidDriver.h
 * @brief Main SolenoidDriver library header
 *
 * High-level interface for controlling solenoids through Adafruit I2C
 * Solenoid Driver boards (MCP23017-based).
 *
 * @author Mechanical MIDI Piano Project
 * @version 1.0.0
 */

#ifndef SOLENOID_DRIVER_H
#define SOLENOID_DRIVER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>

#include "SolenoidConfig.h"
#include "SolenoidChannel.h"

/**
 * @brief Error callback function type
 * @param error The error that occurred
 * @param channel The channel involved (or 255 for global errors)
 */
typedef void (*SolenoidErrorCallback)(SolenoidError error, uint8_t channel);

/**
 * @class SolenoidDriver
 * @brief Main class for controlling solenoid driver boards
 *
 * Provides a high-level, safety-aware interface for controlling multiple
 * solenoid driver boards over I2C.
 *
 * Features:
 * - Support for up to 8 boards per I2C bus (64 channels)
 * - Automatic safety shutoff for over-temperature protection
 * - Minimum cooldown enforcement between activations
 * - Duty cycle monitoring and limiting
 * - Non-blocking operation suitable for real-time applications
 *
 * Example usage:
 * @code
 * SolenoidDriver driver;
 *
 * void setup() {
 *     driver.begin(Wire, 0x20);
 *     driver.on(0);  // Turn on channel 0
 * }
 *
 * void loop() {
 *     driver.update();  // Must be called regularly for safety monitoring
 * }
 * @endcode
 */
class SolenoidDriver {
public:
    // =========================================================================
    // CONSTRUCTOR / DESTRUCTOR
    // =========================================================================

    /**
     * @brief Construct a new SolenoidDriver object
     *
     * Does not initialize hardware - call begin() to initialize.
     */
    SolenoidDriver();

    /**
     * @brief Destroy the SolenoidDriver object
     *
     * Automatically calls emergencyStop() to ensure all solenoids are off.
     */
    ~SolenoidDriver();

    // =========================================================================
    // INITIALIZATION
    // =========================================================================

    /**
     * @brief Initialize with a single driver board
     *
     * @param wire Reference to TwoWire instance (Wire, Wire1, or Wire2)
     * @param address I2C address of the MCP23017 (0x20-0x27)
     * @return true if initialization successful
     * @return false if initialization failed (check getLastError())
     *
     * Example:
     * @code
     * if (!driver.begin(Wire, 0x20)) {
     *     Serial.println(driver.getErrorString(driver.getLastError()));
     * }
     * @endcode
     */
    bool begin(TwoWire& wire, uint8_t address = MCP23017_BASE_ADDRESS);

    /**
     * @brief Initialize with multiple driver boards
     *
     * @param wire Reference to TwoWire instance
     * @param addresses Array of I2C addresses
     * @param count Number of boards (1-8)
     * @return true if all boards initialized successfully
     * @return false if any board failed (check getLastError())
     *
     * Example:
     * @code
     * uint8_t addresses[] = {0x20, 0x21, 0x22};
     * driver.begin(Wire, addresses, 3);
     * @endcode
     */
    bool begin(TwoWire& wire, const uint8_t addresses[], uint8_t count);

    /**
     * @brief Set configuration options
     *
     * @param config Configuration structure
     *
     * Can be called before or after begin(). Changes take effect immediately.
     */
    void setConfig(const SolenoidConfig& config);

    /**
     * @brief Get current configuration
     *
     * @return Current configuration structure
     */
    SolenoidConfig getConfig() const;

    // =========================================================================
    // SINGLE CHANNEL CONTROL
    // =========================================================================

    /**
     * @brief Turn on a single channel
     *
     * @param channel Global channel index (0 to channelCount-1)
     * @return SolenoidError::OK on success, error code on failure
     *
     * Subject to safety checks (cooldown, duty cycle).
     */
    SolenoidError on(uint8_t channel);

    /**
     * @brief Turn off a single channel
     *
     * @param channel Global channel index
     * @return SolenoidError::OK on success, error code on failure
     *
     * Always succeeds unless channel is invalid or I2C error.
     */
    SolenoidError off(uint8_t channel);

    /**
     * @brief Set channel to specific state
     *
     * @param channel Global channel index
     * @param state true=on, false=off
     * @return SolenoidError::OK on success, error code on failure
     */
    SolenoidError set(uint8_t channel, bool state);

    /**
     * @brief Pulse a channel for a specified duration (blocking)
     *
     * @param channel Global channel index
     * @param durationMs Pulse duration in milliseconds
     * @return SolenoidError::OK on success, error code on failure
     *
     * WARNING: This is a blocking function. For non-blocking operation,
     * use on() and off() with your own timing.
     *
     * Duration is clamped to maxOnTimeMs if exceeded.
     */
    SolenoidError pulse(uint8_t channel, uint32_t durationMs);

    // =========================================================================
    // MULTI-CHANNEL CONTROL
    // =========================================================================

    /**
     * @brief Turn on all channels
     *
     * @return SolenoidError::OK on success
     *
     * Applies safety checks to each channel individually.
     * Channels that fail safety checks will remain off.
     */
    SolenoidError allOn();

    /**
     * @brief Turn off all channels immediately
     *
     * @return SolenoidError::OK on success
     *
     * Bypasses safety checks for immediate shutoff.
     * Use emergencyStop() for guaranteed immediate shutoff.
     */
    SolenoidError allOff();

    /**
     * @brief Set all channel states at once
     *
     * @param states Array of bytes, one per board. Each bit = one channel.
     * @return SolenoidError::OK on success
     *
     * Example for 2 boards (16 channels):
     * @code
     * uint8_t states[] = {0b00001111, 0b11110000};  // Channels 0-3 and 12-15 on
     * driver.setAll(states);
     * @endcode
     */
    SolenoidError setAll(const uint8_t states[]);

    /**
     * @brief Set all channels on a single board
     *
     * @param board Board index (0 to boardCount-1)
     * @param states Bitmask of channel states (bit 0 = channel 0)
     * @return SolenoidError::OK on success
     *
     * More efficient than setting channels individually.
     */
    SolenoidError setBoardChannels(uint8_t board, uint8_t states);

    // =========================================================================
    // STATE QUERIES
    // =========================================================================

    /**
     * @brief Check if a channel is currently on
     *
     * @param channel Global channel index
     * @return true if channel is on, false if off or invalid
     */
    bool isOn(uint8_t channel) const;

    /**
     * @brief Get detailed state information for a channel
     *
     * @param channel Global channel index
     * @return Pointer to SolenoidChannel object, or nullptr if invalid
     */
    const SolenoidChannel* getChannelState(uint8_t channel) const;

    /**
     * @brief Get the state of all channels on a board as a bitmask
     *
     * @param board Board index
     * @return Bitmask of channel states (bit 0 = channel 0)
     */
    uint8_t getBoardState(uint8_t board) const;

    // =========================================================================
    // SAFETY AND MAINTENANCE
    // =========================================================================

    /**
     * @brief Update function - MUST be called regularly
     *
     * Performs safety checks and auto-shutoff for channels exceeding maxOnTime.
     * Should be called from loop() at least every 10ms.
     *
     * This function is non-blocking and returns quickly.
     */
    void update();

    /**
     * @brief Immediately turn off all channels
     *
     * Emergency stop - bypasses all safety checks and state tracking.
     * Writes 0x00 directly to all GPIO registers.
     * Use when immediate shutoff is critical.
     */
    void emergencyStop();

    // =========================================================================
    // ERROR HANDLING
    // =========================================================================

    /**
     * @brief Check if driver is initialized and ready
     *
     * @return true if begin() succeeded
     */
    bool isInitialized() const;

    /**
     * @brief Get the last error that occurred
     *
     * @return Last error code
     */
    SolenoidError getLastError() const;

    /**
     * @brief Get human-readable string for error code
     *
     * @param error Error code
     * @return String description
     */
    static const char* getErrorString(SolenoidError error);

    /**
     * @brief Set callback for error notifications
     *
     * @param callback Function to call when errors occur
     *
     * Callback receives error code and channel number (255 for global errors).
     */
    void setErrorCallback(SolenoidErrorCallback callback);

    // =========================================================================
    // DIAGNOSTICS
    // =========================================================================

    /**
     * @brief Get number of initialized boards
     *
     * @return Board count (0-8)
     */
    uint8_t getBoardCount() const;

    /**
     * @brief Get total number of available channels
     *
     * @return Channel count (boards * 8)
     */
    uint8_t getChannelCount() const;

    /**
     * @brief Scan I2C bus for MCP23017 devices
     *
     * @return Number of devices found in address range 0x20-0x27
     */
    uint8_t scanI2C();

private:
    // =========================================================================
    // PRIVATE MEMBERS
    // =========================================================================

    TwoWire* _wire;                                          ///< I2C bus reference
    Adafruit_MCP23X17 _mcp[SOLENOID_MAX_BOARDS_PER_BUS];    ///< MCP23017 instances
    SolenoidChannel _channels[SOLENOID_MAX_CHANNELS];       ///< Channel state objects
    uint8_t _boardAddresses[SOLENOID_MAX_BOARDS_PER_BUS];   ///< Board I2C addresses
    uint8_t _boardStates[SOLENOID_MAX_BOARDS_PER_BUS];      ///< Current GPIO states
    uint8_t _boardCount;                                     ///< Number of boards
    uint8_t _channelCount;                                   ///< Total channels
    bool _initialized;                                       ///< Initialization status
    SolenoidConfig _config;                                  ///< Configuration
    SolenoidError _lastError;                                ///< Last error code
    SolenoidErrorCallback _errorCallback;                    ///< Error callback

    // =========================================================================
    // PRIVATE METHODS
    // =========================================================================

    /**
     * @brief Write state to a single channel on the hardware
     */
    bool writeChannel(uint8_t board, uint8_t channel, bool state);

    /**
     * @brief Write all channels on a board to hardware
     */
    bool writeBoard(uint8_t board, uint8_t states);

    /**
     * @brief Check if a channel activation is safe
     */
    bool isSafeToActivate(uint8_t channel);

    /**
     * @brief Report an error
     */
    void reportError(SolenoidError error, uint8_t channel = 255);

    /**
     * @brief Convert global channel to board/local channel
     */
    void globalToLocal(uint8_t globalChannel, uint8_t& board, uint8_t& localChannel) const;
};

#endif // SOLENOID_DRIVER_H
```

---

## Method Documentation

### Initialization Methods

#### `begin(TwoWire& wire, uint8_t address)`

Initializes the driver with a single board.

**Parameters**:
- `wire`: I2C bus to use (Wire, Wire1, or Wire2 on Teensy 4.1)
- `address`: I2C address (0x20-0x27), default 0x20

**Returns**: `true` if successful, `false` on error

**Behavior**:
1. Stores wire reference
2. Sets I2C clock speed from config
3. Attempts to initialize MCP23017 at specified address
4. Configures Port A as outputs
5. Sets all outputs to LOW (off)
6. Initializes channel state tracking

**Example**:
```cpp
SolenoidDriver driver;

void setup() {
    Wire.begin();

    if (!driver.begin(Wire, 0x20)) {
        Serial.print("Init failed: ");
        Serial.println(SolenoidDriver::getErrorString(driver.getLastError()));
        while (1);
    }
}
```

#### `begin(TwoWire& wire, const uint8_t addresses[], uint8_t count)`

Initializes with multiple boards.

**Parameters**:
- `wire`: I2C bus to use
- `addresses`: Array of I2C addresses
- `count`: Number of boards (1-8)

**Example**:
```cpp
// Initialize 3 boards for 24 channels
uint8_t addrs[] = {0x20, 0x21, 0x22};
driver.begin(Wire, addrs, 3);
```

### Control Methods

#### `on(uint8_t channel)`

Turns on a single solenoid channel.

**Parameters**:
- `channel`: Global channel index (0 to channelCount-1)

**Returns**: `SolenoidError::OK` or error code

**Safety Checks**:
- Validates channel number
- Checks minimum cooldown time elapsed
- Checks duty cycle limit not exceeded (if enabled)

**Example**:
```cpp
SolenoidError err = driver.on(5);
if (err != SolenoidError::OK) {
    if (err == SolenoidError::SAFETY_COOLDOWN) {
        // Channel still cooling down
    }
}
```

#### `off(uint8_t channel)`

Turns off a single solenoid channel.

**Parameters**:
- `channel`: Global channel index

**Returns**: `SolenoidError::OK` or error code

**Notes**: Always succeeds unless invalid channel or I2C error

#### `pulse(uint8_t channel, uint32_t durationMs)`

Activates channel for specified duration (blocking).

**Parameters**:
- `channel`: Global channel index
- `durationMs`: Duration in milliseconds (clamped to maxOnTimeMs)

**Returns**: `SolenoidError::OK` or error code

**Warning**: Blocking function - use on()/off() for non-blocking

#### `setBoardChannels(uint8_t board, uint8_t states)`

Sets all 8 channels on a board simultaneously.

**Parameters**:
- `board`: Board index (0 to boardCount-1)
- `states`: Bitmask (bit 0 = channel 0, etc.)

**Example**:
```cpp
// Turn on channels 0, 2, 4, 6 on board 0
driver.setBoardChannels(0, 0b01010101);
```

### Safety Methods

#### `update()`

**CRITICAL**: Must be called regularly from `loop()`.

**Behavior**:
- Checks each active channel for timeout
- Auto-disables channels exceeding `maxOnTimeMs`
- Triggers error callback if configured
- Should be called at least every 10ms

**Example**:
```cpp
void loop() {
    driver.update();  // Required for safety monitoring

    // Your other code...
}
```

#### `emergencyStop()`

Immediately disables all solenoids.

**Behavior**:
- Writes 0x00 to all GPIO registers
- Bypasses normal state tracking
- Does not enforce cooldown
- Use for emergency situations

---

## Usage Examples

### Example 1: Basic Single Board Setup

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <SolenoidDriver.h>

SolenoidDriver driver;

void setup() {
    Serial.begin(115200);
    Wire.begin();

    // Initialize with default address 0x20
    if (!driver.begin(Wire)) {
        Serial.println("Failed to initialize driver");
        while (1);
    }

    Serial.println("Driver initialized");
}

void loop() {
    // CRITICAL: Call update() regularly
    driver.update();

    // Simple on/off test
    driver.on(0);
    delay(100);
    driver.off(0);
    delay(500);
}
```

### Example 2: Multiple Boards (24 Keys)

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <SolenoidDriver.h>

SolenoidDriver driver;

void setup() {
    Serial.begin(115200);
    Wire.begin();

    // Initialize 3 boards
    uint8_t addresses[] = {0x20, 0x21, 0x22};
    if (!driver.begin(Wire, addresses, 3)) {
        Serial.println("Failed to initialize");
        while (1);
    }

    Serial.print("Initialized ");
    Serial.print(driver.getChannelCount());
    Serial.println(" channels");
}

void loop() {
    driver.update();

    // Cycle through all 24 channels
    static uint8_t channel = 0;
    static uint32_t lastTime = 0;

    if (millis() - lastTime > 200) {
        driver.off(channel);
        channel = (channel + 1) % driver.getChannelCount();
        driver.on(channel);
        lastTime = millis();
    }
}
```

### Example 3: Custom Configuration

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <SolenoidDriver.h>

SolenoidDriver driver;

void errorHandler(SolenoidError error, uint8_t channel) {
    Serial.print("Error on channel ");
    Serial.print(channel);
    Serial.print(": ");
    Serial.println(SolenoidDriver::getErrorString(error));
}

void setup() {
    Serial.begin(115200);
    Wire.begin();

    // Custom configuration
    SolenoidConfig config;
    config.maxOnTimeMs = 3000;     // 3 second max on-time
    config.minOffTimeMs = 100;     // 100ms cooldown
    config.maxDutyCycle = 0.3f;    // 30% max duty cycle
    config.debugEnabled = true;     // Enable debug output

    driver.setConfig(config);
    driver.setErrorCallback(errorHandler);

    driver.begin(Wire, 0x20);
}

void loop() {
    driver.update();
    // ...
}
```

### Example 4: 88-Key Piano Setup (Two I2C Buses)

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <SolenoidDriver.h>

// Two driver instances for two I2C buses
SolenoidDriver driverBus0;  // Keys 1-64
SolenoidDriver driverBus1;  // Keys 65-88

void setup() {
    Serial.begin(115200);

    // Initialize both I2C buses
    Wire.begin();
    Wire1.begin();

    // Bus 0: 8 boards (64 channels)
    uint8_t bus0Addrs[] = {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27};
    if (!driverBus0.begin(Wire, bus0Addrs, 8)) {
        Serial.println("Bus 0 init failed");
    }

    // Bus 1: 3 boards (24 channels, only 24 used for keys 65-88)
    uint8_t bus1Addrs[] = {0x20, 0x21, 0x22};
    if (!driverBus1.begin(Wire1, bus1Addrs, 3)) {
        Serial.println("Bus 1 init failed");
    }

    Serial.println("88-key piano initialized");
}

void loop() {
    driverBus0.update();
    driverBus1.update();
    // ...
}

// Helper to play MIDI note
void playNote(uint8_t midiNote) {
    if (midiNote < 21 || midiNote > 108) return;

    uint8_t keyNum = midiNote - 21;  // 0-87

    if (keyNum < 64) {
        driverBus0.on(keyNum);
    } else {
        driverBus1.on(keyNum - 64);
    }
}

void stopNote(uint8_t midiNote) {
    if (midiNote < 21 || midiNote > 108) return;

    uint8_t keyNum = midiNote - 21;

    if (keyNum < 64) {
        driverBus0.off(keyNum);
    } else {
        driverBus1.off(keyNum - 64);
    }
}
```

### Example 5: Non-Blocking Pulse with Timer

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <SolenoidDriver.h>

SolenoidDriver driver;

// Track pending pulses
struct PendingPulse {
    uint8_t channel;
    uint32_t endTime;
    bool active;
};

PendingPulse pulses[8];

void startPulse(uint8_t channel, uint32_t durationMs) {
    if (channel >= 8) return;

    driver.on(channel);
    pulses[channel].channel = channel;
    pulses[channel].endTime = millis() + durationMs;
    pulses[channel].active = true;
}

void updatePulses() {
    uint32_t now = millis();
    for (int i = 0; i < 8; i++) {
        if (pulses[i].active && now >= pulses[i].endTime) {
            driver.off(pulses[i].channel);
            pulses[i].active = false;
        }
    }
}

void setup() {
    Wire.begin();
    driver.begin(Wire);

    // Initialize pulse tracking
    for (int i = 0; i < 8; i++) {
        pulses[i].active = false;
    }
}

void loop() {
    driver.update();
    updatePulses();

    // Example: start non-blocking pulses
    static uint32_t lastTrigger = 0;
    if (millis() - lastTrigger > 500) {
        startPulse(0, 100);  // 100ms pulse on channel 0
        lastTrigger = millis();
    }
}
```

---

## Error Handling

### Error Codes Reference

| Code | Name | Description | Typical Cause |
|------|------|-------------|---------------|
| 0 | `OK` | Success | - |
| 1 | `NOT_INITIALIZED` | Driver not ready | begin() not called or failed |
| 2 | `INVALID_CHANNEL` | Bad channel number | Channel >= channelCount |
| 3 | `INVALID_BOARD` | Bad board number | Board >= boardCount |
| 4 | `I2C_COMMUNICATION` | I2C error | Wiring, address, or hardware issue |
| 5 | `SAFETY_TIMEOUT` | Auto-shutoff triggered | Channel exceeded maxOnTimeMs |
| 6 | `SAFETY_COOLDOWN` | Cooldown active | Too soon after last off |
| 7 | `DUTY_CYCLE_EXCEEDED` | Duty cycle limit | Channel used too much |
| 8 | `BUSY` | Operation in progress | (Reserved for async) |
| 255 | `UNKNOWN` | Unknown error | Unexpected condition |

### Error Handling Patterns

#### Pattern 1: Check Return Values

```cpp
SolenoidError err = driver.on(channel);
if (err != SolenoidError::OK) {
    Serial.print("Error: ");
    Serial.println(SolenoidDriver::getErrorString(err));
}
```

#### Pattern 2: Use Error Callback

```cpp
void onError(SolenoidError error, uint8_t channel) {
    // Log error
    Serial.print("Channel ");
    Serial.print(channel);
    Serial.print(" error: ");
    Serial.println(SolenoidDriver::getErrorString(error));

    // Take action
    if (error == SolenoidError::SAFETY_TIMEOUT) {
        // Handle timeout - maybe alert user
    }
}

void setup() {
    driver.setErrorCallback(onError);
    driver.begin(Wire);
}
```

#### Pattern 3: Retry on Transient Errors

```cpp
bool safeOn(uint8_t channel, int maxRetries = 3) {
    for (int i = 0; i < maxRetries; i++) {
        SolenoidError err = driver.on(channel);

        if (err == SolenoidError::OK) {
            return true;
        }

        if (err == SolenoidError::SAFETY_COOLDOWN) {
            // Wait for cooldown
            delay(50);
        } else if (err == SolenoidError::I2C_COMMUNICATION) {
            // Brief retry delay
            delay(10);
        } else {
            // Non-recoverable error
            return false;
        }
    }
    return false;
}
```

---

## Configuration Options

### Configuration Parameters

| Parameter | Type | Default | Range | Description |
|-----------|------|---------|-------|-------------|
| `maxOnTimeMs` | uint32_t | 5000 | 0-UINT32_MAX | Max continuous on-time (0=disabled) |
| `minOffTimeMs` | uint32_t | 50 | 0-1000 | Min cooldown between activations |
| `maxDutyCycle` | float | 0.5 | 0.0-1.0 | Max duty cycle (1.0=disabled) |
| `i2cTimeoutMs` | uint32_t | 100 | 10-1000 | I2C timeout |
| `i2cClockHz` | uint32_t | 400000 | 100000-1000000 | I2C speed |
| `safetyEnabled` | bool | true | - | Enable/disable safety features |
| `debugEnabled` | bool | false | - | Enable debug output |

### Configuration Presets

#### Preset: Piano (Recommended)

```cpp
SolenoidConfig config;
config.maxOnTimeMs = 5000;      // 5 second max
config.minOffTimeMs = 30;       // 30ms cooldown (fast repeat)
config.maxDutyCycle = 0.5f;     // 50% duty cycle
config.i2cClockHz = 400000;     // 400kHz
config.safetyEnabled = true;
driver.setConfig(config);
```

#### Preset: Test Mode

```cpp
SolenoidConfig config;
config.maxOnTimeMs = 1000;      // Short max for safety
config.minOffTimeMs = 100;      // Longer cooldown
config.maxDutyCycle = 0.3f;     // Conservative duty
config.debugEnabled = true;     // Show debug info
driver.setConfig(config);
```

#### Preset: Performance (Minimal Safety)

```cpp
SolenoidConfig config;
config.maxOnTimeMs = 10000;     // 10 seconds
config.minOffTimeMs = 10;       // Minimal cooldown
config.maxDutyCycle = 0.8f;     // High duty allowed
config.safetyEnabled = true;    // Keep basic safety
driver.setConfig(config);
```

---

## Multi-Board Management

### I2C Bus Assignment Strategy

For an 88-key piano:

| Bus | Teensy Pins | Boards | Addresses | Channels | Piano Keys |
|-----|-------------|--------|-----------|----------|------------|
| Wire | SDA=18, SCL=19 | 8 | 0x20-0x27 | 0-63 | 1-64 (A0-E5) |
| Wire1 | SDA=17, SCL=16 | 3 | 0x20-0x22 | 64-87 | 65-88 (F5-C8) |

### Global Channel Calculation

```cpp
/**
 * Convert MIDI note (21-108) to global channel (0-87)
 */
uint8_t midiToChannel(uint8_t midiNote) {
    if (midiNote < 21 || midiNote > 108) return 255;  // Invalid
    return midiNote - 21;
}

/**
 * Convert global channel to bus and local channel
 */
void channelToBusLocal(uint8_t globalCh, uint8_t& bus, uint8_t& localCh) {
    if (globalCh < 64) {
        bus = 0;  // Wire
        localCh = globalCh;
    } else {
        bus = 1;  // Wire1
        localCh = globalCh - 64;
    }
}
```

---

## Performance Considerations

### Timing Benchmarks (Expected)

| Operation | Time | Notes |
|-----------|------|-------|
| Single channel write | ~25us | At 400kHz I2C |
| Full board write (8 channels) | ~25us | Single I2C transaction |
| update() (no timeouts) | ~5us | Fast check only |
| update() (with timeout) | ~50us | Includes I2C write |
| I2C scan (8 addresses) | ~2ms | For diagnostics only |

### Optimization Tips

1. **Use `setBoardChannels()` for multiple simultaneous changes**
   - Single I2C transaction vs. one per channel
   - 8x faster for 8 channels

2. **Call `update()` regularly but not excessively**
   - Once per loop() iteration is sufficient
   - More frequent doesn't improve safety

3. **Batch state changes when possible**
   - Collect all note-ons/offs in a frame
   - Apply all changes at once

4. **Use appropriate I2C speed**
   - 400kHz is optimal for most setups
   - Reduce to 100kHz for long cables or noise issues

---

## Future MIDI Integration

### Planned API Extensions

```cpp
// Future MIDI-specific methods

/**
 * Play a MIDI note with velocity
 * Velocity affects activation duration/strength
 */
SolenoidError noteOn(uint8_t midiNote, uint8_t velocity);

/**
 * Stop a MIDI note
 */
SolenoidError noteOff(uint8_t midiNote);

/**
 * Set note-to-channel mapping
 */
void setNoteMapping(uint8_t midiNote, uint8_t channel);

/**
 * Set velocity curve
 */
void setVelocityCurve(uint8_t curve);
```

### MIDI Note Mapping Table (Reference)

| MIDI Note | Note Name | Octave | Channel | Board | Bus |
|-----------|-----------|--------|---------|-------|-----|
| 21 | A | 0 | 0 | 0 | Wire |
| 24 | C | 1 | 3 | 0 | Wire |
| 36 | C | 2 | 15 | 1 | Wire |
| 48 | C | 3 | 27 | 3 | Wire |
| 60 | C | 4 | 39 | 4 | Wire |
| 72 | C | 5 | 51 | 6 | Wire |
| 84 | C | 6 | 63 | 7 | Wire |
| 96 | C | 7 | 75 | 9 | Wire1 |
| 108 | C | 8 | 87 | 10 | Wire1 |
