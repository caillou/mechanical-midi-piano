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
 *   6. SOS morse code solenoid test
 *
 * Serial Commands:
 *   'r' - Re-run all tests
 *   'a' - Activate all channels simultaneously
 *   's' - Run I2C scanner only
 *   '0'-'7' - Toggle individual channel
 *   'o' - Run single SOS sequence on Channel 0
 *   'c' - Toggle continuous SOS mode
 *   'x' - Emergency stop (all off + stop SOS)
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
 * @defgroup SOSConfig SOS Morse Code Configuration
 * @{
 */

/** SOS test channel (GPA0) */
constexpr uint8_t SOS_CHANNEL = 0;

/** Dit (dot) duration in milliseconds - 1 unit */
constexpr uint32_t SOS_DIT_MS = 100;

/** Dah (dash) duration in milliseconds - 3 units */
constexpr uint32_t SOS_DAH_MS = 300;

/** Gap between elements (dits/dahs) within a letter - 1 unit */
constexpr uint32_t SOS_ELEMENT_GAP_MS = 100;

/** Gap between letters - 3 units (actually 2 additional after element gap) */
constexpr uint32_t SOS_LETTER_GAP_MS = 300;

/** Gap between words/SOS repetitions - 7 units */
constexpr uint32_t SOS_WORD_GAP_MS = 700;

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

/** Flag for continuous SOS mode */
volatile bool sosRunning = false;

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

// SOS Morse Code Functions
void playDit();
void playDah();
void playS();
void playO();
void playSOS();
void stopSOS();

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

    // Continuous SOS mode
    if (sosRunning && mcpInitialized) {
        playSOS();
        if (sosRunning) {  // Check again in case stopped during playSOS
            delay(SOS_WORD_GAP_MS);  // Gap between repetitions
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
 *
 * Includes safety checks for minimum off-time when turning channels on.
 */
bool setAllChannels(uint8_t states) {
    if (!mcpInitialized) {
        Serial.println(F("[ERROR] MCP23017 not initialized"));
        return false;
    }

    // Check safety for all channels being turned on
    for (uint8_t i = 0; i < NUM_CHANNELS; i++) {
        bool newState = (states >> i) & 0x01;
        bool oldState = (channelStates >> i) & 0x01;

        if (newState && !oldState) {
            if (!isChannelSafe(i, true)) {
                return false;  // Safety check failed
            }
        }
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
// SOS MORSE CODE FUNCTIONS
// =============================================================================

/**
 * @brief Play a single dit (dot) - short pulse
 *
 * Activates SOS_CHANNEL for SOS_DIT_MS (100ms).
 * This is a blocking function.
 */
void playDit() {
    if (!mcpInitialized || !sosRunning) return;

    digitalWrite(LED_PIN, HIGH);
    mcp.digitalWrite(SOS_CHANNEL, HIGH);
    delay(SOS_DIT_MS);
    mcp.digitalWrite(SOS_CHANNEL, LOW);
    digitalWrite(LED_PIN, LOW);
}

/**
 * @brief Play a single dah (dash) - long pulse
 *
 * Activates SOS_CHANNEL for SOS_DAH_MS (300ms).
 * This is a blocking function.
 */
void playDah() {
    if (!mcpInitialized || !sosRunning) return;

    digitalWrite(LED_PIN, HIGH);
    mcp.digitalWrite(SOS_CHANNEL, HIGH);
    delay(SOS_DAH_MS);
    mcp.digitalWrite(SOS_CHANNEL, LOW);
    digitalWrite(LED_PIN, LOW);
}

/**
 * @brief Play the letter 'S' in morse code (. . .)
 *
 * Three dits with element gaps between them.
 * Pattern: dit-gap-dit-gap-dit
 */
void playS() {
    if (!sosRunning) return;

    // First dit
    playDit();
    if (!sosRunning) return;
    delay(SOS_ELEMENT_GAP_MS);

    // Second dit
    playDit();
    if (!sosRunning) return;
    delay(SOS_ELEMENT_GAP_MS);

    // Third dit
    playDit();
}

/**
 * @brief Play the letter 'O' in morse code (- - -)
 *
 * Three dahs with element gaps between them.
 * Pattern: dah-gap-dah-gap-dah
 */
void playO() {
    if (!sosRunning) return;

    // First dah
    playDah();
    if (!sosRunning) return;
    delay(SOS_ELEMENT_GAP_MS);

    // Second dah
    playDah();
    if (!sosRunning) return;
    delay(SOS_ELEMENT_GAP_MS);

    // Third dah
    playDah();
}

/**
 * @brief Play complete SOS sequence
 *
 * Plays S-O-S with proper letter gaps.
 * Pattern: S (letter gap) O (letter gap) S
 *
 * Timing breakdown:
 *   S = dit(100) + gap(100) + dit(100) + gap(100) + dit(100) = 500ms
 *   Letter gap = 300ms (but 100ms of element gap consumed) = 200ms additional
 *   O = dah(300) + gap(100) + dah(300) + gap(100) + dah(300) = 1100ms
 *   Letter gap = 200ms additional
 *   S = 500ms
 *   Total active time: ~2700ms
 */
void playSOS() {
    if (!mcpInitialized) {
        Serial.println(F("[ERROR] Cannot play SOS - MCP23017 not initialized"));
        return;
    }

    Serial.println(F("Playing SOS: ... --- ..."));

    // Ensure channel starts OFF
    mcp.digitalWrite(SOS_CHANNEL, LOW);

    // Play 'S' (. . .)
    playS();
    if (!sosRunning) {
        mcp.digitalWrite(SOS_CHANNEL, LOW);
        return;
    }

    // Letter gap between S and O (300ms total, minus element gap already taken)
    delay(SOS_LETTER_GAP_MS - SOS_ELEMENT_GAP_MS);
    if (!sosRunning) {
        mcp.digitalWrite(SOS_CHANNEL, LOW);
        return;
    }

    // Play 'O' (- - -)
    playO();
    if (!sosRunning) {
        mcp.digitalWrite(SOS_CHANNEL, LOW);
        return;
    }

    // Letter gap between O and S
    delay(SOS_LETTER_GAP_MS - SOS_ELEMENT_GAP_MS);
    if (!sosRunning) {
        mcp.digitalWrite(SOS_CHANNEL, LOW);
        return;
    }

    // Play 'S' (. . .)
    playS();

    // Ensure channel ends OFF
    mcp.digitalWrite(SOS_CHANNEL, LOW);
    digitalWrite(LED_PIN, LOW);

    Serial.println(F("SOS complete."));
}

/**
 * @brief Stop SOS playback immediately
 *
 * Sets sosRunning flag to false and ensures channel is OFF.
 * Called by emergency stop and continuous mode toggle.
 */
void stopSOS() {
    sosRunning = false;
    if (mcpInitialized) {
        mcp.digitalWrite(SOS_CHANNEL, LOW);
    }
    digitalWrite(LED_PIN, LOW);
    Serial.println(F("[OK] SOS stopped"));
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
    Serial.println(F("  'o' - Run single SOS sequence (Channel 0)"));
    Serial.println(F("  'c' - Toggle continuous SOS mode"));
    Serial.println(F("  'x' - Emergency stop (all off + stop SOS)"));
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

            case 'o':
            case 'O':
                if (!sosRunning) {
                    Serial.println(F("Running single SOS sequence..."));
                    sosRunning = true;  // Enable for this sequence
                    playSOS();
                    sosRunning = false;  // Disable after completion
                } else {
                    Serial.println(F("[INFO] SOS already running"));
                }
                break;

            case 'c':
            case 'C':
                if (sosRunning) {
                    Serial.println(F("Stopping continuous SOS mode..."));
                    stopSOS();
                } else {
                    Serial.println(F("Starting continuous SOS mode..."));
                    Serial.println(F("Press 'x' or 'c' to stop."));
                    sosRunning = true;
                }
                break;

            case 'x':
            case 'X':
                Serial.println(F("EMERGENCY STOP"));
                stopSOS();  // Stop SOS first
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

