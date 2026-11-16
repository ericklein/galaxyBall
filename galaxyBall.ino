/*
  Project:        galaxyBall
  Description:    LED animations simulating light from galaxies in a glass ball

  See README.md for target information
*/

#include "config.h"
#include <Adafruit_NeoPixel.h>    // https://github.com/adafruit/adafruit_neopixel

const RgbColor STELLAR_COLORS[] = {
    {155, 176, 255}, {170, 191, 255}, {202, 215, 255},
    {248, 247, 255}, {255, 224, 64}, {255, 128, 24},
    {255, 48, 12},
};

constexpr uint32_t SUPERNOVA_WHITE = 0xFFFFFF;

enum class Phase { Init, Warble, Supernova, BlackHole };

struct Pixel {
    RgbColor color;
    uint8_t base;
    int16_t flickerAmplitude;
    uint16_t flickerDuration;
    uint32_t flickerStarted;
    bool flickerBrightens;
};

class GalaxyBall {
public:
    /** @brief Seed the animation RNG, configure NeoPixel, and enter INIT. */
    void begin() {
        // Leave the selected analog input unconnected for startup noise.
        constexpr int seedPin = A0;
        uint32_t seed = micros();
        for (uint8_t sample = 0; sample < 32; ++sample) {
            seed = (seed << 1) ^ static_cast<uint32_t>(analogRead(seedPin));
        }
        randomSeed(seed == 0 ? 1 : seed);
        strip.begin();
        const uint32_t now = millis();
        phaseStarted = now;
        lastFrame = now - FRAME_MS;
        onEnter();
    }

    /**
     * @brief Compute and transmit at most one frame per call.
     * @param now Current millis() timestamp, including rollover.
     */
    void tick(uint32_t now) {
        const uint32_t elapsed = now - lastFrame;
        if (elapsed < FRAME_MS) {
            return;
        }
        lastFrame += elapsed - elapsed % FRAME_MS;
        onUpdate(now);
        strip.show();
    }

private:
    Phase phase = Phase::Init;
    Pixel pixels[LED_COUNT] = {};
    // The installed strand uses BGR wire order, confirmed by a primary-color test.
    Adafruit_NeoPixel strip{LED_COUNT, LED_DATA_PIN, NEO_BGR + NEO_KHZ800};
    uint16_t order[LED_COUNT];
    uint32_t phaseStarted = 0;
    uint32_t lastFrame = 0;
    uint32_t warbleDuration = 0;
    uint16_t origin = 0;
    uint16_t radius = 0;
    uint16_t finalRadius = 0;
    uint32_t nextWaveStep = 0;
    uint32_t holdStarted = 0;
    uint32_t lastCollapseStep = 0;
    uint32_t blackoutStarted = 0;

    /** @return A uniformly selected color from the fixed stellar palette. */
    RgbColor pickColor() {
        return STELLAR_COLORS[random(sizeof(STELLAR_COLORS) / sizeof(STELLAR_COLORS[0]))];
    }

    /** @brief Replace the assignment order with a random permutation of LED indices. */
    void shuffleOrder() {
        for (uint16_t i = 0; i < LED_COUNT; ++i) {
            order[i] = i;
        }
        for (uint16_t remaining = LED_COUNT; remaining > 1; --remaining) {
            const uint16_t other = random(remaining);
            const uint16_t last = order[remaining - 1];
            order[remaining - 1] = order[other];
            order[other] = last;
        }
    }

