# WizardBall LED Controller – Code Generation Optimized PRD

## 1. Purpose

This document defines precise, unambiguous requirements for firmware controlling a fixed number of RGB LEDs to simulate stellar phenomena. It is explicitly written to maximize correctness and consistency when used with automated code generation tools (e.g., Claude, Codex).

---

## 2. Target Platform and Constraints

### 2.1 Hardware

- MCU: Adafruit QT Py ESP32 (Adafruit #5426)
- LEDs: 50× Adafruit NeoPixel RGB LEDs (Adafruit #4560)

### 2.2 Software

- Language: Arduino C++
- LED Library: FastLED (latest stable release)
- Documentation: All functions must include Doxygen-compatible comments

### 2.3 Execution Model

- Firmware shall use `setup()` and `loop()`
- Timing shall be **non-blocking** and based on `millis()`
- `delay()` shall not be used except during initialization if explicitly required
- LED update cadence: 20 ms (50 Hz)

---

## 3. Global Definitions

### 3.1 LED Count

```cpp
constexpr uint16_t LED_COUNT = 50;
```

### 3.2 Brightness Range

- Valid brightness range: `[0, 255]` (`uint8_t`)
- All computed brightness values **shall be clamped** to this range

### 3.3 Randomness

- RNG shall be seeded once during `setup()` using a hardware entropy source
- All random selections described below are made **once per phase entry** unless otherwise stated
- Random values remain fixed for the duration of the phase

---

## 4. Star Color Palette

Only colors visibly emitted by stars within the human visual spectrum are permitted.

### 4.1 Allowed Stellar Colors (Human-Visible)

The palette shall include **all human-visible stellar colors**, spanning the full range of stellar spectral classes as perceived by the human eye.

| Spectral Class | Perceived Color | RGB |
|---------------|-----------------|-----|
| O | Blue | (155, 176, 255) |
| B | Blue-White | (170, 191, 255) |
| A | White | (202, 215, 255) |
| F | Yellow-White | (248, 247, 255) |
| G | Yellow | (255, 244, 234) |
| K | Orange | (255, 210, 161) |
| M | Red | (255, 204, 111) |

Notes:
- RGB values approximate black-body radiation mapped to sRGB.
- No green or purple hues are permitted.
- Colors outside this table are invalid.
- No dynamic or procedural color generation is allowed.

---

## 5. System Architecture

### 5.1 State Machine

The system shall be implemented as a finite state machine with the following states:

1. INIT  
2. WARBLE  
3. SUPERNOVA  
4. BLACK_HOLE  

Each state shall define:
- `onEnter()`
- `onUpdate(uint32_t nowMs)`
- `onExit()`

State transitions are strictly sequential.

---

## 6. Phase Specifications

### 6.1 INIT Phase

#### Purpose
Visual power-on animation.

#### Behavior
- LEDs are processed sequentially from index `0` to `LED_COUNT − 1`
- For each LED:
  - Assign a random stellar color
  - Brightness ramps `0 → 255 → 0`
  - Total ramp duration: **5 seconds per LED**
- All other LEDs remain off during each LED’s animation

#### Exit Condition
- Final LED completes ramp-down
- Transition immediately to WARBLE

---

### 6.2 WARBLE Phase

#### Purpose
Long-duration stochastic stellar shimmer.

#### Duration
- Random duration selected at phase entry: **3 to 10 minutes**

#### LED Category Assignment
At phase entry, LEDs are randomly assigned:

| Category | Percentage | Base Brightness Range |
|--------|-----------|----------------------|
| Low | 20% | `[0, 50]` |
| Medium | 50% | `[50, 125]` |
| High | 25% | `[125, 200]` |
| Max | remainder | `[200, 255]` |

Rounding shall ensure total equals `LED_COUNT`.

#### Brightness Modulation Groups

| Group | Percentage | Modulation |
|------|-----------|-----------|
| Static | 20% | No change |
| Slow | 75% | ±25–50 over 120 s |
| Strong | 5% | ±50–100 over 120 s |

- Modulation is smooth and periodic
- Modulation parameters remain fixed for the phase
- Brightness is clamped

#### Exit Condition
- Phase duration expires
- Transition to SUPERNOVA

---

### 6.3 SUPERNOVA Phase

#### Purpose
Rapid full-brightness expansion.

#### Behavior
- Select random starting LED index
- Illuminate outward symmetrically
- Initial delay: **2000 ms**
- Delay decreases linearly per step
- Minimum delay: **100 ms**
- All LEDs reach brightness `255`

#### Hold
- Hold at full brightness for **10 seconds**

#### Exit Condition
- Hold completes
- Transition to BLACK_HOLE

---

### 6.4 BLACK_HOLE Phase

#### Purpose
Simultaneous collapse to darkness.

#### Behavior
- All LEDs start at brightness `255`
- Fixed duration: **30 seconds**
- Update interval: **100 ms**
- Each LED assigned a unique decrement-per-step
- Decrements computed so all LEDs reach `0` simultaneously
- Brightness is clamped

#### Exit Condition
- All LEDs reach `0`
- Transition to WARBLE

---

## 7. Data Structures

Per-LED state may include:
- Current color (`CRGB`)
- Base brightness
- Modulation amplitude
- Modulation phase

Dynamic allocation is permitted but discouraged.

---

## 8. Performance Constraints

- Heap usage < **50 KB**
- Avoid floating point math in per-frame loops
- LED update time < **1 ms per frame**

---

## 9. Documentation Requirements

- All functions must include Doxygen-compatible comments
- Parameters, return values, and side effects documented
- State transitions clearly commented

---

## 10. Non-Goals

- No external communication
- No persistent storage
- No user input

---

## 11. Acceptance Criteria

- Visual behavior matches phase definitions
- No blocking delays outside INIT
- Deterministic behavior within phases
- Compiles cleanly under Arduino IDE for ESP32
