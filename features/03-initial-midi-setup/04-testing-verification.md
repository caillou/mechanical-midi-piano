# Testing and Verification

## Overview

This document describes how to test the USB MIDI implementation after completing the changes outlined in the implementation plan.

---

## Pre-Implementation Checklist

Before starting, verify:

- [ ] Teensy 4.1 connected via USB
- [ ] Adafruit I2C Solenoid Driver connected (SDA=18, SCL=19)
- [ ] MCP23017 at address 0x20 (default)
- [ ] PlatformIO installed and working
- [ ] Current code compiles and uploads successfully

---

## Build Verification

### Step 1: Clean Build

After making changes, perform a clean build:

```bash
cd /Users/caillou/repos/caillou/mechanical-midi-piano
pio run --target clean
pio run
```

### Expected Output

```
Processing teensy41 (platform: teensy; board: teensy41; framework: arduino)
...
Compiling .pio/build/teensy41/src/main.cpp.o
...
Building .pio/build/teensy41/firmware.hex
Memory Usage -> https://bit.ly/pio-memory-usage
DATA:    [=         ]   6.2% (used 32xxxB from 524288B)
PROGRAM: [          ]   1.x% (used xxxxxB from 2031616B)
========================================== [SUCCESS] ...
```

### Common Build Errors

| Error | Cause | Solution |
|-------|-------|----------|
| `'usbMIDI' was not declared` | USB_MIDI_SERIAL not set | Check platformio.ini build_flags |
| `'SolenoidDriver' was not declared` | Include path issue | Check lib/SolenoidDriver exists |
| `undefined reference to 'handleNoteOn'` | Function signature mismatch | Ensure `byte` type used in parameters |

---

## Upload and Initial Test

### Step 2: Upload Firmware

```bash
pio run --target upload
```

### Step 3: Open Serial Monitor

```bash
pio device monitor
```

Or use the PlatformIO Serial Monitor in VS Code.

### Expected Serial Output

```
============================================================
MECHANICAL MIDI PIANO - USB MIDI CONTROLLER
Teensy 4.1 + Adafruit I2C Solenoid Driver
============================================================

Initializing I2C bus...
  SDA Pin: 18, SCL Pin: 19, Speed: 400 kHz
[OK] I2C bus initialized

Initializing MCP23017 at address 0x20...
  SolenoidDriver initialized, all channels OFF
[OK] MCP23017 initialized successfully
[OK] MIDI handlers registered
  Listening for notes 60-67 (C4-G4)

SERIAL COMMANDS:
  'x' - Emergency stop (all solenoids off)
  's' - Print status
  'h' - Show this help menu

MIDI: Listening for notes 60-67 (C4-G4) on all channels

Ready for MIDI input...
```

---

## USB Device Verification

### Step 4: Verify MIDI Device Appears

#### macOS

1. Open "Audio MIDI Setup" (Applications > Utilities)
2. Window > Show MIDI Studio
3. Look for "Teensy MIDI" device

Or use Terminal:
```bash
system_profiler SPUSBDataType | grep -A 10 "Teensy"
```

#### Windows

1. Open Device Manager
2. Expand "Sound, video and game controllers"
3. Look for "Teensy MIDI"

Also check:
1. Settings > Devices > Bluetooth & other devices
2. Look under "Other devices" or "Audio devices"

#### Linux

```bash
# List MIDI devices
aconnect -l

# Should show something like:
# client 20: 'Teensy MIDI' [type=kernel,card=1]
#     0 'Teensy MIDI MIDI 1'

# Alternative: check ALSA
cat /proc/asound/cards
```

---

## MIDI Functionality Testing

### Step 5: Test with MIDI Software

#### Option A: MIDI Monitor (macOS)

1. Download MIDI Monitor from https://www.snoize.com/midimonitor/
2. Open MIDI Monitor
3. Select "Teensy MIDI" as input source
4. Use a MIDI keyboard or software to send notes
5. Verify Note On/Off messages appear

#### Option B: MIDI-OX (Windows)

1. Download MIDI-OX from http://www.midiox.com/
2. Options > MIDI Devices
3. Select "Teensy MIDI" as output
4. Use the keyboard window to send notes

#### Option C: Virtual MIDI Keyboard

**macOS - MIDIKeys:**
1. Download MIDIKeys from https://flit.github.io/projects/midikeys/
2. Select "Teensy MIDI" as destination
3. Press keys C4 through G4

**Cross-platform - VMPK:**
1. Download Virtual MIDI Piano Keyboard (VMPK)
2. Edit > Connections > MIDI OUT
3. Select "Teensy MIDI"
4. Click keys 60-67 on the virtual keyboard

#### Option D: DAW Software

Any DAW (Ableton, Logic, Reaper, etc.):
1. Create a new MIDI track
2. Set output to "Teensy MIDI"
3. Draw notes 60-67 in the piano roll
4. Play the sequence

### Step 6: Verify Solenoid Response

With MIDI notes being sent:

