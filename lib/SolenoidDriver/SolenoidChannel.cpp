/**
 * @file SolenoidChannel.cpp
 * @brief Implementation of SolenoidChannel class
 *
 * @author Mechanical MIDI Piano Project
 * @version 1.0.0
 */

#include "SolenoidChannel.h"

SolenoidChannel::SolenoidChannel(uint8_t boardIndex, uint8_t channelIndex, uint8_t globalIndex)
    : _boardIndex(boardIndex)
    , _channelIndex(channelIndex)
    , _globalIndex(globalIndex)
    , _isOn(false)
    , _lastOnTime(0)
    , _lastOffTime(0)
    , _totalOnTime(0)
    , _activationCount(0)
    , _windowStartTime(0)
    , _windowOnTime(0)
{
}

bool SolenoidChannel::isOn() const {
    return _isOn;
}

uint32_t SolenoidChannel::onDuration() const {
    if (!_isOn) {
        return 0;
    }
    return millis() - _lastOnTime;
}

uint32_t SolenoidChannel::timeSinceOff() const {
    // If never turned off, return max value to indicate no cooldown needed
    if (_lastOffTime == 0) {
        return UINT32_MAX;
    }
    return millis() - _lastOffTime;
}

uint8_t SolenoidChannel::boardIndex() const {
    return _boardIndex;
}

uint8_t SolenoidChannel::channelIndex() const {
    return _channelIndex;
}

uint8_t SolenoidChannel::globalIndex() const {
    return _globalIndex;
}

uint32_t SolenoidChannel::totalOnTime() const {
    // If currently on, include the current on-duration in the total
    if (_isOn) {
        return _totalOnTime + onDuration();
    }
    return _totalOnTime;
}

uint32_t SolenoidChannel::activationCount() const {
    return _activationCount;
}

void SolenoidChannel::resetStats() {
    _totalOnTime = 0;
    _activationCount = 0;
    _windowStartTime = 0;
    _windowOnTime = 0;
}

void SolenoidChannel::updateState(bool isOn) {
    uint32_t now = millis();

    if (isOn && !_isOn) {
        // Turning on
        _lastOnTime = now;
        _activationCount++;
        _isOn = true;

        // Initialize window if this is the first activation
        if (_windowStartTime == 0) {
            _windowStartTime = now;
        }
    } else if (!isOn && _isOn) {
        // Turning off - accumulate the on-time
        uint32_t thisDuration = now - _lastOnTime;
        _totalOnTime += thisDuration;
        _windowOnTime += thisDuration;
        _lastOffTime = now;
        _lastOnTime = 0;
        _isOn = false;
    }
    // No change if state is already the same
}

void SolenoidChannel::updateWindow(uint32_t windowDurationMs, uint32_t now) {
    // Initialize window on first call
    if (_windowStartTime == 0) {
        _windowStartTime = now;
        _windowOnTime = 0;
        return;
    }

    uint32_t windowAge = now - _windowStartTime;

    // If window has expired, reset it
    if (windowAge >= windowDurationMs) {
        // If currently on, we need to track how much of the current activation
        // falls within the new window. Reset window to start now.
        _windowStartTime = now;
        _windowOnTime = 0;

        // If currently on, the ongoing activation will be counted when it ends
        // or when getDutyCyclePercent includes the current on-duration
    }
}

float SolenoidChannel::getDutyCyclePercent(uint32_t windowDurationMs) {
    if (windowDurationMs == 0) {
        return 0.0f;
    }

    uint32_t now = millis();

    // Update window state (may reset if expired)
    updateWindow(windowDurationMs, now);

    // Calculate window elapsed time
    uint32_t windowElapsed = now - _windowStartTime;
    if (windowElapsed == 0) {
        return 0.0f;
    }

    // Calculate on-time within window
    uint32_t onTimeInWindow = _windowOnTime;

    // If currently on, add the ongoing duration
    if (_isOn && _lastOnTime >= _windowStartTime) {
        onTimeInWindow += (now - _lastOnTime);
    } else if (_isOn && _lastOnTime < _windowStartTime) {
        // Channel was on before window started; only count time since window start
        onTimeInWindow += (now - _windowStartTime);
    }

    // Cap window elapsed to the configured duration for percentage calculation
    if (windowElapsed > windowDurationMs) {
        windowElapsed = windowDurationMs;
    }

    return static_cast<float>(onTimeInWindow) / static_cast<float>(windowElapsed);
}

bool SolenoidChannel::wouldExceedDutyCycle(uint32_t windowDurationMs, float maxDutyCycle, uint32_t estimatedOnTimeMs) const {
    if (windowDurationMs == 0 || maxDutyCycle >= 1.0f) {
        return false;  // No limit
    }

    uint32_t now = millis();

    // Calculate current on-time in window
    uint32_t windowElapsed = (_windowStartTime > 0) ? (now - _windowStartTime) : 0;
    uint32_t onTimeInWindow = _windowOnTime;

    // If currently on (shouldn't normally be when calling this, but handle it)
    if (_isOn && _lastOnTime >= _windowStartTime) {
        onTimeInWindow += (now - _lastOnTime);
    } else if (_isOn && _lastOnTime > 0 && _windowStartTime > 0 && _lastOnTime < _windowStartTime) {
        onTimeInWindow += (now - _windowStartTime);
    }

    // Project forward: if we activate for estimatedOnTimeMs
    uint32_t projectedOnTime = onTimeInWindow + estimatedOnTimeMs;
    uint32_t projectedWindowElapsed = windowElapsed + estimatedOnTimeMs;

    // Cap to window duration
    if (projectedWindowElapsed > windowDurationMs) {
        projectedWindowElapsed = windowDurationMs;
    }

    if (projectedWindowElapsed == 0) {
        return false;
    }

    float projectedDutyCycle = static_cast<float>(projectedOnTime) / static_cast<float>(projectedWindowElapsed);
    return projectedDutyCycle > maxDutyCycle;
}
