#include "usb_win.h"
#include <stdio.h>
#include <stdlib.h>

/* Samsung camera VID:PIDs from spec */
#define SAMSUNG_VID  0x04e8
static const uint16_t samsung_pids[] = { 0x205c, 0x2061, 0x2059 };
#define SAMSUNG_PID_COUNT (sizeof(samsung_pids) / sizeof(samsung_pids[0]))

static int is_samsung_camera(uint16_t vid, uint16_t pid)
{
    if (vid != SAMSUNG_VID)
        return 0;
    for (size_t i = 0; i < SAMSUNG_PID_COUNT; i++)
        if (samsung_pids[i] == pid)
            return 1;
    return 0;
}

usb_device_t *usb_open_device(uint16_t vid, uint16_t pid, int *error_out)
{
    usb_device_t *dev = malloc(sizeof(usb_device_t));
    if (!dev) {
        fprintf(stderr, "error: out of memory\n");
        if (error_out)
            *error_out = LIBUSB_ERROR_NO_MEM;
        return NULL;
    }

    if (libusb_init(&dev->ctx) < 0) {
        fprintf(stderr, "error: libusb_init failed\n");
        if (error_out)
            *error_out = LIBUSB_ERROR_OTHER;
        free(dev);
        return NULL;
    }

    dev->handle = NULL;

    libusb_device **list;
    ssize_t count = libusb_get_device_list(dev->ctx, &list);
    if (count < 0) {
        fprintf(stderr, "error: libusb_get_device_list failed\n");
        if (error_out)
            *error_out = (int)count;
        libusb_exit(dev->ctx);
        free(dev);
        return NULL;
    }

    int open_ret = LIBUSB_ERROR_NOT_FOUND;
    for (ssize_t i = 0; i < count; i++) {
        struct libusb_device_descriptor desc;
        int ret = libusb_get_device_descriptor(list[i], &desc);
        if (ret < 0)
            continue;
        if (desc.idVendor != vid || desc.idProduct != pid)
            continue;

        ret = libusb_open(list[i], &dev->handle);
        if (ret == 0)
            break;
        if (ret == LIBUSB_ERROR_ACCESS || open_ret != LIBUSB_ERROR_ACCESS)
            open_ret = ret;
    }

    libusb_free_device_list(list, 1);

    if (!dev->handle) {
        if (open_ret == LIBUSB_ERROR_ACCESS) {
            fprintf(stderr, "error: access denied opening device %04x:%04x\n", vid, pid);
        } else {
            fprintf(stderr, "error: device %04x:%04x not found\n", vid, pid);
        }
        if (error_out)
            *error_out = open_ret;
        libusb_exit(dev->ctx);
        free(dev);
        return NULL;
    }

    if (error_out)
        *error_out = 0;

    libusb_set_auto_detach_kernel_driver(dev->handle, 1);

    int ret = libusb_claim_interface(dev->handle, 0);
    if (ret < 0) {
        fprintf(stderr, "error: failed to claim interface 0: %s\n", libusb_error_name(ret));
        libusb_close(dev->handle);
        libusb_exit(dev->ctx);
        free(dev);
        return NULL;
    }

    return dev;
}

void usb_list_devices(void)
{
    libusb_context *ctx = NULL;
    if (libusb_init(&ctx) < 0) {
        fprintf(stderr, "error: libusb_init failed\n");
        return;
    }

    libusb_device **list;
    ssize_t count = libusb_get_device_list(ctx, &list);
    if (count < 0) {
        fprintf(stderr, "error: libusb_get_device_list failed\n");
        libusb_exit(ctx);
        return;
    }

    printf("Samsung TV cameras found:\n");
    int found = 0;
    for (ssize_t i = 0; i < count; i++) {
        struct libusb_device_descriptor desc;
        int ret = libusb_get_device_descriptor(list[i], &desc);
        if (ret < 0) {
            fprintf(stderr, "[warn] usb_list_devices: skipping device: %s\n", libusb_error_name(ret));
            continue;
        }
        if (!is_samsung_camera(desc.idVendor, desc.idProduct))
            continue;
        printf("  %04x:%04x  bus %d port %d\n",
               desc.idVendor, desc.idProduct,
               libusb_get_bus_number(list[i]),
               libusb_get_port_number(list[i]));
        found++;
    }
    if (!found)
        printf("  (none)\n");

    libusb_free_device_list(list, 1);
    libusb_exit(ctx);
}

void usb_close_device(usb_device_t *dev)
{
    if (!dev)
        return;
    libusb_release_interface(dev->handle, 0);
    libusb_close(dev->handle);
    libusb_exit(dev->ctx);
    free(dev);
}

int usb_bulk_transfer_out(usb_device_t *dev, uint8_t endpoint,
                          uint8_t *data, int length, unsigned int timeout_ms)
{
    int transferred = 0;
    int ret = libusb_bulk_transfer(dev->handle, endpoint, data, length,
                                   &transferred, timeout_ms);
    if (ret < 0) {
        fprintf(stderr, "error: bulk transfer failed: %s\n", libusb_error_name(ret));
        return ret;
    }
    return transferred;
}

int usb_control_transfer(usb_device_t *dev, uint8_t bmRequestType,
                         uint8_t bRequest, uint16_t wValue, uint16_t wIndex,
                         uint8_t *data, uint16_t wLength, unsigned int timeout_ms)
{
    int ret = libusb_control_transfer(dev->handle, bmRequestType, bRequest,
                                      wValue, wIndex, data, wLength, timeout_ms);
    if (ret < 0) {
        fprintf(stderr, "error: control transfer failed: %s\n", libusb_error_name(ret));
    }
    return ret;
}