1. **Note On (60-67):** Corresponding solenoid should activate
2. **Note Off:** Solenoid should deactivate
3. **Notes outside range (< 60 or > 67):** No response

Watch the Serial Monitor for any error messages.

---

## Serial Command Testing

### Step 7: Test Emergency Stop

1. Send MIDI Note On for note 60 (solenoid 0 activates)
2. In Serial Monitor, type `x` and press Enter
3. Verify all solenoids turn off
4. Serial shows: `EMERGENCY STOP` and `[OK] All channels deactivated`

### Step 8: Test Status Command

1. In Serial Monitor, type `s` and press Enter
2. Verify output shows:

```
============================================================
STATUS
============================================================
Driver initialized: Yes
Boards: 1
Channels: 8
Channel states:
  Ch 0 (Note 60): off
  Ch 1 (Note 61): off
  Ch 2 (Note 62): off
  Ch 3 (Note 63): off
  Ch 4 (Note 64): off
  Ch 5 (Note 65): off
  Ch 6 (Note 66): off
  Ch 7 (Note 67): off
============================================================
```

### Step 9: Test Help Command

1. In Serial Monitor, type `h` and press Enter
2. Verify help menu displays correctly

---

## Safety Testing

### Step 10: Test Auto-Shutoff

1. Send Note On for note 60
2. Do NOT send Note Off
3. Wait 2 seconds (MAX_ON_TIME_MS)
4. Solenoid should auto-shutoff
5. Serial may show safety timeout message (if debug enabled)

### Step 11: Test Rapid Triggers

1. Send rapid Note On/Off messages for the same note
2. If notes arrive faster than MIN_OFF_TIME_MS (15ms), some may be blocked
3. Check Serial Monitor for `SAFETY_COOLDOWN` messages

---

## Performance Testing

### Step 12: Latency Check

1. Play notes on a MIDI keyboard
2. Listen/feel for delay between key press and solenoid activation
3. Latency should be imperceptible (< 5ms typical)

### Step 13: Polyphony Test

1. Send multiple Note On messages simultaneously (e.g., notes 60, 62, 64)
2. All three solenoids should activate together
3. Send Note Off for all
4. All should deactivate

---

## Troubleshooting

### Problem: MIDI Device Not Appearing

**Possible Causes:**
1. `USB_MIDI_SERIAL` not in build_flags
2. Firmware not uploaded after change
3. USB cable is charge-only (no data)

**Solutions:**
1. Verify platformio.ini contains `-D USB_MIDI_SERIAL`
2. Clean and rebuild: `pio run --target clean && pio run --target upload`
3. Try a different USB cable
4. Try a different USB port

### Problem: No Response to MIDI Notes

**Possible Causes:**
1. Wrong MIDI channel
2. Notes outside range 60-67
3. MCP23017 not initialized

**Solutions:**
1. The implementation responds to all MIDI channels - verify your source is sending
2. Verify notes are in range 60-67 (C4 to G4)
3. Check Serial Monitor for initialization errors
4. Type 's' to check status

### Problem: Solenoids Stay On

**Possible Causes:**
1. Note Off not being sent
2. MIDI software using velocity=0 for Note Off but not being processed

**Solutions:**
1. Check MIDI source is sending Note Off
2. Verify the `velocity == 0` check is in handleNoteOn()
3. Use emergency stop ('x') to turn off all solenoids

### Problem: Serial Monitor Not Working

**Possible Causes:**
1. Baud rate mismatch
2. Wrong serial port selected

**Solutions:**
1. Ensure monitor_speed is 115200 in platformio.ini
2. Close and reopen Serial Monitor
3. Verify correct port in PlatformIO

### Problem: "Unknown command" Spam

**Cause:** Extra characters being sent (like \r\n from Serial Monitor)

**Solution:** The implementation already handles this by ignoring \r and \n

---

## Test Summary Checklist

### Basic Functionality
- [ ] Firmware compiles without errors
- [ ] Firmware uploads successfully
- [ ] Serial output shows correct startup messages
- [ ] MIDI device appears in OS
- [ ] Note On (60-67) activates corresponding solenoid
- [ ] Note Off deactivates solenoid
- [ ] Notes outside range are ignored

### Serial Commands
- [ ] 'x' - Emergency stop works
- [ ] 's' - Status displays correctly
- [ ] 'h' - Help displays correctly
- [ ] Unknown commands show error message

### Safety Features
- [ ] Auto-shutoff after MAX_ON_TIME_MS
- [ ] Rapid re-trigger protection works
- [ ] Emergency stop reliable

### Performance
- [ ] Low latency (< 10ms)
- [ ] Multiple simultaneous notes work
- [ ] No missed notes during normal play

---

## Next Steps After Verification

Once all tests pass:

1. **Commit changes** to version control
2. **Document any issues** encountered
3. **Consider enhancements:**
   - Add MIDI channel filtering
   - Add velocity-to-duration mapping
   - Add LED feedback for MIDI activity
4. **Monitor during extended use:**
   - Check solenoid temperature
   - Verify no stuck solenoids
   - Ensure emergency stop always works
