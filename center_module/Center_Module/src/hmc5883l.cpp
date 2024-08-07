#include "hmc5883l.h"

#include "math.h"

#include "i2c.h"


HMC5883L_T g_tMag;

static uint8_t hmc5883L_WeReg(uint16_t regAdd, uint8_t *pData, uint16_t Size) {
    HAL_StatusTypeDef status;
    status = HAL_I2C_Mem_Write( &hi2c2, HMC5883L_SLAVE_ADDRESS, regAdd, I2C_MEMADD_SIZE_8BIT, pData, Size, 1000);
    if (status == HAL_OK)
        return HMC5883L_OK;
    else
        return HMC5883L_ERROR;
}

static uint8_t hmc5883L_RdReg(uint16_t regAdd, uint8_t *pData, uint16_t Size) {
    HAL_StatusTypeDef status;
    status = HAL_I2C_Mem_Read( &hi2c2, HMC5883L_SLAVE_ADDRESS, regAdd, I2C_MEMADD_SIZE_8BIT, pData, Size, 1000);
    if (status == HAL_OK)
        return HMC5883L_OK;
    else
        return HMC5883L_ERROR;
}

void hmc5883L_WriteByte(uint8_t _ucRegAddr, uint8_t _ucRegData) {
    hmc5883L_WeReg( _ucRegAddr, &_ucRegData, 1);
}

uint8_t hmc5883L_ReadByte(uint8_t _ucRegAddr) {
    uint8_t _ucRegData;
    hmc5883L_RdReg( _ucRegAddr, &_ucRegData, 1);
    return  _ucRegData;
}

void hmc5883l_Init() {
    /* 设置 Mode 寄存器 */
#if 1
    hmc5883L_WriteByte(0x00, 0x70);
    hmc5883L_WriteByte(0x01, 0x20);
    hmc5883L_WriteByte(0x02, 0x00);
#else /* 自校准模式 */
    hmc5883L_WriteByte(0x00, 0x70 + 2);
    hmc5883L_WriteByte(0x01, 0x20);
    hmc5883L_WriteByte(0x02, 0x00);
#endif

    g_tMag.CfgRegA = hmc5883L_ReadByte(0);
    g_tMag.CfgRegB = hmc5883L_ReadByte(1);
    g_tMag.ModeReg = hmc5883L_ReadByte(2);

    g_tMag.IDReg[0] = hmc5883L_ReadByte(10);
    g_tMag.IDReg[1] = hmc5883L_ReadByte(11);
    g_tMag.IDReg[2] = hmc5883L_ReadByte(12);
    g_tMag.IDReg[3] = 0;

    /* 设置最小最大值初值 */
    g_tMag.X_Min = 4096;
    g_tMag.X_Max = -4096;

    g_tMag.Y_Min = 4096;
    g_tMag.Y_Max = -4096;

    g_tMag.Z_Min = 4096;
    g_tMag.Z_Max = -4096;
}

void hmc5883l_ReadData() {
    uint8_t ucReadBuf[7];

    hmc5883L_RdReg(DATA_OUT_X, ucReadBuf, 7);

    /* 将读出的数据保存到全局结构体变量 */
    g_tMag.X = (int16_t)((ucReadBuf[0] << 8) + ucReadBuf[1]);
    g_tMag.Z = (int16_t)((ucReadBuf[2] << 8) + ucReadBuf[3]);
    g_tMag.Y = (int16_t)((ucReadBuf[4] << 8) + ucReadBuf[5]);

    g_tMag.Status = ucReadBuf[6];

    /* 统计最大值和最小值 */
    if ((g_tMag.X > - 2048) && (g_tMag.X < 2048)) {
        if (g_tMag.X > g_tMag.X_Max) {
            g_tMag.X_Max = g_tMag.X;
        }
        if (g_tMag.X < g_tMag.X_Min) {
            g_tMag.X_Min = g_tMag.X;
        }
    }

    if ((g_tMag.Y > - 2048) && (g_tMag.Y < 2048)) {
        if (g_tMag.Y > g_tMag.Y_Max) {
            g_tMag.Y_Max = g_tMag.Y;
        }
        if (g_tMag.Y < g_tMag.Y_Min) {
            g_tMag.Y_Min = g_tMag.Y;
        }
    }

    if ((g_tMag.Z > - 2048) && (g_tMag.Z < 2048)) {
        if (g_tMag.Z > g_tMag.Z_Max) {
            g_tMag.Z_Max = g_tMag.Z;
        }
        if (g_tMag.Z < g_tMag.Z_Min) {
            g_tMag.Z_Min = g_tMag.Z;
        }
    }
}

float hmc5883l_GetAngle() {
    hmc5883l_ReadData();

    float Magangle = 0.0f;
    float GaX = g_tMag.X / 1090.0f;
    float GaY = g_tMag.Y / 1090.0f;

    if ((GaX > 0) && (GaY > 0)) Magangle = atan(GaY / GaX) * 57;
    else if((GaX > 0)&&(GaY < 0)) Magangle = 360 + atan(GaY / GaX) * 57;
    else if((GaX == 0)&&(GaY > 0)) Magangle = 90;
    else if((GaX == 0)&&(GaY < 0)) Magangle = 270;
    else if(GaX < 0) Magangle = 180 + atan(GaY / GaX) * 57;

    return Magangle;

//    char buffer[20];
//    snprintf(buffer, sizeof(buffer), "%6.2f\n", Magangle);
//    HAL_UART_Transmit(&huart2, (uint8_t*) buffer, 7, HAL_MAX_DELAY);

//    char buffer[20];
//    snprintf(buffer, sizeof(buffer), "%5d, %5d, %5d\n", g_tMag.X, g_tMag.Y, g_tMag.Z);
//    HAL_UART_Transmit(&huart2, (uint8_t*) buffer, 20, HAL_MAX_DELAY);
}
