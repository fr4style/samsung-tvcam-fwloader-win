#include "fwloader.h"

#include <stdio.h>

#define FW_CHUNK_SIZE 512
#define AIT_REQ_TYPE   0x40
#define AIT_REQ_INIT   0xF0
#define AIT_REQ_BOOT   0xF1
#define AIT_BULK_TIMEOUT_MS 5000

int fwloader_run(usb_device_t *dev, uint8_t *fw_buf, size_t fw_size, int verbose)
{
    if (!dev || !dev->handle || (!fw_buf && fw_size != 0)) {
        fprintf(stderr, "error: invalid fwloader arguments\n");
        return -1;
    }

    if (fw_size == 0) {
        fprintf(stderr, "fwloader: firmware buffer is empty\n");
        return -1;
    }

    if (verbose)
        printf("fwloader: INIT\n");

    int ret = usb_control_transfer(dev, AIT_REQ_TYPE, AIT_REQ_INIT,
                                   0, 0, NULL, 0, AIT_TIMEOUT_MS);
    if (ret < 0)
        return -1;

    if (verbose)
        printf("fwloader: FLASH (%zu bytes)\n", fw_size);

    size_t offset = 0;
    while (offset < fw_size) {
        size_t remaining = fw_size - offset;
        size_t chunk_len = remaining < FW_CHUNK_SIZE ? remaining : FW_CHUNK_SIZE;

        ret = usb_bulk_transfer_out(dev, 0x01, fw_buf + offset,
                                    (int)chunk_len, AIT_BULK_TIMEOUT_MS);
        if (ret < 0)
            return -1;
        if ((size_t)ret != chunk_len) {
            fprintf(stderr, "error: short bulk transfer (%d/%zu)\n", ret, chunk_len);
            return -1;
        }

        offset += chunk_len;
    }

    if (verbose)
        printf("fwloader: BOOT\n");

    ret = usb_control_transfer(dev, AIT_REQ_TYPE, AIT_REQ_BOOT,
                               0, 0, NULL, 0, AIT_TIMEOUT_MS);
    if (ret < 0)
        return -1;

    return 0;
}
