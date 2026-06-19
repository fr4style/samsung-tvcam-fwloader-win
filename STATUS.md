# Project Status

## Modules
- [x] 00_planning         (produce workspace/plan.md from spec)
- [x] 01_usb_layer        (USB/HID communication with the camera)
- [x] 02_firmware_parser  (binary firmware file parsing)
- [x] 03_state_machine    (flash logic: INIT→FLASH→VERIFY)
- [x] 04_cli_interface    (command-line interface) ✅ validated
- [x] 05_packaging        (PyInstaller for Windows .exe) ✅ validated
- [ ] 06_tests            (unit tests with USB mock)

## Next step
06_tests: unit tests for fw_load() and mock USB AIT protocol sequence (build/run on Linux)

## Session notes
(Claude will append notes here during work)
