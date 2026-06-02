#include "mqtt_packet.h"
#include <string.h>

uint8_t MQTT_EncodeRemainingLength(uint8_t *buf, uint32_t len)
{
    uint8_t pos = 0;

    do {
        uint8_t digit = (uint8_t)(len % 128U);
        len /= 128U;
        if (len > 0U) {
            digit |= 0x80U;
        }
        buf[pos++] = digit;
    } while (len > 0U);

    return pos;
}

uint16_t MQTT_BuildConnect(uint8_t *buf, const char *client_id, uint16_t keep_alive_sec)
{
    const char proto[] = "MQTT";
    uint16_t cid_len = (uint16_t)strlen(client_id);
    uint32_t remain = 6U + 1U + 1U + 2U + 2U + cid_len;
    uint16_t idx = 0;

    buf[idx++] = 0x10U;
    idx += MQTT_EncodeRemainingLength(&buf[idx], remain);

    buf[idx++] = 0x00U;
    buf[idx++] = 0x04U;
    memcpy(&buf[idx], proto, 4U);
    idx += 4U;
    buf[idx++] = 0x04U;
    buf[idx++] = 0x02U;
    buf[idx++] = (uint8_t)(keep_alive_sec >> 8);
    buf[idx++] = (uint8_t)(keep_alive_sec);
    buf[idx++] = (uint8_t)(cid_len >> 8);
    buf[idx++] = (uint8_t)(cid_len);
    memcpy(&buf[idx], client_id, cid_len);
    idx += cid_len;

    return idx;
}

uint16_t MQTT_BuildPublish(uint8_t *buf, const char *topic, const char *payload,
                           uint8_t qos, uint16_t pkt_id)
{
    uint16_t topic_len = (uint16_t)strlen(topic);
    uint16_t payload_len = (uint16_t)strlen(payload);
    uint32_t remain = 2U + topic_len + payload_len;
    uint16_t idx = 0;

    if (qos > 2U) {
        qos = 0U;
    }
    if (qos > 0U) {
        remain += 2U;
    }

    buf[idx++] = (uint8_t)(0x30U | ((qos & 0x03U) << 1));
    idx += MQTT_EncodeRemainingLength(&buf[idx], remain);

    buf[idx++] = (uint8_t)(topic_len >> 8);
    buf[idx++] = (uint8_t)(topic_len);
    memcpy(&buf[idx], topic, topic_len);
    idx += topic_len;

    if (qos > 0U) {
        buf[idx++] = (uint8_t)(pkt_id >> 8);
        buf[idx++] = (uint8_t)(pkt_id);
    }

    memcpy(&buf[idx], payload, payload_len);
    idx += payload_len;

    return idx;
}
