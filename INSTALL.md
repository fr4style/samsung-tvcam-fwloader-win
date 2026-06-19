# Samsung TV Camera Firmware Loader

## Windows install

1. Install the WinUSB driver for `VID_04E8&PID_205C`.
2. Copy `samsung-fwloader.exe`, `libusb-1.0.dll`, and `FalconFW.bin` into the same folder.
3. Connect the camera in firmware-update mode.
4. Run `samsung-fwloader.exe`.

## Getting FalconFW.bin

FalconFW.bin cannot be downloaded directly — it must be extracted from the official Samsung TV software package.

**Method 1 (recommended): Extract from Samsung AllShare / Smart View installer**
1. Download the Samsung Smart View Windows installer from:
   https://www.samsung.com/us/support/owners/app/smart-view
2. Open the installer with 7-Zip or similar tool (do not run it)
3. Navigate to the embedded cab/zip archive
4. Extract FalconFW.bin to the same folder as samsung-fwloader.exe

**Method 2: Use the Linux project's documented source**
See https://github.com/ondrej-zary/samsung-tvcam-fwloader for alternative extraction methods and supported firmware filenames for other camera models (VG-STC4000, VG-STC5000).

**Supported cameras and their firmware filenames:**
| Model      | VID:PID   | Firmware file     |
|------------|-----------|-------------------|
| VG-STC3000 | 04e8:205c | FalconFW.bin      |
| VG-STC4000 | 04e8:2061 | FalconPlus_FW.bin |
| VG-STC5000 | 04e8:2065 | (see project docs) |

## Building

The MinGW packaging files live under `workspace/src/packaging/`.

Use the provided `Makefile` or `CMakeLists.txt` with the MinGW toolchain file:

- `workspace/src/packaging/Makefile`
- `workspace/src/packaging/CMakeLists.txt`
- `workspace/src/packaging/cmake/mingw-w64-x86_64.cmake`

The build expects libusb headers in `include/` and the libusb import library plus DLL in `lib/`.
