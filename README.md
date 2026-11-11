# wizardBall
art project that simulates stellar activity in a glass ball

## Arduino scripts

Set the LED data GPIO in config.h first. Use Arduino CLI with the ESP32 board
package and Adafruit NeoPixel installed in the library collection configured in
the scripts. The scripts target the Adafruit QT Py ESP32-S3 without PSRAM.

`wizardBall.ino` and `wizardBall_c_style.ino` are alternative sketches. Keep
only the variant you want to build in the sketch folder, with its `.ino` file
named to match the folder and a copy of `config.h`. Arduino combines all `.ino`
files in the folder into one program.

On Windows, run these from the sketch folder:

    .\scripts\build.ps1 -Version 0.1.0
    .\scripts\upload.ps1
    .\scripts\analyze_memory_map.ps1

Use build_discovery.ps1 if your Arduino CLI needs library discovery.
Use build_discovery_upload.ps1 to build and upload in one step, or
build_discovery_upload_log.ps1 to also capture serial output. Enter the board's
COM port number when prompted. Serial
captures go into logs/ at 115200 baud. The firmware itself emits no serial logs.

On Linux, use bash scripts/build.sh or
bash scripts/build_discovery.sh --version 0.1.0. The discovery script requires
Python 3 for ESP-IDF application descriptor stamping and verification.

On Windows the default output is `C:\arduinobuild\wizardBall`. The standalone
upload and memory-map scripts look there by default. On Linux, `build.sh` uses
`./build`, and `build_discovery.sh` defaults to `~/arduinobuild/wizardBall` and
accepts `--build-root` and `--libraries`.
