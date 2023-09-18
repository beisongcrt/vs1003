

#ifndef __VS1003_H__
#define __VS1003_H__

#include <rtthread.h>

#ifdef PKG_USING_VS1003

#include "drv_spi.h"

#define VS_WRITE_COMMAND 0x02
#define VS_READ_COMMAND  0x03
// VS10XX 寄存器定义
#define SPI_MODE         0x00
#define SPI_STATUS       0x01
#define SPI_BASS         0x02
#define SPI_CLOCKF       0x03
#define SPI_DECODE_TIME  0x04
#define SPI_AUDATA       0x05
#define SPI_WRAM         0x06
#define SPI_WRAMADDR     0x07
#define SPI_HDAT0        0x08
#define SPI_HDAT1        0x09
#define SPI_AIADDR       0x0a
#define SPI_VOL          0x0b
#define SPI_AICTRL0      0x0c
#define SPI_AICTRL1      0x0d
#define SPI_AICTRL2      0x0e
#define SPI_AICTRL3      0x0f

#define SM_DIFF          0x01
#define SM_JUMP          0x02
#define SM_RESET         0x04
#define SM_OUTOFWAV      0x08
#define SM_PDOWN         0x10
#define SM_TESTS         0x20
#define SM_STREAM        0x40
#define SM_PLUSV         0x80
#define SM_DACT          0x100
#define SM_SDIORD        0x200
#define SM_SDISHARE      0x400
#define SM_SDINEW        0x800
#define SM_ADPCM         0x1000
#define SM_ADPCM_HP      0x2000

#define I2S_CONFIG       0XC040
#define GPIO_DDR         0XC017
#define GPIO_IDATA       0XC018
#define GPIO_ODATA       0XC019


#define SPI_DEVICE_ATTACH_PIN_NUM    1
#define SPI_DEVICE_ATTACH_GPIO_TYPE  2

#define SPI_DEVICE_ATTACH_MODE       SPI_DEVICE_ATTACH_PIN_NUM 

typedef struct
{
    char * name;
    char * spi_device_name;

#if (SPI_DEVICE_ATTACH_MODE == SPI_DEVICE_ATTACH_GPIO_TYPE)
    GPIO_TypeDef* GPIO_CS_Port;
    unsigned short GPIO_CS_Pin;
#elif (SPI_DEVICE_ATTACH_MODE == SPI_DEVICE_ATTACH_PIN_NUM)
    rt_base_t pin_spi_CS;
#endif

    rt_base_t pin_DREQ;
    rt_base_t pin_XCS;
    rt_base_t pin_XDCS;
    rt_base_t pin_XRESET;

    void (* pin_clk_enable) (void);

}VS1003_CONFIG;



typedef struct 
{
    VS1003_CONFIG * config;
    struct rt_spi_device * spi_dev;

}VS1003_DEVICE;

int VS1003_DREQ_state(rt_base_t pin_DREQ);
void VS1003_Send_data(VS1003_DEVICE * VS1003, unsigned char *data, unsigned char Data_Len);
rt_err_t VS1003_Set_Vol(VS1003_DEVICE * VS1003, unsigned short vol);
rt_err_t VS1003_Sine_Test(VS1003_DEVICE * VS1003, unsigned char Sin_Freq_1, unsigned char Sin_Freq_2);
rt_err_t VS1003_Restart_Play(VS1003_DEVICE * VS1003);
rt_err_t VS1003_init(VS1003_DEVICE * VS1003_dev, VS1003_CONFIG * config);

#endif  // PKG_USING_VS1003

#endif  // __VS1003_H__


