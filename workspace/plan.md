# Implementation Plan — Samsung TV Camera Firmware Loader (Windows)

Generated from spec.md on 2026-06-18.

---

## Goal

Produce `samsung-fwloader.exe` — a Windows CLI tool that loads firmware into Samsung VG-STC3000 TV cameras via USB, cross-compiled on Linux using MinGW.

Primary target: VG-STC3000 (04e8:205c, AIT chip, FalconFW.bin).

---

## Module Breakdown

### 01 — usb_layer (`workspace/src/usb_layer/`)

Files: `usb_win.c`, `usb_win.h`

- Wrap libusb-1.0 for Windows (same API as Linux, different link flags)
- Functions:
  - `usb_open_device(uint16_t vid, uint16_t pid, int *error_out)` → `usb_device_t *` (sets `*error_out` to `LIBUSB_ERROR_ACCESS` on permission failure)
  - `usb_list_devices()` — enumerate and print matching cameras
  - `usb_close_device(usb_device_t *dev)`
  - `usb_bulk_transfer_out(usb_device_t *dev, uint8_t endpoint, uint8_t *data, int length, unsigned int timeout_ms)` → int
  - `usb_control_transfer(usb_device_t *dev, uint8_t bmRequestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint8_t *data, uint16_t wLength, unsigned int timeout_ms)` → int
- Link flags: `-lusb-1.0 -lsetupapi -static-libgcc`
- Compiler: `x86_64-w64-mingw32-gcc`

### 02 — firmware_parser (`workspace/src/firmware_parser/`)

Files: `firmware_parser.c`, `firmware_parser.h`

- Read firmware binary file into a heap buffer
- Validate that file exists and is non-empty
- Functions:
  - `fw_load(const char *path, uint8_t **buf_out, size_t *size_out)` → int (0=ok)
  - `fw_free(uint8_t *buf)`
- No AIT/MAX parsing needed — firmware is sent as raw binary

### 03 — state_machine (`workspace/src/state_machine/`)

Files: `fwloader.c`, `fwloader.h`

- Implement the AIT upload protocol (primary target):
  1. Send vendor control request 0xF0 (firmware download init) to EP0
  2. Send firmware in 512-byte chunks via bulk OUT to EP 0x01
  3. Send vendor control request 0xF1 (boot trigger) to EP0
- States: INIT → FLASH → VERIFY (re-enumerate check optional)
- Functions:
  - `fwloader_run(usb_device_t *dev, uint8_t *fw_buf, size_t fw_size, int verbose)` → int
- Reference: https://github.com/ondrej-zary/samsung-tvcam-fwloader/blob/master/fwloader.c

AIT control transfer parameters:
```
bmRequestType = 0x40  (host-to-device | vendor | device)
bRequest      = 0xF0  (init) / 0xF1 (boot)
wValue        = 0x0000
wIndex        = 0x0000
wLength       = 0
```

### 04 — cli_interface (`workspace/src/cli_interface/`)

Files: `main.c`

- Parse arguments: `-f/--firmware`, `-v/--verbose`, `-l/--list`, `-h/--help`
- Default firmware path: `FalconFW.bin` in the same directory as the exe
- Flow:
  1. Parse args
  2. If `--list`: call `usb_list_devices()` and exit
  3. Load firmware via `fw_load()`
  4. Open USB device via `usb_open_device(0x04e8, 0x205c)`
  5. Call `fwloader_run()`
  6. Close device, print result
- Clear error messages per spec error table

### 05 — packaging (`workspace/src/packaging/`)

Files: `Makefile`, `CMakeLists.txt`, `cmake/mingw-w64-x86_64.cmake`, `drivers/samsung-tvcam.inf`

- Makefile for manual MinGW build
- CMake toolchain for cross-compilation
- INF file binding WinUSB to VID_04E8&PID_205C
- `dist/` target: copies `.exe`, `libusb-1.0.dll`, `INSTALL.md`

### 06 — tests (`workspace/src/tests/`)

Files: `test_firmware_parser.c`, `test_usb_mock.c`

- Unit test for `fw_load()` with a small synthetic binary
- Mock USB layer to test the AIT protocol sequence without hardware
- Build and run on Linux (native gcc, not MinGW)

---

## Build Command (reference)

```bash
x86_64-w64-mingw32-gcc -o samsung-fwloader.exe \
    workspace/src/cli_interface/main.c \
    workspace/src/state_machine/fwloader.c \
    workspace/src/usb_layer/usb_win.c \
    workspace/src/firmware_parser/firmware_parser.c \
    -I./include -L./lib -lusb-1.0 -lsetupapi -static-libgcc \
    -Wall -Wextra
```

---

## Dependency Setup (one-time)

```bash
# Install cross-compiler
sudo apt install gcc-mingw-w64-x86-64

# Get libusb Windows binaries
wget https://github.com/libusb/libusb/releases/latest/download/libusb-1.0.27-binaries.7z
7z x libusb-1.0.27-binaries.7z -o /tmp/libusb-win/
cp /tmp/libusb-win/libusb-1.0.27-binaries/include/libusb-1.0/libusb.h include/
cp "/tmp/libusb-win/libusb-1.0.27-binaries/MinGW64/static/libusb-1.0.a" lib/
cp "/tmp/libusb-win/libusb-1.0.27-binaries/MinGW64/dll/libusb-1.0.dll" lib/
```

---

## Stretch Goals (after primary target works)

- VG-STC4000 support (same AIT protocol, different PID/firmware)
- VG-STC2000 support (MAX chip protocol)
- System tray app (`tray.c`)
- Windows Service (`service.c`)
- Auto-load via Task Scheduler
