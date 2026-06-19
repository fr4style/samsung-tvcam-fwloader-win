#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../firmware_parser/firmware_parser.c"

static void die(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

static void check(int cond, const char *expr, int line)
{
    if (!cond) {
        fprintf(stderr, "test_firmware_parser failed at line %d: %s\n", line, expr);
        exit(EXIT_FAILURE);
    }
}

#define CHECK(expr) check((expr), #expr, __LINE__)

static void write_all(int fd, const uint8_t *buf, size_t len)
{
    while (len > 0) {
        ssize_t n = write(fd, buf, len);
        if (n < 0)
            die("write");
        buf += (size_t)n;
        len -= (size_t)n;
    }
}

static void test_fw_load_success(void)
{
    const uint8_t expected[] = {
        0x00, 0x11, 0x22, 0x33,
        0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xAA, 0xBB,
        0xCC, 0xDD, 0xEE, 0xFF,
    };
    char path[] = "/tmp/fwloader-fw-XXXXXX";
    int fd = mkstemp(path);
    CHECK(fd >= 0);
    write_all(fd, expected, sizeof(expected));
    CHECK(close(fd) == 0);

    uint8_t *buf_out = NULL;
    size_t size_out = 0;
    int ret = fw_load(path, &buf_out, &size_out);

    CHECK(ret == 0);
    CHECK(buf_out != NULL);
    CHECK(size_out == sizeof(expected));
    CHECK(memcmp(buf_out, expected, sizeof(expected)) == 0);

    fw_free(buf_out);
    CHECK(unlink(path) == 0);
}

static void test_fw_load_missing(void)
{
    char path[128];
    snprintf(path, sizeof(path), "/tmp/fwloader-missing-%ld.bin", (long)getpid());
    unlink(path);

    uint8_t *buf_out = (uint8_t *)0x1;
    size_t size_out = 1234;
    int ret = fw_load(path, &buf_out, &size_out);

    CHECK(ret != 0);
}

int main(void)
{
    test_fw_load_success();
    test_fw_load_missing();
    puts("test_firmware_parser: ok");
    return 0;
}
