# LVGL Game Boy Advance Emulator

![image](https://github.com/FASTSHIFT/lv_gba_emu/blob/main/images/lv_gba_emu_2.png)

* GUI: https://github.com/lvgl/lvgl
* GBA Emulator: https://github.com/libretro/vba-next
* Test ROM: https://github.com/XProger/OpenLara

## Feature
* The emulator kernel is based on [vba-next](https://github.com/libretro/vba-next) and does not depend on any third-party libraries.
* Decoupled from the OS, only relying on lvgl's memory allocation and file access interface.
* Support to use GBA framebuffer directly as `lv_canvas` buffer, zero copy overhead.

## To be completed

- [x] Audio output support.
- [x] Frame rate control support.
- [x] Optimize key mapping and decouple from linux event device.
- [x] Virtual key support.
- [x] Memory usage optimization.

## Clone
```bash
git clone https://github.com/FASTSHIFT/lv_gba_emu.git --recurse-submodules
```

## Build & Run
```bash
mkdir build
cd build
cmake ..
make -j
./Simulator ../rom/OpenLara.gba
```
