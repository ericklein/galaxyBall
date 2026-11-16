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

// INIT uses the same duration for the fade in and fade out.
constexpr uint32_t INIT_FADE_MS = 500;
constexpr uint32_t INIT_LED_DURATION_MS = 2 * INIT_FADE_MS;

// Animation timing and fixed-point ramp scale.
constexpr uint32_t FRAME_MS = 20;
constexpr uint32_t WARBLE_MIN_MS = 120000;
constexpr uint32_t WARBLE_MAX_MS = 180000;
constexpr uint16_t WARBLE_FLICKER_MIN_MS = 480;
constexpr uint16_t WARBLE_FLICKER_MAX_MS = 1600;
constexpr uint32_t FIRST_WAVE_DELAY_MS = 1333;
constexpr uint32_t LAST_WAVE_DELAY_MS = 67;
constexpr uint32_t SUPERNOVA_HOLD_MS = 2000;
constexpr uint32_t COLLAPSE_MS = 15000;
constexpr uint32_t COLLAPSE_STEP_MS = 100;
constexpr uint32_t COLLAPSE_STEPS = COLLAPSE_MS / COLLAPSE_STEP_MS;
constexpr uint32_t BLACKOUT_MS = 5000;

constexpr int32_t MIN_STAR_BRIGHTNESS = 25;
constexpr int32_t MAX_STAR_BRIGHTNESS = 250;

// Low, Medium, and High round down. Max gets the remaining LEDs.
// Brightness percentages must total at most 100
constexpr unsigned LOW_INITIAL_BRIGHTNESS_PERCENT = 20; // 25-85 brightness
constexpr unsigned MEDIUM_INITIAL_BRIGHTNESS_PERCENT = 50; // 86-140 brightness
constexpr unsigned HIGH_INITIAL_BRIGHTNESS_PERCENT = 25; // 141-200 brightness
// MAX_INITIAL_BRIGHTNESS_PERCENT is the 100 - these three percentages, 201-250 brightness

// Static and Subtle round down. Strong gets its share plus any rounding remainder.
// Static, subtle, and strong modulation percentages must total 100
constexpr unsigned STATIC_FLICKER_PERCENT = 20; // initial brightness does not change
constexpr unsigned SUBTLE_FLICKER_PERCENT = 60; // Varies by 25-35
// STRONG_FLICKER_PERCENT is 100 - these two percentages, varies 30-50
