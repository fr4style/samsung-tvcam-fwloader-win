# samsung-tvcam-fwloader-win

Windows CLI tool that loads firmware into Samsung VG-STC3000 TV cameras via USB.
Cross-compiled on Linux with MinGW; no runtime dependencies beyond `libusb-1.0.dll`.

Primary target: **VG-STC3000** (`04e8:205c`, AIT chip, `FalconFW.bin`).

## Build requirements

- `x86_64-w64-mingw32-gcc` — `sudo apt install gcc-mingw-w64-x86-64`
- C11
- libusb-1.0 Windows binaries (headers in `include/`, lib + DLL in `lib/`)

Fetch libusb (one-time):

```
wget https://github.com/libusb/libusb/releases/latest/download/libusb-1.0.27-binaries.7z
7z x libusb-1.0.27-binaries.7z -o /tmp/libusb-win/
cp /tmp/libusb-win/libusb-1.0.27-binaries/include/libusb-1.0/libusb.h include/
cp "/tmp/libusb-win/libusb-1.0.27-binaries/MinGW64/static/libusb-1.0.a" lib/
cp "/tmp/libusb-win/libusb-1.0.27-binaries/MinGW64/dll/libusb-1.0.dll" lib/
```

## Build

```
# Make
make -f workspace/src/packaging/Makefile

# Distribution bundle (exe + dll + INSTALL.md → dist/)
make -f workspace/src/packaging/Makefile dist

# CMake
cmake -S workspace/src/packaging -B build \
      -DCMAKE_TOOLCHAIN_FILE=workspace/src/packaging/cmake/mingw-w64-x86_64.cmake
cmake --build build
```

## Usage

```
samsung-fwloader.exe [OPTIONS]

  -f, --firmware <path>   Firmware file (default: FalconFW.bin next to the exe)
  -l, --list              List connected Samsung TV cameras
  -v, --verbose           Verbose output
  -h, --help              Show help
```

Examples:

```
samsung-fwloader.exe --list
samsung-fwloader.exe
samsung-fwloader.exe --firmware C:\path\to\FalconFW.bin
```

## Windows setup

Run **`install.bat`** as Administrator from the `dist/` folder — it installs the
WinUSB driver via `pnputil`, copies the exe and DLL to
`C:\Program Files\SamsungFWLoader`, and updates the system PATH.

See **[INSTALL.md](INSTALL.md)** for full setup details, obtaining `FalconFW.bin`,
manual (Zadig) driver install, and supported camera models.
