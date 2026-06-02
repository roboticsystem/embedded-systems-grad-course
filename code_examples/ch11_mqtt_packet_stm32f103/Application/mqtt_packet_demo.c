#include "mqtt_packet_demo.h"
#include "mqtt_packet.h"
#include <stdio.h>
#include <string.h>

#define MQTT_TX_BUF_SIZE 256U

static void MQTT_PrintPacket(UART_HandleTypeDef *huart, const char *name,
                             const uint8_t *data, uint16_t len)
{
    char line[80];
    int n;

    n = snprintf(line, sizeof(line), "\r\n===== %s (%u bytes) =====\r\n", name, len);
    HAL_UART_Transmit(huart, (uint8_t *)line, (uint16_t)n, 1000);

    for (uint16_t i = 0; i < len; i++) {
        n = snprintf(line, sizeof(line), "%02X ", data[i]);
        HAL_UART_Transmit(huart, (uint8_t *)line, (uint16_t)n, 100);
        if ((i + 1U) % 16U == 0U) {
            const char crlf[] = "\r\n";
            HAL_UART_Transmit(huart, (uint8_t *)crlf, 2U, 100);
        }
    }
    const char end[] = "\r\n";
    HAL_UART_Transmit(huart, (uint8_t *)end, 2U, 100);
}

void MQTT_PacketDemo_Run(UART_HandleTypeDef *huart)
{
    uint8_t buf[MQTT_TX_BUF_SIZE];
    uint16_t len;

    len = MQTT_BuildConnect(buf, "stm32", 60U);
    MQTT_PrintPacket(huart, "MQTT CONNECT", buf, len);

    len = MQTT_BuildPublish(buf, "farm/greenhouse/temp", "26.5", 0U, 0U);
    MQTT_PrintPacket(huart, "MQTT PUBLISH QoS0", buf, len);

    len = MQTT_BuildPublish(buf, "farm/greenhouse/humi", "68", 1U, 1U);
    MQTT_PrintPacket(huart, "MQTT PUBLISH QoS1", buf, len);
}
