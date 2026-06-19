#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "../firmware_parser/firmware_parser.h"
#include "../state_machine/fwloader.h"
#include "../usb_layer/usb_win.h"

#define TARGET_VID 0x04e8
#define TARGET_PID 0x205c

static void print_usage(const char *progname)
{
    const char *name = progname && progname[0] ? progname : "samsung-fwloader.exe";

    printf("Usage: %s [OPTIONS]\n", name);
    printf("\n");
    printf("Options:\n");
    printf("  -f, --firmware <path>   Path to firmware file (default: FalconFW.bin in same dir)\n");
    printf("  -v, --verbose           Verbose output\n");
    printf("  -l, --list              List connected Samsung TV cameras\n");
    printf("  -h, --help              Show help\n");
}

static int build_default_firmware_path(char *out, size_t out_sz)
{
    char exe_path[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, exe_path, (DWORD)sizeof(exe_path));
    int written;

    if (len == 0 || len >= sizeof(exe_path))
        return -1;

    exe_path[len] = '\0';

    {
        char *last_backslash = strrchr(exe_path, '\\');
        char *last_slash = strrchr(exe_path, '/');
        char *sep = last_backslash;

        if (!sep || (last_slash && last_slash > sep))
            sep = last_slash;

        if (sep)
            sep[1] = '\0';
        else
            exe_path[0] = '\0';
    }

    written = snprintf(out, out_sz, "%sFalconFW.bin", exe_path);
    if (written < 0 || (size_t)written >= out_sz)
        return -1;

    return 0;
}

int main(int argc, char **argv)
{
    const char *firmware_path = NULL;
    char default_firmware_path[MAX_PATH] = { 0 };
    uint8_t *fw_buf = NULL;
    size_t fw_size = 0;
    usb_device_t *dev = NULL;
    int usb_open_error = 0;
    int verbose = 0;
    int list_only = 0;
    int using_default_firmware_path = 0;

    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];

        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        }

        if (strcmp(arg, "-v") == 0 || strcmp(arg, "--verbose") == 0) {
            verbose = 1;
            continue;
        }

        if (strcmp(arg, "-l") == 0 || strcmp(arg, "--list") == 0) {
            list_only = 1;
            continue;
        }

        if (strcmp(arg, "-f") == 0 || strcmp(arg, "--firmware") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "error: %s requires a firmware path\n", arg);
                print_usage(argv[0]);
                return 1;
            }
            firmware_path = argv[++i];
            continue;
        }

        if (strncmp(arg, "--firmware=", 11) == 0) {
            firmware_path = arg + 11;
            if (firmware_path[0] == '\0') {
                fprintf(stderr, "error: --firmware requires a firmware path\n");
                return 1;
            }
            continue;
        }

        fprintf(stderr, "error: unknown option '%s'\n", arg);
        print_usage(argv[0]);
        return 1;
    }

    if (list_only) {
        usb_list_devices();
        return 0;
    }

    if (!firmware_path) {
        if (build_default_firmware_path(default_firmware_path, sizeof(default_firmware_path)) != 0) {
            fprintf(stderr, "error: default firmware path is too long\n");
            return 1;
        }
        firmware_path = default_firmware_path;
        using_default_firmware_path = 1;
    }

    if (fw_load(firmware_path, &fw_buf, &fw_size) != 0) {
        if (using_default_firmware_path)
            fprintf(stderr, "FalconFW.bin not found. See INSTALL.md.\n");
        return 1;
    }

    dev = usb_open_device(TARGET_VID, TARGET_PID, &usb_open_error);
    if (!dev) {
        if (usb_open_error == LIBUSB_ERROR_ACCESS) {
            fprintf(stderr, "Camera not detected. Install WinUSB driver via Zadig first.\n");
        } else {
            fprintf(stderr, "Camera not detected. Is the camera connected and in firmware-update mode?\n");
        }
        fw_free(fw_buf);
        return 1;
    }

    if (verbose)
        printf("firmware: %s (%zu bytes)\n", firmware_path, fw_size);

    if (fwloader_run(dev, fw_buf, fw_size, verbose) == 0) {
        usb_close_device(dev);
        fw_free(fw_buf);
        printf("Firmware upload completed successfully.\n");
        return 0;
    }

    usb_close_device(dev);
    fw_free(fw_buf);
    fprintf(stderr, "Firmware upload failed.\n");
    return 1;
}
