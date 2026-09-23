#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "sbc_packet.h"

static void round_trip(const uint8_t *input, size_t length) {
  uint8_t frame[SBC_PACKET_MAX_LEN];
  uint8_t output[SBC_PACKET_MAX_LEN];
  size_t frame_len = 0;
  size_t output_len = 0;
  assert(sbc_packet_encode(input, length, frame, sizeof(frame), &frame_len));
  assert(frame_len <= sizeof(frame) && frame[frame_len - 1] == 0);
  assert(sbc_packet_decode(frame, frame_len, output, sizeof(output), &output_len));
  assert(output_len == length && memcmp(input, output, length) == 0);

  assert(!sbc_packet_decode(frame, frame_len - 1, output, sizeof(output), &output_len));
  if (length > 0) {
    assert(!sbc_packet_decode(frame, frame_len, output, length - 1, &output_len));
    frame[1] ^= 0x20;
    assert(!sbc_packet_decode(frame, frame_len, output, sizeof(output), &output_len));
  }
}

int main(void) {
  const uint8_t empty[1] = {0};
  const uint8_t mixed[] = {0, 1, 2, 0, 0xff, 0, 42};
  uint8_t maximum[58];
  for (size_t i = 0; i < sizeof(maximum); ++i) maximum[i] = (uint8_t)(i + 1);
  round_trip(empty, 0);
  round_trip(mixed, sizeof(mixed));
  round_trip(maximum, sizeof(maximum));
  for (size_t length = 1; length <= sizeof(maximum); ++length) round_trip(maximum, length);

  uint8_t frame[SBC_PACKET_MAX_LEN];
  size_t frame_len = 0;
  assert(!sbc_packet_encode(maximum, sizeof(maximum), frame, 4, &frame_len));
  uint8_t oversized[59] = {0};
  assert(!sbc_packet_encode(oversized, sizeof(oversized), frame, sizeof(frame), &frame_len));

  uint32_t seed = 0x12345678;
  uint8_t guarded[66];
  for (size_t iteration = 0; iteration < 10000; ++iteration) {
    for (size_t i = 0; i < sizeof(frame); ++i) {
      seed = seed * 1664525u + 1013904223u;
      frame[i] = (uint8_t)(seed >> 24);
    }
    guarded[0] = 0xa5;
    guarded[65] = 0x5a;
    size_t decoded_len = 0;
    sbc_packet_decode(frame, iteration % (sizeof(frame) + 1), guarded + 1, 64, &decoded_len);
    assert(guarded[0] == 0xa5 && guarded[65] == 0x5a);
  }
  puts("sbc_packet_test: passed");
  return 0;
}
