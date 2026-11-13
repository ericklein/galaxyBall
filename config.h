/*
  Project:      galaxyBall - LED animations simulating light from galaxies in a glass ball
  Description:  public (non-secret) configuration data
*/

#pragma once

// QT Py SAMD21 A1/D1 connects to the external NeoPixel data input.
constexpr int LED_DATA_PIN = A1;
constexpr uint16_t LED_COUNT = 50;

struct RgbColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

// Total time for one LED to ramp up and back down.
constexpr uint32_t INIT_LED_DURATION_MS = 3000; // duration must be a positive even number of milliseconds

// Animation timing and fixed-point ramp scale.
constexpr uint32_t FRAME_MS = 20;
constexpr uint32_t WARBLE_MIN_MS = 180000;
constexpr uint32_t WARBLE_MAX_MS = 600000;
constexpr uint32_t WARBLE_RAMP_MS = 120000;
constexpr uint32_t FIRST_WAVE_DELAY_MS = 2000;
constexpr uint32_t LAST_WAVE_DELAY_MS = 100;
constexpr uint32_t SUPERNOVA_HOLD_MS = 10000;
constexpr uint32_t COLLAPSE_MS = 30000;
constexpr uint32_t COLLAPSE_STEP_MS = 100;
constexpr uint32_t COLLAPSE_STEPS = COLLAPSE_MS / COLLAPSE_STEP_MS;
constexpr int32_t RAMP_SCALE = 1024;

// Low, Medium, and High round down. Max gets the remaining LEDs.
// Brightness percentages must total at most 100
constexpr unsigned LOW_PERCENT = 20;
constexpr unsigned MEDIUM_PERCENT = 50;
constexpr unsigned HIGH_PERCENT = 25;

// Static and Slow round down. Strong gets its share plus any rounding remainder.
// Static, slow, and strong modulation percentages must total 100
constexpr unsigned STATIC_PERCENT = 20;
constexpr unsigned SLOW_PERCENT = 75;
constexpr unsigned STRONG_PERCENT = 5;