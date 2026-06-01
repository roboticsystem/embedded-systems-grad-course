#include "app_protocol.h"
#include "main.h"
#include "uart_dma_protocol.h"
#include <string.h>

#define PROTOCOL_SOF1 0xAAU
#define PROTOCOL_SOF2 0x55U

typedef enum {
    PARSER_WAIT_SOF1 = 0,
    PARSER_WAIT_SOF2,
    PARSER_WAIT_LEN,
    PARSER_WAIT_CMD,
    PARSER_WAIT_DATA,
    PARSER_WAIT_CRC
} ParserState;

typedef struct {
    ParserState state;
    ProtocolFrame frame;
    uint8_t data_index;
} ProtocolParser;

static ProtocolParser parser;

static void Protocol_ResetParser(void);
static void Protocol_FeedByte(uint8_t byte);
static void Protocol_DispatchFrame(const ProtocolFrame *frame);
static void Protocol_SendFrame(uint8_t cmd, const uint8_t *payload,
                               uint8_t length);
static void Protocol_SendAck(uint8_t source_cmd);
static void Protocol_SendError(uint8_t source_cmd, uint8_t status);

void Protocol_Init(void)
{
    Protocol_ResetParser();
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
}

void Protocol_ParseBytes(const uint8_t *data, uint16_t length)
{
    if (data == NULL) {
        return;
    }

    for (uint16_t i = 0; i < length; i++) {
        Protocol_FeedByte(data[i]);
    }
}

void Protocol_Poll(void)
{
    /* 保留轮询入口，便于后续加入超时处理或环形缓冲区任务。 */
}

uint8_t Protocol_CalcChecksum(uint8_t length, uint8_t cmd,
                              const uint8_t *payload)
{
    uint8_t checksum = length ^ cmd;

    for (uint8_t i = 0; i < length; i++) {
        checksum ^= payload[i];
    }

    return checksum;
}

static void Protocol_ResetParser(void)
{
    memset(&parser, 0, sizeof(parser));
    parser.state = PARSER_WAIT_SOF1;
}

static void Protocol_FeedByte(uint8_t byte)
{
    switch (parser.state) {
    case PARSER_WAIT_SOF1:
        if (byte == PROTOCOL_SOF1) {
            parser.state = PARSER_WAIT_SOF2;
        }
        break;

    case PARSER_WAIT_SOF2:
        if (byte == PROTOCOL_SOF2) {
            parser.state = PARSER_WAIT_LEN;
        } else if (byte != PROTOCOL_SOF1) {
            parser.state = PARSER_WAIT_SOF1;
        }
        break;

    case PARSER_WAIT_LEN:
        if (byte > PROTOCOL_MAX_PAYLOAD_SIZE) {
            Protocol_SendError(0x00U, PROTOCOL_STATUS_BAD_LENGTH);
            Protocol_ResetParser();
            break;
        }
        parser.frame.length = byte;
        parser.data_index = 0;
        parser.state = PARSER_WAIT_CMD;
        break;

    case PARSER_WAIT_CMD:
        parser.frame.cmd = byte;
        parser.state = (parser.frame.length == 0U) ?
                       PARSER_WAIT_CRC : PARSER_WAIT_DATA;
        break;

    case PARSER_WAIT_DATA:
        parser.frame.payload[parser.data_index++] = byte;
        if (parser.data_index >= parser.frame.length) {
            parser.state = PARSER_WAIT_CRC;
        }
        break;

    case PARSER_WAIT_CRC: {
        uint8_t expected = Protocol_CalcChecksum(parser.frame.length,
                                                parser.frame.cmd,
                                                parser.frame.payload);
        if (expected == byte) {
            Protocol_DispatchFrame(&parser.frame);
        } else {
            Protocol_SendError(parser.frame.cmd, PROTOCOL_STATUS_CRC_ERROR);
        }
        Protocol_ResetParser();
        break;
    }

    default:
        Protocol_ResetParser();
        break;
    }
}

static void Protocol_DispatchFrame(const ProtocolFrame *frame)
{
    switch (frame->cmd) {
    case PROTOCOL_CMD_LED_ON:
        if (frame->length != 0U) {
            Protocol_SendError(frame->cmd, PROTOCOL_STATUS_BAD_LENGTH);
            return;
        }
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        Protocol_SendAck(frame->cmd);
        break;

    case PROTOCOL_CMD_LED_OFF:
        if (frame->length != 0U) {
            Protocol_SendError(frame->cmd, PROTOCOL_STATUS_BAD_LENGTH);
            return;
        }
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        Protocol_SendAck(frame->cmd);
        break;

    case PROTOCOL_CMD_ECHO:
        Protocol_SendFrame(PROTOCOL_CMD_ECHO, frame->payload, frame->length);
        break;

    default:
        Protocol_SendError(frame->cmd, PROTOCOL_STATUS_BAD_CMD);
        break;
    }
}

static void Protocol_SendFrame(uint8_t cmd, const uint8_t *payload,
                               uint8_t length)
{
    uint8_t tx_frame[5U + PROTOCOL_MAX_PAYLOAD_SIZE];
    uint16_t index = 0;

    if (length > PROTOCOL_MAX_PAYLOAD_SIZE) {
        return;
    }

    tx_frame[index++] = PROTOCOL_SOF1;
    tx_frame[index++] = PROTOCOL_SOF2;
    tx_frame[index++] = length;
    tx_frame[index++] = cmd;

    if ((payload != NULL) && (length > 0U)) {
        memcpy(&tx_frame[index], payload, length);
        index += length;
    }

    tx_frame[index++] = Protocol_CalcChecksum(length, cmd, payload);

    (void)UART_DMA_Protocol_Send(tx_frame, index);
}

static void Protocol_SendAck(uint8_t source_cmd)
{
    uint8_t payload[2] = {source_cmd, PROTOCOL_STATUS_OK};
    Protocol_SendFrame(PROTOCOL_CMD_ACK, payload, sizeof(payload));
}

static void Protocol_SendError(uint8_t source_cmd, uint8_t status)
{
    uint8_t payload[2] = {source_cmd, status};
    Protocol_SendFrame(PROTOCOL_CMD_ERR, payload, sizeof(payload));
}
