# WizardBall LED Controller PRD

## Purpose

WizardBall is a glass ball containing 50 RGB NeoPixels on a flexible wire. The LEDs are scattered through the volume when the wire is inserted. The firmware uses wire indices for animation.

## Hardware and configuration

### BOM
- Adafruit QT Py SAMD21, part 4600 (https://www.adafruit.com/product/4600).
- Adafruit Soft Flexible Wire NeoPixel Strand - 50 NeoPixels (https://www.adafruit.com/product/4560)
- 5V, 4A power supply

- Create an Arduino-compatible sketch targeting the Adafruit QT Py SAMD21.
- Use Adafruit NeoPixel library to control the LEDS. No specific version of this library is required.
- Configure the installed strand for BGR wire order at 800 kHz. The primary-color test confirmed this order.
- Put the LED data GPIO in `config.h` as a clearly marked placeholder for the user to fill in before compiling.
- Set the duration of each INIT fade with `INIT_FADE_MS` in `config.h`. It defaults to 500 ms. Each pixel takes two fades, for a default of 1000 ms.
- Put the WARBLE category and modulation percentages in `config.h`. Rebuild and upload after changing them for visual testing.
- Clamp computed star brightness to 25–250 during INIT and WARBLE. Inactive INIT pixels are off. BLACK_HOLE can fade to zero. Scale palette channels by brightness divided by 255. The firmware applies no current limit.

## Timing and state machine

Run INIT once at power-on, then repeat WARBLE → SUPERNOVA → BLACK_HOLE → WARBLE. Use `setup()` and `loop()`, with state entry, update, and exit hooks. Seed the random generator once at startup using hardware entropy. Make random choices on phase entry and keep them fixed until the next entry.

Use elapsed time from `millis()` with a 20 ms frame cadence. Keep the animation non-blocking and handle `millis()` rollover. Frame computation must take less than 1 ms, excluding transmission to the LEDs. Avoid floating point math in frame updates.

## Stellar palette

Select colors from this table. Apply brightness by scaling the selected RGB triplet.

| Class | Color | RGB |
| --- | --- | --- |
| O | Blue | (155, 176, 255) |
| B | Blue-white | (170, 191, 255) |
| A | White | (202, 215, 255) |
| F | Yellow-white | (248, 247, 255) |
| G | Yellow | (255, 224, 64) |
| K | Orange | (255, 128, 24) |
| M | Red | (255, 48, 12) |

## INIT

At power-on, assign a random stellar color to each pixel. Animate one pixel at a time in physical wiring order, starting at data-in and proceeding to the far end. Keep all other pixels dark so the installer can see where each pixel sits inside the ball. By default, each pixel fades linearly from min to max over 500 ms, then fades linearly back to min over 500 ms. The default 50-pixel strip takes 50 seconds. Enter WARBLE after the last pixel's fade-out.

## WARBLE

The first WARBLE uses the colors assigned in INIT. On each entry from BLACK_HOLE, independently select a random stellar color for each pixel. Keep those colors through SUPERNOVA and BLACK_HOLE. On every entry, choose a random duration from 2 to 3 minutes. Randomly assign each pixel to a brightness category and draw its base brightness from that category’s inclusive range. Then randomly assign modulation groups independently of brightness categories.

| Category | Default share | Default count | Base brightness |
| --- | ---: | ---: | --- |
| Low | 20% | 10 | 25–85 |
| Medium | 50% | 25 | 86–140 |
| High | 25% | 12 | 141–200 |
| Max | Remainder | 3 | 201–250 |

| Modulation group | Default share | Default count | Change from base |
| --- | ---: | ---: | --- |
| Static | 20% | 10 | 0 points |
| Subtle | 60% | 30 | 25–35 points |
| Strong | Remainder | 10 | 30–50 points |

Calculate each explicit count by rounding down its percentage of the LED count. Max and Strong get the remaining pixels in their respective sets.

Each non-static pixel starts at a random point in its first excursion. At the start of each excursion, independently choose whether the pixel brightens or dims and draw a fresh duration of 480–1600 ms, including the return to base. Clamp brightness to 25–250. SUPERNOVA takes control of the final WARBLE colors and brightnesses.

## SUPERNOVA

Choose a random starting index and immediately set that LED to white (255, 255, 255). After 1333 ms, set the next LED on each side to full white. Continue at increasing distances along the wire, stopping at each end and continuing on the other side as needed. Shorten the interval from 1333 ms on the first outward step to 67 ms on the last. Scale the interval above 67 ms by the square of the remaining fraction so the wave speeds up sooner.

Each LED keeps its final WARBLE color and brightness until the wave reaches it. Hold all LEDs at full white for 2 seconds after the wave finishes, then enter BLACK_HOLE.

## BLACK_HOLE

Keep all LEDs white and dim them together from brightness 255 to 0 at a constant rate over 15 seconds. Update brightness every 100 ms. Keep the strip dark for 5 seconds before starting WARBLE.

## Deliverables and acceptance

- Deliver a standalone Arduino sketch and `config.h` in the Dropbox WizardBall folder.
- Include Doxygen-compatible function comments documenting parameters, return values, and side effects where applicable. Comment the state transitions.
- With the GPIO configured, the sketch should compile for the target board. Confirm the phase order, timing, and behavior during visual testing.
- The firmware requires no user input, persistent storage, or external communication.
