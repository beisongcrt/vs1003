

#include "vs1003.h"

#ifdef VS1003_EXAMPLE

#define VS1003_SPI_DEVICE_NAME    "spi2"
#define VS1003_DEVICE_NAME        "spi20"

extern unsigned char mp3[38400];

VS1003_DEVICE VS1003_dev;

void VS1003_pin_clk_enable(void);

VS1003_CONFIG VS1003_config = 
{
        .name = VS1003_DEVICE_NAME,
        .spi_device_name = VS1003_SPI_DEVICE_NAME,

        /* 这个片选脚是给 spi 设备的，
       由于 VS1003 的命令脚和数据脚是分开的，
       所有这个脚位需要设置为空脚  */

#if (SPI_DEVICE_ATTACH_MODE == SPI_DEVICE_ATTACH_GPIO_TYPE)
        .GPIO_CS_Port = GPIOD,
        .GPIO_CS_Pin  = GPIO_PIN_13,
#elif (SPI_DEVICE_ATTACH_MODE == SPI_DEVICE_ATTACH_PIN_NUM)
        .pin_spi_CS = 61,    // PD13
#endif
        

        .pin_DREQ   = 54,    // PD6
        .pin_XCS    = 28,    // PB12
        .pin_XDCS   = 55,    // PD7
        .pin_XRESET = 59,    // PD11

        .pin_clk_enable = VS1003_pin_clk_enable,
};

void VS1003_pin_clk_enable(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
}

int vs1003_example_init(void)
{
    if(RT_EOK != VS1003_init(&VS1003_dev, &VS1003_config))
    {
        rt_kprintf("VS1003_init VS1003 err\r\n");
        return -RT_ERROR;
    }

    return RT_EOK;
}

INIT_APP_EXPORT(vs1003_example_init);


void vs1003_sine(void)
{
    if(RT_EOK != VS1003_Sine_Test(&VS1003_dev, 0x24, 0x74))
    {
        rt_kprintf("VS1003_Sine_Test VS1003 err\r\n");
    }
}
MSH_CMD_EXPORT(vs1003_sine, 'vs1003 example sine');

void vs1003_play(void)
{
    int size = 38400;
    int pos = 0;

    VS1003_Set_Vol(&VS1003_dev, 0X2020);

    while (1)
    {
        if(0 != VS1003_DREQ_state(VS1003_dev.config->pin_DREQ))
        {
            VS1003_Send_data(&VS1003_dev, &mp3[pos], 32);
            pos += 32;
            if(size < pos)
            {
                VS1003_Restart_Play(&VS1003_dev);
                break;
            }
        }
    }
}
MSH_CMD_EXPORT(vs1003_play, 'vs1003 example play');

#endif

