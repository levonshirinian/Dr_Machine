# DTX - Technical Documentation

**Project Name:** DTX  
**Type:** Arduino/Embedded Systems Project  
**Platform:** Arduino (Mega or compatible with ATmega2560 architecture)  
**Language:** C++ (Arduino)  
**Last Updated:** December 2025

---

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [System Goals](#2-system-goals)
3. [Hardware Components](#3-hardware-components)
4. [System Architecture](#4-system-architecture)
5. [Firmware Architecture](#5-firmware-architecture)
6. [Calibration Procedures](#6-calibration-procedures)
7. [File Structure Explanation](#7-file-structure-explanation)
8. [Data Logging Format](#8-data-logging-format)
9. [Known Limitations and Considerations](#9-known-limitations-and-considerations)
10. [Future Enhancements](#10-future-enhancements)

---

## 1. Project Overview

**DTX** is an Arduino-based multi-modal testing instrument designed for conducting **tensile and hardness tests** on thread/filament materials. The system integrates real-time data acquisition, environmental monitoring, SD card logging, and a full-color touchscreen interface to provide a complete testing and analysis platform.

The device measures mechanical properties (tension/stress), environmental conditions (temperature and humidity), and timestamps all data using a real-time clock (RTC). Test sessions are stored on an SD card and can be retrieved for further analysis.

---

## 2. System Goals

The DTX firmware achieves the following objectives:

1. **Real-Time Sensor Acquisition:** Continuously read load cell, DHT11, and limit switch data
2. **Environmental Monitoring:** Track temperature and humidity during tests
3. **Timestamped Logging:** Persist all measurements with precise timestamps via DS3231 RTC
4. **Data Persistence:** Store session data on SD card in structured CSV format
5. **Interactive Testing Modes:** Support tensile testing (T), calibration (C), memory/storage (M), and hardness testing (H) modes
6. **Material Management:** Track different filament materials with configurable test parameters
7. **Real-Time Visualization:** Display live sensor readings and stress-strain graphs
8. **Calibration Support:** Enable scale calibration and zeroing procedures
9. **Battery Health Monitoring:** Track battery voltage and charge percentage with visual LED feedback
10. **User Interface:** Provide intuitive touchscreen controls for mode selection, parameter adjustment, and data management

---

## 3. Hardware Components

### 3.1 Microcontroller
- **Device:** Arduino Mega 2560 (ATmega2560-16AU)
- **Processor Speed:** 16 MHz
- **Flash Memory:** 256 KB
- **SRAM:** 8 KB
- **Pin Count:** 54 digital I/O pins, 16 analog inputs

### 3.2 Load Cell Sensor
- **Type:** Strain gauge load cell (tension sensor)
- **Interface:** HX711 ADC module
- **Data Pin:** 5 (LOADCELL_DOUT_PIN)
- **Clock Pin:** 6 (LOADCELL_SCK_PIN)
- **Secondary Pins (Unused/Duplicate):** 26, 28 (defined in Config.h but not used in SensorManager)
- **Library:** HX711 (custom implementation)
- **Calibration Factor:** -4083333 (adjustable)
- **Reading Method:** Averaged over 5 samples per read cycle
- **Output Scaling:** Converted to centinewtons (cN) via formula: `weight = -scale.get_units(5) * 400 * 100`

### 3.3 Temperature and Humidity Sensor
- **Type:** DHT11 (Digital Humidity and Temperature)
- **Data Pin:** 24 (DHTPIN)
- **Temperature Range:** 0-50°C
- **Humidity Range:** 20-90% RH
- **Library:** DHT (Adafruit or compatible)

### 3.4 Mechanical Inputs
- **Limit Switch Pin:** 23 (LIMIT_SWITCH_PIN)
  - Active LOW (pulled high via INPUT_PULLUP)
  - Indicates when load cell is under tension (test active)
- **Calibration Button:** 3 (CALIBRATE_BUTTON_PIN) [defined but not actively used in current code]
- **Tare Button:** 2 (TARE_BUTTON_PIN) [defined but not actively used in current code]
- **Read Button:** 4 (READ_BUTTON_PIN) [defined but not actively used in current code]

### 3.5 Real-Time Clock (RTC)
- **Device:** DS3231 (I2C precision RTC)
- **Communication:** I2C (Wire library)
- **SDA Pin:** A4 (default Arduino Mega) or GPIO21 (ESP32)
- **SCL Pin:** A5 (default Arduino Mega) or GPIO22 (ESP32)
- **Battery:** Coin cell (CR2032) for timekeeping during power loss
- **Library:** RTClib (Adafruit)

### 3.6 SD Card Storage
- **Module:** SPI SD Card Reader
- **Chip Select Pin:** 53 (SD_CS_PIN)
- **Communication Protocol:** SPI (pins 50-52 for MISO/MOSI/SCK on Mega)
- **File Format:** CSV (comma-separated values)
- **Filename:** `sessions.csv`
- **Library:** SdFat

### 3.7 Display Module
- **Type:** 3.5" TFT LCD Touchscreen (ILI9486)
- **Resolution:** 480x320 pixels
- **Interface:** 16-bit parallel + touch sensing
- **Chip Select:** Pin 10 (TFT_CS)
- **Reset:** Pin 9 (TFT_RST)
- **Data/Command:** Pin 8 (TFT_DC)
- **Touch Interface:** Resistive touch with 4-wire interface
- **Touch Calibration Values:**
  - TS_MINX: 906
  - TS_MAXX: 116
  - TS_MINY: 92
  - TS_MAXY: 952
- **Library:** LCDWIKI_KBV and TouchScreen

### 3.8 Status Indicator LED
- **Type:** NeoPixel (WS2812B or compatible RGB LED)
- **Pin:** 22 (LED_PIN)
- **Quantity:** 1 LED
- **Library:** Adafruit_NeoPixel
- **Color Codes:**
  - Green: Battery ≥ 95%
  - Red: Battery ≤ 20%
  - Dark Blue: Calibration mode active
  - Off: Normal operation (20-95% charge)
  - Orange (Blinking): Charging [currently disabled in code]

### 3.9 Power Supply
- **Battery Pin:** A8 (batteryPin)
- **Analog ADC Conversion:** 10-bit (0-1023 maps to 0-5V)
- **Voltage Range:** 3.0V (0%) to 4.2V (100%)
- **Monitoring:** Battery percentage calculated via analog read with voltage mapping
- **Charging Detection:** Monitored via small voltage increase threshold (0.02V)

---

## 4. System Architecture

### 4.1 Block Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                      Arduino Mega 2560                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │           Core Firmware (DTX.ino)                        │   │
│  │  • setup() - Hardware initialization                     │   │
│  │  • loop() - Main execution cycle                         │   │
│  │  • Global state management                               │   │
│  └──────────────────────────────────────────────────────────┘   │
│                           │                                     │
│    ┌──────────────────────┼──────────────────────┐              │
│    │                      │                      │              │
│    ▼                      ▼                      ▼              │
│  ┌──────────┐      ┌──────────┐         ┌──────────┐            │
│  │ SENSORS  │      │ RTC/TIME │         │ DISPLAY  │            │
│  ├──────────┤      ├──────────┤         ├──────────┤            │
│  │ HX711    │      │ DS3231   │         │ ILI9486  │            │
│  │ DHT11    │      │ I2C      │         │ Touch    │            │
│  │ Limit SW │      └──────────┘         └──────────┘            │
│  └──────────┘             │                    │                │
│    │ (SensorManager)      │ (RTCManager)       │ (DisplayMgr)   │
│    │                      │                    │                │
│    └──────────────────────┼────────────────────┘                │
│                           │                                     │
│                    ┌──────▼──────┐                              │
│                    │SESSION LOGIC │                             │
│                    ├──────────────┤                             │
│                    │TestSession   │                             │
│                    │SessionManager│                             │
│                    │Sample buffer │                             │
│                    └──────┬───────┘                             │
│                           │                                     │
│    ┌──────────────────────┼──────────────────────┐              │
│    │                      │                      │              │
│    ▼                      ▼                      ▼              │
│  ┌──────────┐      ┌──────────┐         ┌──────────┐            │
│  │ STORAGE  │      │ BATTERY  │         │ LED      │            │
│  ├──────────┤      ├──────────┤         ├──────────┤            │
│  │ SD Card  │      │ Voltage  │         │ NeoPixel │            │
│  │ CSV File │      │ Monitor  │         │ Status   │            │
│  └──────────┘      └──────────┘         └──────────┘            │
│   (StorageMgr)     (BatteryMgr)         (LEDManager)            │
│                                                                 │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │ Supporting Modules                                       │   │
│  ├──────────────────────────────────────────────────────────┤   │
│  │ • ScaleController - HX711 interface and calibration      │   │
│  │ • GraphViewer - Data visualization and stress curves     │   │
│  │ • Config.h - Pin definitions and calibration settings    │   │
│  └──────────────────────────────────────────────────────────┘   │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### 4.2 Module Interactions

```
┌─────────────────────────────────────────────────────────────────┐
│                    Data Flow During Testing                     │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│   SensorManager                                                 │
│   ┌────────────────┐                                            │
│   │ readSensors()  │                                            │
│   │ • HX711 load   │                                            │
│   │ • DHT11 temp   │                                            │
│   │ • DHT11 hum    │                                            │
│   │ • Limit switch │                                            │
│   └────────┬───────┘                                            │
│            │                                                    │
│            │ SensorData struct                                  │
│            ▼                                                    │
│   DisplayManager                    SessionManager              │
│   ┌────────────────┐               ┌────────────────┐           │
│   │ updateDisplay()│               │ addSample()    │           │
│   │ • Render real- │               │ • Buffer data  │           │
│   │   time values  │               │ • Circular buf │           │
│   │ • Update graph │               │ • 1s interval  │           │
│   └────────────────┘               └────────┬───────┘           │
│                                             │                   │
│                                    RTCManager                   │
│                                    ┌────────────────┐           │
│                                    │ datetime info  │           │
│                                    │ • Date         │           │
│                                    │ • Time         │           │
│                                    └────────────────┘           │
│                                             │                   │
│                                    StorageManager               │
│                                    ┌────────────────┐           │
│                                    │ saveSession()  │           │
│                                    │ • CSV append   │           │
│                                    │ • SD write     │           │
│                                    │ • Statistics   │           │
│                                    └────────────────┘           │
└─────────────────────────────────────────────────────────────────┘
```

---

## 5. Firmware Architecture

### 5.1 Execution Flow: `setup()`

```cpp
setup()
├── Serial.begin(115200)
│   └── [DEBUG] Serial output initialization
├── initializeRTC()
│   ├── Wire.begin()           // I2C initialization
│   ├── rtc.begin()            // DS3231 probe
│   ├── rtc.adjust()           // Set time from compile date
│   └── [RETURN] true/false
├── initializeSD()
│   ├── sd.begin(SD_CS_PIN)    // SPI SD initialization
│   ├── [IF] sessions.csv doesn't exist
│   │   └── Create file with CSV header
│   └── [RETURN] true/false
├── initializeLEDs()
│   ├── strip.begin()          // NeoPixel init
│   └── [RETURN] true/false
├── initializeDisplay()
│   ├── mylcd.Init_LCD()       // TFT parallel init
│   ├── Set rotation (3)
│   ├── Display welcome screen
│   ├── Load currentN from EEPROM
│   └── Show tensile test screen
├── initializeBatteryMonitor()
│   └── pinMode(batteryPin, INPUT)
└── initializeSensors()
    ├── dht.begin()            // DHT11 initialization
    ├── scale.begin()          // HX711 initialization
    ├── scale.set_scale()      // Apply calibration factor
    ├── scale.tare()           // Zero the load cell
    └── pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP)
```

### 5.2 Execution Flow: `loop()`

The main loop executes continuously and manages the complete system state:

```cpp
loop() [MAIN EXECUTION CYCLE]
├── 1. DATA ACQUISITION
│   ├── getBatteryVoltage()     // ADC read + mapping
│   ├── getBatteryPercentage()  // Voltage to %
│   ├── readSensors()           // HX711 + DHT11 + limit switch
│   ├── handleTouch()           // Touchscreen polling
│   └── getCurrentMaterial()    // Material from global array
│
├── 2. STATE UPDATE
│   ├── updateLEDStatus()       // LED color based on battery
│   ├── session.setDateTime()   // RTC timestamp
│   ├── updateDisplay()         // Refresh screen values
│   └── isRecording = limitSwitch state
│
├── 3. SERIAL DEBUG OUTPUT
│   ├── Print formatted datetime
│   ├── Print battery V and %
│   └── Print all sensor readings
│
├── 4. CONDITIONAL LOGIC
│   ├── IF (isRecording && screen != 'M')
│   │   └── showCalibrationMode()  // Blue LED + calibration UI
│   │
│   ├── IF (isShouldSaved)
│   │   ├── session.printSession()        // Serial debug print
│   │   ├── session.saveSessionToStorage()// SD card write
│   │   └── isShouldSaved = false
│   │
│   └── IF (isRecording && shouldTakeSample() && screen == 'T')
│       ├── session.addSample()              // Circular buffer (10 slots)
│       ├── session.setEnvironment()         // Store material, temp, hum, diam
│       └── isShouldSaved = true
│
└── [LOOP CONTINUES at ~60Hz or faster based on delays]
```

### 5.3 Sensor Reading Logic

#### Load Cell (HX711) Reading Process

```
readSensors() → HX711 flow:
1. scale.get_units(5)          // Average of 5 ADC readings
2. Result multiplied by -400   // Calibration scaling
3. Result multiplied by 100    // Convert to centinewtons (cN)
4. IF (result < 0) → clamp to 0
5. Return as float weight

Formula: weight = -scale.get_units(5) * 400 * 100 [cN]

Calibration Factor: -4083333
- Negative indicates inverted load cell orientation
- Large magnitude indicates high-resolution ADC output
```

#### Temperature and Humidity Reading

```
readSensors() → DHT11 flow:
1. dht.readTemperature()   → Returns °C
2. dht.readHumidity()      → Returns % RH
3. Return in SensorData struct
```

#### Limit Switch Logic

```
readSensors() → Limit Switch flow:
1. digitalRead(LIMIT_SWITCH_PIN)
2. Inverted logic: !reading
   - Switch HIGH (unpressed) → returns FALSE
   - Switch LOW (pressed)    → returns TRUE
3. Controls recording state (isRecording)
```

### 5.4 Session Sampling and Storage

#### Session Buffer Management

The system maintains a **circular buffer of 10 samples** in the `TestSession` class:

```cpp
// TestSession private members
Sample samples[10];              // Circular buffer
int head = 0;                    // Current write position
int count = 0;                   // Number of valid samples (0-10)

// Sampling rate
const unsigned long sampleInterval = 1000;  // 1 second between samples
```

#### Sample Acquisition Flow

```
Main Loop:
├── IF (isRecording && shouldTakeSample() && screen == 'T')
│   ├── shouldTakeSample() checks if 1000ms elapsed
│   ├── session.addSample(weight, millis())
│   │   ├── samples[head] = {timestamp, weight}
│   │   ├── head = (head + 1) % 10  // Wrap around
│   │   ├── IF (count < 10) count++
│   │   └── [RESULT] Sample stored
│   │
│   ├── session.setEnvironment(material, temp, hum, diameter)
│   │   ├── material = Material string
│   │   ├── temperature = DHT11 value
│   │   ├── humidity = DHT11 value
│   │   └── thread_diameter = 1.75 (hardcoded)
│   │
│   └── isShouldSaved = true
│
└── IF (isShouldSaved)
    ├── session.printSession()      // Serial dump
    ├── session.saveSessionToStorage()
    │   ├── Compute statistics:
    │   │   ├── Average of samples
    │   │   ├── Minimum value
    │   │   └── Maximum value
    │   ├── Call appendSession() with stats
    │   └── Write CSV row to SD
    └── isShouldSaved = false
```

### 5.5 Screen Modes and Navigation

The system implements a finite state machine with 4 main screens + 3 helper screens:

| Screen | Code | Purpose |
|--------|------|---------|
| **Tensile Test** | 'T' | Main testing interface, displays live load, temperature, humidity, limit status |
| **Calibration** | 'C' | Scale calibration mode with +/- buttons to adjust reference weight |
| **Memory/Storage** | 'M' | Browse and manage stored sessions, delete sessions, view free space |
| **Hardness Test** | 'H' | Alternative testing mode, displays load in G/cm units |
| **Select Material** | 's' | Choose filament material from dropdown |
| **Edit N Value** | 'e' | Adjust test load parameter N (20-70N) via ±10, ±1, ±0.1 increments |
| **Graph Viewer** | 'g' | Display stress-strain curve of selected session |

#### Screen Layout Template

```
┌────────────────────────────────────────────────────┐
│ [T][C][M][H] ← Menu buttons (70px wide)            │
│ [70px]  HEADER (Material/Title + T/H data)         │
├────────────────────────────────────────────────────┤
│                                                    │
│        MAIN CONTENT AREA                           │
│        • Large font weight display                 │
│        • Sensor readings                           │
│        • Navigation elements                       │
│        • Status indicators                         │
│                                                    │
└────────────────────────────────────────────────────┘

Dimensions: 480x320 pixels
Touch-sensitive areas calibrated for display rotation
```

### 5.6 Touchscreen Calibration and Mapping

```
Raw Touch Coordinates:
  X: 0-1023 (ADC range)
  Y: 0-1023 (ADC range)
  
Calibration Map:
  TS_MINX = 906,  TS_MAXX = 116    (X range)
  TS_MINY = 92,   TS_MAXY = 952    (Y range)
  
Mapped to Display:
  X: 0-480 pixels
  Y: 0-320 pixels
  
Pressure Thresholds:
  MINPRESSURE = 10   (Minimum touch)
  MAXPRESSURE = 1000 (Maximum touch/noise rejection)
```

### 5.7 State Machine Transitions

```
                    START
                      │
                      ▼
         ┌──────────────────────────┐
         │  Welcome Screen (W)      │
         │  Boot initialization     │
         └──────────┬───────────────┘
                    │
                    ▼
    ┌────────────────────────────────────────┐
    │     MAIN STATE LOOP                    │
    │  Current Screen: 'T', 'C', 'M', 'H'    │
    └────────┬───────────────────────────────┘
             │
    ┌────────┴────────┬──────────┬──────────┬──────────┐
    │                 │          │          │          │
    ▼                 ▼          ▼          ▼          ▼
   [T]               [C]        [M]        [H]     [Touch Menu]
  Tensile          Calibrate  Memory    Hardness       │
    │                 │          │          │          ├─→ 's' Select Material
    │                 │          │          │          ├─→ 'e' Edit N Value
    │                 │          │          │          ├─→ 'g' Graph Viewer
    │                 │          │          │          │
    └─────────────────┴──────────┴──────────┴──────────┘
             │
             │ Screen Change
             ▼
          Clear Display
          Redraw New Screen
          Return to Loop
```

---

## 6. Calibration Procedures

### 6.1 Load Cell Calibration (HX711)

#### Initial Setup

The HX711 uses a **calibration factor** stored as a global variable:

```cpp
long CALIBRATION_FACTOR = -4083333;  // Default value
```

#### Calibration Process

```
calibrateScale(knownWeight) function:

INPUT: knownWeight [float] = known reference weight in grams or target units

PROCESS:
1. Take 20 raw ADC readings and average
   → raw = scale.read_average(20)

2. Calculate new calibration factor
   → CALIBRATION_FACTOR = raw / knownWeight

3. Apply factor to scale
   → scale.set_scale(CALIBRATION_FACTOR)

OUTPUT: Scale now correctly maps ADC output to grams (or custom units)

Example:
- Place 100g calibration weight on scale
- Call calibrateScale(100.0)
- System recalculates factor
```

#### Taring (Zero Offset)

```cpp
tareScale()  →  scale.tare()

PURPOSE: Reset current reading to zero (removes any static offset)

TIMING: Called during initializeSensors() at startup
Can be called anytime to re-zero the scale
```

#### Current Conversion Chain

```
Raw ADC Output
     ↓
[HX711 Library] get_units(5) - Applies CALIBRATION_FACTOR
     ↓
Gram equivalent (or custom calibrated units)
     ↓
[SensorManager] Multiply by -400 × 100
     ↓
Display as Centinewtons (cN)
```

### 6.2 UI Calibration Mode

In the **Calibration (C)** screen, users can:
- View live load cell output in cN
- Press **+** button to increase calibration reference
- Press **-** button to decrease calibration reference
- Press **SAVE** button to apply new calibration

[Implementation status: UI drawn but calibration button handlers may need completion]

### 6.3 Parameter Persistence

```
EEPROM Storage (1 byte address 0):
├── currentN (float)  → Test load parameter
├── Value range: 20.0 to 70.0 N
├── Default: 20.0 N
└── Loaded at startup and when editing N value

Load at boot:
  EEPROM.get(0, currentN)
  IF (isnan(currentN)) currentN = 20.0

Save after edit:
  EEPROM.put(0, currentN)
```

---

## 7. File Structure Explanation

### 7.1 File Organization

```
DTX/
├── DTX.ino          [Main sketch - Setup & Loop]
├── Config.h                      [Pin definitions & settings]
│
├── Session Management
│   ├── SessionManager.h          [TestSession class definition]
│   ├── SessionManager.cpp        [Session logic & sample buffering]
│   └── StorageManager.h/.cpp     [SD card I/O & CSV management]
│
├── Sensor I/O
│   ├── SensorManager.h/.cpp      [DHT11, HX711, Limit Switch reading]
│   ├── ScaleController.h/.cpp    [HX711 calibration & control]
│   └── RTCManager.h/.cpp         [DS3231 RTC interface]
│
├── User Interface
│   ├── DisplayManager.h/.cpp     [TFT & Touchscreen UI logic]
│   └── GraphViewer.h/.cpp        [Stress-strain visualization]
│
├── Peripherals
│   ├── LEDManager.h/.cpp         [NeoPixel status indicator]
│   └── BatteryManager.h/.cpp     [Voltage monitoring & %age]
│
└── README.md                     [Project overview]
```

### 7.2 Detailed File Descriptions

#### **DTX.ino** - Main Sketch
- **Purpose:** Entry point; defines setup() and loop()
- **Global State:**
  - `TestSession session` - Active session object
  - `char current_screen` - Current UI screen
  - `bool isRecording` - Test active flag (linked to limit switch)
  - `bool isShouldSaved` - Flag to trigger session save
- **Key Functions:**
  - `setup()` - Initialize all hardware and peripherals
  - `loop()` - Main 60Hz+ execution cycle with sensor reading and UI updates
- **Lines:** ~80 lines

#### **Config.h** - Configuration Header
- **Purpose:** Centralized pin definitions and calibration settings
- **Contents:**
  - Pin definitions (HX711, DHT, Buttons, LEDs, etc.)
  - Display calibration values
  - Session buffer size (MAX_SESSION_SAMPLES = 40, but actual code uses 10)
  - Default calibration factor
- **No .cpp** - Header-only configuration file
- **Lines:** ~40 lines

#### **SessionManager.h / SessionManager.cpp** - Test Session Management
- **Class:** `TestSession`
- **Private Members:**
  - `Sample samples[10]` - Circular buffer of timestamp + value pairs
  - `int head` - Write pointer for circular buffer
  - `int count` - Number of valid samples in buffer
  - `float temperature, humidity, thread_diameter` - Environment context
  - `String datetime, material` - Session metadata
  - `unsigned long lastSampleTime` - Timestamp of last sample
- **Public Methods:**
  - `addSample(float val, uint32_t ts)` - Insert sample into circular buffer
  - `setEnvironment(String Material, float temp, float hum, float diameter)` - Store environment
  - `setDateTime(String dt)` - Store timestamp
  - `printSession()` - Dump session to Serial with emoji formatting
  - `saveSessionToStorage()` - Compute stats and append to SD card
  - `shouldTakeSample()` - Return true if 1000ms elapsed
- **Features:**
  - Circular buffer automatically overwrites oldest data when full
  - Automatic statistics computation (min, max, average)
  - 1-second sampling interval
- **Lines:** ~60 lines (combined)

#### **SensorManager.h / SensorManager.cpp** - Sensor Reading
- **Data Structure:** `SensorData`
  - `float temperature` - DHT11 temp reading °C
  - `float humidity` - DHT11 RH% reading
  - `float weight` - HX711 load cell in cN
  - `float filamentDiameter` - Hardcoded to 1.75mm
  - `bool limitSwitch` - Tension applied state
- **Functions:**
  - `initializeSensors()` - Setup DHT11, HX711, limit switch
  - `readSensors()` - Poll all sensors and return SensorData struct
- **Calibration Factor:** -4083333 (stored as global `long CALIBRATION_FACTOR`)
- **Lines:** ~50 lines (combined)

#### **ScaleController.h / ScaleController.cpp** - HX711 Control
- **Purpose:** High-level HX711 interface and calibration
- **Functions:**
  - `initializeScale()` - Initialize HX711 with pins and calibration
  - `tareScale()` - Reset load cell to zero
  - `calibrateScale(float knownWeight)` - Recalibrate using reference weight
  - `readWeight()` - Return current weight from scale
- **External References:**
  - `extern HX711 scale` - Global HX711 object
  - `extern long CALIBRATION_FACTOR` - Global calibration factor
- **Lines:** ~30 lines (combined)

#### **RTCManager.h / RTCManager.cpp** - Real-Time Clock
- **Hardware:** DS3231 I2C RTC module
- **Functions:**
  - `initializeRTC()` - Initialize I2C and DS3231, set time from compile date
  - `getFormattedDateTime()` - Return string formatted as "DD/MM/YYYY HH:MM:SS"
- **Features:**
  - Platform-aware (ESP32 uses GPIO21/22, Arduino uses A4/A5)
  - Auto-adjusts time on each upload to compile time
- **Lines:** ~30 lines (combined)

#### **StorageManager.h / StorageManager.cpp** - SD Card Management
- **File Format:** CSV (sessions.csv)
- **Header Row:**
  ```
  id,material,diameter,humidity,temperature,dateTime,average,min,max,count,head,sample0,sample1,...,sample9
  ```
- **Key Functions:**
  - `initializeSD()` - Mount SD, create file with header if needed
  - `appendSession(...)` - Add new session record with 10 sample values
  - `getSessionById(int id)` - Retrieve session CSV line by ID
  - `deleteSessionById(int id)` - Remove session and rewrite file
  - `getSessionCount()` - Count records (excluding header)
  - `getAllSessionIds(int* list, int& count)` - Populate array of session IDs
  - `getFreeSpacePercent()` - Return SD card free space percentage
  - `clearSessionFile()` - Erase all session data, keep header
- **Data Persistence:** Each session auto-increments ID based on highest existing ID
- **Lines:** ~200 lines

#### **DisplayManager.h / DisplayManager.cpp** - UI & Touchscreen
- **Hardware:** ILI9486 TFT LCD with resistive touch
- **Resolution:** 480x320 pixels
- **Color Palette:**
  - BLACK (0x0000), BLUE, WHITE, RED, GREEN, CYAN, GRAY, DARKBLUE
- **Main Screens:**
  - `W()` - Welcome screen with "DTX" logo
  - `T()` - Tensile test screen (main interface)
  - `C()` - Calibration mode
  - `M()` - Memory/storage browser
  - `H()` - Hardness test
  - `select_M()` - Material selection dropdown
  - `editN()` - N value adjuster (20-70N, three adjustment rates)
- **Key Functions:**
  - `initializeDisplay()` - Initialize LCD, load stored settings
  - `updateDisplay(temp, hum, weight, limitState)` - Refresh values every 500ms
  - `handleTouch()` - Poll touchscreen, return current screen code
  - `getCurrentMaterial()` - Return selected material name string
  - `displayCurrentScreen()` - Render current screen based on `currentScreen` variable
  - `loadSessionToGraph(id)` - Parse CSV row and draw stress curve
  - `showSessionInfoScreen(id)` - Display session summary overlay
- **Global Variables:**
  - `materials[7]` - Array of material names (cotton, wool, linen, jute, silk, polyester, nylon)
  - `currentM` - Selected material index (0-6)
  - `currentN` - Current test load parameter (20-70N)
  - `currentScreen` - Current screen code
  - `sessionIndex[128]` - Array of session IDs from SD
  - `sessionCount` - Number of sessions stored
- **Touch Detection:**
  - Calibrated for display rotation (currentRotation = 3)
  - `is_pressed()` - Check if touch coordinate is in rectangle
  - `MapPointRotation()` - Convert raw ADC to display coordinates
- **Lines:** ~450 lines

#### **GraphViewer.h / GraphViewer.cpp** - Data Visualization
- **Data Structure:** `GraphPoint`
  - `uint32_t timestamp` - Milliseconds
  - `float value` - Stress value
- **Class:** `GraphSession`
  - `GraphPoint dataPoints[50]` - Max 50 data points per graph
  - `int count` - Current point count
  - `addPoint(float val, float seconds)` - Append point
  - `reset()` - Clear all points
- **Functions:**
  - `drawGraph(GraphSession& sess)` - Render stress-strain curve with grid and axes
  - `drawAxes(float maxY)` - Draw coordinate system with labels
- **Graph Parameters:**
  - X-axis: Time (seconds), divided into 10 segments
  - Y-axis: Stress value, auto-scaled and rounded up to nearest 10
  - Grid lines at 5 Y-divisions and 10 X-divisions
  - Data plotted as cyan line connecting points
- **Lines:** ~150 lines

#### **LEDManager.h / LEDManager.cpp** - Status LED
- **Hardware:** 1x NeoPixel RGB LED (WS2812B compatible)
- **Functions:**
  - `initializeLEDs()` - Initialize NeoPixel library
  - `updateLEDStatus()` - Set color based on battery percentage
  - `showCalibrationMode()` - Set to dark blue (0, 0, 128)
  - `setColor(r, g, b)` - Direct RGB control
  - `blinkOrange()` - Blink at 500ms interval (currently disabled)
- **Color Scheme:**
  - Green (0, 255, 0) - Battery ≥ 95%
  - Red (255, 0, 0) - Battery ≤ 20%
  - Dark Blue (0, 0, 128) - Calibration mode
  - Off (0, 0, 0) - Normal operation 20-95%
- **Lines:** ~60 lines

#### **BatteryManager.h / BatteryManager.cpp** - Power Monitoring
- **Hardware:** ADC on pin A8 reading battery voltage divider
- **Functions:**
  - `initializeBatteryMonitor()` - Set pin to INPUT mode
  - `getBatteryVoltage()` - Return voltage in volts (0-5V)
  - `getBatteryPercentage()` - Return percentage (0-100%)
  - `isBatteryCharging()` - Return true if voltage increasing > 0.02V
- **Voltage Mapping:**
  - 3.0V → 0%
  - 4.2V → 100%
  - Linearly interpolated and clamped
- **Lines:** ~40 lines

---

## 8. Data Logging Format

### 8.1 CSV File Structure (sessions.csv)

**Filename:** `sessions.csv`  
**Location:** SD card root directory  
**Format:** Comma-separated values (CSV)  
**Encoding:** ASCII

#### Header Row
```
id,material,diameter,humidity,temperature,dateTime,average,min,max,count,head,sample0,sample1,sample2,sample3,sample4,sample5,sample6,sample7,sample8,sample9
```

#### Data Row Example
```
1,cotton,1.75,65.5,24.2,15/12/2025 14:32:45,125.8,118.3,134.2,10,3,120.5,125.3,128.1,125.8,124.2,126.9,129.3,127.1,122.4,123.8
```

### 8.2 Field Descriptions

| Field | Index | Type | Example | Description |
|-------|-------|------|---------|-------------|
| id | 0 | Integer | 1 | Auto-incrementing session ID |
| material | 1 | String | cotton | Selected material name |
| diameter | 2 | Float | 1.75 | Filament diameter in mm |
| humidity | 3 | Float | 65.5 | Relative humidity % RH |
| temperature | 4 | Float | 24.2 | Temperature in °C |
| dateTime | 5 | String | 15/12/2025 14:32:45 | Date and time of test |
| average | 6 | Float | 125.8 | Mean of all samples (cN) |
| min | 7 | Float | 118.3 | Minimum value (cN) |
| max | 8 | Float | 134.2 | Maximum value (cN) |
| count | 9 | Integer | 10 | Number of samples collected |
| head | 10 | Integer | 3 | Circular buffer head position |
| sample0-9 | 11-20 | Float | 120.5, 125.3, ... | Individual weight readings (cN) |

### 8.3 Data Recording Process

1. **Test Session Starts:**
   - Limit switch pressed → `isRecording = true`
   - User on 'T' screen

2. **Sampling Loop (1 second intervals):**
   - `shouldTakeSample()` returns true every 1000ms
   - `readSensors()` acquires current weight + environment
   - `session.addSample(weight, timestamp)` stores in circular buffer
   - `session.setEnvironment(material, temp, hum, 1.75)` updates context
   - `isShouldSaved = true` flag set

3. **Session Save:**
   - On flag `isShouldSaved == true`:
   - Calculate statistics: average, min, max
   - `appendSession()` formats CSV row and writes to SD
   - `isShouldSaved = false` to prevent duplicate saves

4. **Circular Buffer Behavior:**
   - Max 10 samples stored in `samples[10]` array
   - `head` pointer wraps around: `head = (head + 1) % 10`
   - `count` increments until reaching 10 (then stays at 10)
   - Oldest data overwritten if session continues beyond 10 samples

### 8.4 Calculation Methods

#### Average
```
average = sum(all sample values) / count
```

#### Minimum
```
min = minimum value in samples array
```

#### Maximum
```
max = maximum value in samples array
```

#### Weight Conversion Chain
```
HX711 ADC reading
  ↓
scale.get_units(5)           [Applies CALIBRATION_FACTOR = -4083333]
  ↓
multiply by -400 * 100       [In SensorManager.cpp]
  ↓
Result stored and displayed as cN (centinewtons)
```

---

## 9. Known Limitations and Considerations

### 9.1 Software Limitations

1. **Circular Buffer Size:** 
   - Session stores only 10 samples maximum
   - If test continues beyond 10 samples, oldest data is lost
   - Recommend ~16.7 minute maximum test duration at 1Hz sampling

2. **Hardcoded Thread Diameter:**
   - `data.filamentDiameter = 1.75` (line in SensorManager.cpp)
   - Always assumes 1.75mm filament
   - No UI to change this parameter
   - Consider adding to N value editor or material settings

3. **Material String Storage:**
   - Material names stored as strings in CSV
   - No material-specific properties (density, expected range, etc.)
   - Could be enhanced with material profile database

4. **Touch Calibration:**
   - Calibration values hardcoded for specific display and rotation
   - Touch input may need recalibration if display is replaced
   - Calibration not accessible from UI

5. **RTC Time Adjustment:**
   - Time set at upload from compiler timestamp
   - No user interface to adjust time manually
   - Requires re-upload to correct time drift
   - DS3231 maintains time during power loss (has battery)

6. **Limited Session Storage:**
   - SD card can store 128 sessions max (array size in getAllSessionIds)
   - Beyond 128, behavior undefined
   - Recommend implementing session archiving

7. **No Data Export:**
   - Sessions visible on device, but no USB/wireless export
   - CSV must be read via SD card reader on PC
   - No real-time streaming option

8. **Sample Interval Fixed:**
   - 1-second sampling interval hardcoded in SessionManager
   - `const unsigned long sampleInterval = 1000` (milliseconds)
   - No UI to adjust sampling rate

9. **Calibration Mode Incomplete:**
   - Calibration screen (C) drawn but button handlers not fully implemented
   - +/- buttons may not actually update calibration factor
   - Recommend verifying calibration flow

10. **No Error Recovery:**
    - Failed SD writes don't trigger alerts
    - Failed RTC init doesn't prevent system boot
    - No watchdog timer for hang recovery

### 9.2 Hardware Limitations

1. **Load Cell Resolution:**
   - HX711 provides 24-bit ADC output
   - Calibration factor is very large (-4083333)
   - May suffer from noise on lower-resolution filament materials
   - Recommend low-pass filtering if noise observed

2. **DHT11 Sensor Limitations:**
   - ±2°C temperature accuracy
   - ±5% humidity accuracy
   - ~2 second response time
   - Only one reading per loop iteration (no internal buffering)

3. **Display Touchscreen Drift:**
   - Resistive touch can drift over temperature changes
   - Calibration valid only for current conditions
   - No temperature-compensated calibration

4. **Battery Monitoring:**
   - Uses single ADC reading without filtering
   - No hysteresis on LED color transitions
   - May flutter between colors near thresholds

5. **Memory Constraints:**
   - Arduino Mega has 8KB SRAM (limited for image buffers)
   - No dynamic memory allocation (all arrays pre-sized)
   - Graph limited to 50 points max

6. **SD Card Compatibility:**
   - SdFat library used; may not support all card types
   - No hotplug support (card must be inserted before power)
   - Slow write speeds for large CSV files

### 9.3 Data Quality Considerations

1. **Weight Scaling Uncertainty:**
   - Formula `weight = -scale.get_units(5) * 400 * 100` appears ad-hoc
   - Scaling factors (400, 100) not documented
   - Need to verify unit output matches physical measurements

2. **Timestamp Resolution:**
   - Uses `millis()` for sample timestamp (1ms resolution)
   - Sufficient for 1Hz sampling but may lose precision in analysis

3. **Environmental Drift:**
   - Temperature/humidity recorded once per loop
   - May not reflect true average during session
   - Consider periodic averaging if high precision needed

4. **Limit Switch Hysteresis:**
   - No debouncing implemented for limit switch
   - May record false triggers on noisy signal
   - Recommend adding software debouncing (e.g., 50ms hold)

### 9.4 Missing Documentation & Unclear Elements

1. **Calibration Factor Origin:** Where does -4083333 come from? (Needs verification)
2. **Weight Conversion Rationale:** Why multiply by -400 * 100? (Physics/engineering rationale needed)
3. **Material List Completeness:** Are 7 materials sufficient or extensible?
4. **Test Load Parameter (N):** What does "N" represent? (Newtons? Load target?)
5. **Hardness Test Mode:** Functionality not fully implemented; purpose unclear
6. **Graph Axes Labels:** X-axis says "Time (s)" but max value is 10, unclear if this is scaled

---

## 10. Future Enhancements

### 10.1 Software Improvements

1. **Increase Buffer Size:**
   - Expand `samples[10]` to `samples[100]` or dynamic allocation
   - Requires additional SRAM evaluation
   - Benefit: Capture longer test sessions

2. **Configurable Sampling Rate:**
   - Add UI to select sampling interval (500ms, 1s, 2s, 5s)
   - Store preference in EEPROM
   - Benefit: Trade-off between resolution and storage

3. **Material Profiles:**
   - Create database of material properties
   - Store expected stress ranges, color coding for pass/fail
   - Load profile on material selection
   - Benefit: Real-time validation and alerts

4. **Manual Time Adjustment:**
   - Add screen to set RTC date/time via touchscreen
   - Prevent reliance on compile-time adjustment
   - Benefit: Correct time tracking after power loss

5. **Noise Filtering:**
   - Implement moving-average or low-pass filter on HX711 readings
   - Reduce jitter on display and data logging
   - Benefit: Cleaner stress-strain curves

6. **User Alerts:**
   - Display pop-up warnings on sensor failures
   - Log errors to SD card
   - Audio/visual alert on test limits exceeded
   - Benefit: Improved troubleshooting and data integrity

7. **Data Export via USB:**
   - Implement USB serial or CDC protocol
   - Stream session data to PC without removing SD card
   - Benefit: Faster data analysis workflow

8. **Session Compression:**
   - Implement gzip or RLE compression for CSV storage
   - Extend storage capacity beyond 128 sessions
   - Benefit: Archive more history on fixed SD card

9. **Calibration History:**
   - Log calibration factor changes with timestamp
   - Allow rollback to previous calibration
   - Benefit: Track instrument drift over time

10. **Wi-Fi/Bluetooth Connectivity:**
    - Add Wi-Fi module (ESP32 integration already partially present)
    - Real-time cloud data logging
    - Remote monitoring and control
    - Benefit: Multi-device access and data backup

### 10.2 Hardware Improvements

1. **Higher Resolution Load Cell:**
   - Upgrade to 50kg or 100kg HX711 load cell
   - Provides finer granularity for small filaments
   - Benefit: Improved measurement precision

2. **Better Environmental Sensor:**
   - Replace DHT11 with DHT22 or BME680
   - ±0.5°C accuracy, faster response
   - Benefit: More reliable environment tracking

3. **Strain Gauge Backup:**
   - Implement secondary load measurement method
   - Cross-reference HX711 readings for validation
   - Benefit: Detect sensor failures

4. **Rotary Encoder for N Value:**
   - Replace touchscreen N adjustment with rotary knob
   - Faster parameter changes, more tactile feedback
   - Benefit: Improved ergonomics during testing

5. **Larger Display:**
   - Upgrade to 4" or 5" screen
   - Higher resolution (1024x600 or better)
   - Benefit: Larger data display, reduced eye strain

6. **Battery Management IC:**
   - Replace simple voltage divider with dedicated charger/monitor
   - Li-Po battery with overcharge/discharge protection
   - Display accurate SoC (State of Charge)
   - Benefit: Safer power management, longer battery life

7. **Backup Power:**
   - Add UPS/super-capacitor for graceful shutdown
   - Final session saved even on power loss
   - Benefit: Data integrity during unexpected power cycles

### 10.3 Feature Additions

1. **Multi-Test Averaging:**
   - Run multiple tests back-to-back
   - Compute statistical analysis across tests
   - Benefit: Material property variability assessment

2. **Temperature Compensation:**
   - Auto-adjust calibration based on measured temperature
   - Compensate for HX711 thermal drift
   - Benefit: More accurate readings across temperature range

3. **Test Templates:**
   - Pre-defined test profiles (quick setup)
   - Industry standards (ASTM, ISO) built-in
   - Benefit: Faster test execution, standardized results

4. **Live Data Streaming Chart:**
   - Display real-time stress-strain curve during test
   - Update plot every 500ms or faster
   - Benefit: Visual feedback of test progress

5. **Pass/Fail Criteria:**
   - Define acceptable stress ranges per material
   - Auto-flag out-of-spec results
   - Benefit: Quality control integration

6. **Test Report Generation:**
   - Generate PDF report with graphs and statistics
   - Print-friendly layout
   - Benefit: Paperless documentation

7. **Data Analysis Tools:**
   - Compute tensile strength, modulus, elongation
   - Statistical analysis (std dev, confidence intervals)
   - Benefit: Advanced material characterization

### 10.4 Robustness Improvements

1. **Watchdog Timer:**
   - Detect firmware hangs and auto-reset
   - Prevent unresponsive system states
   - Benefit: 24/7 reliability

2. **Comprehensive Logging:**
   - Event log with timestamps (startup, calibration, errors)
   - Diagnostic information for troubleshooting
   - Benefit: Post-failure analysis

3. **Graceful Degradation:**
   - Continue operation if non-critical sensor fails
   - Display warning instead of complete shutdown
   - Benefit: Improved uptime

4. **Settings Backup:**
   - Store calibration and material profiles to SD
   - Auto-restore on boot
   - Benefit: Configuration persistence across updates

---

## Appendix A: Electrical Schematic Notes

### Pin Usage Summary

**Digital Pins (used):**
- Pin 2: TARE_BUTTON_PIN (not actively used)
- Pin 3: CALIBRATE_BUTTON_PIN (not actively used)
- Pin 4: READ_BUTTON_PIN (not actively used)
- Pin 5: LOADCELL_DOUT_PIN (HX711 data)
- Pin 6: LOADCELL_SCK_PIN (HX711 clock)
- Pin 8: TFT_DC (Display data/command)
- Pin 9: TFT_RST (Display reset)
- Pin 10: TFT_CS (Display chip select)
- Pin 22: LED_PIN (NeoPixel data)
- Pin 23: LIMIT_SWITCH_PIN
- Pin 24: DHTPIN (DHT11 data)
- Pin 26: LOADCELL_DOUT_PIN (duplicate, unused)
- Pin 28: LOADCELL_SCK_PIN (duplicate, unused)
- Pin 53: SD_CS_PIN (SD card chip select)

**Analog Pins (used):**
- A8: batteryPin (Battery voltage monitoring)
- A2, A3: Touchscreen interface (calibrated coordinates)
- A0-A1, A4: TFT parallel interface
- A4, A5: I2C (SDA, SCL for RTC)

**SPI Pins (shared):**
- Pin 50: MISO (shared with SD and TFT)
- Pin 51: MOSI (shared with SD and TFT)
- Pin 52: SCK (shared with SD and TFT)

---

## Appendix B: Default Calibration Values

| Parameter | Value | Unit | Notes |
|-----------|-------|------|-------|
| CALIBRATION_FACTOR | -4083333 | dimensionless | Applied to HX711 raw output |
| DEFAULT_CALIBRATION_FACTOR | 420.0 | dimensionless | Initial/fallback value |
| currentN | 20.0 | N (Newtons?) | Default test load parameter |
| currentN range | 20.0 - 70.0 | N | Adjustable via UI |
| currentN step | 10, 1, 0.1 | N | Three adjustment rates |
| sampleInterval | 1000 | ms | Time between samples |
| MAX_SESSION_SAMPLES | 10 | count | Circular buffer depth |
| MAX_POINTS (graph) | 50 | count | Graph visualization limit |
| refreshInterval | 500 | ms | UI screen update rate |
| blinkInterval | 500 | ms | LED blink frequency (if enabled) |

---

## Appendix C: Build and Upload Instructions

### Prerequisites
- Arduino IDE v1.8.x or later
- Arduino Mega 2560 board selected in IDE
- Required libraries installed:
  - DHT (temperature/humidity)
  - HX711 (load cell)
  - RTClib (DS3231 real-time clock)
  - Adafruit_NeoPixel (status LED)
  - SdFat (SD card)
  - LCDWIKI_KBV (display)
  - TouchScreen
  - MCUFRIEND_kbv (alternative display lib)

### Build Steps
1. Open `DTX.ino` in Arduino IDE
2. Select Tools → Board → Arduino Mega 2560
3. Select Tools → Port → (appropriate COM port)
4. Verify code: Sketch → Verify/Compile
5. Upload: Sketch → Upload

### Post-Upload
1. Monitor serial output at 115200 baud
2. Verify RTC, SD, LED, and display initialization messages
3. Test touchscreen in 'T' screen mode
4. Perform load cell calibration if needed

---

## Document Control

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | Dec 2025 | Documentation Generator | Initial comprehensive documentation |

---

**End of Documentation**

For questions or clarifications regarding this documentation, please refer to the source code comments or contact the project maintainers.
