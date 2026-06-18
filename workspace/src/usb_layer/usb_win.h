#ifndef USB_WIN_H
#define USB_WIN_H

#include <stdint.h>
#include <libusb-1.0/libusb.h>

typedef struct {
    libusb_context       *ctx;
    libusb_device_handle *handle;
} usb_device_t;

usb_device_t *usb_open_device(uint16_t vid, uint16_t pid);
void usb_list_devices(void);
void usb_close_device(usb_device_t *dev);
int usb_bulk_transfer_out(usb_device_t *dev, uint8_t endpoint,
                          uint8_t *data, int length, unsigned int timeout_ms);
int usb_control_transfer(usb_device_t *dev, uint8_t bmRequestType,
                         uint8_t bRequest, uint16_t wValue, uint16_t wIndex,
                         uint8_t *data, uint16_t wLength, unsigned int timeout_ms);

#endif /* USB_WIN_H */
