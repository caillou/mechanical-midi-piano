/**
 * @file SolenoidConfig.h
 * @brief Configuration structures and constants for SolenoidDriver library
 *
 * This file contains all configuration constants, error codes, and the
 * SolenoidConfig structure used throughout the SolenoidDriver library.
 *
 * @author Mechanical MIDI Piano Project
 * @version 1.0.0
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

/** Default duty cycle window duration (ms) - 10 second rolling window */
constexpr uint32_t SOLENOID_DEFAULT_DUTY_CYCLE_WINDOW_MS = 10000;

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
 *
 * These error codes provide detailed information about the success or failure
 * of operations. Safety-related errors (SAFETY_TIMEOUT, SAFETY_COOLDOWN,
 * DUTY_CYCLE_EXCEEDED) indicate protective measures have been triggered.
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
 *
 * This structure holds all configurable parameters for the SolenoidDriver.
 * Default values are provided for safe operation. Configuration can be
 * modified at any time via SolenoidDriver::setConfig().
 *
 * Example usage:
 * @code
 * SolenoidConfig config;
 * config.maxOnTimeMs = 3000;     // 3 second max on-time
 * config.minOffTimeMs = 100;     // 100ms cooldown
 * config.debugEnabled = true;     // Enable debug output
 * driver.setConfig(config);
 * @endcode
 */
struct SolenoidConfig {
    /**
     * Maximum time a solenoid can stay on continuously (milliseconds)
     *
     * Solenoid will auto-shutoff after this duration to prevent overheating.
     * Set to 0 to disable (NOT RECOMMENDED for typical solenoids).
     * Default: 5000ms
     */
    uint32_t maxOnTimeMs = SOLENOID_DEFAULT_MAX_ON_TIME_MS;

    /**
     * Minimum time solenoid must be off before re-activation (milliseconds)
     *
     * Enforces cooling period between activations.
     * Set to 0 to disable cooldown enforcement.
     * Default: 50ms
     */
    uint32_t minOffTimeMs = SOLENOID_DEFAULT_MIN_OFF_TIME_MS;

    /**
     * Maximum duty cycle (0.0 to 1.0)
     *
     * Calculated over a rolling window. Activation blocked if exceeded.
     * Set to 1.0 to disable duty cycle limiting.
     * Default: 0.5 (50%)
     */
    float maxDutyCycle = SOLENOID_DEFAULT_MAX_DUTY_CYCLE;

    /**
     * Duty cycle rolling window duration (milliseconds)
     *
     * The time period over which duty cycle is calculated.
     * A longer window provides smoother averaging but slower response.
     * A shorter window reacts faster but may be more restrictive.
     * Default: 10000ms (10 seconds)
     */
    uint32_t dutyCycleWindowMs = SOLENOID_DEFAULT_DUTY_CYCLE_WINDOW_MS;

    /**
     * I2C communication timeout (milliseconds)
     *
     * Time to wait for I2C response before declaring communication error.
     * Default: 100ms
     */
    uint32_t i2cTimeoutMs = SOLENOID_DEFAULT_I2C_TIMEOUT_MS;

    /**
     * I2C clock frequency (Hz)
     *
     * Standard: 100000 (100kHz), Fast: 400000 (400kHz)
     * Default: 400000 (400kHz)
     */
    uint32_t i2cClockHz = SOLENOID_DEFAULT_I2C_CLOCK_HZ;

    /**
     * Enable safety features
     *
     * When false, maxOnTime and minOffTime checks are bypassed.
     * WARNING: Disabling safety can damage solenoids!
     * Default: true
     */
    bool safetyEnabled = true;

    /**
     * Enable verbose debug output to Serial
     *
     * When true, detailed operation information is printed to Serial.
     * Default: false
     */
    bool debugEnabled = false;
};

#endif // SOLENOID_CONFIG_H
