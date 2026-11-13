# galaxyBall
art project that simulates stellar activity in a glass ball

## Arduino scripts

Set the LED data GPIO in config.h first. Use Arduino CLI with the Adafruit SAMD board
package and Adafruit NeoPixel installed in the library collection configured in
the scripts. The scripts target the Adafruit QT Py SAMD21 (`adafruit:samd:adafruit_qtpy_m0`).

The Arduino sketch is galaxyBall.ino.

On Windows, run these from the sketch folder:

    .\scripts\build.ps1
    .\scripts\upload.ps1
    .\scripts\analyze_memory_map.ps1

The Windows build script looks for the library collection in the sibling libraries folder.
Use build_discovery_upload.ps1 to build and upload in one step, or
build_discovery_upload_log.ps1 to also capture serial output. Enter the board's
COM port number when prompted. Serial
captures go into logs/ at 115200 baud. The firmware itself emits no serial logs.

On Linux, use bash scripts/build.sh or
bash scripts/build_discovery.sh.

On Windows the default output is `C:\arduinobuild\galaxyBall`. The standalone
upload and memory-map scripts look there by default. On Linux, `build.sh` uses
`./build`, and `build_discovery.sh` defaults to `~/arduinobuild/galaxyBall` and
accepts `--build-root` and `--libraries`.
