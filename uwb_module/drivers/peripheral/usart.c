#include "usart.h"


void ConfigureUSART(void) {
    RCC_APB2PeriphClockCmd(USARTx_RCC, ENABLE);

    // GPIO configuration
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = USARTx_TX;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(USARTx_GPIO, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = USARTx_RX;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(USARTx_GPIO, &GPIO_InitStructure);

    // USART configuration
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = USARTx_BAUDRATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USARTx, &USART_InitStructure);

    // Enable the USART1 receive interrupt
    USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);

    // NVIC configuration
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 7;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 7;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Enable USART1
    USART_Cmd(USARTx, ENABLE);
}

//uint8_t CalculateCRC8(uint8_t *data, uint16_t length) {
//    uint8_t crc = 0x00;
//    for (uint16_t i = 0; i < length; i++) {
//        crc ^= data[i];
//        for (uint8_t j = 0; j < 8; j++) {
//            if (crc & 0x80) {
//                crc = (crc << 1) ^ 0x07;
//            } else {
//                crc <<= 1;
//            }
//        }
//    }
//    return crc;
//}
//
//uint16_t CalculateCRC16(uint8_t *data, uint16_t length) {
//    uint16_t crc = 0xFFFF;
//    for (uint16_t i = 0; i < length; i++) {
//        crc ^= data[i];
//        for (uint8_t j = 0; j < 8; j++) {
//            if (crc & 0x0001) {
//                crc = (crc >> 1) ^ 0xA001;
//            } else {
//                crc >>= 1;
//            }
//        }
//    }
//    return crc;
//}
//
//void Frame_Init(FrameContext *ctx) {
//    ctx->state = STATE_WAIT_HEADER;
//    ctx->length = 0;
//    ctx->crc8 = 0;
//    ctx->crc16 = 0;
//    ctx->data_index = 0;
//}
//
//void Frame_ProcessByte(FrameContext *ctx, uint8_t byte) {
//    switch (ctx->state) {
//        case STATE_WAIT_HEADER:
//            if (byte == FRAME_HEADER) {
//                ctx->buffer[0] = byte;
//                ctx->state = STATE_WAIT_LENGTH;
//                ctx->data_index = 0;
//            }
//            break;
//        case STATE_WAIT_LENGTH:
//            ctx->buffer[1 + ctx->data_index ++] = byte;
//            if (ctx->data_index == 2) {
//                ctx->length = ctx->buffer[1] | (ctx->buffer[2] << 8);
//                if (ctx->length > FRAME_MAX_DATA_LENGTH) {
//                    ctx->state = STATE_WAIT_HEADER;
//                } else {
//                    ctx->state = STATE_WAIT_CRC8;
//                }
//                ctx->data_index = 0;
//            }
//            break;
//        case STATE_WAIT_CRC8:
//            ctx->buffer[3] = byte;
//            ctx->crc8 = CalculateCRC8(ctx->buffer, 3);
//            if (ctx->crc8 == byte) {
//                ctx->state = STATE_WAIT_DATA;
//            } else {
//                ctx->state = STATE_WAIT_HEADER;
//            }
//            break;
//        case STATE_WAIT_DATA:
//            ctx->buffer[4 + ctx->data_index ++] = byte;
//            if (ctx->data_index == ctx->length) {
//                ctx->state = STATE_WAIT_CRC16;
//                ctx->data_index = 0;
//            }
//            break;
//        case STATE_WAIT_CRC16:
//            ctx->buffer[4 + ctx->length + ctx->data_index ++] = byte;
//            if (ctx->data_index == 2) {
//                ctx->crc16 = (ctx->buffer[4 + ctx->length] | (ctx->buffer[5 + ctx->length] << 8));
//                uint16_t calculated_crc16 = CalculateCRC16(ctx->buffer, 4 + ctx->length);
//                if (ctx->crc16 == calculated_crc16) {
//                    // Frame successfully received and validated
//                    // Handle the frame data here
////                    printf("Data Length: %d\n", ctx->length);
////                    printf("%.*s", ctx->length, ctx->buffer + 4);
//                }
//                ctx->state = STATE_WAIT_HEADER;
//            }
//            break;
//        default:
//            ctx->state = STATE_WAIT_HEADER;
//            break;
//    }
//}
//
//FrameContext frame_ctx;
//
//void USART1_IRQHandler(void) {
//    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
//        uint8_t byte = USART_ReceiveData(USART1);
//        Frame_ProcessByte(&frame_ctx, byte);
//        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
//    }
//}