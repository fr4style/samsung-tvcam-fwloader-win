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

usb_device_t *usb_open_device(uint16_t vid, uint16_t pid)
{
    usb_device_t *dev = malloc(sizeof(usb_device_t));
    if (!dev) {
        fprintf(stderr, "error: out of memory\n");
        return NULL;
    }

    if (libusb_init(&dev->ctx) < 0) {
        fprintf(stderr, "error: libusb_init failed\n");
        free(dev);
        return NULL;
    }

    dev->handle = libusb_open_device_with_vid_pid(dev->ctx, vid, pid);
    if (!dev->handle) {
        fprintf(stderr, "error: device %04x:%04x not found\n", vid, pid);
        libusb_exit(dev->ctx);
        free(dev);
        return NULL;
    }

    libusb_set_auto_detach_kernel_driver(dev->handle, 1);

    if (libusb_claim_interface(dev->handle, 0) < 0) {
        fprintf(stderr, "error: failed to claim interface 0\n");
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
        if (libusb_get_device_descriptor(list[i], &desc) < 0)
            continue;
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
