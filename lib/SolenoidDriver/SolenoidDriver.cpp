/**
 * @file SolenoidDriver.cpp
 * @brief Implementation of SolenoidDriver class
 *
 * This file contains the complete implementation of the SolenoidDriver class,
 * including I2C communication, safety features, and state management.
 *
 * @author Mechanical MIDI Piano Project
 * @version 1.0.0
 */

#include "SolenoidDriver.h"

// =============================================================================
// ERROR STRINGS (stored in PROGMEM to save RAM)
// =============================================================================

static const char STR_OK[] PROGMEM = "OK";
static const char STR_NOT_INIT[] PROGMEM = "Not initialized";
static const char STR_INV_CHAN[] PROGMEM = "Invalid channel";
static const char STR_INV_BOARD[] PROGMEM = "Invalid board";
static const char STR_I2C_ERR[] PROGMEM = "I2C communication error";
static const char STR_TIMEOUT[] PROGMEM = "Safety timeout";
static const char STR_COOLDOWN[] PROGMEM = "Safety cooldown";
static const char STR_DUTY[] PROGMEM = "Duty cycle exceeded";
static const char STR_BUSY[] PROGMEM = "Busy";
static const char STR_UNKNOWN[] PROGMEM = "Unknown error";

// =============================================================================
// CONSTRUCTOR / DESTRUCTOR
// =============================================================================

SolenoidDriver::SolenoidDriver()
    : _wire(nullptr)
    , _boardCount(0)
    , _channelCount(0)
    , _initialized(false)
    , _config()
    , _lastError(SolenoidError::OK)
    , _errorCallback(nullptr)
{
    // Initialize board states to all off
    for (uint8_t i = 0; i < SOLENOID_MAX_BOARDS_PER_BUS; i++) {
        _boardAddresses[i] = 0;
        _boardStates[i] = 0;
    }
}

SolenoidDriver::~SolenoidDriver() {
    // Ensure all solenoids are off when driver is destroyed
    if (_initialized) {
        emergencyStop();
    }
}

// =============================================================================
// INITIALIZATION
// =============================================================================

bool SolenoidDriver::begin(TwoWire& wire, uint8_t address) {
    // Single board initialization - delegate to multi-board version
    uint8_t addresses[] = { address };
    return begin(wire, addresses, 1);
}

bool SolenoidDriver::begin(TwoWire& wire, const uint8_t addresses[], uint8_t count) {
    // Validate parameters
    if (count == 0 || count > SOLENOID_MAX_BOARDS_PER_BUS) {
        reportError(SolenoidError::INVALID_BOARD);
        return false;
    }

    // Validate addresses
    for (uint8_t i = 0; i < count; i++) {
        if (addresses[i] < MCP23017_BASE_ADDRESS || addresses[i] > MCP23017_MAX_ADDRESS) {
            reportError(SolenoidError::INVALID_BOARD);
            return false;
        }
    }

    // Store wire reference
    _wire = &wire;

    // Set I2C clock speed
    _wire->setClock(_config.i2cClockHz);

    // Reset state
    _boardCount = 0;
    _channelCount = 0;
    _initialized = false;

    // Initialize each board
    for (uint8_t i = 0; i < count; i++) {
        uint8_t addr = addresses[i];

        if (_config.debugEnabled) {
            Serial.print(F("[SolenoidDriver] Initializing board at 0x"));
            Serial.println(addr, HEX);
        }

        // Initialize MCP23017
        if (!_mcp[i].begin_I2C(addr, _wire)) {
            debugPrint("Failed to initialize MCP23017");
            reportError(SolenoidError::I2C_COMMUNICATION);
            return false;
        }

        // Configure Port A as outputs (solenoid channels)
        for (uint8_t pin = 0; pin < 8; pin++) {
            _mcp[i].pinMode(pin, OUTPUT);
        }

        // Turn all channels off initially
        _mcp[i].writeGPIOA(0x00);

        // Store board info
        _boardAddresses[i] = addr;
        _boardStates[i] = 0x00;
        _boardCount++;

        // Initialize channel objects for this board
        for (uint8_t ch = 0; ch < SOLENOID_CHANNELS_PER_BOARD; ch++) {
            uint8_t globalIdx = (i * SOLENOID_CHANNELS_PER_BOARD) + ch;
            _channels[globalIdx] = SolenoidChannel(i, ch, globalIdx);
        }

        _channelCount = _boardCount * SOLENOID_CHANNELS_PER_BOARD;
    }

    _initialized = true;
    _lastError = SolenoidError::OK;

    debugPrint("SolenoidDriver initialized successfully");

    return true;
}

