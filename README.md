# Chase's Paw Patrol police car lights

Standalone firmware for 16 individually addressable **5 V WS2811 RGB pixels** on
a homemade Chase police car Halloween costume. It uses an ESP32, PlatformIO,
Arduino, C++17, and FastLED. Power on to start alternating red/blue lights;
after four seconds the effect changes to police strobes. The two effects repeat
indefinitely. No Wi-Fi, Bluetooth, network services, stored settings, or secrets
are used or required.

## Hardware and defaults

| Item | Default / requirement |
| --- | --- |
| Controller | Classic ESP32 development board (`esp32dev`) |
| Pixels | 16 × individually addressable **5 V** WS2811 RGB pixels |
| Data pin | **GPIO 18** (GPIO number, not connector position) |
| Chipset / byte order | FastLED `WS2811` / `RGB` |
| Global brightness | **50 / 255**, approximately 20% |
| ESP32 power | USB connector from a regulated 5 V USB power bank |
| Pixel power | Separate power branch from the **same** 5 V supply |
| Data level conversion | External 3.3 V → 5 V logic level converter |
| Ground | Shared by ESP32, pixels, supply, and converter |

Also use properly rated power wiring/connectors, insulation, strain relief, and
a suitable fuse on the LED power branch. A data resistor and supply capacitor
are recommended below. Confirm the pixels' actual voltage and pinout from their
datasheet: WS2811 products also exist with other supply voltages and with multiple
physical LEDs per address. This project assumes one addressable RGB pixel per
index and a 5 V string.

## Build and flash

