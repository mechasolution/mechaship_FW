#include <stdio.h>
#include <stdlib.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <stream_buffer.h>
#include <task.h>

#include <tusb.h>

#include <pb_decode.h>
#include <pb_encode.h>

#include "cobs/cobs.h" // https://github.com/cmcqueen/cobs-c
#include "crc32/crc32.h"
#include "nanopb/messages.pb.h"
#include "ringbuffer/ringbuffer.h" // https://github.com/AndersKaloer/Ring-Buffer

#include "tusb/tusb_.h"

#include "driver/log/log.h"

#include "sbc.h"

#define TAG "mw/sbc"

#define PACKET_MAX_LEN 64 // expected

#ifndef BUILD_EPOCH
#define BUILD_EPOCH 0
#endif

#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH "0000000"
#endif

typedef struct {
  enum {
    SND_TASK_COMMAND_NONE = 0x00,

    SND_TASK_COMMAND_BATTERY_INFO,
    SND_TASK_COMMAND_POWER_OFF,
    SND_TASK_COMMAND_DOMAIN_ID,

    SND_TASK_COMMAND_REQUEST_NETWORK_INFO,
    SND_TASK_COMMAND_REQUEST_PING,

    SND_TASK_COMMAND_SEND_HW_INFO,
  } command;

  union {
    struct { // SND_TASK_COMMAND_BATTERY_INFO
      float voltage;
      float percentage;
    } battery_info;

    struct {  // SND_TASK_COMMAND_POWER_OFF
      bool _; // no data needed
    } power_off;

    struct { // SND_TASK_COMMAND_DOMAIN_ID
      uint8_t id;
    } domain_id;

    struct {  // SND_TASK_COMMAND_SEND_HW_INFO
      bool _; // no data needed
    } send_hw_info;
  } data;
} snd_task_queue_data_t;
#define SND_TASK_QUEUE_LENGTH 4
#define SND_TASK_QUEUE_ITEM_SIZE sizeof(snd_task_queue_data_t)
static QueueHandle_t s_snd_task_queue_hd;
static uint8_t s_sbc_queue_buff[SND_TASK_QUEUE_LENGTH * SND_TASK_QUEUE_ITEM_SIZE];
static StaticQueue_t s_sbc_queue_struct;

#define SND_TASK_SIZE (4096)
static TaskHandle_t s_snd_task_hd = NULL;
static StackType_t s_snd_task_buff[SND_TASK_SIZE];
static StaticTask_t s_snd_task_struct;

#define RCV_STREAM_BUFFER_SIZE 64
static uint8_t rcv_stream_buffer_storage[RCV_STREAM_BUFFER_SIZE + 1];
static StaticStreamBuffer_t rcv_stream_buffer_struct;
static StreamBufferHandle_t s_rcv_stream_buffer_hd = NULL;

#define RCV_TASK_SIZE (1024)
static TaskHandle_t s_rcv_task_hd = NULL;
static StackType_t s_rcv_task_buff[RCV_TASK_SIZE];
static StaticTask_t s_rcv_task_struct;

static mw_sbc_ipv4_change_callback s_ipv4_change_callback = NULL;
static mw_sbc_connection_change_callback s_connection_change_callback = NULL;
static mw_sbc_power_off_request_callback s_power_off_request_callback = NULL;
static mw_sbc_network_info_response_callback s_network_info_response_callback = NULL;
static mw_sbc_ping_response_callback s_ping_response_callback = NULL;

static uint32_t s_last_ip = 0;

static void s_tusb_rx_callback(void) {
  while (tud_cdc_n_available(1)) {
    int32_t ret = tud_cdc_n_read_char(1);
    if (ret == -1) {
      break;
    }

    char c = (char)ret;
    xStreamBufferSend(s_rcv_stream_buffer_hd, &c, sizeof(char), portMAX_DELAY);
  }
}

static void s_tusb_connection_change_callback(tusb_connection_t tusb_status) {
  mw_sbc_connection_status_t sbc_connection_status;
  switch (tusb_status) {
  case TUSB_CONNECTION_USB:
    sbc_connection_status = MW_SBC_CONNECTION_USB;
    break;

  case TUSB_CONNECTION_CDC:
    sbc_connection_status = MW_SBC_CONNECTION_CDC;
    break;

  case TUSB_CONNECTION_NONE:
  default:
    sbc_connection_status = MW_SBC_CONNECTION_NONE;
    break;
  }

  if (s_connection_change_callback != NULL) {
    s_connection_change_callback(sbc_connection_status);
  }

  if (sbc_connection_status == MW_SBC_CONNECTION_NONE) {
    s_last_ip = 0;
    if (s_ipv4_change_callback != NULL) {
      s_ipv4_change_callback(0);
    }
  }
}