void SolenoidDriver::setConfig(const SolenoidConfig& config) {
    _config = config;

    // Apply I2C clock speed if already initialized
    if (_wire != nullptr) {
        _wire->setClock(_config.i2cClockHz);
    }
}

SolenoidConfig SolenoidDriver::getConfig() const {
    return _config;
}

// =============================================================================
// SINGLE CHANNEL CONTROL
// =============================================================================

SolenoidError SolenoidDriver::on(uint8_t channel) {
    // Check initialization
    if (!_initialized) {
        reportError(SolenoidError::NOT_INITIALIZED, channel);
        return _lastError;
    }

    // Validate channel
    if (channel >= _channelCount) {
        reportError(SolenoidError::INVALID_CHANNEL, channel);
        return _lastError;
    }

    // Check if already on (no-op)
    if (_channels[channel].isOn()) {
        _lastError = SolenoidError::OK;
        return _lastError;
    }

    // Safety check
    if (_config.safetyEnabled && !isSafeToActivate(channel)) {
        // Error already set by isSafeToActivate
        return _lastError;
    }

    // Convert to board/local channel
    uint8_t board, localChannel;
    globalToLocal(channel, board, localChannel);

    // Write to hardware
    if (!writeChannel(board, localChannel, true)) {
        reportError(SolenoidError::I2C_COMMUNICATION, channel);
        return _lastError;
    }

    // Update state tracking
    _channels[channel].updateState(true);

    _lastError = SolenoidError::OK;
    return _lastError;
}

SolenoidError SolenoidDriver::off(uint8_t channel) {
    // Check initialization
    if (!_initialized) {
        reportError(SolenoidError::NOT_INITIALIZED, channel);
        return _lastError;
    }

    // Validate channel
    if (channel >= _channelCount) {
        reportError(SolenoidError::INVALID_CHANNEL, channel);
        return _lastError;
    }

    // Check if already off (no-op)
    if (!_channels[channel].isOn()) {
        _lastError = SolenoidError::OK;
        return _lastError;
    }

    // Convert to board/local channel
    uint8_t board, localChannel;
    globalToLocal(channel, board, localChannel);

    // Write to hardware
    if (!writeChannel(board, localChannel, false)) {
        reportError(SolenoidError::I2C_COMMUNICATION, channel);
        return _lastError;
    }

    // Update state tracking
    _channels[channel].updateState(false);

    _lastError = SolenoidError::OK;
    return _lastError;
}

SolenoidError SolenoidDriver::set(uint8_t channel, bool state) {
    return state ? on(channel) : off(channel);
}

SolenoidError SolenoidDriver::toggle(uint8_t channel) {
    // Validate channel first
    if (channel >= _channelCount) {
        reportError(SolenoidError::INVALID_CHANNEL, channel);
        return _lastError;
    }

    return set(channel, !_channels[channel].isOn());
}

SolenoidError SolenoidDriver::pulse(uint8_t channel, uint32_t durationMs) {
    // Clamp duration to max on-time
    if (_config.maxOnTimeMs > 0 && durationMs > _config.maxOnTimeMs) {
        durationMs = _config.maxOnTimeMs;
        debugPrint("Pulse duration clamped to maxOnTimeMs");
    }

    // Turn on
    SolenoidError err = on(channel);
    if (err != SolenoidError::OK) {
        return err;
    }

    // Wait (blocking)
    delay(durationMs);

    // Turn off
    return off(channel);
}

// =============================================================================
// MULTI-CHANNEL CONTROL
// =============================================================================

SolenoidError SolenoidDriver::allOn() {
    if (!_initialized) {
        reportError(SolenoidError::NOT_INITIALIZED);
        return _lastError;
    }

    uint8_t failedCount = 0;
    SolenoidError firstError = SolenoidError::OK;

    // Turn on each channel individually (respects safety checks)
    for (uint8_t ch = 0; ch < _channelCount; ch++) {
        SolenoidError err = on(ch);
        if (err != SolenoidError::OK) {
            failedCount++;
            // Store the first error encountered
            if (firstError == SolenoidError::OK) {
                firstError = err;
            }
            // I2C errors are critical - stop immediately
            if (err == SolenoidError::I2C_COMMUNICATION) {
                debugPrint("allOn: I2C error, aborting");
                return err;
            }
        }
    }

    // Report if any channels failed
    if (failedCount > 0) {
        if (_config.debugEnabled) {
            Serial.print(F("[SolenoidDriver] allOn: "));
            Serial.print(failedCount);
            Serial.println(F(" channel(s) failed safety checks"));
        }
        // Return the first error encountered (already set in _lastError by on())
        return firstError;
    }

    _lastError = SolenoidError::OK;
    return _lastError;
}

