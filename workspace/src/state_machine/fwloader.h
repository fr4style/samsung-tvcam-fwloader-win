#ifndef FWLOADER_H
#define FWLOADER_H

#include <stddef.h>
#include <stdint.h>

#include "../usb_layer/usb_win.h"

#define AIT_TIMEOUT_MS 1000

int fwloader_run(usb_device_t *dev, uint8_t *fw_buf, size_t fw_size, int verbose);

#endif /* FWLOADER_H */
