#include "usb_win.h"
#include <stdio.h>
#include <stdlib.h>

libusb_device_handle *usb_open_device(uint16_t vid, uint16_t pid)
{
    libusb_context *ctx = NULL;
    if (libusb_init(&ctx) < 0) {
        fprintf(stderr, "error: libusb_init failed\n");
        return NULL;
    }

    libusb_device_handle *handle = libusb_open_device_with_vid_pid(ctx, vid, pid);
    if (!handle) {
        fprintf(stderr, "error: device %04x:%04x not found\n", vid, pid);
        libusb_exit(ctx);
        return NULL;
    }

    libusb_set_auto_detach_kernel_driver(handle, 1);

    if (libusb_claim_interface(handle, 0) < 0) {
        fprintf(stderr, "error: failed to claim interface 0\n");
        libusb_close(handle);
        libusb_exit(ctx);
        return NULL;
    }

    return handle;
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

    printf("USB devices:\n");
    for (ssize_t i = 0; i < count; i++) {
        struct libusb_device_descriptor desc;
        if (libusb_get_device_descriptor(list[i], &desc) < 0)
            continue;
        printf("  %04x:%04x  bus %d port %d\n",
               desc.idVendor, desc.idProduct,
               libusb_get_bus_number(list[i]),
               libusb_get_port_number(list[i]));
    }

    libusb_free_device_list(list, 1);
    libusb_exit(ctx);
}

void usb_close_device(libusb_device_handle *handle)
{
    if (!handle)
        return;
    libusb_release_interface(handle, 0);
    libusb_close(handle);
}

int usb_bulk_transfer_out(libusb_device_handle *handle, uint8_t endpoint,
                          uint8_t *data, int length, unsigned int timeout_ms)
{
    int transferred = 0;
    int ret = libusb_bulk_transfer(handle, endpoint, data, length,
                                   &transferred, timeout_ms);
    if (ret < 0) {
        fprintf(stderr, "error: bulk transfer failed: %s\n", libusb_error_name(ret));
        return ret;
    }
    return transferred;
}

int usb_control_transfer(libusb_device_handle *handle, uint8_t bmRequestType,
                         uint8_t bRequest, uint16_t wValue, uint16_t wIndex,
                         uint8_t *data, uint16_t wLength, unsigned int timeout_ms)
{
    int ret = libusb_control_transfer(handle, bmRequestType, bRequest,
                                      wValue, wIndex, data, wLength, timeout_ms);
    if (ret < 0) {
        fprintf(stderr, "error: control transfer failed: %s\n", libusb_error_name(ret));
    }
    return ret;
}
