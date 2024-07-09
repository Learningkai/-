#ifndef PN532_HSU_H
#define PN532_HSU_H

#include "stm32wbxx_hal.h" // 根据具体的STM32型号修改

#define PN532_PREAMBLE              0x00
#define PN532_STARTCODE1            0x00
#define PN532_STARTCODE2            0xFF
#define PN532_POSTAMBLE             0x00
#define PN532_HOSTTOPN532           0xD4
#define PN532_PN532TOHOST           0xD5

#define PN532_ACK_WAIT_TIME         10
#define PN532_TIMEOUT               -1
#define PN532_INVALID_FRAME         -2
#define PN532_NO_SPACE              -3
#define PN532_INVALID_ACK           -4

class PN532_HSU {
public:
    PN532_HSU(UART_HandleTypeDef &huart);
    void begin();
    void wakeup();
    int8_t writeCommand(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen);
    int16_t readResponse(uint8_t buf[], uint8_t len, uint16_t timeout);
    int8_t readAckFrame();

private:
    UART_HandleTypeDef *_huart;
    uint8_t command;
    int8_t receive(uint8_t *buf, int len, uint16_t timeout);
};

#endif
