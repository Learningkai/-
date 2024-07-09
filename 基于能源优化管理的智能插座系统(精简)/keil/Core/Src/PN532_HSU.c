#include "PN532_HSU.h"

PN532_HSU::PN532_HSU(UART_HandleTypeDef &huart1)
{
    _huart = &huart1;
    command = 0;
}

void PN532_HSU::begin()
{
    // UART初始化代码已经在CubeMX或手动代码中完成
}

void PN532_HSU::wakeup()
{
    uint8_t wakeup_cmd[] = {0x55, 0x55, 0x00, 0x00, 0x00};
    HAL_UART_Transmit(_huart, wakeup_cmd, sizeof(wakeup_cmd), HAL_MAX_DELAY);

    // dump serial buffer
    uint8_t ret;
    while(HAL_UART_Receive(_huart, &ret, 1, 10) == HAL_OK){
        // DMSG_HEX(ret);  // Debug消息输出函数，可以使用printf或其他调试工具
    }
}

int8_t PN532_HSU::writeCommand(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{
    // dump serial buffer
    uint8_t ret;
    while(HAL_UART_Receive(_huart, &ret, 1, 10) == HAL_OK){
        // DMSG_HEX(ret);
    }

    command = header[0];

    uint8_t length = hlen + blen + 1;
    uint8_t packet[6 + length + 2 + 1] = {0};
    packet[0] = PN532_PREAMBLE;
    packet[1] = PN532_STARTCODE1;
    packet[2] = PN532_STARTCODE2;
    packet[3] = length;
    packet[4] = ~length + 1;
    packet[5] = PN532_HOSTTOPN532;

    uint8_t sum = PN532_HOSTTOPN532;
    for (uint8_t i = 0; i < hlen; i++) {
        packet[6 + i] = header[i];
        sum += header[i];
    }
    for (uint8_t i = 0; i < blen; i++) {
        packet[6 + hlen + i] = body[i];
        sum += body[i];
    }

    uint8_t checksum = ~sum + 1;
    packet[6 + length] = checksum;
    packet[6 + length + 1] = PN532_POSTAMBLE;

    HAL_UART_Transmit(_huart, packet, sizeof(packet), HAL_MAX_DELAY);

    return readAckFrame();
}

int16_t PN532_HSU::readResponse(uint8_t buf[], uint8_t len, uint16_t timeout)
{
    uint8_t tmp[3];

    // Frame Preamble and Start Code
    if(receive(tmp, 3, timeout) <= 0){
        return PN532_TIMEOUT;
    }
    if(0 != tmp[0] || 0 != tmp[1] || 0xFF != tmp[2]){
        // DMSG("Preamble error");
        return PN532_INVALID_FRAME;
    }

    // receive length and check
    uint8_t length[2];
    if(receive(length, 2, timeout) <= 0){
        return PN532_TIMEOUT;
    }
    if(0 != (uint8_t)(length[0] + length[1])){
        // DMSG("Length error");
        return PN532_INVALID_FRAME;
    }
    length[0] -= 2;
    if(length[0] > len){
        return PN532_NO_SPACE;
    }

    // receive command byte
    uint8_t cmd = command + 1;
    if(receive(tmp, 2, timeout) <= 0){
        return PN532_TIMEOUT;
    }
    if(PN532_PN532TOHOST != tmp[0] || cmd != tmp[1]){
        // DMSG("Command error");
        return PN532_INVALID_FRAME;
    }

    if(receive(buf, length[0], timeout) != length[0]){
        return PN532_TIMEOUT;
    }
    uint8_t sum = PN532_PN532TOHOST + cmd;
    for(uint8_t i = 0; i < length[0]; i++){
        sum += buf[i];
    }

    // checksum and postamble
    if(receive(tmp, 2, timeout) <= 0){
        return PN532_TIMEOUT;
    }
    if(0 != (uint8_t)(sum + tmp[0]) || 0 != tmp[1]){
        // DMSG("Checksum error");
        return PN532_INVALID_FRAME;
    }

    return length[0];
}

int8_t PN532_HSU::readAckFrame()
{
    const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};
    uint8_t ackBuf[sizeof(PN532_ACK)];

    if(receive(ackBuf, sizeof(PN532_ACK), PN532_ACK_WAIT_TIME) <= 0){
        // DMSG("Timeout\n");
        return PN532_TIMEOUT;
    }

    if(memcmp(ackBuf, PN532_ACK, sizeof(PN532_ACK))){
        // DMSG("Invalid\n");
        return PN532_INVALID_ACK;
    }
    return 0;
}

int8_t PN532_HSU::receive(uint8_t *buf, int len, uint16_t timeout)
{
    int read_bytes = 0;
    int ret;
    unsigned long start_millis;

    while (read_bytes < len) {
        start_millis = HAL_GetTick();
        do {
            ret = HAL_UART_Receive(_huart, &buf[read_bytes], 1, 10);
            if (ret == HAL_OK) {
                break;
            }
        } while((timeout == 0) || ((HAL_GetTick() - start_millis) < timeout));

        if (ret != HAL_OK) {
            if(read_bytes){
                return read_bytes;
            }else{
                return PN532_TIMEOUT;
            }
        }
        read_bytes++;
    }
    return read_bytes;
}