    /** @brief Assign base brightnesses and independent modulation groups. */
    void configureWarble() {
        warbleDuration = random(WARBLE_MIN_MS, WARBLE_MAX_MS + 1);
        constexpr unsigned lowEnd = LED_COUNT * LOW_INITIAL_BRIGHTNESS_PERCENT / 100;
        constexpr unsigned mediumEnd = lowEnd + LED_COUNT * MEDIUM_INITIAL_BRIGHTNESS_PERCENT / 100;
        constexpr unsigned highEnd = mediumEnd + LED_COUNT * HIGH_INITIAL_BRIGHTNESS_PERCENT / 100;
        constexpr unsigned staticEnd = LED_COUNT * STATIC_FLICKER_PERCENT / 100;
        constexpr unsigned subtleEnd = staticEnd + LED_COUNT * SUBTLE_FLICKER_PERCENT / 100;

        shuffleOrder();
        for (uint16_t rank = 0; rank < LED_COUNT; ++rank) {
            Pixel& pixel = pixels[order[rank]];
            if (rank < lowEnd) {
                pixel.base = random(MIN_STAR_BRIGHTNESS, 86);
            } else if (rank < mediumEnd) {
                pixel.base = random(86, 141);
            } else if (rank < highEnd) {
                pixel.base = random(141, 201);
            } else {
                pixel.base = random(201, MAX_STAR_BRIGHTNESS + 1);
            }
        }

        shuffleOrder();
        for (uint16_t rank = 0; rank < LED_COUNT; ++rank) {
            Pixel& pixel = pixels[order[rank]];
            if (rank < staticEnd) {
                pixel.flickerAmplitude = 0;
            } else {
                pixel.flickerAmplitude = rank < subtleEnd ? random(25, 36) : random(30, 51);
                pixel.flickerDuration = random(WARBLE_FLICKER_MIN_MS, WARBLE_FLICKER_MAX_MS + 1);
                pixel.flickerStarted = 0U - static_cast<uint32_t>(random(pixel.flickerDuration));
                pixel.flickerBrightens = random(2) == 0;
            }
        }
    }

    /**
     * @brief Clamp brightness and write a scaled palette color into the frame.
     * @param index LED to update.
     * @param value Brightness before clamping.
     */
    void paint(uint16_t index, int32_t value) {
        const uint32_t brightness = constrain(value, MIN_STAR_BRIGHTNESS, MAX_STAR_BRIGHTNESS);
        const RgbColor& color = pixels[index].color;
        strip.setPixelColor(index,
                            color.r * brightness / 255,
                            color.g * brightness / 255,
                            color.b * brightness / 255);
    }

    /**
     * @brief Render staggered flickers around each pixel's base brightness.
     * @param elapsed Milliseconds since WARBLE began.
     */
    void renderWarble(uint32_t elapsed) {
        for (uint16_t i = 0; i < LED_COUNT; ++i) {
            Pixel& pixel = pixels[i];
            int32_t brightness = pixel.base;
            if (pixel.flickerAmplitude != 0) {
                if (elapsed - pixel.flickerStarted >= pixel.flickerDuration) {
                    pixel.flickerStarted = elapsed;
                    pixel.flickerDuration = random(WARBLE_FLICKER_MIN_MS, WARBLE_FLICKER_MAX_MS + 1);
                    pixel.flickerBrightens = random(2) == 0;
                }
                const int32_t duration = pixel.flickerDuration;
                const int32_t offset = elapsed - pixel.flickerStarted;
                const int32_t ramp = min(offset, duration - offset);
                const int32_t change = 2 * ramp * pixel.flickerAmplitude / duration;
                brightness += pixel.flickerBrightens ? change : -change;
            }
            paint(i, brightness);
        }
    }

    /** @brief Initialize the current phase, including all its random choices. */
    void onEnter() {
        switch (phase) {
            case Phase::Init:
                for (Pixel& pixel : pixels) {
                    pixel.color = pickColor();
                }
                break;
            case Phase::Warble:
                configureWarble();
                renderWarble(0);
                break;
            case Phase::Supernova: {
                origin = random(LED_COUNT);
                radius = 0;
                const uint16_t right = LED_COUNT - 1 - origin;
                finalRadius = origin > right ? origin : right;
                nextWaveStep = FIRST_WAVE_DELAY_MS;
                holdStarted = 0;
                strip.setPixelColor(origin, SUPERNOVA_WHITE);
                break;
            }
            case Phase::BlackHole:
                lastCollapseStep = 0;
                break;
        }
    }

    /** @brief Preserve the final WARBLE frame for SUPERNOVA. Other phases need no cleanup. */
    void onExit() {
        if (phase == Phase::Warble) {
            renderWarble(warbleDuration);
        }
    }