static size_t s_nanopb_encode(char *buff, size_t buff_len, snd_task_queue_data_t *qdata) {
  if (buff == NULL) {
    return 0;
  }

  ToSbcMessage pb_struct;
  switch (qdata->command) {
  case SND_TASK_COMMAND_BATTERY_INFO:
    pb_struct.which_data = ToSbcMessage_battery_info_tag;
    pb_struct.data.battery_info.percentage = qdata->data.battery_info.percentage;
    pb_struct.data.battery_info.voltage = qdata->data.battery_info.voltage;
    break;

  case SND_TASK_COMMAND_POWER_OFF:
    pb_struct.which_data = ToSbcMessage_power_off_req_tag;
    break;

  case SND_TASK_COMMAND_DOMAIN_ID:
    pb_struct.which_data = ToSbcMessage_domain_id_info_tag;
    pb_struct.data.domain_id_info.id = qdata->data.domain_id.id;
    break;

  case SND_TASK_COMMAND_REQUEST_NETWORK_INFO:
    pb_struct.which_data = ToSbcMessage_network_info_req_tag;
    break;

  case SND_TASK_COMMAND_REQUEST_PING:
    pb_struct.which_data = ToSbcMessage_ping_req_tag;
    break;

  case SND_TASK_COMMAND_SEND_HW_INFO:
    pb_struct.which_data = ToSbcMessage_hw_info_res_tag;
    pb_struct.data.hw_info_res.build_timestamp = (uint64_t)BUILD_EPOCH;
    memcpy(pb_struct.data.hw_info_res.git_commit_hash, GIT_COMMIT_HASH, sizeof(pb_struct.data.hw_info_res.git_commit_hash));
    break;

  case SND_TASK_COMMAND_NONE:
  default:
    return 0;
    break;
  }

  pb_ostream_t stream = pb_ostream_from_buffer(buff, buff_len);
  bool ret = pb_encode(&stream, ToSbcMessage_fields, &pb_struct);
  if (ret == false) {
    return 0;
  }

  return stream.bytes_written;
}

static void s_snd_task(void *arg) {
  (void)arg;

  snd_task_queue_data_t queue_data = {0};

  TickType_t last_connection_check_tick = xTaskGetTickCount();

  tud_cdc_n_write_clear(1);

  for (;;) {
    if (xQueueReceive(s_snd_task_queue_hd, &queue_data, portMAX_DELAY) != pdTRUE) {
      continue;
    }

    if (!tud_cdc_n_connected(1)) {
      continue;
    }

    // protobuff encoding
    char pb_buff[PACKET_MAX_LEN];
    size_t pb_encoded_size = s_nanopb_encode(pb_buff, sizeof(pb_buff) - 4, &queue_data);
    if (pb_encoded_size == 0) { // encode failed
      continue;
    }

    // add CRC32
    uint32_t crc32 = crc32_calc(pb_buff, pb_encoded_size);
    memcpy(&pb_buff[pb_encoded_size], &crc32, sizeof(crc32));
    pb_encoded_size += 4;

    // COBS encoding
    char cobs_buff[PACKET_MAX_LEN];
    cobs_encode_result cobs_res = cobs_encode(cobs_buff, sizeof(cobs_buff) - 1, pb_buff, pb_encoded_size);
    if (cobs_res.status != COBS_ENCODE_OK) {
      continue;
    }
    cobs_buff[cobs_res.out_len] = '\0';
    cobs_res.out_len++;

    // send
    if (tud_cdc_n_connected(1)) {
      tud_cdc_n_write(1, cobs_buff, cobs_res.out_len);
      tud_cdc_n_write_flush(1);
    }
  }
}

