# Hardware Setup Guide: 5-Board MCP23017 Configuration

This document covers physical hardware setup, wiring, and power considerations for the 5-board expansion.

## MCP23017 Address Configuration

The MCP23017 uses three address pins (A0, A1, A2) to set its I2C address. The base address is 0x20, and each pin adds to this:
- A0 = +1 (0x01)
- A1 = +2 (0x02)
- A2 = +4 (0x04)

### Address Pin Settings

| Board | I2C Address | A2 | A1 | A0 | Binary |
|-------|-------------|----|----|-----|--------|
| 0 | 0x20 | GND (0) | GND (0) | GND (0) | 0b00100000 |
| 1 | 0x21 | GND (0) | GND (0) | VCC (1) | 0b00100001 |
| 2 | 0x22 | GND (0) | VCC (1) | GND (0) | 0b00100010 |
| 3 | 0x23 | GND (0) | VCC (1) | VCC (1) | 0b00100011 |
| 4 | 0x24 | VCC (1) | GND (0) | GND (0) | 0b00100100 |

### Physical Jumper Configuration

On the Adafruit I2C Solenoid Driver boards (Product ID 6318), the address pins are typically configured via solder jumpers or pads:

```
Board 0 (0x20):  A2=open  A1=open  A0=open   (default - no jumpers)
Board 1 (0x21):  A2=open  A1=open  A0=closed
Board 2 (0x22):  A2=open  A1=closed A0=open
Board 3 (0x23):  A2=open  A1=closed A0=closed
Board 4 (0x24):  A2=closed A1=open  A0=open
```

**Important:** Verify the specific jumper/solder pad configuration for your board revision. The Adafruit documentation should be consulted for exact details.

## I2C Bus Wiring

### Wiring Diagram

```
                                    +3.3V
                                      │
                                    ┌─┴─┐
                               4.7kΩ│   │4.7kΩ  (Pull-up resistors)
                                    └─┬─┘
                                      │
TEENSY 4.1                            │
┌──────────┐                          │
│      SDA │──────────────────────────┼──────┬──────┬──────┬──────┬──────┐
│   (Pin18)│                          │      │      │      │      │      │
│          │                          │      │      │      │      │      │
│      SCL │──────────────────────────┼──┬───┼──┬───┼──┬───┼──┬───┼──┬───┤
│   (Pin19)│                          │  │   │  │   │  │   │  │   │  │   │
│          │                          │  │   │  │   │  │   │  │   │  │   │
│      GND │──────────────────────────┴──┴───┴──┴───┴──┴───┴──┴───┴──┴───┤
│          │                             │      │      │      │      │   │
│     3.3V │─────────────────────────────┼──────┼──────┼──────┼──────┼───┤
│          │                             │      │      │      │      │   │
└──────────┘                             │      │      │      │      │   │
                                         │      │      │      │      │   │
                                    ┌────┴──┐┌──┴───┐┌─┴────┐┌┴─────┐┌┴──┴──┐
                                    │ Board ││Board ││Board ││Board ││Board │
                                    │   0   ││  1   ││  2   ││  3   ││  4   │
                                    │ 0x20  ││ 0x21 ││ 0x22 ││ 0x23 ││ 0x24 │
                                    │       ││      ││      ││      ││      │
                                    │  SDA  ││ SDA  ││ SDA  ││ SDA  ││ SDA  │
                                    │  SCL  ││ SCL  ││ SCL  ││ SCL  ││ SCL  │
                                    │  GND  ││ GND  ││ GND  ││ GND  ││ GND  │
                                    │  VCC  ││ VCC  ││ VCC  ││ VCC  ││ VCC  │
                                    └───────┘└──────┘└──────┘└──────┘└──────┘
```

### Wiring Best Practices

1. **Wire Length:** Keep total I2C bus length under 2 feet (60cm) for reliable 400kHz operation
2. **Star vs Daisy-Chain:** Either topology works; star from a central point is preferred
3. **Wire Gauge:** 22-24 AWG stranded wire recommended
4. **Avoid Noise Sources:** Route I2C wires away from motor/solenoid power wires

### Pull-up Resistor Selection

| Configuration | Recommended Pull-up | Notes |
|--------------|---------------------|-------|
| Short wires (<30cm) | 4.7kΩ | Standard value, works for most cases |
| Longer wires (30-60cm) | 2.2kΩ | Stronger pull-up for capacitance |
| Communication issues | 1.8kΩ - 2.2kΩ | Try stronger pull-ups first |

