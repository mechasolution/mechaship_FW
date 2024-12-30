#ifndef C04356A6_C688_48F2_AD8B_6E72BC0FDD2C_H_
#define C04356A6_C688_48F2_AD8B_6E72BC0FDD2C_H_

#include <stdint.h>
#include <stdio.h>

#include <uxr/client/profile/transport/custom/custom_transport.h>

bool serial_transport_open(struct uxrCustomTransport *transport);
bool serial_transport_close(struct uxrCustomTransport *transport);
size_t serial_transport_write(struct uxrCustomTransport *transport, const uint8_t *buf, size_t len, uint8_t *err);
size_t serial_transport_read(struct uxrCustomTransport *transport, uint8_t *buf, size_t len, int timeout, uint8_t *err);

#endif /* C04356A6_C688_48F2_AD8B_6E72BC0FDD2C_H_ */