    /**
     * @brief Run the exit and entry hooks for a phase transition.
     * @param next Phase to enter.
     * @param now Current millis() timestamp.
     */
    void transition(Phase next, uint32_t now) {
        onExit();
        if (phase == Phase::BlackHole && next == Phase::Warble) {
            for (Pixel& pixel : pixels) {
                pixel.color = pickColor();
            }
        }
        phase = next;
        phaseStarted = now;
        onEnter();
    }

    /**
     * @brief Update the active phase and advance INIT → WARBLE → SUPERNOVA → BLACK_HOLE → WARBLE.
     * @param now Current millis() timestamp.
     */
    void onUpdate(uint32_t now) {
        const uint32_t elapsed = now - phaseStarted;
        switch (phase) {
            case Phase::Init: {
                const uint32_t index = elapsed / INIT_LED_DURATION_MS;
                if (index >= LED_COUNT) {
                    transition(Phase::Warble, now);
                    break;
                }
                const uint32_t offset = elapsed % INIT_LED_DURATION_MS;
                const uint32_t ramp = min(offset, INIT_LED_DURATION_MS - offset);
                strip.clear();
                paint(index, MIN_STAR_BRIGHTNESS + ramp * (MAX_STAR_BRIGHTNESS - MIN_STAR_BRIGHTNESS) / INIT_FADE_MS);
                break;
            }
            case Phase::Warble:
                if (elapsed >= warbleDuration) {
                    transition(Phase::Supernova, now);
                } else {
                    renderWarble(elapsed);
                }
                break;
            case Phase::Supernova:
                updateSupernova(elapsed, now);
                break;
            case Phase::BlackHole:
                updateBlackHole(elapsed, now);
                break;
        }
    }

    /**
     * @brief Brighten successive pairs in wire order, then hold the completed wave.
     * @param elapsed Milliseconds since SUPERNOVA began.
     * @param now Current millis() timestamp.
     */
    void updateSupernova(uint32_t elapsed, uint32_t now) {
        if (radius == finalRadius) {
            if (now - holdStarted >= SUPERNOVA_HOLD_MS) {
                transition(Phase::BlackHole, now);
            }
            return;
        }

        // Catch up after a late frame without restarting the wave timer.
        while (radius < finalRadius && elapsed >= nextWaveStep) {
            ++radius;
            if (radius <= origin) {
                strip.setPixelColor(origin - radius, SUPERNOVA_WHITE);
            }
            if (origin + radius < LED_COUNT) {
                strip.setPixelColor(origin + radius, SUPERNOVA_WHITE);
            }
            if (radius == finalRadius) {
                holdStarted = now;
            } else {
                const uint32_t intervals = finalRadius - 1;
                const uint32_t remaining = intervals - radius;
                // Square the remaining fraction to bring the shorter delays in sooner.
                nextWaveStep += LAST_WAVE_DELAY_MS +
                    (FIRST_WAVE_DELAY_MS - LAST_WAVE_DELAY_MS) * remaining * remaining /
                    (intervals * intervals);
            }
        }
    }

    /**
     * @brief Fade all pixels together from full white to black, then hold the blackout.
     * @param elapsed Milliseconds since BLACK_HOLE began.
     * @param now Current millis() timestamp.
     */
    void updateBlackHole(uint32_t elapsed, uint32_t now) {
        if (lastCollapseStep == COLLAPSE_STEPS) {
            if (now - blackoutStarted >= BLACKOUT_MS) {
                transition(Phase::Warble, now);
            }
            return;
        }
        uint32_t step = elapsed / COLLAPSE_STEP_MS;
        if (step == lastCollapseStep) {
            return;
        }
        if (step > COLLAPSE_STEPS) {
            step = COLLAPSE_STEPS;
        }

        const uint32_t remaining = COLLAPSE_STEPS - step;
        const uint8_t brightness = 255 * remaining / COLLAPSE_STEPS;
        strip.fill(strip.Color(brightness, brightness, brightness));
        lastCollapseStep = step;
        if (step == COLLAPSE_STEPS) {
            blackoutStarted = now;
        }
    }
};

GalaxyBall ball;

/** @brief Initialize the LED controller once at power-on. */
void setup() {
    ball.begin();
}

/** @brief Service the LED controller without blocking delays. */
void loop() {
    ball.tick(millis());
}