static void s_process_rx(ToMcuMessage *pb_struct) {
  switch (pb_struct->which_data) {
  case ToMcuMessage_ip_info_tag: {
    uint32_t curr_ip =
        ((uint32_t)pb_struct->data.ip_info.ipv4.bytes[0] << 24) |
        ((uint32_t)pb_struct->data.ip_info.ipv4.bytes[1] << 16) |
        ((uint32_t)pb_struct->data.ip_info.ipv4.bytes[2] << 8) |
        ((uint32_t)pb_struct->data.ip_info.ipv4.bytes[3] << 0);

    if (s_last_ip != curr_ip) {
      log_debug(TAG, "IP Changed: %d.%d.%d.%d -> %d.%d.%d.%d",
                (int)(s_last_ip >> 24), (int)(s_last_ip >> 16 & 0b11111111), (int)(s_last_ip >> 8 & 0b11111111), (int)(s_last_ip & 0b11111111),
                pb_struct->data.ip_info.ipv4.bytes[0],
                pb_struct->data.ip_info.ipv4.bytes[1],
                pb_struct->data.ip_info.ipv4.bytes[2],
                pb_struct->data.ip_info.ipv4.bytes[3]);
      s_last_ip = curr_ip;
      if (s_ipv4_change_callback != NULL) {
        s_ipv4_change_callback(curr_ip);
      }
    }

    break;
  }

  case ToMcuMessage_hw_info_req_tag: {
    snd_task_queue_data_t queue_data = {0};
    queue_data.command = SND_TASK_COMMAND_SEND_HW_INFO;
    xQueueSend(s_snd_task_queue_hd, &queue_data, 0);
    break;
  }

  case ToMcuMessage_power_off_evt_tag:
    if (s_power_off_request_callback != NULL) {
      s_power_off_request_callback();
    }
    break;

  case ToMcuMessage_network_info_res_tag: {
    mw_sbc_network_status_t type;
    switch (pb_struct->data.network_info_res.type) {

    case NetworkType_NETWORK_TYPE_WLAN:
      type = NetworkType_NETWORK_TYPE_WLAN;
      break;

    case NetworkType_NETWORK_TYPE_LAN:
      type = NetworkType_NETWORK_TYPE_LAN;
      break;

    case NetworkType_NETWORK_TYPE_NONE:
    default:
      type = NetworkType_NETWORK_TYPE_NONE;
      break;
    }

    if (s_network_info_response_callback != NULL) {
      s_network_info_response_callback(type,
                                       pb_struct->data.network_info_res.ssid,
                                       (int8_t)pb_struct->data.network_info_res.rssi,
                                       (uint16_t)pb_struct->data.network_info_res.frequency);
    }
  }

  case ToMcuMessage_ping_res_tag:
    if (s_ping_response_callback != NULL) {
      s_ping_response_callback(pb_struct->data.ping_res.ping_ms);
    }
    break;

  default:
    break;
  }
}

#define CHUNK_LEN 16
#define RINGBUFF_SIZE 128
static void s_rcv_task(void *arg) {
  ring_buffer_t ringbuffer_struct;
  char ringbuff[RINGBUFF_SIZE];
  ring_buffer_init(&ringbuffer_struct, ringbuff, sizeof(ringbuff));

  bool resync = false;

  for (;;) {
    char chunk_buff[CHUNK_LEN] = {0};
    size_t n = xStreamBufferReceive(s_rcv_stream_buffer_hd, chunk_buff, CHUNK_LEN, portMAX_DELAY);

    if (n == 0) {
      continue;
    }

    char *chunk_buff_ptr = chunk_buff;
    size_t chunk_buff_ptr_len = n;

    if (resync == true) { // resync
      char *ptr = memchr(chunk_buff, '\0', n);
      if (ptr == NULL) {
        continue;
      } else {
        resync = false;
        chunk_buff_ptr += ptr - chunk_buff + 1;
        chunk_buff_ptr_len -= ptr - chunk_buff + 1;
      }
    }

    while (chunk_buff_ptr_len > 0) {
      char *ptr = memchr(chunk_buff_ptr, '\0', chunk_buff_ptr_len);
      size_t ring_buff_item_cnt = ring_buffer_num_items(&ringbuffer_struct);

      if (ptr == NULL) {                                               // '\0' 못찾은 경우
        if (ring_buff_item_cnt + chunk_buff_ptr_len > RINGBUFF_SIZE) { // 링버퍼 가득참 -> 링버퍼 초기화 후 resync 진행
          resync = true;
          ring_buffer_init(&ringbuffer_struct, ringbuff, RINGBUFF_SIZE);
        } else { // 링버퍼에 모두 저장
          ring_buffer_queue_arr(&ringbuffer_struct, chunk_buff_ptr, chunk_buff_ptr_len);
        }

        break;
      }

      size_t len_to_copy = ptr - chunk_buff_ptr + 1;

      if (ring_buff_item_cnt + len_to_copy > PACKET_MAX_LEN) { // 예상 패킷 크기보다 큼 -> 링버퍼 초기화 후 chunk_buff_ptr 이동
        ring_buffer_init(&ringbuffer_struct, ringbuff, RINGBUFF_SIZE);
        chunk_buff_ptr += len_to_copy;
        chunk_buff_ptr_len -= len_to_copy;

        continue;
      }

      // 패킷 조립
      char packet_buff[PACKET_MAX_LEN];
      ring_buffer_dequeue_arr(&ringbuffer_struct, packet_buff, ring_buff_item_cnt);
      memcpy(packet_buff + ring_buff_item_cnt, chunk_buff_ptr, len_to_copy);
      size_t packet_buff_size = ring_buff_item_cnt + len_to_copy;

      // COBS decoding
      if (packet_buff_size == 0) {
        goto decode_failed;
      }
      packet_buff_size--; // remove '\0'
      char cobs_dec_buff[PACKET_MAX_LEN];
      cobs_decode_result cobs_res = cobs_decode(cobs_dec_buff, sizeof(cobs_dec_buff), packet_buff, packet_buff_size);
      if (cobs_res.status != COBS_DECODE_OK) {
        goto decode_failed;
      }

      // CRC32 check
      if (cobs_res.out_len < 4) {
        goto decode_failed;
      }
      uint32_t crc32 = crc32_calc(cobs_dec_buff, cobs_res.out_len - 4);
      int crc32_res = memcmp(&crc32, cobs_dec_buff + (cobs_res.out_len - 4), sizeof(uint32_t));
      if (crc32_res != 0) {
        goto decode_failed;
      }

      // protobuff decoding
      ToMcuMessage pb_struct;
      pb_istream_t stream = pb_istream_from_buffer(cobs_dec_buff, cobs_res.out_len - 4);
      bool pb_res = pb_decode(&stream, ToMcuMessage_fields, &pb_struct);
      if (pb_res == false) {
        goto decode_failed;
      }

      // execute
      s_process_rx(&pb_struct);

    decode_failed:
      chunk_buff_ptr += len_to_copy;
      chunk_buff_ptr_len -= len_to_copy;
    }
  }
}

