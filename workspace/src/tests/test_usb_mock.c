#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USB_WIN_H

typedef struct usb_device_t {
    void *ctx;
    void *handle;
} usb_device_t;

usb_device_t *usb_open_device(uint16_t vid, uint16_t pid, int *error_out);
void usb_close_device(usb_device_t *dev);
int usb_bulk_transfer_out(usb_device_t *dev, uint8_t endpoint, uint8_t *data,
                          int length, unsigned int timeout_ms);
int usb_control_transfer(usb_device_t *dev, uint8_t bmRequestType,
                         uint8_t bRequest, uint16_t wValue, uint16_t wIndex,
                         uint8_t *data, uint16_t wLength, unsigned int timeout_ms);

#include "../state_machine/fwloader.c"

enum log_kind {
    LOG_OPEN,
    LOG_CLOSE,
    LOG_BULK,
    LOG_CONTROL,
};

struct log_entry {
    enum log_kind kind;
    uint16_t a;
    uint16_t b;
    uint16_t c;
    uint16_t d;
    unsigned int timeout_ms;
    int length;
    uint8_t payload[512];
};

static struct log_entry g_log[16];
static size_t g_log_count;

static void failf(const char *msg, int line)
{
    fprintf(stderr, "test_usb_mock failed at line %d: %s\n", line, msg);
    exit(EXIT_FAILURE);
}

#define CHECK(expr) do { if (!(expr)) failf(#expr, __LINE__); } while (0)

static void log_reset(void)
{
    memset(g_log, 0, sizeof(g_log));
    g_log_count = 0;
}

static struct log_entry *log_next(enum log_kind kind)
{
    CHECK(g_log_count < (sizeof(g_log) / sizeof(g_log[0])));
    struct log_entry *entry = &g_log[g_log_count++];
    memset(entry, 0, sizeof(*entry));
    entry->kind = kind;
    return entry;
}

usb_device_t *usb_open_device(uint16_t vid, uint16_t pid, int *error_out)
{
    static usb_device_t dev = { 0 };
    struct log_entry *entry = log_next(LOG_OPEN);
    entry->a = vid;
    entry->b = pid;
    if (error_out)
        *error_out = 0;
    dev.handle = (void *)0x1;
    return &dev;
}

void usb_close_device(usb_device_t *dev)
{
    struct log_entry *entry = log_next(LOG_CLOSE);
    entry->a = (uint16_t)(uintptr_t)dev;
}

int usb_bulk_transfer_out(usb_device_t *dev, uint8_t endpoint, uint8_t *data,
                          int length, unsigned int timeout_ms)
{
    (void)dev;
    struct log_entry *entry = log_next(LOG_BULK);
    entry->a = endpoint;
    entry->timeout_ms = timeout_ms;
    entry->length = length;
    if (length > 0) {
        CHECK(length <= (int)sizeof(entry->payload));
        memcpy(entry->payload, data, (size_t)length);
    }
    return length;
}

int usb_control_transfer(usb_device_t *dev, uint8_t bmRequestType,
                         uint8_t bRequest, uint16_t wValue, uint16_t wIndex,
                         uint8_t *data, uint16_t wLength, unsigned int timeout_ms)
{
    (void)dev;
    (void)data;
    struct log_entry *entry = log_next(LOG_CONTROL);
    entry->a = bmRequestType;
    entry->b = bRequest;
    entry->c = wValue;
    entry->d = wIndex;
    entry->length = (int)wLength;
    entry->timeout_ms = timeout_ms;
    return 0;
}

static void expect_control(size_t index, uint8_t request)
{
    const struct log_entry *entry = &g_log[index];
    CHECK(entry->kind == LOG_CONTROL);
    CHECK(entry->a == 0x40);
    CHECK(entry->b == request);
    CHECK(entry->c == 0);
    CHECK(entry->d == 0);
    CHECK(entry->length == 0);
    CHECK(entry->timeout_ms == 1000);
}

static void expect_bulk(size_t index, const uint8_t *expected, int length)
{
    const struct log_entry *entry = &g_log[index];
    CHECK(entry->kind == LOG_BULK);
    CHECK(entry->a == 0x01);
    CHECK(entry->length == length);
    CHECK(entry->timeout_ms == 5000);
    CHECK(memcmp(entry->payload, expected, (size_t)length) == 0);
}

int main(void)
{
    uint8_t firmware[1025];
    for (size_t i = 0; i < sizeof(firmware); i++)
        firmware[i] = (uint8_t)(i & 0xFF);

    usb_device_t dev = {
        .ctx = NULL,
        .handle = (void *)0x1,
    };

    log_reset();
    int ret = fwloader_run(&dev, firmware, sizeof(firmware), 0);

    CHECK(ret == 0);
    CHECK(g_log_count == 5);

    expect_control(0, 0xF0);
    expect_bulk(1, &firmware[0], 512);
    expect_bulk(2, &firmware[512], 512);
    expect_bulk(3, &firmware[1024], 1);
    expect_control(4, 0xF1);

    puts("test_usb_mock: ok");
    return 0;
}
