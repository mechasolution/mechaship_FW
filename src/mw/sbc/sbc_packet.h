#ifndef MECHASHIP_SBC_PACKET_H_
#define MECHASHIP_SBC_PACKET_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SBC_PACKET_MAX_LEN 64

// Frames include the final zero delimiter. CRC32 is stored little-endian.
bool sbc_packet_encode(const uint8_t *payload, size_t payload_len,
                       uint8_t *frame, size_t frame_capacity, size_t *frame_len);
bool sbc_packet_decode(const uint8_t *frame, size_t frame_len,
                       uint8_t *payload, size_t payload_capacity, size_t *payload_len);

#endif
