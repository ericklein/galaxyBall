For a fixed number of RGB LEDs, create a algorithm that cycles the LEDs through three distinct and repeating phases after a one time initialization phase.
The LEDs are emulating stars, so the colors that the LEDs can use should be limited to colors  that mimic what stars can emit.
Initialization phase - when the device is powered on, each LED is sequentially powered from zero brightness to maximum brightness to zero brightness with a random color from the before described "star" palette. This ramp from zero to maximum brightness to zero should take 5 seconds per LED.
Repeating Phase 1 - Warble
Warble is the moniker for when the LEDs are randomly assigned a color and a brightness range they are smoothly ramping up and down within.
The duration of this phase is random between 3 and 10 minutes. During this phase, the LEDs exhibit the following characteristics:
20% of the LEDs have a low base brightness, between 0 and 50 on a 255 (uint8_t) scale.
50% of the LEDs have a medium base brightness, between 50-125 on a 255 (uint8_t) scale.
25% of the LEDs have a high base brightness, between 125-200 on a 255 (uint8_t) scale.
5% of the LEDs have a maximum brightness, between 200-255 on a 255 (uint8_t) scale.
75% of the LEDs, randomly selected, vary their base brightness up or down a random but fixed 25 to 50, on the same 255 scale, over a two minute interval. They continue to do this until Phase 1 is completed.
 20% of the LEDs, randomly selected, do not vary their brightness throughout Phase 1.
5% of the LEDs, randomly selected, vary their base brightness up or down a random but fixed 50 to 100, on the same 255 scale, over a two minute interval. They continue to do this until Phase 1 is completed.
Repeating Phase 2 - Supernova
Supernova is the moniker for when each LED is taken to maximum brightness in a random order.
Start with any single LED and take its brightness to maximum, which is 255. Use a mathematical progression to take the two adjacent LEDs to the last LEDs and change their brightness to 255 two seconds later. Continue that process but lowering the time delay in the regular step until all of the LEDs are at maximum brightness. Once this occurs, all LEDs stay at maximum brightness for 10 seconds.
Repeating Phase 3 - Black Hole
Black Hole is a moniker for when each LED is taken from maximum to no brightness is a regular step down functin.
Step all LEDs from maximum brightness to minimum brightness in a series of regular time intervals but random brightness decreases.
The hardware for this project is a Adafruit QT-Py ESP32 board (adafruit part 5426) and a strand of 50 Adafruit neopixels (adafruit part 4560). The LEDs should be controlled using the latest version of the fastLED library and built using Arduino code. All functions should have Doxygen compatible documentation associated with them.