**Note:** The Teensy 4.1 has internal pull-ups that can be enabled, but external pull-ups are recommended for multi-device I2C buses.

### I2C Signal Integrity

At 400kHz with 5 devices, monitor for:
- **Rise time degradation:** Should be <300ns
- **Bus capacitance:** Keep under 200pF total
- **Voltage levels:** VIH > 2.1V (for 3.3V logic)

If issues occur:
1. First try 2.2kΩ pull-ups
2. If still failing, reduce I2C clock to 100kHz (change `I2C_CLOCK_SPEED` to 100000)
3. Check for loose connections

## Power Supply Requirements

### Per-Board Current Analysis

| Component | Idle Current | Active Current |
|-----------|--------------|----------------|
| MCP23017 IC | ~1mA | ~1mA |
| Solenoid Driver Circuitry | ~5mA | Variable |
| Single Solenoid (typical) | 0mA | 200-500mA |

### Power Budget Calculation

**Logic Power (3.3V):**
- 5 boards × ~6mA = 30mA typical
- Teensy 4.1 = ~100mA typical
- **Total 3.3V:** ~150mA (well within Teensy regulator capacity)

**Solenoid Power (typically 12V or 24V):**

| Scenario | Calculation | Current Draw |
|----------|-------------|--------------|
| Minimum (1 solenoid) | 1 × 300mA | 300mA |
| Typical (4 solenoids) | 4 × 300mA | 1.2A |
| Maximum (all 36) | 36 × 300mA | 10.8A |
| Safety margin (+20%) | 10.8A × 1.2 | **13A** |

### Power Supply Recommendations

| Use Case | Recommended Supply |
|----------|-------------------|
| Testing (few solenoids) | 12V/2A bench supply |
| Normal operation | 12V/5A switching supply |
| Full system (all 36) | 12V/15A switching supply |

### Power Wiring Guidelines

```
                    ┌─────────────────┐
   AC INPUT ────────│  Power Supply   │
                    │   12V / 15A     │
                    └────────┬────────┘
                             │
                    ┌────────┴────────┐
                    │  Distribution   │
                    │  Terminal Block │
                    └─┬───┬───┬───┬───┬─┘
                      │   │   │   │   │
                      │   │   │   │   │
                   ┌──┴┐┌─┴─┐┌┴──┐┌┴──┐┌┴──┐
                   │B0 ││B1 ││B2 ││B3 ││B4 │
                   │V+ ││V+ ││V+ ││V+ ││V+ │
                   │GND││GND││GND││GND││GND│
                   └───┘└───┘└───┘└───┘└───┘
```

**Important:**
1. Use adequate wire gauge for solenoid power (14-16 AWG for 10A+)
2. Keep solenoid power wiring separate from logic/I2C
3. Add bulk capacitance (1000µF+) at the distribution point
4. Consider individual fuses per board for protection

## Board Identification and Labeling

Label each board clearly to avoid confusion during assembly and debugging:

```
┌─────────────────────────────────────────────────────────────┐
│  LABEL TEMPLATE FOR EACH BOARD                              │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│   Board #: _____     Address: 0x___                         │
│                                                             │
│   Channels: _____ - _____                                   │
│                                                             │
│   MIDI Notes: _____ - _____ (_____ - _____)                │
│                                                             │
│   Address Jumpers: A2=___  A1=___  A0=___                  │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

**Filled examples:**

| Board | Label Content |
|-------|---------------|
| 0 | Board #0, Addr: 0x20, Ch: 0-7, MIDI: 24-31 (C1-G1), A2=0 A1=0 A0=0 |
| 1 | Board #1, Addr: 0x21, Ch: 8-15, MIDI: 32-35 (G#1-B1) +unused, A2=0 A1=0 A0=1 |
| 2 | Board #2, Addr: 0x22, Ch: 16-23, MIDI: 48-55 (C3-G3), A2=0 A1=1 A0=0 |
| 3 | Board #3, Addr: 0x23, Ch: 24-31, MIDI: 56-63 (G#3-D#4), A2=0 A1=1 A0=1 |
| 4 | Board #4, Addr: 0x24, Ch: 32-39, MIDI: 64-71 (E4-B4), A2=1 A1=0 A0=0 |

## Solenoid Connections

### Channel to Physical Pin Mapping

Each MCP23017 board uses Port A (pins GPA0-GPA7) for solenoid outputs:

| Local Channel | MCP23017 Pin | Physical Pin # |
|--------------|--------------|----------------|
| 0 | GPA0 | Pin 21 |
| 1 | GPA1 | Pin 22 |
| 2 | GPA2 | Pin 23 |
| 3 | GPA3 | Pin 24 |
| 4 | GPA4 | Pin 25 |
| 5 | GPA5 | Pin 26 |
| 6 | GPA6 | Pin 27 |
| 7 | GPA7 | Pin 28 |

**Note:** On Adafruit boards, these are typically broken out to screw terminals or headers. Consult the specific board pinout.

### Solenoid Wiring per Channel

```
MCP23017            Driver              Solenoid
  GPA[n] ────────── Gate ────┬──────── Coil (+)
                             │
                             │ Flyback
                             │ Diode
                             │
  GND ───────────────────────┴──────── Coil (-)
