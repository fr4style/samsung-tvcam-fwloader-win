# Samsung TV Camera Firmware Loader for Windows
## Technical Specification v1.1

***

### Overview

Tool to load firmware into Samsung VG-STC3000 (and compatible) TV cameras when connected via USB to a Windows PC.
The camera contains only a bootloader and requires firmware to be injected into RAM at every connection.
On Linux this is handled by `samsung-tvcam-fwloader` (https://github.com/ondrej-zary/samsung-tvcam-fwloader).
This project is a Windows port of that tool.

***

### Supported Devices

| Model      | USB VID:PID | Firmware File       | Chip | Status   |
|------------|-------------|----------------------|------|----------|
| VG-STC3000 | 04e8:205c   | FalconFW.bin         | AIT  | Target   |
| VG-STC4000 | 04e8:2061   | FalconPlus_FW.bin    | AIT  | Optional |
| VG-STC2000 | 04e8:2059   | raptor_firmware.img  | MAX  | Optional |

***

### Architecture

The tool operates in **user-space** using WinUSB / libusb-1.0 — no kernel-mode driver is required.
This avoids the need for a Microsoft WHQL-signed kernel driver.

#### Components

1. **`samsung-fwloader.exe`** — CLI tool (C, compiled with MinGW or MSVC)
2. **`samsung-fwloader-tray.exe`** — Optional system tray app (C or C++ with WinAPI)
3. **`samsung-fwloader-service`** — Optional Windows Service for auto-load on USB insertion
4. **INF file** — Associates VID:PID with WinUSB/libusbK so Windows uses it instead of null driver

***

### Dependencies

- **libusb-1.0** for Windows (https://libusb.info) — precompiled DLL included in release
- OR **libusbK** as alternative backend
- **Zadig** (https://zadig.akeo.ie) — used once by the user to bind WinUSB/libusbK to the device

***

### How It Works (Protocol)

Reference: https://github.com/ondrej-zary/samsung-tvcam-fwloader/blob/master/fwloader.c

1. Open USB device by VID:PID using libusb
2. Claim interface 0
3. Read firmware file into memory
4. Send firmware via USB bulk transfer in chunks
   - AIT chip (VG-STC3000/4000): uses AIT protocol
   - MAX chip (VG-STC2000): uses MAX protocol
5. Send "boot" command to start firmware execution
6. Release interface and close device
7. Camera re-enumerates as a standard UVC webcam (USB Video Class)

#### AIT Protocol (VG-STC3000)

Based on reverse-engineering in the Linux driver:

```
1. Send command block: 0x40 (vendor request) to EP0
   bmRequestType = 0x40 (host-to-device, vendor, device)
   bRequest      = 0xF0 (firmware download)
   wValue        = 0x0000
   wIndex        = 0x0000
   wLength       = 0 (control transfer, no data stage)

2. Send firmware in 512-byte chunks via bulk OUT (endpoint 0x01 or as detected):
   for each chunk:
     libusb_bulk_transfer(handle, ep_out, chunk, chunk_size, &transferred, timeout)

3. Send boot trigger:
   bmRequestType = 0x40
   bRequest      = 0xF1 (boot)
   wValue        = 0x0000
   wIndex        = 0x0000
```

> Note: Exact endpoint numbers and request codes must be verified against the Linux source:
> https://github.com/ondrej-zary/samsung-tvcam-fwloader/blob/master/fwloader.c
> Cross-reference with USB captures if available.

***

### File Structure

```
samsung-fwloader-win/
├── src/
│   ├── main.c              # Entry point, argument parsing
│   ├── fwloader.c          # Core firmware loading logic (ported from Linux)
│   ├── fwloader.h
│   ├── usb_win.c           # Windows libusb wrapper
│   ├── usb_win.h
│   ├── tray.c              # (optional) System tray WinAPI code
│   └── service.c           # (optional) Windows Service wrapper
├── firmware/
│   └── FalconFW.bin        # NOT included — user must provide
├── drivers/
│   └── samsung-tvcam.inf   # INF file to bind WinUSB to device
├── cmake/
│   └── mingw-w64-x86_64.cmake  # Cross-compilation toolchain file
├── lib/
│   ├── libusb-1.0.dll      # Precompiled Windows DLL, bundled in release
│   └── libusb-1.0.lib
├── include/
│   └── libusb.h
├── Makefile                # MinGW build
├── CMakeLists.txt          # CMake build (MSVC + MinGW)
├── README.md
└── INSTALL.md
```

***

### Cross-Compilation (Linux → Windows)

Development and compilation happen **entirely on Linux**. No Windows environment is needed to write or build the tool.
Only the final USB hardware test requires a physical Windows machine.

#### Install cross-compiler

```bash
# Ubuntu/Debian
sudo apt install gcc-mingw-w64-x86-64 mingw-w64-tools

# Arch/Manjaro
sudo pacman -S mingw-w64-gcc

# Verify
x86_64-w64-mingw32-gcc --version
```

#### Download precompiled libusb for Windows

```bash
# Download official Windows binaries
wget https://github.com/libusb/libusb/releases/latest/download/libusb-1.0.27-binaries.7z
7z x libusb-1.0.27-binaries.7z -o libusb-win/

# Copy headers and libraries into project
cp libusb-win/include/libusb-1.0/libusb.h include/
cp libusb-win/MinGW64/static/libusb-1.0.a lib/
cp libusb-win/MinGW64/dll/libusb-1.0.dll lib/
```

#### Build

```bash
# Manual compilation
x86_64-w64-mingw32-gcc -o samsung-fwloader.exe \
    src/main.c src/fwloader.c src/usb_win.c \
    -I./include -L./lib -lusb-1.0 \
    -lsetupapi -static-libgcc

# Or via CMake with toolchain file
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=./cmake/mingw-w64-x86_64.cmake \
    -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

#### CMake toolchain file (cmake/mingw-w64-x86_64.cmake)

```cmake
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
```

#### Build output

After compilation, the `dist/` folder contains everything needed on the Windows machine:

```
dist/
├── samsung-fwloader.exe   # Windows executable
├── libusb-1.0.dll         # DLL — must stay in the same folder as the exe
├── FalconFW.bin           # Firmware (provided by user)
└── INSTALL.md             # End-user instructions
```

#### Transfer to Windows for testing

```bash
# Via SCP (if Windows has OpenSSH enabled)
scp -r dist/ user@windows-pc:C:/samsung-fwloader/

# Via rsync
rsync -avz dist/ user@windows-pc:C:/samsung-fwloader/
```

#### Agent workflow

```
[Linux AI Agent]
    │
    ├── 1. Read SPEC.md
    ├── 2. Fetch Linux reference source (GitHub)
    ├── 3. Write src/*.c ported for Windows
    ├── 4. Compile with x86_64-w64-mingw32-gcc
    ├── 5. Fix compilation errors → loop until build succeeds
    └── 6. Produce dist/samsung-fwloader.exe

[User on Windows]
    └── 7. Copy dist/, install WinUSB via Zadig, test with physical camera
```

Step 7 is the only phase that requires a Windows machine.
Steps 1–6 are fully automated by Linux-based AI agents.

***

### Build Instructions (native Windows, alternative)

#### Option A: MinGW on Windows

```bash
gcc -o samsung-fwloader.exe src/main.c src/fwloader.c src/usb_win.c \
    -I./include -L./lib -lusb-1.0 -lsetupapi -mwindows
```

#### Option B: CMake (MSVC or MinGW)

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"   # or -G "Visual Studio 17 2022"
cmake --build .
```

***

### INF File (WinUSB binding)

The user must install the INF once via Device Manager or `pnputil`, or use **Zadig** (recommended).

```ini
; samsung-tvcam.inf
[Version]
Signature="$Windows NT$"
Class=USB
ClassGUID={36FC9E60-C465-11CF-8056-444553540000}
Provider=%ManufacturerName%
DriverVer=01/01/2024,1.0.0.0

[Manufacturer]
%ManufacturerName%=Standard,NTamd64

[Standard.NTamd64]
%DeviceName%=USB_Install,USB\VID_04E8&PID_205C

[USB_Install]
Include=winusb.inf
Needs=WINUSB.NT

[USB_Install.Services]
Include=winusb.inf
AddService=WinUSB,0x00000002,WinUSB_ServiceInstall

[Strings]
ManufacturerName="Samsung"
DeviceName="Samsung TV Camera VG-STC3000"
```

***

### CLI Interface

```
samsung-fwloader.exe [OPTIONS]

Options:
  -f, --firmware <path>   Path to firmware file (default: FalconFW.bin in same dir)
  -v, --verbose           Verbose output
  -l, --list              List connected Samsung TV cameras
  -h, --help              Show help

Examples:
  samsung-fwloader.exe
  samsung-fwloader.exe -f C:\firmware\FalconFW.bin -v
  samsung-fwloader.exe --list
```

***

### Auto-Load on USB Insert (Optional)

#### Method 1: Windows Service

A lightweight Windows Service monitors WM_DEVICECHANGE events and triggers the loader automatically.

```c
// Pseudocode
RegisterDeviceNotification(hwnd, &notificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
// On DBT_DEVICEARRIVAL: check VID:PID, call fwloader_load()
```

#### Method 2: Task Scheduler

Register a task triggered on USB device insertion:
- Trigger: Event log — System, Source: Microsoft-Windows-UserPnp, Event ID 20001
- Action: Run `samsung-fwloader.exe`

***

### UVC Behavior After Firmware Load

After successful firmware injection:
- The camera re-enumerates as `USB\VID_04E8&PID_205D` (or similar PID change)
- Windows recognizes it as a standard UVC camera
- No additional driver needed — Windows ships with `usbvideo.sys` (UVC driver)
- Accessible from any app: OBS, Zoom, Teams, Camera app, etc.

***

### Error Handling

| Error              | Message                                          | Recovery                        |
|--------------------|--------------------------------------------------|---------------------------------|
| Device not found   | "Camera not detected. Ensure it is plugged in." | Re-plug, check driver binding   |
| WinUSB not bound   | "Install WinUSB driver via Zadig first."         | Link to Zadig + instructions    |
| Firmware missing   | "FalconFW.bin not found. See INSTALL.md."        | Link to extraction guide        |
| Transfer timeout   | "USB transfer timed out."                        | Re-plug and retry               |
| Transfer error     | "USB error: <libusb error string>"               | Run with --verbose for details  |

***

### Testing Checklist

- [ ] Camera detected via `--list` before firmware load
- [ ] Firmware loads without error on first plug
- [ ] Camera re-enumerates as UVC after load
- [ ] Camera visible in Device Manager as "USB Video Device"
- [ ] Camera accessible in Windows Camera app
- [ ] Auto-load works on second plug (service/task)
- [ ] Error messages are clear and actionable
- [ ] Works on Windows 10 x64 and Windows 11 x64

***

### References

- Linux reference implementation: https://github.com/ondrej-zary/samsung-tvcam-fwloader
- libusb Windows guide: https://github.com/libusb/libusb/wiki/Windows
- WinUSB INF: https://learn.microsoft.com/en-us/windows-hardware/drivers/usbcon/winusb-installation
- Zadig: https://zadig.akeo.ie
- UVC driver (built-in): https://learn.microsoft.com/en-us/windows-hardware/drivers/stream/usb-video-class-driver-overview

***

### Notes for AI Agent

- Port the core logic from `fwloader.c` in the Linux repo verbatim, replacing `libusb` Linux calls with `libusb` Windows API (identical API, different linking)
- The USB protocol (AIT/MAX) is identical on Windows — only the OS-level USB access layer changes
- Start with a minimal CLI (`main.c` + `fwloader.c`) before adding tray/service
- Always use `x86_64-w64-mingw32-gcc` as the compiler, never native `gcc`
- Always link `-lsetupapi` — required by libusb on Windows for USB device enumeration
- Use `-static-libgcc` to avoid runtime dependency on `libgcc_s_seh-1.dll` on the target Windows machine
- Test with `--list` first to confirm libusb can enumerate the device before attempting firmware load
- For compilation debugging use `-Wall -Wextra -v`
- Prioritize VG-STC3000 (AIT, 04e8:205c) — other models are stretch goals
