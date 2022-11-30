# lv_gba_emu
LVGL Game Boy Advance Emulator
* GUI: https://github.com/lvgl/lvgl
* GBA Emulator: https://github.com/libretro/vba-next
* Test ROM: https://github.com/XProger/OpenLara

# Clone
```bash
git clone https://github.com/FASTSHIFT/lv_gba_emu.git --recurse-submodules
```

# Build
```bash
mkdir build
cd build
cmake ..
make -j
sudo ./build/Simulator 
```