bool mw_sbc_init(void) {
  mw_tusb_set_cdc1_callback(s_tusb_rx_callback);
  mw_tusb_set_connection_change_callback(s_tusb_connection_change_callback);

  s_snd_task_queue_hd = xQueueCreateStatic(
      SND_TASK_QUEUE_LENGTH,
      SND_TASK_QUEUE_ITEM_SIZE,
      s_sbc_queue_buff,
      &s_sbc_queue_struct);

  s_rcv_stream_buffer_hd = xStreamBufferCreateStatic(
      RCV_STREAM_BUFFER_SIZE,
      1,
      rcv_stream_buffer_storage,
      &rcv_stream_buffer_struct);

  s_snd_task_hd = xTaskCreateStatic(
      s_snd_task,
      "sbc_send",
      SND_TASK_SIZE,
      NULL,
      configMAX_PRIORITIES - 2,
      s_snd_task_buff,
      &s_snd_task_struct);

  s_rcv_task_hd = xTaskCreateStatic(
      s_rcv_task,
      "sbc_receive",
      RCV_TASK_SIZE,
      NULL,
      configMAX_PRIORITIES - 2,
      s_rcv_task_buff,
      &s_rcv_task_struct);

  return true;
}

static bool s_send_queue(snd_task_queue_data_t *queue_data) {
  bool ret = xQueueSend(s_snd_task_queue_hd, queue_data, 0) == pdTRUE;
  if (ret == false) {
    log_warning(TAG, "Publish queue full!! message dropped!!");
  }

  return ret;
}

bool mw_sbc_report_battery_info(float voltage, float percentage) {
  snd_task_queue_data_t queue_data;
  queue_data.command = SND_TASK_COMMAND_BATTERY_INFO;
  queue_data.data.battery_info.voltage = voltage;
  queue_data.data.battery_info.percentage = percentage;

  return s_send_queue(&queue_data);
}

bool mw_sbc_report_power_off(void) {
  snd_task_queue_data_t queue_data;
  queue_data.command = SND_TASK_COMMAND_POWER_OFF;

  return s_send_queue(&queue_data);
}

bool mw_sbc_report_domain_id(uint8_t id) {
  snd_task_queue_data_t queue_data;
  queue_data.command = SND_TASK_COMMAND_DOMAIN_ID;
  queue_data.data.domain_id.id = id;

  return s_send_queue(&queue_data);
}

void mw_sbc_request_network_info(void) {
  snd_task_queue_data_t queue_data;
  queue_data.command = SND_TASK_COMMAND_REQUEST_NETWORK_INFO;

  s_send_queue(&queue_data);
}

void mw_sbc_request_ping(void) {
  snd_task_queue_data_t queue_data;
  queue_data.command = SND_TASK_COMMAND_REQUEST_PING;

  s_send_queue(&queue_data);
}

void mw_sbc_set_ipv4_change_callback(mw_sbc_ipv4_change_callback callback) {
  s_ipv4_change_callback = callback;
}

void mw_sbc_set_connection_change_callback(mw_sbc_connection_change_callback callback) {
  s_connection_change_callback = callback;
}

void mw_sbc_set_power_off_request_callback(mw_sbc_power_off_request_callback callback) {
  s_power_off_request_callback = callback;
}

void mw_sbc_set_network_info_response_callback(mw_sbc_network_info_response_callback callback) {
  s_network_info_response_callback = callback;
}

void mw_sbc_set_ping_response_callback(mw_sbc_ping_response_callback callback) {
  s_ping_response_callback = callback;
}
