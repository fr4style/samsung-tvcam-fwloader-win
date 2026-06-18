#ifndef FWLOADER_H
#define FWLOADER_H

#include <stddef.h>
#include <stdint.h>

#include "../usb_layer/usb_win.h"

int fwloader_run(usb_device_t *dev, uint8_t *fw_buf, size_t fw_size, int verbose);

#endif /* FWLOADER_H */
