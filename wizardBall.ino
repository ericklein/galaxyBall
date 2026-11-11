/*
  Project:        galaxyBall
  Description:    LED animations simulating light from galaxies in a glass ball

  See README.md for target information
*/

#include "config.h"
#include <Adafruit_NeoPixel.h>    // https://github.com/adafruit/adafruit_neopixel

const RgbColor STELLAR_COLORS[] = {
    {155, 176, 255}, {170, 191, 255}, {202, 215, 255},
    {248, 247, 255}, {255, 244, 234}, {255, 210, 161},
    {255, 204, 111},
};

enum class Phase { Init, Warble, Supernova, BlackHole };

struct Pixel {
    RgbColor color;
    uint8_t base;
    int16_t change;
    uint16_t curve;
};

class WizardBall {
public:
    /** @brief Seed the animation RNG, configure NeoPixel, and enter INIT. */
    void begin() {
        const uint32_t seed = esp_random();
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
    Adafruit_NeoPixel strip{LED_COUNT, LED_DATA_PIN, NEO_GRB + NEO_KHZ800};
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

    /** @brief Assign fresh colors, base brightnesses, and independent modulation groups. */
    void configureWarble() {
        warbleDuration = random(WARBLE_MIN_MS, WARBLE_MAX_MS + 1);
        constexpr unsigned lowEnd = LED_COUNT * LOW_PERCENT / 100;
        constexpr unsigned mediumEnd = lowEnd + LED_COUNT * MEDIUM_PERCENT / 100;
        constexpr unsigned highEnd = mediumEnd + LED_COUNT * HIGH_PERCENT / 100;
        constexpr unsigned staticEnd = LED_COUNT * STATIC_PERCENT / 100;
        constexpr unsigned slowEnd = staticEnd + LED_COUNT * SLOW_PERCENT / 100;

        shuffleOrder();
        for (uint16_t rank = 0; rank < LED_COUNT; ++rank) {
            Pixel& pixel = pixels[order[rank]];
            pixel.color = pickColor();
            if (rank < lowEnd) {
                pixel.base = random(0, 51);
            } else if (rank < mediumEnd) {
                pixel.base = random(50, 126);
            } else if (rank < highEnd) {
                pixel.base = random(125, 201);
            } else {
                pixel.base = random(200, 256);
            }
        }

        shuffleOrder();
        for (uint16_t rank = 0; rank < LED_COUNT; ++rank) {
            Pixel& pixel = pixels[order[rank]];
            if (rank < staticEnd) {
                pixel.change = 0;
            } else {
                const int16_t amount = rank < slowEnd ? random(25, 51) : random(50, 101);
                pixel.change = random(2) == 0 ? -amount : amount;
            }
        }
    }

    /**
     * @brief Clamp brightness and write a scaled palette color into the frame.
     * @param index LED to update.
     * @param value Brightness before clamping to 0–255.
     */
    void paint(uint16_t index, int32_t value) {
        const uint32_t brightness = constrain(value, 0, 255);
        const RgbColor& color = pixels[index].color;
        strip.setPixelColor(index,
                            color.r * brightness / 255,
                            color.g * brightness / 255,
                            color.b * brightness / 255);
    }

    /**
     * @brief Render smooth ramps from base to target and back using integer smoothstep.
     * @param elapsed Milliseconds since WARBLE began.
     */
    void renderWarble(uint32_t elapsed) {
        const uint32_t cycle = elapsed % (2 * WARBLE_RAMP_MS);
        const uint32_t leg = cycle <= WARBLE_RAMP_MS ? cycle : 2 * WARBLE_RAMP_MS - cycle;
        const int32_t x = leg * RAMP_SCALE / WARBLE_RAMP_MS;
        const int32_t blend = x * x * (3 * RAMP_SCALE - 2 * x) / (RAMP_SCALE * RAMP_SCALE);
        for (uint16_t i = 0; i < LED_COUNT; ++i) {
            paint(i, pixels[i].base + pixels[i].change * blend / RAMP_SCALE);
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
                paint(origin, 255);
                break;
            }
            case Phase::BlackHole:
                shuffleOrder();
                for (uint16_t rank = 0; rank < LED_COUNT; ++rank) {
                    pixels[order[rank]].curve = rank;
                }
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
                const uint32_t half = INIT_LED_DURATION_MS / 2;
                const uint32_t ramp = offset <= half ? offset : INIT_LED_DURATION_MS - offset;
                strip.clear();
                paint(index, uint64_t(ramp) * 255 / half);
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
                paint(origin - radius, 255);
            }
            if (origin + radius < LED_COUNT) {
                paint(origin + radius, 255);
            }
            if (radius == finalRadius) {
                holdStarted = now;
            } else {
                nextWaveStep += FIRST_WAVE_DELAY_MS -
                    (FIRST_WAVE_DELAY_MS - LAST_WAVE_DELAY_MS) * radius / (finalRadius - 1);
            }
        }
    }

    /**
     * @brief Fade along distinct monotonic curves and send a dark frame before restarting.
     * @param elapsed Milliseconds since BLACK_HOLE began.
     * @param now Current millis() timestamp.
     */
    void updateBlackHole(uint32_t elapsed, uint32_t now) {
        if (lastCollapseStep == COLLAPSE_STEPS) {
            transition(Phase::Warble, now);
            return;
        }
        uint32_t step = elapsed / COLLAPSE_STEP_MS;
        if (step == lastCollapseStep) {
            return;
        }
        if (step > COLLAPSE_STEPS) {
            step = COLLAPSE_STEPS;
        }

        // Blend quadratic ease-in and ease-out with a different weight per pixel.
        // Round up so every pixel stays lit until the shared zero endpoint.
        constexpr uint32_t maxWeight = LED_COUNT - 1;
        constexpr uint32_t divisor = COLLAPSE_STEPS * COLLAPSE_STEPS * maxWeight;
        const uint32_t remaining = COLLAPSE_STEPS - step;
        const uint32_t fast = remaining * remaining;
        const uint32_t slow = remaining * (2 * COLLAPSE_STEPS - remaining);
        for (uint16_t i = 0; i < LED_COUNT; ++i) {
            const uint32_t weight = pixels[i].curve;
            const uint32_t level = fast * (maxWeight - weight) + slow * weight;
            paint(i, (255 * level + divisor - 1) / divisor);
        }
        lastCollapseStep = step;
    }
};

WizardBall ball;

/** @brief Initialize the LED controller once at power-on. */
void setup() {
    ball.begin();
}

/** @brief Service the LED controller without blocking delays. */
void loop() {
    ball.tick(millis());
}