SolenoidError SolenoidDriver::allOff() {
    if (!_initialized) {
        reportError(SolenoidError::NOT_INITIALIZED);
        return _lastError;
    }

    // Turn off each board
    for (uint8_t board = 0; board < _boardCount; board++) {
        if (!writeBoard(board, 0x00)) {
            reportError(SolenoidError::I2C_COMMUNICATION);
            return _lastError;
        }

        // Update channel states
        for (uint8_t ch = 0; ch < SOLENOID_CHANNELS_PER_BOARD; ch++) {
            uint8_t globalCh = (board * SOLENOID_CHANNELS_PER_BOARD) + ch;
            _channels[globalCh].updateState(false);
        }
    }

    _lastError = SolenoidError::OK;
    return _lastError;
}

SolenoidError SolenoidDriver::setAll(const uint8_t states[], uint8_t stateCount) {
    if (!_initialized) {
        reportError(SolenoidError::NOT_INITIALIZED);
        return _lastError;
    }

    // Validate that the states array has enough elements
    if (stateCount < _boardCount) {
        debugPrint("setAll: stateCount less than board count");
        reportError(SolenoidError::INVALID_BOARD);
        return _lastError;
    }

    // Track if any board had blocked channels
    bool anyBlocked = false;

    // Apply each board's states
    for (uint8_t board = 0; board < _boardCount; board++) {
        SolenoidError err = setBoardChannels(board, states[board]);
        if (err == SolenoidError::I2C_COMMUNICATION) {
            // I2C errors are critical - stop immediately
            return err;
        } else if (err != SolenoidError::OK) {
            // Safety-related errors (cooldown, duty cycle) - track but continue
            anyBlocked = true;
        }
    }

    // If any channels were blocked, _lastError already contains the error
    if (anyBlocked) {
        return _lastError;
    }

    _lastError = SolenoidError::OK;
    return _lastError;
}

SolenoidError SolenoidDriver::setBoardChannels(uint8_t board, uint8_t states) {
    if (!_initialized) {
        reportError(SolenoidError::NOT_INITIALIZED);
        return _lastError;
    }

    if (board >= _boardCount) {
        reportError(SolenoidError::INVALID_BOARD);
        return _lastError;
    }

    // Get current state
    uint8_t currentStates = _boardStates[board];
    uint8_t blockedChannels = 0;  // Track which channels were blocked by safety

    // Check safety for each channel that is being turned on
    if (_config.safetyEnabled) {
        for (uint8_t ch = 0; ch < SOLENOID_CHANNELS_PER_BOARD; ch++) {
            bool wasOn = (currentStates >> ch) & 0x01;
            bool willBeOn = (states >> ch) & 0x01;

            if (willBeOn && !wasOn) {
                uint8_t globalCh = (board * SOLENOID_CHANNELS_PER_BOARD) + ch;
                if (!isSafeToActivate(globalCh)) {
                    // Clear this bit to prevent activation and track it
                    states &= ~(1 << ch);
                    blockedChannels |= (1 << ch);
                }
            }
        }
    }

    // Write to hardware
    if (!writeBoard(board, states)) {
        reportError(SolenoidError::I2C_COMMUNICATION);
        return _lastError;
    }

    // Update channel states
    for (uint8_t ch = 0; ch < SOLENOID_CHANNELS_PER_BOARD; ch++) {
        uint8_t globalCh = (board * SOLENOID_CHANNELS_PER_BOARD) + ch;
        bool newState = (states >> ch) & 0x01;
        _channels[globalCh].updateState(newState);
    }

    // If any channels were blocked, report the error but still return success
    // for the channels that were activated. The _lastError will indicate
    // that some channels were blocked.
    if (blockedChannels != 0) {
        // Report the first blocked channel (the specific error was already
        // reported by isSafeToActivate, but we want to indicate partial failure)
        for (uint8_t ch = 0; ch < SOLENOID_CHANNELS_PER_BOARD; ch++) {
            if ((blockedChannels >> ch) & 0x01) {
                uint8_t globalCh = (board * SOLENOID_CHANNELS_PER_BOARD) + ch;
                debugPrintChannel("Channel blocked by safety: ", globalCh);
                break;
            }
        }
        // Return the last error set by isSafeToActivate (SAFETY_COOLDOWN or DUTY_CYCLE_EXCEEDED)
        return _lastError;
    }

    _lastError = SolenoidError::OK;
    return _lastError;
}

