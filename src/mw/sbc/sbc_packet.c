#include "sbc_packet.h"

#include <string.h>

#include "cobs/cobs.h"
#include "crc32/crc32.h"

bool sbc_packet_encode(const uint8_t *payload, size_t payload_len,
                       uint8_t *frame, size_t frame_capacity, size_t *frame_len) {
  if (payload == NULL || frame == NULL || frame_len == NULL ||
      payload_len > SBC_PACKET_MAX_LEN - 4 || frame_capacity < 2) return false;

  uint8_t data[SBC_PACKET_MAX_LEN];
  memcpy(data, payload, payload_len);
  uint32_t crc = crc32_calc(payload, payload_len);
  for (size_t i = 0; i < 4; ++i) data[payload_len + i] = (uint8_t)(crc >> (8 * i));

  cobs_encode_result result = cobs_encode(frame, frame_capacity - 1, data, payload_len + 4);
  if (result.status != COBS_ENCODE_OK || result.out_len >= frame_capacity ||
      result.out_len + 1 > SBC_PACKET_MAX_LEN) return false;
  frame[result.out_len] = 0;
  *frame_len = result.out_len + 1;
  return true;
}

bool sbc_packet_decode(const uint8_t *frame, size_t frame_len,
                       uint8_t *payload, size_t payload_capacity, size_t *payload_len) {
  if (frame == NULL || payload == NULL || payload_len == NULL || frame_len < 2 ||
      frame_len > SBC_PACKET_MAX_LEN || frame[frame_len - 1] != 0) return false;

  uint8_t data[SBC_PACKET_MAX_LEN];
  cobs_decode_result result = cobs_decode(data, sizeof(data), frame, frame_len - 1);
  if (result.status != COBS_DECODE_OK || result.out_len < 4) return false;

  size_t decoded_len = result.out_len - 4;
  if (decoded_len > payload_capacity) return false;
  uint32_t received_crc = 0;
  for (size_t i = 0; i < 4; ++i) received_crc |= (uint32_t)data[decoded_len + i] << (8 * i);
  if (crc32_calc(data, decoded_len) != received_crc) return false;

  memcpy(payload, data, decoded_len);
  *payload_len = decoded_len;
  return true;
}
