/**
 * @file SolenoidDriver.h
 * @brief Main SolenoidDriver library header
 *
 * High-level interface for controlling solenoids through Adafruit I2C
 * Solenoid Driver boards (MCP23017-based).
 *
 * Features:
 * - Support for up to 8 boards per I2C bus (64 channels)
 * - Automatic safety shutoff for over-temperature protection
 * - Minimum cooldown enforcement between activations
 * - Duty cycle monitoring and limiting
 * - Non-blocking operation suitable for real-time applications
 * - Support for multiple I2C buses (Wire, Wire1, Wire2)
 * - Error callback system for monitoring
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
 *
 * @param error The error that occurred
 * @param channel The channel involved (255 for global errors)
 *
 * Example:
 * @code
 * void onError(SolenoidError error, uint8_t channel) {
 *     Serial.print("Error on channel ");
 *     Serial.print(channel);
 *     Serial.print(": ");
 *     Serial.println(SolenoidDriver::getErrorString(error));
 * }
 * driver.setErrorCallback(onError);
 * @endcode
 */
typedef void (*SolenoidErrorCallback)(SolenoidError error, uint8_t channel);

/**
 * @class SolenoidDriver
 * @brief Main class for controlling solenoid driver boards
 *
 * Provides a high-level, safety-aware interface for controlling multiple
 * solenoid driver boards over I2C.
 *
 * The driver uses Port A (pins 0-7) of the MCP23017 for solenoid control.
 * Port B is available for future expansion (e.g., feedback sensors).
 *
 * Safety Features:
 * - Maximum on-time: Channels auto-shutoff after configurable duration
 * - Minimum off-time: Enforces cooldown period between activations
 * - Duty cycle limiting: Prevents overuse of individual channels
 * - Emergency stop: Immediate shutoff of all channels
 *
 * Thread Safety:
 * This class is NOT thread-safe. All calls should be made from the same
 * thread/context (typically the main Arduino loop).
 *
 * Example usage:
 * @code
 * SolenoidDriver driver;
 *
 * void setup() {
 *     Wire.begin();
 *     if (!driver.begin(Wire, 0x20)) {
 *         Serial.println("Init failed!");
 *         while (1);
 *     }
 * }
 *
 * void loop() {
 *     driver.update();  // Must be called regularly for safety monitoring
 *
 *     // Control solenoids
 *     driver.on(0);     // Turn on channel 0
 *     delay(100);
 *     driver.off(0);    // Turn off channel 0
 * }
 * @endcode
 */
