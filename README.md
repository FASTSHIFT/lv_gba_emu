# LVGL Game Boy Advance Emulator

![image](https://github.com/FASTSHIFT/lv_gba_emu/blob/main/images/lv_gba_emu_2.png)

* GUI: https://github.com/lvgl/lvgl
* GBA Emulator: https://github.com/libretro/vba-next
* Test ROM: https://github.com/XProger/OpenLara

## Feature
* The emulator kernel is based on [vba-next](https://github.com/libretro/vba-next) and does not depend on any third-party libraries.
* Decoupled from the OS, only relying on lvgl's memory allocation and file access interface.
* Support to use GBA framebuffer directly as `lv_canvas` buffer, zero copy overhead.
* Audio output.
* Frame rate control.
* Multiple input device.
* Virtual key.
* Memory usage optimization(~800KB + ROM size).
* Game saves support (auto load/save).
* Game Launcher (ROM selection menu).

## Controls
* **Exit to Menu**: Long press `Select` (Backspace on Keyboard) for 2 seconds.

## Clone
```bash
git clone https://github.com/FASTSHIFT/lv_gba_emu.git --recurse-submodules
```

## Build & Run (PC)
```bash
mkdir build
cd build
cmake ..
make -j
./gba_emu -d ../rom
```

### Command Line Options
```bash
Usage: ./gba_emu -f <string> -d <string> -m <decimal-value> -v <decimal-value> -s -h

Where:
  -f <string> rom file path.
  -d <string> rom directory path (default: .).
  -m <decimal-value> view mode: 0: simple; 1: virtual keypad.
  -v <decimal-value> set volume: 0 ~ 100.
  -s skip intro animation.
  -h help.
```

## Raspberry Pi Setup
The project includes an installation script for Raspberry Pi that sets up the emulator to start automatically on boot.

```bash
# Run as root
sudo ./install_rpi_autostart.sh
```

To uninstall:
```bash
sudo ./install_rpi_autostart.sh uninstall
```

## Key Mapping
### SDL2
|KeyBoard|GBA|
|-|-|
|Backspace|Select|
|Tab|Start|
|Up|Up|
|Down|Down|
|Left|Left|
|Right|Right|
|X|B|
|Z|A|
|L|L|
|R|R|
