# WizardBall LED Controller – Code Generation Optimized PRD (Enhanced v2)

## 1. Purpose

This document defines precise, unambiguous requirements for firmware controlling a fixed number of RGB LEDs to simulate stellar phenomena. It is explicitly written to maximize correctness and consistency when used with automated code generation tools (e.g., Claude Code, Codex).

---

## 2. Target Platform and Constraints

### 2.1 Hardware

- **MCU**: Adafruit QT Py ESP32 (Adafruit #5426)
- **LEDs**: 50× Adafruit NeoPixel RGB LEDs (Adafruit #4560)
- **GPIO Pin**: Pin A3 (GPIO 17) for LED data output
- **Power**: USB 5V, budget for ~1A LED current at full brightness

### 2.2 Software

- **Language**: Arduino C++ (C++11 or later)
- **LED Library**: FastLED (version 3.6.0 or later)
- **Framework**: Arduino ESP32 core (version 2.x or later)
- **Documentation**: All functions must include Doxygen-compatible comments with `@brief`, `@param`, `@return` tags

### 2.3 Execution Model

- Firmware shall use `setup()` and `loop()`
- Timing shall be **non-blocking** and based on `millis()`
- `delay()` shall **not** be used except during initialization if explicitly required
  - Maximum allowable delay in `setup()`: 100 ms
- **LED update cadence**: 20 ms (50 Hz), controlled by frame timer
- **Frame timer implementation pattern**:
  ```cpp
  static uint32_t lastFrame = 0;
  uint32_t now = millis();
  if (now - lastFrame >= 20) {
    lastFrame = now;
    // Update LED logic here
    FastLED.show();
  }
  ```

---

## 3. Global Definitions

### 3.1 LED Configuration

```cpp
constexpr uint16_t LED_COUNT = 50;
constexpr uint8_t LED_PIN = A3;      // GPIO 17 on QT Py ESP32
constexpr uint8_t LED_TYPE = WS2812B;
constexpr EOrder LED_ORDER = GRB;    // NeoPixel color order
```

### 3.2 Brightness Range

- Valid brightness range: `[0, 255]` (`uint8_t`)
- All computed brightness values **shall be clamped** to this range using:
  ```cpp
  brightness = constrain(brightness, 0, 255);
  ```

### 3.3 Randomness

- **Seeding**: RNG shall be seeded once during `setup()` using a hardware entropy source
  ```cpp
  randomSeed(analogRead(A0));  // A0 is unconnected, provides noise
  ```
- **Selection timing**: All random selections described below are made **once per phase entry** unless otherwise stated
- Random values remain fixed for the duration of the phase
- **Random ranges**: All `random()` calls use inclusive lower bound, exclusive upper bound

### 3.4 Memory Management

- **Static allocation preferred**: All LED arrays shall be statically allocated
- **LED buffer**: `CRGB leds[LED_COUNT];`
- **Dynamic allocation**: Permitted for per-LED state but discouraged
- **Maximum auxiliary state per phase**: 512 bytes

---

## 4. Star Color Palette

Only colors visibly emitted by stars within the human visual spectrum are permitted.

### 4.1 Allowed Stellar Colors (Human-Visible)

The palette shall include **all human-visible stellar colors**, spanning the full range of stellar spectral classes as perceived by the human eye.

```cpp
const CRGB STELLAR_COLORS[] = {
  CRGB(155, 176, 255),  // O - Blue
  CRGB(170, 191, 255),  // B - Blue-White
  CRGB(202, 215, 255),  // A - White
  CRGB(248, 247, 255),  // F - Yellow-White
  CRGB(255, 244, 234),  // G - Yellow
  CRGB(255, 210, 161),  // K - Orange
  CRGB(255, 204, 111)   // M - Red
};
constexpr uint8_t STELLAR_COLOR_COUNT = 7;
```

**Color Selection Helper**:
```cpp
/**
 * @brief Get a random stellar color from the palette
 * @return CRGB color value
 */
CRGB randomStellarColor() {
  return STELLAR_COLORS[random(0, STELLAR_COLOR_COUNT)];
}
```

**Constraints**:
- RGB values approximate black-body radiation mapped to sRGB
- No green or purple hues are permitted (stars don't emit these to human eyes)
- Colors outside this table are invalid
- No dynamic or procedural color generation is allowed
- No color interpolation between palette entries

---

## 5. System Architecture

### 5.1 State Machine

The system shall be implemented as a finite state machine with the following states:

```cpp
enum class SystemState : uint8_t {
  INIT,
  WARBLE,
  SUPERNOVA,
  BLACK_HOLE
};
```

#### State Management Functions

Each state shall define these handlers:

```cpp
// Called once when entering the state
void enterInit();
void enterWarble();
void enterSupernova();
void enterBlackHole();

// Called every frame (20ms) while in the state
void updateInit(uint32_t nowMs);
void updateWarble(uint32_t nowMs);
void updateSupernova(uint32_t nowMs);
void updateBlackHole(uint32_t nowMs);

// Called once when leaving the state
void exitInit();
void exitWarble();
void exitSupernova();
void exitBlackHole();
```

#### State Transition Logic

```cpp
SystemState currentState = SystemState::INIT;

/**
 * @brief Transition to a new state with proper cleanup
 * @param newState The state to transition to
 */
void transitionTo(SystemState newState) {
  // Exit current state
  switch (currentState) {
    case SystemState::INIT: exitInit(); break;
    case SystemState::WARBLE: exitWarble(); break;
    case SystemState::SUPERNOVA: exitSupernova(); break;
    case SystemState::BLACK_HOLE: exitBlackHole(); break;
  }
  
  currentState = newState;
  
  // Enter new state
  switch (newState) {
    case SystemState::INIT: enterInit(); break;
    case SystemState::WARBLE: enterWarble(); break;
    case SystemState::SUPERNOVA: enterSupernova(); break;
    case SystemState::BLACK_HOLE: enterBlackHole(); break;
  }
}
```

#### State Transition Sequence

```
INIT → WARBLE → SUPERNOVA → BLACK_HOLE → WARBLE → SUPERNOVA → BLACK_HOLE → ...
```

**Cycle pattern**: After BLACK_HOLE completes, system returns to WARBLE (not INIT).

---

## 6. Phase Specifications

### 6.1 INIT Phase

#### Purpose
Visual power-on animation to establish initial stellar field.

#### Entry Conditions
- System power-on or reset only (never re-entered after initial boot)
- All LEDs start off: `fill_solid(leds, LED_COUNT, CRGB::Black)`

#### Behavior

**Sequential Animation**:
- LEDs are processed one at a time from index `0` to `LED_COUNT − 1`
- Only one LED animates at a time; all others remain off

**Per-LED Animation**:
- **Color assignment**: Assign a random stellar color (selected once per LED)
  ```cpp
  ledColor[i] = randomStellarColor();
  ```
- **Brightness profile**: Symmetric triangle wave `0 → 255 → 0`
- **Total duration per LED**: 5000 ms (5 seconds)
  - Ramp up phase: 0–2500 ms → brightness 0 to 255
  - Ramp down phase: 2500–5000 ms → brightness 255 to 0

**Mathematical Formula**:
```cpp
// t = milliseconds since this LED started animating
uint8_t calculateInitBrightness(uint32_t t) {
  if (t < 2500) {
    // Ramp up: 0 → 255
    return (uint8_t)((t * 255UL) / 2500UL);
  } else if (t < 5000) {
    // Ramp down: 255 → 0
    return (uint8_t)(255 - ((t - 2500) * 255UL) / 2500UL);
  } else {
    // Complete
    return 0;
  }
}
```

**Total Phase Duration**: `50 LEDs × 5 seconds = 250 seconds (4 min 10 sec)`

#### Exit Condition
- The final LED (index 49) completes its ramp-down (`elapsed >= 5000 ms`)
- **All LED colors are preserved** from their INIT assignment
- Transition immediately to WARBLE

#### Implementation Notes
- **State variables**:
  - Current LED index: `uint8_t currentLED`
  - LED activation time: `uint32_t ledStartTime`
  - Per-LED colors: `CRGB ledColors[LED_COUNT]` (preserved for all subsequent phases)
- **Memory budget**: ~150 bytes (LED colors array)

---

### 6.2 WARBLE Phase

#### Purpose
Long-duration stochastic stellar shimmer simulating atmospheric turbulence effects.

#### Entry Conditions
- Entered after INIT phase completes, or after BLACK_HOLE phase completes
- All LEDs retain their assigned stellar colors from INIT

#### Random Parameters (selected once at phase entry)

**Phase Duration**:
```cpp
// Random duration: 3 to 10 minutes (180,000 to 600,000 ms)
uint32_t warbleDuration = random(180000, 600001);  // Exclusive upper bound
```

**LED Category Assignment**:

Each LED is randomly assigned to one of four brightness categories:

| Category | Percentage | LED Count | Base Brightness Range |
|----------|-----------|-----------|----------------------|
| Low      | 20%       | 10 LEDs   | `[0, 50]`           |
| Medium   | 50%       | 25 LEDs   | `[50, 125]`         |
| High     | 25%       | 12 LEDs   | `[125, 200]`        |
| Max      | 5%        | 3 LEDs    | `[200, 255]`        |

**Assignment Algorithm**:
```cpp
// Shuffle LED indices, then assign first 10 to Low, next 25 to Medium, etc.
// OR: For each LED, random weighted selection ensuring counts match

struct LEDState {
  uint8_t baseBrightness;  // Random value within category range
  uint8_t category;        // 0=Low, 1=Medium, 2=High, 3=Max
};
```

**Brightness Modulation Groups**:

Each LED is randomly assigned to one of three modulation groups:

| Group  | Percentage | LED Count | Modulation Behavior |
|--------|-----------|-----------|---------------------|
| Static | 20%       | 10 LEDs   | No brightness change |
| Slow   | 75%       | 37 LEDs   | ±25–50 over 120 s   |
| Strong | 5%        | 3 LEDs    | ±50–100 over 120 s  |

**Modulation Parameters** (per LED in Slow/Strong groups):
```cpp
struct ModulationParams {
  uint8_t amplitude;    // Random in [25, 50] for Slow, [50, 100] for Strong
  uint32_t period;      // Fixed: 120000 ms (120 seconds)
  float phaseOffset;    // Random in [0, TWO_PI] for visual variety
};
```

#### Behavior

**Brightness Calculation** (per frame, per LED):

For **Static** LEDs:
```cpp
brightness = baseBrightness;  // No change
```

For **Slow** and **Strong** LEDs:
```cpp
// Sine wave modulation
float elapsed = (nowMs - warbleStartTime) / 1000.0;  // seconds
float phase = (elapsed / 120.0) * TWO_PI + phaseOffset[i];
int16_t modulation = amplitude[i] * sin(phase);
int16_t brightness = baseBrightness[i] + modulation;
brightness = constrain(brightness, 0, 255);
```

**Visual Effect**:
- LEDs shimmer independently with smooth, periodic variations
- Brighter LEDs (High/Max categories) are more prominent
- Slow modulation creates gentle, organic twinkling
- Strong modulation creates occasional dramatic flares

#### Exit Condition
- Phase duration expires: `(nowMs - warbleStartTime) >= warbleDuration`
- Transition immediately to SUPERNOVA

#### Implementation Notes
- **State variables**:
  - Phase start time: `uint32_t warbleStartTime`
  - Phase duration: `uint32_t warbleDuration`
  - Per-LED state: `LEDState ledStates[LED_COUNT]`
  - Per-LED modulation: `ModulationParams modParams[LED_COUNT]`
- **Memory budget**: ~400 bytes
- **Optimization**: Pre-calculate as much as possible at phase entry to avoid per-frame overhead

---

### 6.3 SUPERNOVA Phase

#### Purpose
Rapid full-brightness expansion simulating stellar explosion.

#### Entry Conditions
- Entered after WARBLE phase completes
- All LEDs retain their stellar colors

#### Random Parameters (selected once at phase entry)

**Epicenter LED**:
```cpp
uint8_t epicenterIndex = random(0, LED_COUNT);
```

#### Behavior

**Timeline**:

1. **Initial Delay**: 2000 ms (2 seconds)
   - All LEDs remain at their WARBLE brightness
   - Builds tension before explosion

2. **Expansion Phase**: Variable duration
   - **Starting point**: Epicenter LED illuminates to brightness 255
   - **Propagation**: Illuminate LEDs outward symmetrically in both directions
   - **Delay pattern**: Delay between steps decreases linearly
     - Initial delay (first step): 2000 ms
     - Minimum delay (final steps): 100 ms
     - **Linear decrease formula**:
       ```cpp
       // totalSteps = maximum distance from epicenter to either end
       // step = current step number (0 to totalSteps-1)
       uint32_t stepDelay = 2000 - ((2000 - 100) * step / (totalSteps - 1));
       stepDelay = constrain(stepDelay, 100, 2000);
       ```
   
   **Symmetric Expansion Algorithm**:
   ```cpp
   // Start from epicenter, expand outward
   // Step 0: epicenterIndex
   // Step 1: epicenterIndex±1
   // Step 2: epicenterIndex±2
   // etc.
   
   for (int step = 0; step < maxDistance; step++) {
     // Set LEDs at epicenterIndex - step
     // Set LEDs at epicenterIndex + step
     // (with bounds checking)
     // Wait stepDelay
     // All LEDs set this step go to brightness 255
   }
   ```

3. **Full Illumination Hold**: 10 seconds
   - All LEDs at brightness 255
   - Colors remain as assigned in INIT

**Total Phase Duration**: ~2s initial + ~20s expansion (varies by epicenter) + 10s hold ≈ 32 seconds

#### Exit Condition
- Hold duration completes: 10 seconds after all LEDs reach full brightness
- Transition immediately to BLACK_HOLE

#### Implementation Notes
- **State variables**:
  - Phase start time: `uint32_t supernovaStartTime`
  - Epicenter index: `uint8_t epicenterIndex`
  - Current expansion step: `uint8_t currentStep`
  - Last step time: `uint32_t lastStepTime`
  - Total steps: `uint8_t totalSteps`
  - Hold start time: `uint32_t holdStartTime`
- **Memory budget**: ~20 bytes
- **Non-blocking implementation**: Use state sub-phases (DELAY, EXPAND, HOLD)

---

### 6.4 BLACK_HOLE Phase

#### Purpose
Simultaneous collapse to darkness simulating gravitational singularity.

#### Entry Conditions
- Entered after SUPERNOVA phase completes
- All LEDs start at brightness 255

#### Fixed Parameters

```cpp
constexpr uint32_t BLACK_HOLE_DURATION = 30000;  // 30 seconds
constexpr uint32_t UPDATE_INTERVAL = 100;        // 100 ms between updates
constexpr uint16_t TOTAL_STEPS = BLACK_HOLE_DURATION / UPDATE_INTERVAL;  // 300 steps
```

#### Behavior

**Synchronized Fade**:
- All LEDs fade from brightness 255 to 0 over exactly 30 seconds
- Updates occur every 100 ms (300 total updates)
- Each LED has a **unique decrement value** to ensure simultaneous arrival at zero

**Decrement Calculation**:

To achieve perfect synchronization despite integer rounding:

```cpp
// At phase entry, for each LED:
struct FadeParams {
  uint8_t currentBrightness;  // Starts at 255
  uint8_t decrement;          // Per-step decrease
  uint8_t remainder;          // Tracks rounding error
};

// Initialize per LED:
void initializeFade(uint8_t ledIndex) {
  fadeParams[ledIndex].currentBrightness = 255;
  
  // Random decrement ensures visual variety while hitting zero
  // Use different decrements per LED, but all must reach 0 in 300 steps
  // Simple approach: decrement = 255 / 300 ≈ 0.85 per step
  // Better approach: Add jitter around average
  
  uint8_t baseDecrement = 255 / TOTAL_STEPS;  // 0 (integer division)
  // Use fractional tracking instead:
  // Each step: brightness -= 255.0 / 300.0 = 0.85
}
```

**Better Implementation Using Fractional Progress**:

```cpp
// Avoid decrement calculation complexity
void updateBlackHole(uint32_t nowMs) {
  uint32_t elapsed = nowMs - blackHoleStartTime;
  
  if (elapsed >= BLACK_HOLE_DURATION) {
    // All LEDs to zero, transition out
    fill_solid(leds, LED_COUNT, CRGB::Black);
    transitionTo(SystemState::WARBLE);
    return;
  }
  
  // Only update every 100ms
  if (nowMs - lastUpdateTime >= UPDATE_INTERVAL) {
    lastUpdateTime = nowMs;
    
    // Linear fade: progress from 1.0 to 0.0
    float progress = 1.0 - (float)elapsed / (float)BLACK_HOLE_DURATION;
    
    for (uint8_t i = 0; i < LED_COUNT; i++) {
      // Apply fade to existing color
      uint8_t brightness = (uint8_t)(255 * progress);
      leds[i] = ledColors[i];  // Original color from INIT
      leds[i].nscale8(brightness);
    }
  }
}
```

**Visual Effect**:
- All LEDs fade smoothly and simultaneously
- Colors shift through dimmer shades of their original hues
- Final second: barely visible glow → complete darkness
- No flickering or uneven fading

#### Exit Condition
- Full duration elapsed: `(nowMs - blackHoleStartTime) >= 30000`
- All LEDs at brightness 0
- Transition immediately to WARBLE (creating a cycle)

#### Implementation Notes
- **State variables**:
  - Phase start time: `uint32_t blackHoleStartTime`
  - Last update time: `uint32_t lastUpdateTime`
- **Memory budget**: ~8 bytes
- **Optimization**: Use `FastLED.setBrightness()` for uniform scaling, or `nscale8()` per LED

---

## 7. Data Structures

### 7.1 Core LED State

```cpp
// Global LED buffer
CRGB leds[LED_COUNT];

// Per-LED persistent color (set in INIT, preserved forever)
CRGB ledColors[LED_COUNT];
```

### 7.2 Phase-Specific State

**INIT Phase**:
```cpp
struct InitState {
  uint8_t currentLED;
  uint32_t ledStartTime;
};
```

**WARBLE Phase**:
```cpp
enum BrightnessCategory : uint8_t { LOW, MEDIUM, HIGH, MAX };
enum ModulationGroup : uint8_t { STATIC, SLOW, STRONG };

struct WARBLEState {
  uint32_t phaseStartTime;
  uint32_t phaseDuration;
  
  struct PerLED {
    uint8_t baseBrightness;
    BrightnessCategory category;
    ModulationGroup group;
    uint8_t amplitude;      // Only for SLOW/STRONG
    float phaseOffset;      // Only for SLOW/STRONG
  } leds[LED_COUNT];
};
```

**SUPERNOVA Phase**:
```cpp
enum SupernovaSubState : uint8_t { INITIAL_DELAY, EXPANDING, HOLDING };

struct SUPERNOVAState {
  uint32_t phaseStartTime;
  uint8_t epicenterIndex;
  SupernovaSubState subState;
  uint8_t currentStep;
  uint8_t totalSteps;
  uint32_t lastStepTime;
  uint32_t holdStartTime;
};
```

**BLACK_HOLE Phase**:
```cpp
struct BLACKHOLEState {
  uint32_t phaseStartTime;
  uint32_t lastUpdateTime;
};
```

### 7.3 Allocation Strategy

- **Static allocation preferred** for all state structures
- Use union to overlay phase-specific state (only one phase active at a time):
  ```cpp
  union PhaseState {
    InitState init;
    WARBLEState warble;
    SUPERNOVAState supernova;
    BLACKHOLEState blackHole;
  } phaseState;
  ```
- Dynamic allocation permitted for per-LED arrays if absolutely necessary

---

## 8. Mathematical Helpers

### 8.1 Required Utility Functions

```cpp
/**
 * @brief Clamp value to valid brightness range
 * @param value Input value (may be outside range)
 * @return Clamped value in [0, 255]
 */
uint8_t clampBrightness(int16_t value) {
  return (uint8_t)constrain(value, 0, 255);
}

/**
 * @brief Linear interpolation between two values
 * @param a Start value
 * @param b End value
 * @param t Interpolation factor [0.0, 1.0]
 * @return Interpolated value
 */
uint8_t lerp8(uint8_t a, uint8_t b, float t) {
  t = constrain(t, 0.0f, 1.0f);
  return a + (uint8_t)((b - a) * t);
}

/**
 * @brief Calculate symmetric distance from epicenter
 * @param index LED index
 * @param epicenter Epicenter LED index
 * @return Minimum distance considering both directions
 */
uint8_t symmetricDistance(uint8_t index, uint8_t epicenter) {
  if (index >= epicenter) {
    return index - epicenter;
  } else {
    return epicenter - index;
  }
}

/**
 * @brief Map step number to delay duration (linear decrease)
 * @param step Current step (0 to maxSteps-1)
 * @param maxSteps Total number of steps
 * @param startDelay Initial delay (2000 ms)
 * @param endDelay Final delay (100 ms)
 * @return Delay in milliseconds
 */
uint32_t mapStepDelay(uint8_t step, uint8_t maxSteps, uint32_t startDelay, uint32_t endDelay) {
  if (maxSteps <= 1) return endDelay;
  uint32_t range = startDelay - endDelay;
  uint32_t decrease = (range * step) / (maxSteps - 1);
  return startDelay - decrease;
}
```

### 8.2 Floating-Point Usage Guidelines

- **Avoid** floating-point math in per-frame loops when possible
- **Permitted** for one-time calculations at phase entry
- **Permitted** for brightness modulation if optimized (pre-calculate periods)
- **Use** integer-only alternatives where performance-critical:
  ```cpp
  // Instead of: float(x) / float(y)
  // Use: (x * 256) / y  // Fixed-point math
  ```

---

## 9. Performance Constraints

### 9.1 Memory Limits

- **Heap usage**: Must remain below **50 KB**
  - Monitor with: `ESP.getFreeHeap()`
  - Log at startup and phase transitions (optional)
- **Stack usage**: Keep local variables minimal in deep call chains
- **Flash usage**: Must fit within **1 MB** program space

### 9.2 Timing Requirements

- **LED update processing**: Shall not exceed **1 ms per frame**
  - Measure with: `micros()` before and after LED calculations
  - Target: 500 μs average
- **Loop iteration time**: Target < 20 ms average to maintain 50 Hz
- **Frame cadence**: Must maintain 20 ms (±2 ms tolerance acceptable)

### 9.3 Watchdog Timer

- Enable ESP32 hardware watchdog with 5-second timeout:
  ```cpp
  // In setup():
  esp_task_wdt_init(5, true);  // 5 sec timeout, panic on timeout
  esp_task_wdt_add(NULL);      // Add current task
  ```
- Call `yield()` or `esp_task_wdt_reset()` in `loop()` to prevent reset

---

## 10. Documentation Requirements

### 10.1 Code Comments

- **All functions** must include Doxygen-compatible comments:
  ```cpp
  /**
   * @brief Brief description of function
   * @param paramName Description of parameter
   * @return Description of return value
   */
  ```
- **Parameters, return values, and side effects** must be documented
- **State transitions** clearly commented in transition logic
- **Magic numbers** must be replaced with named constants and explained

### 10.2 Inline Comments

- Complex algorithms must include inline comments explaining:
  - Mathematical formulas and their derivations
  - Non-obvious optimization choices
  - Edge case handling

### 10.3 README File

Create a README.md with:
- Hardware connection diagram (pin assignments)
- Library dependencies and versions
- Upload instructions for QT Py ESP32
- Visual description of each phase
- Troubleshooting section

---

## 11. Code Structure

### 11.1 File Organization

```
WizardBall/
├── WizardBall.ino           // Main sketch: setup(), loop()
├── config.h                  // Global constants, pin definitions
├── colors.h                  // Stellar color palette
├── state_machine.h/cpp       // State transition logic
├── phase_init.h/cpp          // INIT phase implementation
├── phase_warble.h/cpp        // WARBLE phase implementation
├── phase_supernova.h/cpp     // SUPERNOVA phase implementation
├── phase_blackhole.h/cpp     // BLACK_HOLE phase implementation
├── utils.h/cpp               // Math helpers
└── README.md                 // Documentation
```

### 11.2 Naming Conventions

- **Constants**: `UPPER_SNAKE_CASE` (e.g., `LED_COUNT`, `UPDATE_INTERVAL`)
- **Functions**: `camelCase` (e.g., `updateWarble`, `randomStellarColor`)
- **Classes/Structs/Enums**: `PascalCase` (e.g., `SystemState`, `WARBLEState`)
- **Variables**: `camelCase` (e.g., `currentLED`, `phaseStartTime`)
- **Private members**: Prefix with underscore `_memberName` (if using classes)

### 11.3 Include Guards

All header files must use:
```cpp
#ifndef FILENAME_H
#define FILENAME_H
// ... content ...
#endif
```

---

## 12. Error Handling

### 12.1 Startup Validation

In `setup()`, validate:
```cpp
void setup() {
  // Initialize FastLED
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, LED_COUNT);
  FastLED.setBrightness(255);
  
  // Seed RNG
  randomSeed(analogRead(A0));
  
  // Validate hardware (optional)
  Serial.begin(115200);
  Serial.println("WizardBall v1.0 - Initializing...");
  Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
  
  // Initialize to INIT state
  transitionTo(SystemState::INIT);
}
```

### 12.2 Runtime Safeguards

- **Array bounds checking**: Use constrain for all LED indices:
  ```cpp
  ledIndex = constrain(ledIndex, 0, LED_COUNT - 1);
  ```
- **Overflow protection**: Use `uint32_t` for all `millis()` timestamps
  - Handles 49-day rollover automatically with unsigned arithmetic
- **Brightness clamping**: Always apply `constrain()` or `clampBrightness()`
- **Division by zero**: Validate denominators in calculations

### 12.3 Fail-Safe Behavior

- If any phase encounters an error, log to Serial and transition to WARBLE
- Never allow the system to hang or enter an infinite loop
- Watchdog timer will reset system if `loop()` blocks for >5 seconds

---

## 13. Testing & Validation

### 13.1 Unit Tests (Manual Verification)

For each phase, verify:

**INIT Phase**:
- [ ] LEDs animate sequentially (0 to 49)
- [ ] Each LED displays correct brightness ramp (0→255→0)
- [ ] Timing is accurate: 5 seconds per LED (±200 ms tolerance)
- [ ] Only one LED is lit at a time
- [ ] Colors are from stellar palette
- [ ] Transitions to WARBLE after LED 49

**WARBLE Phase**:
- [ ] All LEDs shimmer with independent periods
- [ ] Brightness distribution matches category percentages
- [ ] Static LEDs remain constant
- [ ] Slow LEDs modulate gently
- [ ] Strong LEDs show dramatic variation
- [ ] Duration is between 3-10 minutes
- [ ] Transitions to SUPERNOVA at end

**SUPERNOVA Phase**:
- [ ] 2-second initial delay before expansion
- [ ] Epicenter LED illuminates first
- [ ] Expansion is symmetric from epicenter
- [ ] Step delays decrease linearly (2000→100 ms)
- [ ] All LEDs reach brightness 255
- [ ] Holds at full brightness for 10 seconds
- [ ] Transitions to BLACK_HOLE

**BLACK_HOLE Phase**:
- [ ] All LEDs start at brightness 255
- [ ] Fade is smooth (no flickering)
- [ ] All LEDs reach zero simultaneously
- [ ] Total duration is exactly 30 seconds (±500 ms)
- [ ] Updates occur every 100 ms
- [ ] Transitions back to WARBLE

### 13.2 Integration Tests

- [ ] Complete cycle: INIT → WARBLE → SUPERNOVA → BLACK_HOLE → WARBLE
- [ ] System runs for 1 hour without crashes
- [ ] Heap usage remains stable (check `ESP.getFreeHeap()` every 5 minutes)
- [ ] No watchdog timer resets occur
- [ ] Power cycle recovery: system restarts correctly in INIT

### 13.3 Performance Benchmarks

- [ ] Frame rate: Consistently 50 Hz (20 ms ±2 ms per frame)
- [ ] Heap usage: Stays below 50 KB throughout all phases
- [ ] LED update time: <1 ms per frame (measure with `micros()`)
- [ ] Loop iteration: <20 ms average (measure with `micros()`)

### 13.4 Visual Acceptance Criteria

- [ ] Colors match expected stellar palette (no green/purple)
- [ ] Brightness transitions are smooth (no sudden jumps)
- [ ] INIT animation is visually distinct (sequential reveal)
- [ ] WARBLE creates organic twinkling effect
- [ ] SUPERNOVA expansion is dramatic and symmetric
- [ ] BLACK_HOLE fade is simultaneous and elegant

---

## 14. Non-Goals

The following features are explicitly **out of scope**:

- ❌ External communication (WiFi, Bluetooth, Serial commands beyond debug)
- ❌ Persistent storage (EEPROM, SPIFFS)
- ❌ User input (buttons, sensors, touchpads)
- ❌ Web interface or remote control
- ❌ Battery management or low-power modes
- ❌ Audio synchronization
- ❌ Multiple animation presets or user customization

---

## 15. Acceptance Criteria

### Must Have ✅

- ✅ Visual behavior matches all four phase definitions exactly
- ✅ No blocking delays outside `setup()` (except initial hardware delay ≤100 ms)
- ✅ Deterministic behavior within each phase (same RNG seed → same visuals)
- ✅ Code compiles cleanly under Arduino IDE 2.x targeting ESP32
- ✅ All 50 LEDs function correctly (no dead pixels)
- ✅ Phases cycle indefinitely: INIT once, then WARBLE ↔ SUPERNOVA ↔ BLACK_HOLE

### Should Have ✅

- ✅ Comprehensive Doxygen comments on all functions
- ✅ Modular code structure (separate .h/.cpp files per phase)
- ✅ Performance metrics logged to Serial (heap, frame time)
- ✅ README with setup instructions

### Nice to Have 🎯

- 🎯 Serial commands to skip to specific phases (for testing/demo)
- 🎯 Global brightness scaling control (dim entire display)
- 🎯 Adjustable phase durations via compile-time constants
- 🎯 Visual error indicators (e.g., red flash on watchdog reset)

---

## 16. Example Pseudocode

### 16.1 Main Loop Structure

```cpp
void loop() {
  static uint32_t lastFrame = 0;
  uint32_t now = millis();
  
  // Frame rate limiting: 50 Hz (20 ms period)
  if (now - lastFrame >= 20) {
    lastFrame = now;
    
    // Update current phase
    switch (currentState) {
      case SystemState::INIT:
        updateInit(now);
        break;
      case SystemState::WARBLE:
        updateWarble(now);
        break;
      case SystemState::SUPERNOVA:
        updateSupernova(now);
        break;
      case SystemState::BLACK_HOLE:
        updateBlackHole(now);
        break;
    }
    
    // Push to LEDs
    FastLED.show();
  }
  
  // Prevent watchdog timeout
  yield();
}
```

### 16.2 WARBLE Phase Brightness Calculation

```cpp
void updateWarble(uint32_t nowMs) {
  uint32_t elapsed = nowMs - phaseState.warble.phaseStartTime;
  
  // Check exit condition
  if (elapsed >= phaseState.warble.phaseDuration) {
    transitionTo(SystemState::SUPERNOVA);
    return;
  }
  
  // Update each LED
  for (uint16_t i = 0; i < LED_COUNT; i++) {
    auto& ledState = phaseState.warble.leds[i];
    uint8_t brightness;
    
    if (ledState.group == ModulationGroup::STATIC) {
      // No modulation
      brightness = ledState.baseBrightness;
    } else {
      // Sine wave modulation
      float time = elapsed / 1000.0;  // Convert to seconds
      float phase = (time / 120.0) * TWO_PI + ledState.phaseOffset;
      int16_t modulation = ledState.amplitude * sin(phase);
      brightness = clampBrightness(ledState.baseBrightness + modulation);
    }
    
    // Apply color and brightness
    leds[i] = ledColors[i];
    leds[i].nscale8(brightness);
  }
}
```

### 16.3 SUPERNOVA Expansion Logic

```cpp
void updateSupernova(uint32_t nowMs) {
  auto& state = phaseState.supernova;
  uint32_t elapsed = nowMs - state.phaseStartTime;
  
  switch (state.subState) {
    case SupernovaSubState::INITIAL_DELAY:
      if (elapsed >= 2000) {
        state.subState = SupernovaSubState::EXPANDING;
        state.currentStep = 0;
        state.lastStepTime = nowMs;
      }
      break;
      
    case SupernovaSubState::EXPANDING:
      uint32_t stepDelay = mapStepDelay(state.currentStep, state.totalSteps, 2000, 100);
      if (nowMs - state.lastStepTime >= stepDelay) {
        // Illuminate LEDs at current distance from epicenter
        illuminateStep(state.epicenterIndex, state.currentStep);
        
        state.currentStep++;
        state.lastStepTime = nowMs;
        
        if (state.currentStep >= state.totalSteps) {
          state.subState = SupernovaSubState::HOLDING;
          state.holdStartTime = nowMs;
        }
      }
      break;
      
    case SupernovaSubState::HOLDING:
      if (nowMs - state.holdStartTime >= 10000) {
        transitionTo(SystemState::BLACK_HOLE);
      }
      break;
  }
}

void illuminateStep(uint8_t epicenter, uint8_t step) {
  // Set LEDs at distance 'step' from epicenter to full brightness
  int16_t leftIndex = epicenter - step;
  int16_t rightIndex = epicenter + step;
  
  if (leftIndex >= 0 && leftIndex < LED_COUNT) {
    leds[leftIndex] = ledColors[leftIndex];
    leds[leftIndex].nscale8(255);
  }
  if (rightIndex >= 0 && rightIndex < LED_COUNT && rightIndex != leftIndex) {
    leds[rightIndex] = ledColors[rightIndex];
    leds[rightIndex].nscale8(255);
  }
}
```

---

## Appendix A: Hardware Setup

### Pin Connections

| QT Py ESP32 Pin | Connection | Notes |
|----------------|-----------|-------|
| A3 (GPIO 17) | LED Data In | NeoPixel DIN |
| 5V | LED Power (+) | Via USB, max 1A |
| GND | LED Power (-) | Common ground |

### Power Considerations

- **USB power**: 5V @ ~500 mA typical
- **LED current**: 50 LEDs × 60 mA max = 3A (theoretical)
- **Practical limit**: Brightness capped at 255, actual draw ~1A
- **Recommendation**: Use external 5V power supply if driving all LEDs to full white

---

## Appendix B: FastLED Quick Reference

```cpp
// Initialization
FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, LED_COUNT);

// Set color
leds[i] = CRGB(255, 0, 0);           // Red
leds[i] = CRGB::Blue;                 // Named color
leds[i].setRGB(r, g, b);              // Set RGB components

// Scale brightness
leds[i].nscale8(128);                 // Scale to 50% brightness
FastLED.setBrightness(128);           // Global brightness (affects all LEDs)

// Utility functions
fill_solid(leds, LED_COUNT, CRGB::Black);  // Clear all LEDs
fadeToBlackBy(leds, LED_COUNT, 64);        // Fade all LEDs by factor

// Update display
FastLED.show();                       // Push buffer to LEDs
```

---

## Revision History

| Version | Date | Author | Changes |
|---------|------|--------|---------|
| 1.0 | Original | User | Initial draft with INIT only |
| 2.0 | 2025-02-01 | Enhanced | Complete phase specs, cycle logic, testing |