1. Install [VS Code](https://code.visualstudio.com/) and the
   [PlatformIO IDE extension](https://docs.platformio.org/en/latest/integration/ide/vscode/installation.html),
   or install [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html).
2. Open this repository's root folder (the one containing `platformio.ini`).
3. Check the board and LED settings below. Connect the ESP32 with a USB **data**
   cable. For the first upload, keep the LED power branch disconnected.
4. In PlatformIO, choose **Build**, then **Upload** for `esp32dev`, or run:

   ```sh
   pio run -e esp32dev
   pio run -e esp32dev -t upload
   ```

5. Disconnect power, complete and check the LED wiring, then reconnect the power
   bank. Normal lighting starts immediately after firmware initialization.

Dependencies download on the build computer during the first build. The costume
itself always runs offline. `platformio.ini` pins Espressif32 **6.12.0** (Arduino
ESP32 **2.0.17**) and FastLED **3.9.15**. See the
[platform release](https://github.com/platformio/platform-espressif32/releases/tag/v6.12.0)
and [FastLED release](https://github.com/FastLED/FastLED/releases/tag/3.9.15).

If automatic port detection fails, run `pio device list`, then append
`--upload-port YOUR_SERIAL_PORT` to the upload command. Some boards need BOOT
held while the uploader connects; release it once writing begins. Use a lower
`upload_speed = 115200` in `platformio.ini` if uploading is unreliable.

The default [ESP32 Dev Module target](https://docs.platformio.org/en/stable/boards/espressif32/esp32dev.html)
is for a classic ESP32, not an ESP32-C3/S3 or every vendor variant. Change
`board = esp32dev` to your actual [PlatformIO board ID](https://docs.platformio.org/en/latest/boards/index.html)
if needed; also verify GPIO 18 is available on that board. The environment name
can stay `esp32dev` so the commands above still work. Other boards have not been
build-verified here.

To enable selection messages, set `SerialDebug = true` in `include/Config.h`,
rebuild, and run `pio device monitor -e esp32dev`. Default baud is 115200; keep
`monitor_speed` and `SerialBaud` consistent. Firmware never waits for a monitor.

## Power and wiring

**Disconnect power while wiring. Never apply 5 V to an ESP32 GPIO or 3V3 pin.**
Do not infer wire function from insulation color alone.

```text
Regulated 5 V USB power bank
  +-- USB cable --------------------------> ESP32 USB connector
  +-- rated 5 V distribution -- fuse ----> LED string +5V
  |                         +------------> converter HV reference
  +-- GND -------------------------------> LED string GND
                            +------------> ESP32 GND
                            +------------> converter GND

ESP32 3V3 -------------------------------> converter LV reference
ESP32 GPIO 18 --> converter LV channel
                 matching HV channel --> 330-470 ohm resistor --> pixel 0 DIN

Data direction: GPIO 18 --> converter --> DIN [0] DOUT --> DIN [1] ... [15]
```

The resistor belongs near the first pixel's DIN. Keep the data run short and
route a ground wire alongside it. A 500–1000 µF capacitor rated for at least
6.3 V across LED +5V/GND near the string input can help with supply transients;
observe its polarity. These are common addressable-LED practices, described in
[Adafruit's wiring guidance](https://learn.adafruit.com/adafruit-neopixel-uberguide/best-practices).
Check your pixel manufacturer's requirements as well.

| Connection | Destination |
| --- | --- |
| Power bank USB | ESP32 USB connector |
| Direct 5 V branch | LED +5V and converter HV |
| ESP32 3V3 | Converter LV reference only |
| ESP32 GPIO 18 | One converter low-voltage channel |
| Corresponding converter high-voltage channel | Resistor, then pixel 0 **DIN** |
| Supply GND | ESP32 GND, LED GND, and converter GND |
| Each pixel DOUT | Next pixel DIN |

All grounds must connect even when power travels through separate branches.
The input end is **DIN**, often marked with an arrow pointing into the string;
connecting the controller to DOUT will not work. The external converter needs
no firmware driver or special handling. If it has an enable pin, wire that
according to its own documentation.

Some bidirectional MOSFET-based **I²C** level converters have slow signal edges
and may be unreliable for WS2811 data. If flickering persists with good power and
short wiring, use a suitable fast unidirectional buffer such as a **74AHCT125**.
For an SN74AHCT125, use a 5 V VCC, common GND, a 3.3 V input from GPIO 18, and the
corresponding 5 V output to DIN; enable the selected channel by holding its
active-low OE low. Add local decoupling (typically 0.1 µF) and handle unused
inputs per the datasheet. It does not have separate LV/HV reference pins like
the bidirectional module. See [TI's part documentation](https://www.ti.com/product/SN74AHCT125)
and [Adafruit's level shifting guide](https://learn.adafruit.com/adafruit-neopixel-uberguide/logic-level).

**Do not assume the ESP32 board's 5V/VIN pin, USB connector, fuse, diode, or PCB
traces can carry the full LED current.** The preferred wiring distributes 5 V
directly to the LED string, bypassing the ESP32 board for the LED load. Verify
the actual board schematic before using its 5 V pin for anything. Never power
the LEDs from 3V3. Avoid backfeeding a computer's USB port: when programming,
disconnect the power bank and do not tie separate USB 5 V sources together.

Size the bank, fuse, cables, and connectors using the pixels' documented maximum
current plus ESP32 demand and margin. As a conservative planning example,
60 mA per RGB pixel at full white gives **0.96 A for 16 pixels**, before the
ESP32. Real WS2811 products differ; measure your assembly. A properly rated
5 V / 2 A supply is a reasonable starting budget for this example, subject to
the actual parts. The supplied red/blue effects at brightness 50 use less than
that full-white example, but brightness is **not a current limiter or fuse**.
See [addressable-LED power planning](https://learn.adafruit.com/adafruit-neopixel-uberguide/powering-neopixels).

## Pixel indexing and animations

Pixel 0 is the first address after DIN, then 1 through 15 follow the data chain.
Physical placement is documented as:

| Physical group | Pixel indices |
| --- | --- |
| A | 0, 1, 2, 3 |
| B | 4, 5, 6, 7 |
| C | 8, 9, 10, 11 |
| D | 12, 13, 14, 15 |

These labels describe placement only. The engine uses one independent `CRGB`
per pixel. A future animation can assign four different colors or intensities
within group A; no group object forces pixels to share output.

### 1. Alternating red and blue

Each phase lasts **500 ms**, continuously illuminated with no off interval.
The animation runs for **4000 ms** per selection.

| Animation-relative time | Blue pixels | Red pixels |
| --- | --- | --- |
| 0–499 ms | 0–3, 8–11 | 4–7, 12–15 |
| 500–999 ms | 4–7, 12–15 | 0–3, 8–11 |
| 1000–1499 ms | Same as initial phase | Same as initial phase |

The phases continue alternating. Colors are full-intensity `CRGB::Blue`
`(0, 0, 255)` and `CRGB::Red` `(255, 0, 0)` before global brightness scaling.
Every re-entry starts with the first row.

### 2. Alternating police strobes

Each color phase lasts **500 ms**. Active pixels repeat **25 ms ON, 25 ms OFF**;
all other pixels remain black. The animation runs for **4000 ms** per selection.

| Animation-relative time | Active color | Active pixels |
| --- | --- | --- |
| 0–499 ms | Blue | 0–3, 12–15 |
| 500–999 ms | Red | 4–11 |
| 1000–1499 ms | Blue | 0–3, 12–15 |

At 0 ms pixels start ON, at 25 ms they turn OFF, at 50 ms they turn ON, and
so forth. Each color phase restarts the strobe clock, including when a phase
duration is not divisible by the ON+OFF cycle. Thus red starts ON at exactly
500 ms. Re-entry always starts with blue ON. ON and OFF durations are independent
settings; no additional strobe patterns are implemented.

## Configuration

All firmware settings live in [`include/Config.h`](include/Config.h).
Change them, then rebuild and upload:

| Setting | Default | Purpose |
| --- | --- | --- |
| `CHASE_LED_CHIPSET` | `WS2811` | FastLED chipset template |
| `LedDataPin` | `18` | GPIO data output |
| `LedCount` | `16` | Allocated number of addressable RGB pixels |
| `LedColorOrder` | `RGB` | Physical channel byte order |
| `GlobalBrightness` | `50` | Output cap, 0–255 |
| `AnimationRotationMs` | `4000` | Wall-clock duration per selected animation |
| `AlternatingColorsPhaseMs` | `500` | Animation 1 phase length |
| `PoliceColorPhaseMs` | `500` | Animation 2 color phase length |
| `PoliceStrobeOnMs` | `25` | Strobe ON duration |
| `PoliceStrobeOffMs` | `25` | Strobe OFF duration |
| `SerialDebug` | `false` | Log startup and animation selections |
| `SerialBaud` | `115200` | Debug UART baud rate |

For wrong colors, try the correct `GRB`, `BRG`, or other FastLED `EOrder` value
instead of changing the effect mappings. Verify against your actual pixels.
Changing the board target belongs in `platformio.ini`.

Brightness scales each pixel's RGB channels at transmission time: a pixel with
red 128 remains dimmer than a pixel with red 255 under the same global cap.
It does not overwrite the source buffer. Temporal dithering is disabled so
unchanged frames do not need repeated transmissions.

Compile-time checks reject zero timing values and overflowing strobe cycles.
Changing `LedCount` does not require scheduler changes. Existing effect mappings
are explicit in their respective `.cpp` files: pixels beyond 15 default to OFF,
and indices missing from a shorter string are safely ignored. Update mappings
when adding pixels that should participate in an effect.

## Architecture and expansion

```text
platformio.ini
include/
  Config.h
src/
  main.cpp                         FastLED initialization, registry, output
  animations/
    Animation.h                    Common elapsed-time render interface
    PixelMapping.h                 Optional index-list helper for effects
    AlternatingColors.h / .cpp
    PoliceStrobe.h / .cpp
  engine/
    PixelBuffer.h                  Bounds-checked view of the CRGB array
    AnimationScheduler.h / .cpp     Selection and wall-clock timing
test/
  run_tests.py                     Host compiler runner; no Python packages
  test_firmware.cpp                Automated production-logic tests
  support/FastLED.h                Host-only CRGB shim
```

`setup()` initializes FastLED, sets brightness, initializes the RAM buffer OFF,
selects animation 1, and renders/transmits its elapsed-zero frame immediately.
There is no diagnostic animation, transmitted startup blackout, or serial wait.

The scheduler receives wall-clock `millis()` as a `uint32_t`. Unsigned
`now - activatedAt` arithmetic remains correct across its roughly 49.7-day
rollover, provided execution does not stop for an entire rollover period.
Animations receive only their elapsed milliseconds since activation.

At the first loop observing 4000 ms elapsed, the scheduler selects the next
registered effect and renders it at zero. A late loop starts the next effect
at observation time; it does not skip effects or start partway through one.
Inside an effect, phase comes from elapsed time directly, so missed frames do
not accumulate timing drift. Selection clears RAM before rendering, and only
the final frame is transmitted: no added transition blackout or fade.

The loop has no `delay()` calls and does not wait for an animation phase to
finish. It idles for one FreeRTOS tick (1 ms on the pinned target) between polls
to avoid busy-spinning. This bounds ordinary polling latency to about a tick
plus processing/transmission time; it is not used to count animation intervals.
Transitions have no separate wait. `FastLED.show()` is called only for a changed
frame or an activation's first frame. There is one static LED buffer, no frame
copies, and no application heap allocation in rendering or scheduling.

To add an animation:

1. Add a class under `src/animations/` deriving from `Animation`.
2. Implement `name()` and `render(PixelBuffer&, uint32_t elapsedMs) const`.
   Calculate output from elapsed time. Write every pixel, including black for
   inactive pixels, and return whether any assignment changed the buffer.
3. Use `pixels.size()` and `pixels.set(index, CRGB(red, green, blue))`; intensities
   can differ per pixel. Accumulate change flags with
   `changed = pixels.set(i, color) || changed` so every write executes even when
   an earlier write changed something. Render elapsed zero as the initial state.
4. Create a static instance in `main.cpp` and append its address to `animations`
   in the desired playback order. Registry length is calculated automatically.
5. Add tests for its frame boundaries and restart behavior.

For example, a renderer can set pixel 0 to `CRGB(255, 0, 0)`, pixel 1 to
`CRGB(128, 0, 0)`, pixel 2 to `CRGB(0, 0, 64)`, and pixel 3 to `CRGB(0, 0, 191)`.
The rest of the frame remains the renderer's responsibility.

Future button code can call `scheduler.next(now)` or `scheduler.select(index,
now)` before `update()`. Both reset activation timing when the first frame is
rendered; `select()` also permits
restarting the current effect and rejects invalid indices. No button pins,
debouncing, runtime brightness controls, or special-effect modes exist yet.
A future runtime brightness change should also request an LED transmission.

Future headlights, indicators, or brake lights can use additional pixels and
expanded renderers. They need explicit output ownership/composition if they must
remain lit across effects: these initial animations deliberately turn unmapped
pixels OFF. No vehicle-light behavior, chasing effects, extra strobe patterns,
or connectivity is implemented.

## Verification

Run the hardware-independent tests with Python 3 and a C++17 host compiler:

```sh
python3 test/run_tests.py
python3 test/run_tests.py --sanitize
pio run -e esp32dev
```

The sanitizer option requires compiler support for AddressSanitizer and
UndefinedBehaviorSanitizer (Clang/GCC). Set `CXX` to choose a host compiler if
needed. Temporary executables are created outside the project and cleaned up.
No third-party test framework or Python packages are needed. This suite is run
by the script above, not `pio test`.

The 25 tests compile the **actual animation and scheduler `.cpp` files** against
a minimal host color shim. They check complete pixel mappings, exact phase and
strobe boundaries, independently configured timing, phase clock resets,
5000 ms test rotation, N-effect registration, per-pixel intensity, re-entry,
stale-pixel clearing, skipped loop frames, invalid configurations, bounds,
larger/shorter strings, unchanged-frame suppression, and unsigned rollover.
Boundary tests use fixed 5000 ms rotation and 50/50 ms strobe inputs, independently
of the current settings in `Config.h`, so tuning playback does not invalidate
their expected frames. Additional cases exercise other timing values.
The shim does not test the ESP32 driver, global brightness scaling, or physical
signal timings; the real FastLED integration is checked by the ESP32 build.

Implementation verification completed: **25/25 host tests passed with both
sanitizers; `pio run -e esp32dev` succeeded**. The build used 21,968 bytes of
static RAM and 268,921 bytes of application flash. No upload or physical hardware
test has been performed.

Before wearing, verify on a workbench with the intended power bank:

1. Start at lower brightness if needed; power on with no one looking directly
   at the LEDs. Confirm pixel 0 follows the DIN end and the initial blue/red
   mapping matches the Animation 1 table.
2. Confirm the full mapping swaps at 0.5 s and returns at 1.0 s, without an off
   interval. At 4.0 s, confirm blue strobe pixels are 0–3 and 12–15 and others OFF.
3. Use slow-motion video or a logic analyzer if available to check ON at
   animation 2 time 0 ms, OFF at 25 ms, ON at 50 ms, red ON at 500 ms, and blue
   ON again at 1000 ms. Allow normal polling and transmission latency.
4. At 8 s confirm animation 1 restarts in its original mapping, with no
   transition blackout. Observe several rotations and power-cycle to check the
   startup always returns to animation 1. Debug logs can confirm selection times.
5. Inspect for flickering, resets, warm wiring/connectors, and supply droop under
   load. Check the far end's 5 V rail and the bank's low-current auto-off behavior.
   Software tests cannot establish power, converter, or wearable reliability.

## Troubleshooting

| Symptom | Checks |
| --- | --- |
| No pixels light | 5 V/polarity, shared ground, GPIO 18, DIN rather than DOUT, converter references/enable, correct firmware uploaded |
| Wrong colors | `LedColorOrder` versus the string's byte order; rebuild/upload after edits |
| Flickering / random colors | Ground continuity, converter suitability, short data/ground routing, resistor, supply voltage under load, loose connectors; consider 74AHCT125 |
| Resets / brownouts / dim far end | Power-bank and cable current ratings, direct LED power branch, voltage drop; lower brightness or improve distribution |
| Only part of string works | `LedCount`, damaged pixel/connector, data direction, actual addresses versus physical lamps |
| Power bank switches itself off | Verify bank supports the load across both effects; try a bank with an appropriate always-on mode |
| Upload does not connect | USB data cable, correct serial port/driver, BOOT procedure, board target, upload speed |
| Serial monitor is quiet | `SerialDebug` defaults OFF; enable, rebuild/upload, and match baud |

## Wearable and strobe precautions

Use insulated, secured connections and strain relief. Keep conductors away from
skin and conductive costume parts, avoid sharp edges and snagging loops, and
secure the power bank so it cannot pull on wires. Keep the assembly dry and
ventilated. Have an adult supervise, check for heat, and provide an easy way to
disconnect power. Test the fully assembled costume before use; do not charge
the bank while it is being worn.

Start with the lowest practical brightness, diffuse exposed pixels, and aim
lights away from the child's and bystanders' eyes. The configured 25/25 ms effect
flashes at 20 Hz within each color phase. Flashing lights can trigger seizures
in susceptible people; reducing brightness does not establish that a strobe is
safe. Avoid using flashing modes around anyone with photosensitivity, follow
venue restrictions, and stop use if anyone feels unwell. See the
[Epilepsy Foundation's photosensitivity guidance](https://www.epilepsy.com/what-is-epilepsy/seizure-triggers/photosensitivity).
To omit animation 2, remove `&policeStrobe` from the registry and rebuild; the
remaining animation still changes colors. Power off if flashing should be avoided
entirely. Do not shorten flash timings or add more aggressive patterns without
an explicit configuration decision and a fresh review of suitability.
