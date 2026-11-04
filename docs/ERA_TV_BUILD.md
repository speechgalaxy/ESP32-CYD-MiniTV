# Era TV build guide

This guide turns a CYD display board, the [CYD Retro MiniTV](https://makerworld.com/en/models/2777502-cyd-retro-minitv) case, and the parts in [ERA_TV_BOM.md](ERA_TV_BOM.md) into a self-contained miniature TV with:

| Control | What it does |
|---------|--------------|
| **Channel knob** (turn) | Tunes up or down one channel per click. A green `CH 03` appears top-right. When the knob stops, a burst of TV static and hiss plays and the new channel starts. |
| **Channel knob** (press) | Click = next channel, double-click = previous channel, hold 1 s = mute or unmute. |
| **VOL + / VOL −** | Changes volume in steps of 10. A volume bar appears at the bottom, and holding a button repeats. Either button cancels mute. |
| **Rear toggle** | Real power switch for the rear USB-C input. |

The gift firmware runs fully offline. It boots straight into the last channel watched, remembers volume and channel across power cycles, and never asks the recipient to set up Wi-Fi.

Plan for about 2–3 hours per TV once the parts and prints are ready, plus print time.

---

## 1. Suggested timeline

1. **Now:** print one test case and knob, and make the case modifications in section 3.
2. **When the small parts arrive (around Nov 13–19):** build one complete prototype harness.
3. **When the boards arrive (around Nov 14–21):** set up the firmware tools (section 2), flash every board, and bench-test each one bare before wiring it.
4. **Then:** wire, test, assemble, and load media for all five.

Keep the sixth board as your development and test unit.

---

## 2. Firmware setup and flashing

### 2.1 One-time Arduino IDE setup

The firmware requires **ESP32 Arduino core 2.0.17**. Your Arduino IDE currently has core 3.3.12, which will not compile this project.

1. Open **Arduino IDE → Tools → Board → Boards Manager**, search **esp32** (by Espressif Systems), choose version **2.0.17** from the dropdown, and click **Install**. This replaces 3.3.12 for all sketches; reinstall 3.3.12 later if another project needs it.
2. Open **Library Manager** and install:
   - **GFX Library for Arduino** — version **1.6.0** exactly.
   - **JPEGDEC** — version **1.8.4**. Accept the `bb_spi_lcd` dependency if prompted.
3. Download `https://github.com/pschatzmann/arduino-libhelix/archive/refs/heads/main.zip` and add it with **Sketch → Include Library → Add .ZIP Library…**.

These exact versions were used to compile the Era TV firmware cleanly: 1.04 MB of flash, 22% of RAM.

### 2.2 Settings in `MiniTV/config.h`

`BOARD_CYD_ERA_TV` is already selected. The settings you might change are:

| Setting | Default | Change it when |
|---------|---------|----------------|
| `SCREEN_INVERT` | `true` | Colors look like a photo negative. Set to `false`. |
| `ENCODER_REVERSE` | `0` | Clockwise lowers the channel. Set to `1`, or swap the encoder A and B wires. |
| `ENCODER_STEPS_PER_DETENT` | `4` | Two clicks are needed per channel: set to `2`. Each click jumps two channels: set back to `4`. |
| `VOLUME_STEP` | `10` | You want finer or coarser volume steps. |
| `CHANNEL_STATIC_MS` | `300` | You want longer or shorter static between channels; `0` turns it off. |
| `WIFI_ENABLED` | `0` | Set to `1` on **your** test unit to use the web interface (see section 8). Set it back to `0` before flashing gifts. |

### 2.3 Flash each board

1. Connect the board's own USB port (either the USB-C or the micro-USB on the 2USB board) to your computer. Leave the rear power jack disconnected while doing this.
2. **Tools** settings: Board **ESP32 Dev Module**, Upload Speed **921600**, Partition Scheme **Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)**, and the board's COM port.
3. Open `MiniTV/MiniTV.ino` and click **Upload**. If the upload stalls at `Connecting…`, hold the board's **BOOT** button until writing starts.
4. Open **Serial Monitor** at **115200 baud** and press **RST**. You should see `MiniTV firmware 1.0.1` and `Wi-Fi disabled in config.h; web control is unavailable`.

