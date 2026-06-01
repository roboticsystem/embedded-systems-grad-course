#ifndef __APP_PROTOCOL_H
#define __APP_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define PROTOCOL_MAX_PAYLOAD_SIZE 64U

#define PROTOCOL_CMD_LED_ON  0x01U
#define PROTOCOL_CMD_LED_OFF 0x02U
#define PROTOCOL_CMD_ECHO    0x03U
#define PROTOCOL_CMD_ACK     0x80U
#define PROTOCOL_CMD_ERR     0x81U

#define PROTOCOL_STATUS_OK          0x00U
#define PROTOCOL_STATUS_CRC_ERROR   0x01U
#define PROTOCOL_STATUS_BAD_CMD     0x02U
#define PROTOCOL_STATUS_BAD_LENGTH  0x03U
#define PROTOCOL_STATUS_TX_BUSY     0x04U

typedef struct {
    uint8_t cmd;
    uint8_t length;
    uint8_t payload[PROTOCOL_MAX_PAYLOAD_SIZE];
} ProtocolFrame;

void Protocol_Init(void);
void Protocol_ParseBytes(const uint8_t *data, uint16_t length);
void Protocol_Poll(void);
uint8_t Protocol_CalcChecksum(uint8_t length, uint8_t cmd,
                              const uint8_t *payload);

#ifdef __cplusplus
}
#endif

#endif
