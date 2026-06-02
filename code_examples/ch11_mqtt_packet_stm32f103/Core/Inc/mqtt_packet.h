/**
 * @file mqtt_packet.h
 * @brief MQTT 3.1.1 CONNECT / PUBLISH 报文手动构造
 */
#ifndef MQTT_PACKET_H
#define MQTT_PACKET_H

#include <stdint.h>

uint8_t MQTT_EncodeRemainingLength(uint8_t *buf, uint32_t len);

uint16_t MQTT_BuildConnect(uint8_t *buf, const char *client_id, uint16_t keep_alive_sec);

uint16_t MQTT_BuildPublish(uint8_t *buf, const char *topic, const char *payload,
                           uint8_t qos, uint16_t pkt_id);

#endif