/**
 * @note Supports up to 8 MCP23017 boards per I2C bus.
 * Use begin() with an array of addresses for multi-board setups.
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
     * Default configuration is applied (see SolenoidConfig).
     */
    SolenoidDriver();

    /**
     * @brief Destroy the SolenoidDriver object
     *
     * Automatically calls emergencyStop() to ensure all solenoids are off.
     * This prevents solenoids from being left on if the driver is destroyed.
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
     * This method:
     * 1. Configures the I2C clock speed from config
     * 2. Initializes the MCP23017 at the specified address
     * 3. Configures Port A as outputs
     * 4. Sets all outputs to LOW (off)
     * 5. Initializes channel state tracking
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
     * Boards are initialized in order. If any board fails, initialization
     * stops and false is returned. Successfully initialized boards remain
     * available.
     *
     * Example:
     * @code
     * uint8_t addresses[] = {0x20, 0x21, 0x22};
     * if (!driver.begin(Wire, addresses, 3)) {
     *     Serial.print("Only ");
     *     Serial.print(driver.getBoardCount());
     *     Serial.println(" boards initialized");
     * }
     * @endcode
     */
    bool begin(TwoWire& wire, const uint8_t addresses[], uint8_t count);

    /**
     * @brief Set configuration options
     *
     * @param config Configuration structure
     *
     * Can be called before or after begin(). Changes take effect immediately.
     * If called before begin(), the I2C clock speed will be applied during
     * initialization.
     */
    void setConfig(const SolenoidConfig& config);

    /**
     * @brief Get current configuration
     *
     * @return Copy of current configuration structure
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
     * Subject to safety checks when safetyEnabled is true:
     * - SAFETY_COOLDOWN: Returned if minOffTimeMs has not elapsed
     * - DUTY_CYCLE_EXCEEDED: Returned if duty cycle limit exceeded
     *
     * If the channel is already on, this is a no-op and returns OK.
     */
    SolenoidError on(uint8_t channel);

    /**
     * @brief Turn off a single channel
     *
     * @param channel Global channel index
     * @return SolenoidError::OK on success, error code on failure
     *
     * Always succeeds unless:
     * - Channel is invalid (INVALID_CHANNEL)
     * - Driver not initialized (NOT_INITIALIZED)
     * - I2C error occurs (I2C_COMMUNICATION)
     *
     * If the channel is already off, this is a no-op and returns OK.
     */
    SolenoidError off(uint8_t channel);

    /**
     * @brief Set channel to specific state
     *
     * @param channel Global channel index
     * @param state true=on, false=off
     * @return SolenoidError::OK on success, error code on failure
     *
     * Convenience method that calls on() or off() based on state.
     */
    SolenoidError set(uint8_t channel, bool state);

    /**
     * @brief Toggle channel state
     *
     * @param channel Global channel index
     * @return SolenoidError::OK on success, error code on failure
     *
     * If channel is on, turns it off. If off, turns it on.
     * Subject to same safety checks as on().
     */
    SolenoidError toggle(uint8_t channel);

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
     * @return SolenoidError::OK if all channels turned on successfully.
     *         Returns safety error code (SAFETY_COOLDOWN or DUTY_CYCLE_EXCEEDED)
     *         if some channels were blocked by safety checks. Channels that fail
     *         safety checks remain off while others are turned on.
     *
     * Applies safety checks to each channel individually.
     */
    SolenoidError allOn();

    /**
     * @brief Turn off all channels immediately
     *
     * @return SolenoidError::OK on success
     *
     * Turns off all channels without safety checks (always allowed).
     * For guaranteed immediate shutoff, use emergencyStop() instead.
     */
    SolenoidError allOff();

    /**
     * @brief Set all channel states at once
     *
     * @param states Array of bytes, one per board. Each bit = one channel.
     * @param stateCount Number of elements in the states array
     * @return SolenoidError::OK on success, INVALID_BOARD if stateCount < boardCount
     *
     * The stateCount parameter prevents array out-of-bounds access.
     * Each byte represents 8 channels (bit 0 = channel 0, etc.)
     *
     * Example for 2 boards (16 channels):
     * @code
     * uint8_t states[] = {0b00001111, 0b11110000};  // Channels 0-3 and 12-15 on
     * driver.setAll(states, 2);
     * @endcode
     */
    SolenoidError setAll(const uint8_t states[], uint8_t stateCount);

    /**
     * @brief Set all channels on a single board
     *
     * @param board Board index (0 to boardCount-1)
     * @param states Bitmask of channel states (bit 0 = channel 0)
     * @return SolenoidError::OK if all channels set successfully.
     *         Returns safety error code if some channels were blocked by
     *         safety checks (cooldown or duty cycle). Channels that fail
     *         safety checks remain in their previous state.
     *
     * More efficient than setting channels individually - uses a single
     * I2C transaction.
     *
     * Example:
     * @code
     * driver.setBoardChannels(0, 0b01010101);  // Channels 0, 2, 4, 6 on
     * @endcode
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
     *
     * The returned pointer is valid until the driver is destroyed.
     * Do not store the pointer long-term.
     */
    const SolenoidChannel* getChannelState(uint8_t channel) const;

    /**
     * @brief Get the state of all channels on a board as a bitmask
     *
     * @param board Board index
     * @return Bitmask of channel states (bit 0 = channel 0), or 0 if invalid
     */
    uint8_t getBoardState(uint8_t board) const;

    // =========================================================================
    // SAFETY AND MAINTENANCE
    // =========================================================================

    /**
     * @brief Update function - MUST be called regularly
     *
     * Performs safety checks and auto-shutoff for channels exceeding maxOnTime.
     * Should be called from loop() at least every 10ms for reliable safety.
     *
     * This function is non-blocking and returns quickly.
     *
     * When a channel is auto-shutoff:
     * - The channel is turned off
     * - SAFETY_TIMEOUT error is reported via callback (if set)
     * - Debug message is printed (if debugEnabled)
     */
    void update();

    /**
     * @brief Immediately turn off all channels
     *
     * Emergency stop - bypasses all safety checks and state tracking.
     * Writes 0x00 directly to all GPIO registers.
     *
     * Use when immediate shutoff is critical. This is also called
     * automatically when the driver is destroyed.
     */
    void emergencyStop();

    /**
     * @brief Reset statistics for all channels
     *
     * Clears the duty cycle tracking window, total on-time, and activation
     * count for all channels. This gives a clean slate after an emergency
     * stop, preventing "duty cycle exceeded" errors from persisting.
     *
     * Call this after emergencyStop() to fully reset the driver state.
     */
    void resetAllStats();

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
     *
     * This is cleared on successful operations.
     */
    SolenoidError getLastError() const;

    /**
     * @brief Get human-readable string for error code
     *
     * @param error Error code
     * @return String description (stored in PROGMEM)
     */
    static const char* getErrorString(SolenoidError error);

    /**
     * @brief Set callback for error notifications
     *
     * @param callback Function to call when errors occur, or nullptr to disable
     *
     * Callback receives error code and channel number (255 for global errors).
     * The callback is called synchronously - keep it short to avoid
     * blocking I2C operations.
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
     *
     * This is a diagnostic function that scans for potential MCP23017
     * devices. It does not modify the driver state.
     */
    uint8_t scanI2C();

    /**
     * @brief Get the I2C address of a specific board
     *
     * @param board Board index (0 to boardCount-1)
     * @return I2C address, or 0 if invalid board index
     */
    uint8_t getBoardAddress(uint8_t board) const;

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
     * @brief Validate that the driver is initialized
     *
     * @return true if initialized, false otherwise (reports NOT_INITIALIZED error)
     */
    bool validateInitialized();

    /**
     * @brief Validate that a channel index is valid and the driver is initialized
     *
     * @param channel Global channel index to validate
     * @return true if valid, false otherwise (reports appropriate error)
     */
    bool validateChannel(uint8_t channel);

    /**
     * @brief Write state to a single channel on the hardware
     *
     * @param board Board index
     * @param channel Local channel index (0-7)
     * @param state Desired state
     * @return true if write succeeded
     */
    bool writeChannel(uint8_t board, uint8_t channel, bool state);

    /**
     * @brief Write all channels on a board to hardware
     *
     * @param board Board index
     * @param states Bitmask of channel states
     * @return true if write succeeded
     */
    bool writeBoard(uint8_t board, uint8_t states);

    /**
     * @brief Check if a channel activation is safe
     *
     * @param channel Global channel index
     * @return true if safe to activate
     *
     * Checks cooldown time and duty cycle limits.
     * Sets _lastError if not safe.
     */
    bool isSafeToActivate(uint8_t channel);

    /**
     * @brief Report an error
     *
     * @param error Error code
     * @param channel Channel involved (255 for global)
     *
     * Sets _lastError and calls error callback if set.
     */
    void reportError(SolenoidError error, uint8_t channel = 255);

    /**
     * @brief Convert global channel to board/local channel
     *
     * @param globalChannel Global channel index
     * @param board Output: board index
     * @param localChannel Output: local channel index (0-7)
     */
    void globalToLocal(uint8_t globalChannel, uint8_t& board, uint8_t& localChannel) const;

    /**
     * @brief Print debug message if debug is enabled
     *
     * @param msg Message to print
     */
    void debugPrint(const char* msg) const;

    /**
     * @brief Print debug message with channel number
     *
     * @param msg Message prefix
     * @param channel Channel number
     */
    void debugPrintChannel(const char* msg, uint8_t channel) const;

    /**
     * @brief Update channel state objects for all channels on a board
     *
     * @param board Board index
     * @param states Bitmask of channel states
     */
    void updateBoardChannelStates(uint8_t board, uint8_t states);
};

#endif // SOLENOID_DRIVER_H