### 2.4 Bare-board bench test (before any soldering)

Insert a FAT32 microSD card with at least one channel (see section 7) and power the board from its USB port.

- The picture should have natural colors. A photo-negative look means `SCREEN_INVERT` needs to be `false`.
- Plug a speaker into the board's **SPEAK** socket to check sound before you build the harness. Any 8 Ω speaker with a 1.25 mm 2-pin plug works.
- A board that fails here should go back to the seller now, before you solder to it.

---

## 3. Print and modify the case

Download the CYD Retro MiniTV files from MakerWorld (the *Retro TV Profile*): **TV Body**, **TV Bezel**, **Grill**, **VCR**, and the **solder** version of the back panel, *TV Back Magnetic Pogo*. That back panel has the rear USB-C cutout. The `.step` file is included for CAD edits. Print in the provided orientation with no supports. The project hero photo also adds splayed legs; a commenter on the MakerWorld page shared a legs remix if you want that look.

Make these changes in CAD (Fusion, Onshape, or Bambu Studio's cut tools) or by drilling the prints:

| Location | Original | Change to | Why |
|----------|----------|-----------|-----|
| Front round button hole (right of the screen, above the grille) | Pocket for a 6 × 6 tact switch | **7.3 mm through-hole**. Remove the switch pocket behind it. Wall thickness ≤ 3 mm. | EC11 encoder bushing (M7 × 0.75) with its nut on the outside |
| Right side wall (or the top, right of the VCR) | — | **Two 3.8 mm holes**, 12 mm apart, each with a 6.3 × 6.3 mm square pocket behind it | 6 × 6 × 9 mm tact switches for VOL + (upper) and VOL − (lower) |
| Back panel | Pogo-pin cutout | Leave the pogo cutout open or cover it. Add a **6.2 mm hole** for the toggle switch. | MTS-101 toggle; the USB-C cutout is already there |

**Dry-fit before committing.** I couldn't check the case's inside clearances from the published files.

- The EC11 body is 12 × 12 × 7 mm behind the panel, plus its pins. Fit the CYD into the body, then check that the encoder clears the board, the speaker, and the board's USB connectors. If it is tight, bend the encoder pins sideways before soldering. If it still won't fit, put the knob on the right side wall instead.
- Mark the volume-button positions with the CYD installed so the switches miss its USB connectors.
- The toggle switch needs about 13 mm of depth behind the back panel. Place it clear of the ESP32 module on the back of the board.

**Knob.** Print a knob with a **6.0 mm D-shaped bore (4.5 mm across the flat)**. Shorten the encoder shaft so about 10 mm sticks out past the nut. The EC11 has a small anti-rotation tab; either print a 1.2 × 2 mm slot for it, or snap it off with pliers and rely on the nut plus a drop of glue.

**Antennas.** Use 1.75 mm filament cut to length, as in the original design.

---

## 4. Wiring

### 4.1 Find each connector's pin order

The CYD has three 4-pin 1.25 mm sockets you'll use, plus the 2-pin speaker socket.

| Socket | Signals |
|--------|---------|
| **CN1** | GND, IO22, IO27, 3.3V |
| **P3** | GND, IO35, IO22, IO21 |
| **P1** | VIN (5 V), TX, RX, GND |
| **SPEAK** | Speaker + / − |

The physical order of the pins varies between board revisions, so **read the silkscreen labels next to each socket on your board** and write the wire color for each signal on a piece of tape. Confirm with a multimeter before soldering:

- **GND** beeps to the metal shell of the USB connector.
- **3.3V** on CN1 reads about 3.3 V with the board powered over USB.
- **VIN** on P1 reads about 4.7–5 V with the board powered over USB.

Use the plug half of each 1.25 mm pair: the one whose white housing fits onto the board's pins.

### 4.2 Wiring diagram

```text
KNOB — EC11 encoder (3-pin side: A C B, 2-pin side: SW1 SW2)
  CN1 IO27 ───────────── A
  CN1 IO22 ───────────── B
  CN1 GND  ───────────── C (middle) ── short jumper ── SW2
  P1  RX   ───────────── SW1

VOLUME LADDER — at the side buttons
  CN1 3.3V ── 10 kΩ ──┐
  P3  IO35 ───────────┼──────────────── VOL+ ──┐
                      └── 10 kΩ ─────── VOL− ──┤
  P3  GND  ────────────────────────────────────┘

POWER — back panel
  USB-C red (+5 V) ─── toggle switch ─── P1 VIN
  USB-C black (GND) ──────────────────── P1 GND

SPEAKER
  SPEAK plug (2-pin) ─── speaker (either polarity)

CUT SHORT AND INSULATE
  P1 TX,  P3 IO22 (duplicate of CN1 IO22),  P3 IO21 (display backlight!)
```

How the ladder works: idle, the pull-up holds IO35 at 3.3 V. VOL+ shorts it to ground (reads near 0). VOL− forms a half-voltage divider (reads about 1.65 V). The firmware tells them apart on a single input-only pin.

**Never connect P3's IO21** (it drives the backlight) or the spare IO22 wire.

On the tact switches, use **diagonally opposite legs**. Each side's legs are joined inside the switch, so diagonal legs always give a working switch.

### 4.3 Build the harnesses

Slide heat shrink onto every wire before soldering, and test each joint with the multimeter's continuity beep afterwards.

1. **Speaker lead.** Solder a 2-pin plug lead to the speaker's two wires.
2. **Knob harness (CN1 plus one P1 wire).**
   1. Solder CN1 **IO27 → A**, **IO22 → B**, **GND → C**.
   2. Add a short jumper from **C** to one push-switch pin (**SW2**).
   3. Solder P1 **RX → the other push-switch pin (SW1)**.
   4. Leave CN1's **3.3V** wire long. It runs to the volume ladder.
3. **Volume harness (P3 plus CN1 3.3V).**
   1. Solder P3 **GND** to one diagonal leg of each button (daisy-chain).
   2. Solder P3 **IO35** to VOL+'s other diagonal leg.
   3. Solder a **10 kΩ** resistor from IO35 to **CN1 3.3V** (the pull-up).
   4. Solder a second **10 kΩ** resistor from IO35 to VOL−'s other diagonal leg.
   5. Cover the resistors with heat shrink.
   6. Cut and insulate P3's **IO22** and **IO21** wires.
4. **Power harness (P1 VIN and GND).**
   1. USB-C **red (+5 V) → toggle switch → P1 VIN**.
   2. USB-C **black → P1 GND**.
   3. Cut and insulate P1's **TX** wire.
   4. **Check polarity before plugging into the board:** with a USB charger in the jack and the toggle on, the P1 plug's VIN wire must read +5 V against its GND wire.

---

## 5. Bench test the wired board (case still open)

Plug all four harnesses into the board, insert the SD card, and power it from the **rear USB-C jack** with the toggle on. Don't connect the board's own USB port at the same time; the two 5 V supplies would back-feed each other.

- [ ] Toggle off/on powers the TV down and up.
- [ ] Video plays with sound.
- [ ] Each knob click shows `CH xx` top-right. When the knob stops, static plays and then the new channel starts.
- [ ] Clockwise goes **up**. If not, set `ENCODER_REVERSE 1` or swap the A and B wires.
- [ ] One click equals one channel. If not, see `ENCODER_STEPS_PER_DETENT` in section 2.2.
- [ ] VOL+ and VOL− move the green bar; holding repeats; nothing changes when no button is pressed.
- [ ] Knob press = next channel, double-press = previous, hold 1 s = `MUTING` / `UNMUTING`.
- [ ] Power-cycle: the TV returns to the same channel and volume.

---

## 6. Final assembly

1. Glue the **grille** and **VCR** to the body. The grille has a little wiggle room, so line it up before the glue sets. The VCR uses locating nubs.
2. Fix the **speaker** behind the grille with double-sided foam tape or a ring of hot glue around its rim. Keep glue off the speaker cone.
3. Mount the **encoder** through the front hole with its washer and nut, then push on the knob.
4. Press the **volume tact switches** into their pockets. Add a small drop of glue on the switch body only, never on the plunger.
5. Plug the harnesses onto CN1, P3, P1 and SPEAK, then fit the CYD and secure it with **4 × M3 × 6 mm** button-head screws without forcing it.
6. Mount the **USB-C jack** and **toggle** in the back panel. Route wires so nothing presses on the back of the display or the ESP32 module.
7. Close the back with **4 × M2 × 6 mm** countersunk screws, then glue in the filament antennas.
8. Run through the section 5 checklist once more.

---

## 7. Load media

Format each card **FAT32** and lay it out as described in the main [README](../README.md#prepare-the-sd-card): `Videos/1/`, `Videos/2/`, … with matching `.mjpeg` + `.mp3` pairs at 320 × 240, 24 fps. The [MiniTV Video Converter](https://github.com/DynaMight1124/MiniTV-Video-Converter) handles the conversion; choose **320×240**, **24 FPS**, **MP3**. The SD slot stays reachable from underneath the case.

**Playback note for shows with commercial breaks.** A numbered channel plays the **first** `.mjpeg` in its folder, and when that file ends the TV moves to the **next channel**. To keep a show-and-commercials lineup on one channel today, join the lineup into one long file per channel before converting. The firmware could instead be changed to loop each channel through all of its files in order. The `Videos/random/` folder already plays random matching pairs back to back.

---

## 8. Using Wi-Fi on your own test unit (optional)

The web interface makes uploading and renaming channels easier while you're building. On your test unit only:

1. Set `WIFI_ENABLED 1` in `config.h` and flash.
2. Join the `MiniTV-Setup-XXXX` network shown on screen and open `http://192.168.4.1` to choose your Wi-Fi.
3. Use `http://minitv.local` afterwards.

Flash the gifts with `WIFI_ENABLED 0`. A Wi-Fi build that can't find its saved network stops on the setup screen instead of playing TV, which is exactly what would happen at a recipient's house. Large media is much faster to copy with an SD card reader than over Wi-Fi.

---

## 9. Troubleshooting

| Symptom | Fix |
|---------|-----|
| Colors look like a negative | Set `SCREEN_INVERT false`. |
| Red and blue swapped, or washed-out colors | Check that all boards came from the same listing (ST7789 variant). Note the board markings and report it; the display driver may need a profile tweak. |
| Knob changes channel the wrong way | `ENCODER_REVERSE 1`, or swap the encoder A/B wires. |
| Two clicks per channel / two channels per click | `ENCODER_STEPS_PER_DETENT 2` / `4`. |
| Volume changes by itself | The 10 kΩ pull-up from 3.3V to IO35 is missing or broken. IO35 has no internal pull-up and floats without it. |
| Only VOL+ works | Check the second 10 kΩ resistor between IO35 and VOL−. |
| Screen goes dark or flickers when a harness is connected | P3's **IO21** (backlight) wire is touching something. Cut and insulate it. |
| Knob press does nothing | Check the jumper from encoder C to SW2 and the P1 **RX** wire to SW1. |
| No power from the rear jack | Use a USB-A charger and cable. Check the toggle wiring and the polarity reading in section 4.3. |
| No sound | Check the speaker plug in **SPEAK**, and that volume isn't 0 or muted (press VOL+). |
| `ERROR: File system mount failed!` | Reformat the card as FAT32 (32 GB or smaller is easiest) and reseat it. |
| `No videos found!` | The card needs `Videos/1/` (or `Videos/random/`) with a matching `.mjpeg` + `.mp3` pair. |
| Upload fails | Disconnect the rear jack, hold **BOOT** during `Connecting…`, and use a USB data cable, not a charge-only one. |

Firmware details for the controls live in [`MiniTV/front_panel.h`](../MiniTV/front_panel.h).
