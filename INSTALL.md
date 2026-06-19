# Samsung TV Camera Firmware Loader

## Windows install

1. Install the WinUSB driver for `VID_04E8&PID_205C`.
2. Copy `samsung-fwloader.exe`, `libusb-1.0.dll`, and `FalconFW.bin` into the same folder.
3. Connect the camera in firmware-update mode.
4. Run `samsung-fwloader.exe`.

## Building

The MinGW packaging files live under `workspace/src/packaging/`.

Use the provided `Makefile` or `CMakeLists.txt` with the MinGW toolchain file:

- `workspace/src/packaging/Makefile`
- `workspace/src/packaging/CMakeLists.txt`
- `workspace/src/packaging/cmake/mingw-w64-x86_64.cmake`

The build expects libusb headers in `include/` and the libusb import library plus DLL in `lib/`.
