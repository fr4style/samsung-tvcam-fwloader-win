#include "firmware_parser.h"

#include <stdio.h>
#include <stdlib.h>

int fw_load(const char *path, uint8_t **buf_out, size_t *size_out)
{
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "fw_load: cannot open '%s'\n", path);
        return -1;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fprintf(stderr, "fw_load: seek failed on '%s'\n", path);
        fclose(f);
        return -1;
    }

    long sz = ftell(f);
    if (sz <= 0) {
        fprintf(stderr, "fw_load: '%s' is empty or unreadable\n", path);
        fclose(f);
        return -1;
    }
    rewind(f);

    uint8_t *buf = malloc((size_t)sz);
    if (!buf) {
        fprintf(stderr, "fw_load: out of memory (%ld bytes)\n", sz);
        fclose(f);
        return -1;
    }

    if (fread(buf, 1, (size_t)sz, f) != (size_t)sz) {
        fprintf(stderr, "fw_load: read error on '%s'\n", path);
        free(buf);
        fclose(f);
        return -1;
    }

    fclose(f);
    *buf_out  = buf;
    *size_out = (size_t)sz;
    return 0;
}

void fw_free(uint8_t *buf)
{
    free(buf);
}
