/**
 * @file SolenoidChannel.h
 * @brief Individual solenoid channel state tracking
 *
 * This class tracks the state, timing, and statistics of a single solenoid
 * channel. It is used internally by SolenoidDriver for safety enforcement
 * and diagnostics.
 *
 * @author Mechanical MIDI Piano Project
 * @version 1.0.0
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
 * Each channel tracks:
 * - Current on/off state
 * - Time when last turned on (for timeout detection)
 * - Time when last turned off (for cooldown enforcement)
 * - Rolling window duty cycle calculation
 * - Activation count (for statistics)
 *
 * Duty Cycle Tracking:
 * The duty cycle is calculated over a configurable rolling window (default 10 seconds).
 * When the window expires, it resets and begins tracking again. This provides
 * protection against sustained high-duty-cycle operation that could overheat solenoids.
 *
 * Example usage (internal to SolenoidDriver):
 * @code
 * SolenoidChannel channel(0, 3, 3);  // Board 0, channel 3, global index 3
 * channel.updateState(true);          // Turn on
 * if (channel.onDuration() > maxTime) {
 *     // Timeout - need to turn off
 * }
 * if (channel.getDutyCyclePercent(10000) > 0.5f) {
 *     // Duty cycle exceeded 50% in the last 10 seconds
 * }
 * @endcode
 */
class SolenoidChannel {
public:
    /**
     * @brief Construct a new SolenoidChannel object
     *
     * @param boardIndex Index of the driver board (0-15)
     * @param channelIndex Index of the channel on the board (0-7)
     * @param globalIndex Global channel index across all boards (0-127)
     *
     * All timing values are initialized to 0, and the channel starts in the
     * off state.
     */
    SolenoidChannel(uint8_t boardIndex = 0, uint8_t channelIndex = 0, uint8_t globalIndex = 0);

    /**
     * @brief Check if the channel is currently on
     *
     * @return true if solenoid is activated
     * @return false if solenoid is off
     */
    bool isOn() const;

    /**
     * @brief Get duration the channel has been on (if currently on)
     *
     * @return Duration in milliseconds since the channel was turned on,
     *         or 0 if the channel is currently off
     *
     * This is calculated as the difference between now and the last on time.
     */
    uint32_t onDuration() const;

    /**
     * @brief Get time since channel was turned off
     *
     * @return Duration in milliseconds since last off,
     *         or UINT32_MAX if never turned off
     *
     * Used for cooldown enforcement. If the channel has never been turned off
     * (never activated), returns UINT32_MAX to indicate cooldown is not needed.
     */
    uint32_t timeSinceOff() const;

    /**
     * @brief Get the board index
     *
     * @return Board index (0-15)
     */
    uint8_t boardIndex() const;

    /**
     * @brief Get the channel index on the board
     *
     * @return Channel index (0-7)
     */
    uint8_t channelIndex() const;

    /**
     * @brief Get the global channel index
     *
     * @return Global index (0-127)
     */
    uint8_t globalIndex() const;

    /**
     * @brief Get total on-time for statistics
     *
     * @return Total milliseconds the channel has been on since last reset
     *
     * This accumulates the total time the channel has spent in the on state.
     * Use resetStats() to clear this value.
     *
     * @note For duty cycle enforcement, use getDutyCyclePercent() instead,
     *       which uses the rolling window calculation.
     */
    uint32_t totalOnTime() const;

    /**
     * @brief Get the current duty cycle percentage within the rolling window
     *
     * @param windowDurationMs The duration of the rolling window in milliseconds
     * @return Duty cycle as a value between 0.0 and 1.0
     *
     * Calculates the percentage of time the channel has been on within the
     * specified rolling window. If the window has expired (more time has passed
     * than windowDurationMs since the window started), the window is reset
     * and tracking begins fresh.
     *
     * If the channel is currently on, the calculation includes the ongoing
     * on-duration up to the current moment.
     */
    float getDutyCyclePercent(uint32_t windowDurationMs);

    /**
     * @brief Check if activating would exceed the duty cycle limit
     *
     * @param windowDurationMs The duration of the rolling window in milliseconds
     * @param maxDutyCycle Maximum allowed duty cycle (0.0 to 1.0)
     * @param estimatedOnTimeMs Estimated duration the channel will be on
     * @return true if activation would exceed duty cycle, false if safe
     *
     * This method estimates whether turning on the channel would cause
     * the duty cycle to exceed the limit, assuming the channel stays on
     * for the estimated duration.
     */
    bool wouldExceedDutyCycle(uint32_t windowDurationMs, float maxDutyCycle, uint32_t estimatedOnTimeMs) const;

    /**
     * @brief Get the total number of activations
     *
     * @return Number of times the channel has been turned on since last reset
     */
    uint32_t activationCount() const;

    /**
     * @brief Reset accumulated statistics
     *
     * Clears totalOnTime and activationCount. Does not affect current state
     * or timing information.
     */
    void resetStats();

    /**
     * @brief Update the channel state (called internally by SolenoidDriver)
     *
     * @param isOn New state (true = on, false = off)
     *
     * This method updates the internal state and timing information:
     * - When turning on: Records the on-time and increments activation count
     * - When turning off: Records the off-time and accumulates on-duration
     *
     * This should only be called by SolenoidDriver after successfully
     * writing the state to hardware.
     */
    void updateState(bool isOn);

private:
    uint8_t _boardIndex;           ///< Board index (0-15)
    uint8_t _channelIndex;         ///< Channel on board (0-7)
    uint8_t _globalIndex;          ///< Global channel index
    bool _isOn;                    ///< Current state
    uint32_t _lastOnTime;          ///< millis() when last turned on
    uint32_t _lastOffTime;         ///< millis() when last turned off
    uint32_t _totalOnTime;         ///< Accumulated on-time for statistics
    uint32_t _activationCount;     ///< Number of activations

    // Rolling window duty cycle tracking
    uint32_t _windowStartTime;     ///< millis() when duty cycle window started
    uint32_t _windowOnTime;        ///< Accumulated on-time within current window

    /**
     * @brief Update the rolling window, resetting if expired
     *
     * @param windowDurationMs Duration of the rolling window
     * @param now Current time from millis()
     *
     * If more time has passed than windowDurationMs since _windowStartTime,
     * the window is reset. Any partial on-time from a currently-on channel
     * is preserved when the window resets.
     */
    void updateWindow(uint32_t windowDurationMs, uint32_t now);
};

#endif // SOLENOID_CHANNEL_H