// =============================================================================
// STATE QUERIES
// =============================================================================

bool SolenoidDriver::isOn(uint8_t channel) const {
    if (channel >= _channelCount) {
        return false;
    }
    return _channels[channel].isOn();
}

const SolenoidChannel* SolenoidDriver::getChannelState(uint8_t channel) const {
    if (channel >= _channelCount) {
        return nullptr;
    }
    return &_channels[channel];
}

uint8_t SolenoidDriver::getBoardState(uint8_t board) const {
    if (board >= _boardCount) {
        return 0;
    }
    return _boardStates[board];
}

// =============================================================================
// SAFETY AND MAINTENANCE
// =============================================================================

void SolenoidDriver::update() {
    if (!_initialized) {
        return;
    }

    uint32_t now = millis();

    // Check each channel for timeout
    for (uint8_t ch = 0; ch < _channelCount; ch++) {
        if (_channels[ch].isOn()) {
            // Check if on-time exceeded
            if (_config.maxOnTimeMs > 0) {
                uint32_t onDuration = _channels[ch].onDuration();
                if (onDuration >= _config.maxOnTimeMs) {
                    // Auto-shutoff
                    debugPrintChannel("Safety timeout on channel ", ch);

                    uint8_t board, localChannel;
                    globalToLocal(ch, board, localChannel);
                    writeChannel(board, localChannel, false);
                    _channels[ch].updateState(false);

                    reportError(SolenoidError::SAFETY_TIMEOUT, ch);
                }
            }
        }
    }
}

void SolenoidDriver::emergencyStop() {
    // Bypass all checks - write directly to hardware
    for (uint8_t board = 0; board < _boardCount; board++) {
        _mcp[board].writeGPIOA(0x00);
        _boardStates[board] = 0x00;
    }

    // Update all channel states
    for (uint8_t ch = 0; ch < _channelCount; ch++) {
        _channels[ch].updateState(false);
    }

    debugPrint("Emergency stop - all channels off");
}

void SolenoidDriver::resetAllStats() {
    for (uint8_t ch = 0; ch < _channelCount; ch++) {
        _channels[ch].resetStats();
    }

    debugPrint("All channel stats reset");
}

// =============================================================================
// ERROR HANDLING
// =============================================================================

bool SolenoidDriver::isInitialized() const {
    return _initialized;
}

SolenoidError SolenoidDriver::getLastError() const {
    return _lastError;
}

const char* SolenoidDriver::getErrorString(SolenoidError error) {
    switch (error) {
        case SolenoidError::OK:
            return STR_OK;
        case SolenoidError::NOT_INITIALIZED:
            return STR_NOT_INIT;
        case SolenoidError::INVALID_CHANNEL:
            return STR_INV_CHAN;
        case SolenoidError::INVALID_BOARD:
            return STR_INV_BOARD;
        case SolenoidError::I2C_COMMUNICATION:
            return STR_I2C_ERR;
        case SolenoidError::SAFETY_TIMEOUT:
            return STR_TIMEOUT;
        case SolenoidError::SAFETY_COOLDOWN:
            return STR_COOLDOWN;
        case SolenoidError::DUTY_CYCLE_EXCEEDED:
            return STR_DUTY;
        case SolenoidError::BUSY:
            return STR_BUSY;
        default:
            return STR_UNKNOWN;
    }
}

void SolenoidDriver::setErrorCallback(SolenoidErrorCallback callback) {
    _errorCallback = callback;
}

// =============================================================================
// DIAGNOSTICS
// =============================================================================

uint8_t SolenoidDriver::getBoardCount() const {
    return _boardCount;
}

uint8_t SolenoidDriver::getChannelCount() const {
    return _channelCount;
}