```

**Critical:** Ensure flyback diodes are in place to protect the driver from inductive kickback.

## Pre-Power Checklist

Before applying power, verify:

- [ ] All 5 boards have unique addresses (no conflicts)
- [ ] Address jumpers/pads are correctly configured
- [ ] SDA connected to Teensy Pin 18 on all boards
- [ ] SCL connected to Teensy Pin 19 on all boards
- [ ] GND connected between Teensy and all boards
- [ ] 3.3V connected to all boards (logic power)
- [ ] Pull-up resistors installed (4.7kΩ recommended)
- [ ] Solenoid power supply OFF during initial testing
- [ ] No shorts between SDA/SCL/VCC/GND

## Initial Power-On Procedure

1. **Logic power only first:**
   - Connect USB to Teensy (provides 3.3V/5V logic power)
   - Leave solenoid power supply OFF
   - Open serial monitor at 115200 baud
   - Verify all 5 boards initialize successfully

2. **I2C bus scan (optional debug step):**
   ```cpp
   // Add this temporary code to scan for devices:
   for (uint8_t addr = 0x20; addr <= 0x27; addr++) {
       Wire.beginTransmission(addr);
       if (Wire.endTransmission() == 0) {
           Serial.print("Found device at 0x");
           Serial.println(addr, HEX);
       }
   }
   ```

3. **Verify initialization messages:**
   ```
   Initializing 5 MCP23017 board(s)...
     Board 0: address 0x20
     Board 1: address 0x21
     Board 2: address 0x22
     Board 3: address 0x23
     Board 4: address 0x24
   [OK] MCP23017 initialized successfully
     SolenoidDriver initialized: 5 board(s), 40 channel(s), all OFF
   ```

4. **Apply solenoid power:**
   - After successful initialization, enable solenoid power supply
   - Test individual channels with MIDI notes

## Troubleshooting

### Board Not Found

| Symptom | Possible Cause | Solution |
|---------|---------------|----------|
| "Failed to initialize MCP23017" | Wrong address jumpers | Verify A0/A1/A2 settings |
| | No power to board | Check VCC/GND connections |
| | Bad I2C wiring | Check SDA/SCL connections |
| | Pull-ups missing | Add 4.7kΩ pull-ups |

### Intermittent Communication

| Symptom | Possible Cause | Solution |
|---------|---------------|----------|
| Random failures | Weak pull-ups | Use 2.2kΩ instead of 4.7kΩ |
| | Long wires | Shorten bus, reduce clock speed |
| | Noise interference | Route I2C away from power wires |

### Wrong Channels Activated

| Symptom | Possible Cause | Solution |
|---------|---------------|----------|
| Wrong solenoid fires | Board addresses swapped | Re-verify address jumpers |
| | Wiring to wrong board | Follow labeling scheme |

### I2C Bus Lockup

| Symptom | Possible Cause | Solution |
|---------|---------------|----------|
| All communication stops | SDA stuck low | Power cycle, check for shorts |
| | Corrupted transaction | Add bus recovery code |

**Bus Recovery Code (if needed):**
```cpp
void recoverI2C() {
    // Toggle SCL to release stuck slave
    pinMode(19, OUTPUT);
    for (int i = 0; i < 9; i++) {
        digitalWrite(19, HIGH);
        delayMicroseconds(5);
        digitalWrite(19, LOW);
        delayMicroseconds(5);
    }
    pinMode(19, INPUT);
    Wire.begin();
    Wire.setClock(I2C_CLOCK_SPEED);
}
```
