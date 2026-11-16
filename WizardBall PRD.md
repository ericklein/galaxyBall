# WizardBall LED Controller PRD

## Purpose

WizardBall is a glass ball containing 50 RGB NeoPixels on a flexible wire. The LEDs are scattered through the volume when the wire is inserted. The firmware uses wire indices for animation.

## Hardware and configuration

### Enclosure BOM

| Qty | Part | Source | Notes |
| --- | --- | --- | --- |
| 1 | Adafruit QT Py SAMD21, part 4600 | On hand | Mount on the Perma-Proto with its USB-C socket facing the cable approach. [Adafruit](https://www.adafruit.com/product/4600) |
| 1 | Adafruit Soft Flexible Wire NeoPixel Strand, 50 RGB pixels, part 4560 | On hand | Sits outside the enclosure. [Adafruit](https://www.adafruit.com/product/4560) |
| 1 | 5 V DC, 4 A adapter with 2.1 mm barrel plug | On hand | Confirm polarity and plug fit before connecting. |
| 1 | Adafruit 1609 half-size Perma-Proto PCB, 81 × 51 mm | On hand | Holds the QT Py and wiring. [Adafruit](https://www.adafruit.com/product/1609) |
| 1 | BY500-600 silicon diode | On hand | Series diode on the QT Py power branch. Gray band toward QT Py 5V. |
| 1 | 470 Ω resistor | On hand | Measured 469 Ω. Install in series between A1 and the first pixel DIN, near DIN where practical. |
| 1 | Polarized bulk capacitor | On hand | Verify 500–1,000 µF and at least 6.3 V. Install across switched LED +5 V and GND, negative stripe toward GND. |
| 1 | 3-pin JST-SM mating pigtail | On hand | LED power, data, and ground. The 4 mm wire bundle exits through the grommet with the connector outside. |
| 1 | Hammond 1591XXGBK black ABS enclosure | [DigiKey HM2041-ND](https://www.digikey.com/en/products/detail/hammond-manufacturing/1591XXGBK/2357885) | 122.45 × 95.91 × 38 mm externally. |
| 1 | Switchcraft 721A panel-mount barrel jack | [DigiKey SC3508-ND](https://www.digikey.com/en/products/detail/switchcraft-inc/721A/8571716) | 5.5 × 2.1 mm, 5 A, solder lugs. Nominal 8 mm mounting hole. |
| 1 | CW Industries GRB112A802BB round rocker | [DigiKey CW102-ND](https://www.digikey.com/en/products/detail/cw-industries/GRB112A802BB/2349682) | Preferred lid switch. Unlit SPST on/off, 10 A at 28 V DC, 4.7 mm tabs, 20.2 mm round cutout. Check clip fit before drilling. |
| 1 | NKK S1A toggle | [DigiKey 360-2893-ND](https://www.digikey.com/en/products/detail/nkk-switches/S1A/1049381) | Alternate lid switch included in the cart. Solder lugs and 12.5 mm clearance hole. Install one switch. |
| 1 | Adafruit 4218 panel-mount USB-C data extension | [DigiKey 1528-4218-ND](https://www.digikey.com/en/products/detail/adafruit-industries-llc/4218/10244657) | 30 cm. Outside receptacle for programming, inside plug to QT Py. Minimum 21.5 mm hole and approximately 29.5 mm nut clearance. |
| 1 | Essentra 498074 rubber grommet | [DigiKey 2082-498074-ND](https://www.digikey.com/en/products/detail/essentra-access-solutions/498074/26982837) | 8 mm panel hole, 6 mm wire opening, nominal 2 mm panel thickness. |
| 1 | Panduit TM1S4-M screw-down cable tie mount | [DigiKey 298-15887-ND](https://www.digikey.com/en/products/detail/panduit-corp/TM1S4-M/4569098) | Anchor the LED wire bundle just inside the cable exit. |
| 1 | 2.5 mm-wide miniature cable tie | Bench stock or source separately | Use with the Panduit mount. The PLT1M-M0 listing required multiples of 1,000 and was omitted from the cart. |
| As needed | Two 1×7 headers, PCB standoffs and fasteners, #4 or M2.5 tie-mount screw, insulated hookup wire, and heat-shrink | Verify bench stock | Size power wiring for the LED load. |

The DigiKey entries were verified in the cart. Checkout status is unconfirmed.

### Enclosure wiring and layout

- Put the switch on the lid and use three separate walls for **5 V IN**, **USB-C**, and **LED OUT**. Dry-fit the USB-C nut, cable bend, switch terminals, protoboard on standoffs, and capacitor before drilling.
- Route barrel jack +5 V through the lid switch, then split it at the protoboard. Feed the LED strand directly from switched +5 V and feed QT Py 5V through the BY500-600. The QT Py 5V header pin and underside 5V pad share the same net.
- Connect adapter, QT Py, and LED grounds at the common ground tie point. Route A1 through the 470 Ω resistor to the first pixel DIN. Verify the pigtail pinout against the installed strand before soldering.
- Label the switch **5 V IN**. USB-C can still power the QT Py with the switch off.

### Firmware configuration


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