uint8_t SolenoidDriver::scanI2C() {
    if (_wire == nullptr) {
        return 0;
    }

    uint8_t count = 0;

    for (uint8_t addr = MCP23017_BASE_ADDRESS; addr <= MCP23017_MAX_ADDRESS; addr++) {
        _wire->beginTransmission(addr);
        uint8_t error = _wire->endTransmission();

        if (error == 0) {
            count++;
            if (_config.debugEnabled) {
                Serial.print(F("Found device at 0x"));
                Serial.println(addr, HEX);
            }
        }
    }

    return count;
}

uint8_t SolenoidDriver::getBoardAddress(uint8_t board) const {
    if (board >= _boardCount) {
        return 0;
    }
    return _boardAddresses[board];
}

// =============================================================================
// PRIVATE METHODS
// =============================================================================

bool SolenoidDriver::writeChannel(uint8_t board, uint8_t channel, bool state) {
    if (board >= _boardCount || channel >= SOLENOID_CHANNELS_PER_BOARD) {
        return false;
    }

    // Update local state cache
    if (state) {
        _boardStates[board] |= (1 << channel);
    } else {
        _boardStates[board] &= ~(1 << channel);
    }

    // Write single pin (the library handles I2C internally)
    _mcp[board].digitalWrite(channel, state ? HIGH : LOW);

    return true;
}

bool SolenoidDriver::writeBoard(uint8_t board, uint8_t states) {
    if (board >= _boardCount) {
        return false;
    }

    // Update local state cache
    _boardStates[board] = states;

    // Write full port (more efficient than individual pins)
    _mcp[board].writeGPIOA(states);

    return true;
}

bool SolenoidDriver::isSafeToActivate(uint8_t channel) {
    if (channel >= _channelCount) {
        return false;
    }

    SolenoidChannel& ch = _channels[channel];

    // Check cooldown time
    if (_config.minOffTimeMs > 0) {
        uint32_t timeSinceOff = ch.timeSinceOff();
        if (timeSinceOff < _config.minOffTimeMs) {
            debugPrintChannel("Cooldown not elapsed for channel ", channel);
            reportError(SolenoidError::SAFETY_COOLDOWN, channel);
            return false;
        }
    }

    // Check duty cycle using rolling window
    if (_config.maxDutyCycle < 1.0f && _config.dutyCycleWindowMs > 0) {
        // Estimate that the solenoid will be on for at least minOffTimeMs
        // (a conservative estimate for duty cycle projection)
        uint32_t estimatedOnTime = _config.minOffTimeMs > 0 ? _config.minOffTimeMs : 100;

        // Check current duty cycle in the window
        float currentDutyCycle = ch.getDutyCyclePercent(_config.dutyCycleWindowMs);
        if (currentDutyCycle >= _config.maxDutyCycle) {
            debugPrintChannel("Duty cycle exceeded for channel ", channel);
            reportError(SolenoidError::DUTY_CYCLE_EXCEEDED, channel);
            return false;
        }

        // Also check if activating would exceed the limit
        if (ch.wouldExceedDutyCycle(_config.dutyCycleWindowMs, _config.maxDutyCycle, estimatedOnTime)) {
            debugPrintChannel("Activation would exceed duty cycle for channel ", channel);
            reportError(SolenoidError::DUTY_CYCLE_EXCEEDED, channel);
            return false;
        }
    }

    return true;
}

void SolenoidDriver::reportError(SolenoidError error, uint8_t channel) {
    _lastError = error;

    if (_errorCallback != nullptr) {
        _errorCallback(error, channel);
    }

    if (_config.debugEnabled) {
        Serial.print(F("[SolenoidDriver] Error: "));
        Serial.print(getErrorString(error));
        if (channel != 255) {
            Serial.print(F(" on channel "));
            Serial.print(channel);
        }
        Serial.println();
    }
}

void SolenoidDriver::globalToLocal(uint8_t globalChannel, uint8_t& board, uint8_t& localChannel) const {
    board = globalChannel / SOLENOID_CHANNELS_PER_BOARD;
    localChannel = globalChannel % SOLENOID_CHANNELS_PER_BOARD;
}

void SolenoidDriver::debugPrint(const char* msg) const {
    if (_config.debugEnabled) {
        Serial.print(F("[SolenoidDriver] "));
        Serial.println(msg);
    }
}

void SolenoidDriver::debugPrintChannel(const char* msg, uint8_t channel) const {
    if (_config.debugEnabled) {
        Serial.print(F("[SolenoidDriver] "));
        Serial.print(msg);
        Serial.println(channel);
    }
}
