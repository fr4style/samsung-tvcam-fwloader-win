#ifndef FIRMWARE_PARSER_H
#define FIRMWARE_PARSER_H

#include <stdint.h>
#include <stddef.h>

/* Load firmware file into a heap buffer.
 * Returns 0 on success, -1 on error (error printed to stderr).
 * Caller must free *buf_out with fw_free(). */
int fw_load(const char *path, uint8_t **buf_out, size_t *size_out);

/* Free a buffer returned by fw_load(). */
void fw_free(uint8_t *buf);

#endif /* FIRMWARE_PARSER_H */
