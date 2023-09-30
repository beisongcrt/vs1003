


#include "vs1003.h"

#ifdef PKG_USING_VS1003

#if defined(BSP_USING_SPI1) || defined(BSP_USING_SPI2) || defined(BSP_USING_SPI3) || defined(BSP_USING_SPI4) || defined(BSP_USING_SPI5) || defined(BSP_USING_SPI6)


#define VS1003_DEBUG

#ifdef VS1003_DEBUG
#define VS1003_printf         rt_kprintf
#else 
#define VS1003_printf(...)
#endif /* #ifdef VS1003_DEBUG */


int VS1003_DREQ_state(rt_base_t pin_DREQ)
{
    return rt_pin_read(pin_DREQ);
}


static void VS1003_XCS_set(rt_base_t pin_XCS, rt_base_t value)
{
    rt_pin_write(pin_XCS, value);
}


static void VS1003_XDCS_set(rt_base_t pin_XDCS, rt_base_t value)
{
    rt_pin_write(pin_XDCS, value);
}


static void VS1003_XRESET_set(rt_base_t pin_XRESET, rt_base_t value)
{
    rt_pin_write(pin_XRESET, value);
}

static rt_err_t VS1003_wait_DREQ(VS1003_DEVICE * VS1003)
{
    int cun = 0;

    while(0 == VS1003_DREQ_state(VS1003->config->pin_DREQ))
    {
        rt_thread_mdelay(10);
        cun++;
        if(10 <= cun)
        {
            return -RT_ERROR;
        }
    }

    return RT_EOK;
}


static rt_err_t VS1003_WR_Cmd(VS1003_DEVICE * VS1003, unsigned char addr, unsigned short data)
{
    unsigned char buf[4] = {0};

    VS1003_XCS_set(VS1003->config->pin_XCS, PIN_HIGH); 

    if(RT_EOK != VS1003_wait_DREQ(VS1003)) 
    {
        return -RT_ERROR;
    }

    buf[0] = VS_WRITE_COMMAND;
    buf[1] = addr;
    buf[2] = (unsigned char)(data >> 8);
    buf[3] = (unsigned char)data;

    VS1003_XDCS_set(VS1003->config->pin_XDCS, PIN_HIGH);
    VS1003_XCS_set(VS1003->config->pin_XCS, PIN_LOW);

    rt_spi_send(VS1003->spi_dev, buf, 4); 

    VS1003_XCS_set(VS1003->config->pin_XCS, PIN_HIGH);

    return RT_EOK;
}


static rt_err_t VS1003_RD_Reg(VS1003_DEVICE * VS1003, unsigned char addr, unsigned short *data)
{
    unsigned char buf[4] = {0};

    VS1003_XCS_set(VS1003->config->pin_XCS, PIN_HIGH);

    if(RT_EOK != VS1003_wait_DREQ(VS1003)) 
    {
        return -RT_ERROR;
    }

    buf[0] = VS_READ_COMMAND;
    buf[1] = addr;

    VS1003_XDCS_set(VS1003->config->pin_XDCS, PIN_HIGH);
    VS1003_XCS_set(VS1003->config->pin_XCS, PIN_LOW);

    rt_spi_send_then_recv(VS1003->spi_dev, buf, 2, &buf[2], 2);

    *data = (buf[2] << 8) | buf[3];

    VS1003_XCS_set(VS1003->config->pin_XCS, PIN_HIGH);

    return RT_EOK;
}


static rt_err_t VS1003_set_param(VS1003_DEVICE * VS1003, 
                                 unsigned char addr,
                                 unsigned short des_data,
                                 unsigned short set_data)
{
    unsigned char retry = 10;
    unsigned short data;

    while(retry--)
    {
        if(RT_EOK != VS1003_RD_Reg(VS1003, addr, &data))
        {
            return -RT_ERROR;
        }

        if(des_data != data)
        {
            if(RT_EOK != VS1003_WR_Cmd(VS1003, addr, set_data))
            {
                return -RT_ERROR;
            }

            rt_thread_mdelay(2);

        }
        else
        {
            return RT_EOK;
        }

    }

    return -RT_ERROR;
}



static rt_err_t VS1003_Soft_Reset(VS1003_DEVICE * VS1003)
{
    unsigned char buf = 0xFF;

    VS1003_XCS_set(VS1003->config->pin_XCS, PIN_HIGH);

    if(RT_EOK != VS1003_wait_DREQ(VS1003))
    {
        return -RT_ERROR;
    }

    rt_spi_send(VS1003->spi_dev, &buf, 1);


    if(RT_EOK != VS1003_set_param(VS1003, SPI_MODE, 0x0800, 0x0804))
    {
        return -RT_ERROR;
    }

    if(RT_EOK != VS1003_wait_DREQ(VS1003))
    {
        return -RT_ERROR;
    }


    if(RT_EOK != VS1003_set_param(VS1003, SPI_CLOCKF, 0X9800, 0X9800))
    {
        return -RT_ERROR;
    }

    if(RT_EOK != VS1003_set_param(VS1003, SPI_AUDATA, 0XBB81, 0XBB81))
    {
        return -RT_ERROR;
    }

    if(RT_EOK != VS1003_set_param(VS1003, SPI_BASS, 0, 0))
    {
        return -RT_ERROR;
    }

    rt_thread_mdelay(10);

    return RT_EOK;
}


static rt_err_t VS1003_Hard_Reset(VS1003_DEVICE * VS1003)
{
    VS1003_XRESET_set(VS1003->config->pin_XRESET, PIN_LOW);
    rt_thread_mdelay(5);
    VS1003_XCS_set(VS1003->config->pin_XRESET, PIN_HIGH);
    VS1003_XDCS_set(VS1003->config->pin_XRESET, PIN_HIGH);
    VS1003_XRESET_set(VS1003->config->pin_XRESET, PIN_HIGH);

    rt_thread_mdelay(5);

    if(RT_EOK != VS1003_wait_DREQ(VS1003))
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}


rt_err_t VS1003_Sine_Test(VS1003_DEVICE * VS1003, 
        unsigned char Sin_Freq_1,
        unsigned char Sin_Freq_2)
{
    unsigned char buf_1[8] = {0x53, 0xef, 0x6e, 0x24, 0x00, 0x00, 0x00, 0x00};
    unsigned char buf_2[8] = {0x45, 0x78, 0x69, 0x74, 0x00, 0x00, 0x00, 0x00};

    // 第一次正弦音
    buf_1[3] = Sin_Freq_1;
    if(RT_EOK != VS1003_Soft_Reset(VS1003)) //
    {
        return -RT_ERROR;
    }

    if(RT_EOK != VS1003_WR_Cmd(VS1003, SPI_VOL, 0X0A0A))
    {
        return -RT_ERROR;
    }

    if(RT_EOK != VS1003_WR_Cmd(VS1003, SPI_MODE, 0x0820))
    {
        return -RT_ERROR;
    }

    if(RT_EOK != VS1003_wait_DREQ(VS1003))
    {
        return -RT_ERROR;
    }

    VS1003_XDCS_set(VS1003->config->pin_XDCS, PIN_LOW);
    rt_spi_send(VS1003->spi_dev, buf_1, 8);
    rt_thread_mdelay(100);
    VS1003_XDCS_set(VS1003->config->pin_XDCS, PIN_HIGH);

    //退出正弦测试
    VS1003_XDCS_set(VS1003->config->pin_XDCS, PIN_LOW);
    rt_spi_send(VS1003->spi_dev, buf_2, 8);
    rt_thread_mdelay(100);
    VS1003_XDCS_set(VS1003->config->pin_XDCS, PIN_HIGH);

    // 第二次正弦音
    buf_1[3] = Sin_Freq_2;
    if(RT_EOK != VS1003_Soft_Reset(VS1003)) //
    {
        return -RT_ERROR;
    }

    if(RT_EOK != VS1003_WR_Cmd(VS1003, SPI_VOL, 0X0A0A))
    {
        return -RT_ERROR;
    }

    if(RT_EOK != VS1003_WR_Cmd(VS1003, SPI_MODE, 0x0820))
    {
        return -RT_ERROR;
    }

    if(RT_EOK != VS1003_wait_DREQ(VS1003))
    {
        return -RT_ERROR;
    }

    VS1003_XDCS_set(VS1003->config->pin_XDCS, PIN_LOW);
    rt_spi_send(VS1003->spi_dev, buf_1, 8);
    rt_thread_mdelay(100);
    VS1003_XDCS_set(VS1003->config->pin_XDCS, PIN_HIGH);

    //退出正弦测试
    VS1003_XDCS_set(VS1003->config->pin_XDCS, PIN_LOW);
    rt_spi_send(VS1003->spi_dev, buf_2, 8);
    rt_thread_mdelay(100);
    VS1003_XDCS_set(VS1003->config->pin_XDCS, PIN_HIGH);

    return RT_EOK;

}


rt_err_t VS1003_Set_Vol(VS1003_DEVICE * VS1003, unsigned short vol)
{
    if(RT_EOK != VS1003_WR_Cmd(VS1003, SPI_VOL, vol))
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}


void VS1003_Send_data(VS1003_DEVICE * VS1003, unsigned char *data, unsigned char data_len)
{
    VS1003_XCS_set(VS1003->config->pin_XCS, PIN_HIGH);
    VS1003_XDCS_set(VS1003->config->pin_XDCS, PIN_LOW);

    rt_spi_send(VS1003->spi_dev, data, data_len);

    VS1003_XDCS_set(VS1003->config->pin_XDCS, PIN_HIGH);
}


static rt_err_t VS1003_Play_End(VS1003_DEVICE * VS1003, unsigned short *data)
{
    unsigned short temp;

    if(RT_EOK != VS1003_RD_Reg(VS1003, SPI_HDAT0, &temp))
    {
        return -RT_ERROR;
    }

    *data = temp;

    if(RT_EOK != VS1003_RD_Reg(VS1003, SPI_HDAT1, &temp))
    {
        return -RT_ERROR;
    }

    *data += temp;

    return RT_EOK;
}


static rt_err_t VS1003_WRAM_Read(VS1003_DEVICE * VS1003, unsigned short addr, unsigned short *data)
{
    if(RT_EOK != VS1003_WR_Cmd(VS1003, SPI_WRAMADDR, addr))
    {
        return -RT_ERROR;
    }

    if(RT_EOK != VS1003_RD_Reg(VS1003, SPI_WRAM, data))
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}


rt_err_t VS1003_WRAM_Write(VS1003_DEVICE * VS1003, unsigned short addr, unsigned short data)
{
    if(RT_EOK != VS1003_WR_Cmd(VS1003, SPI_WRAMADDR, addr))
    {
        return -RT_ERROR;
    }

    if(RT_EOK != VS1003_wait_DREQ(VS1003))
    {
        return -RT_ERROR;
    }

    if(RT_EOK != VS1003_WR_Cmd(VS1003, SPI_WRAM, data))
    {
        return -RT_ERROR;
    }

    return RT_EOK;

}

static rt_err_t VS1003_Get_EndFillByte(VS1003_DEVICE * VS1003, unsigned short *data)
{
    if(RT_EOK != VS1003_WRAM_Read(VS1003, 0X1E06, data))
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

rt_err_t VS1003_Restart_Play(VS1003_DEVICE * VS1003)
{
    unsigned short temp;
    unsigned short i;
    unsigned char n;
    unsigned char vsbuf[32] = {0};

    if(RT_EOK != VS1003_RD_Reg(VS1003, SPI_MODE, &temp))
    {
        return -RT_ERROR;
    }

    temp |= 1 << 3;
    temp |= 1 << 2;

    if(RT_EOK != VS1003_WR_Cmd(VS1003, SPI_MODE, temp))
    {
        return -RT_ERROR;
    }

    for (i = 0; i < 2048;)
    {
        if(RT_EOK != VS1003_wait_DREQ(VS1003))
        {
            return -RT_ERROR;
        }

        VS1003_Send_data(VS1003, vsbuf, 32);
        i += 32;

        if(RT_EOK != VS1003_RD_Reg(VS1003, SPI_MODE, &temp))
        {
            return -RT_ERROR;
        }
        if ((temp & (1 << 3)) == 0)
            break;
    }

    if (i < 2048)
    {
        if(RT_EOK != VS1003_Get_EndFillByte(VS1003, &temp))
        {
            return -RT_ERROR;
        }
        temp &= 0xFF;
        for (n = 0; n < 32; n++)
            vsbuf[n] = temp;

        for (i = 0; i < 2052;)
        {
            if(RT_EOK != VS1003_wait_DREQ(VS1003))
            {
                return -RT_ERROR;
            }
            VS1003_Send_data(VS1003, vsbuf, 32);
            i += 32;
        }
    }
    else
    {
        if(RT_EOK != VS1003_Soft_Reset(VS1003))
        {
            return -RT_ERROR;
        }
    }

    if(RT_EOK != VS1003_Play_End(VS1003, &temp))
    {
        return -RT_ERROR;
    }

    if (temp)
    {
        if(RT_EOK != VS1003_Hard_Reset(VS1003))
        {
            return -RT_ERROR;
        }


        if(RT_EOK != VS1003_Soft_Reset(VS1003))
        {
            return -RT_ERROR;
        }
    }

    return RT_EOK;
}


rt_err_t VS1003_init(VS1003_DEVICE * VS1003_dev, VS1003_CONFIG * config)
{
    rt_err_t res = RT_EOK;

    RT_ASSERT(VS1003_dev != RT_NULL);
    RT_ASSERT(config != RT_NULL);

    if(RT_NULL != VS1003_dev->config)
    {
        return -RT_ERROR;
    }

    VS1003_dev->config = config;


#if (SPI_DEVICE_ATTACH_MODE == SPI_DEVICE_ATTACH_GPIO_TYPE)
    res = rt_hw_spi_device_attach(VS1003_dev->config->spi_device_name, 
            VS1003_dev->config->name,
            VS1003_dev->config->GPIO_CS_Port,
            VS1003_dev->config->GPIO_CS_Pin);
#elif (SPI_DEVICE_ATTACH_MODE == SPI_DEVICE_ATTACH_PIN_NUM)
    res = rt_hw_spi_device_attach(VS1003_dev->config->spi_device_name, 
            VS1003_dev->config->name,
            VS1003_dev->config->pin_spi_CS);
#endif
    
    if (res != RT_EOK)
    {
        VS1003_printf("VS1003 Failed to attach device %s\n", VS1003_dev->config->name);
        return res;
    }

    if(RT_NULL != VS1003_dev->config->pin_clk_enable)
    {
        VS1003_dev->config->pin_clk_enable();
    }

    rt_pin_mode(VS1003_dev->config->pin_DREQ, PIN_MODE_INPUT);

    rt_pin_mode(VS1003_dev->config->pin_XCS, PIN_MODE_OUTPUT);
    rt_pin_write(VS1003_dev->config->pin_XCS, PIN_HIGH);

    rt_pin_mode(VS1003_dev->config->pin_XDCS, PIN_MODE_OUTPUT);
    rt_pin_write(VS1003_dev->config->pin_XDCS, PIN_HIGH);

    rt_pin_mode(VS1003_dev->config->pin_XRESET, PIN_MODE_OUTPUT);
    rt_pin_write(VS1003_dev->config->pin_XRESET, PIN_HIGH);

    VS1003_dev->spi_dev = (struct rt_spi_device *)rt_device_find(VS1003_dev->config->name);
    if (!VS1003_dev->spi_dev)
    {
        VS1003_printf("spi find run failed! cant't find %s device!\n", VS1003_dev->config->name);
        return -RT_ENOSYS;
    }

#ifdef RT_SPI_MASTER
    //Set device SPI Mode
    struct rt_spi_configuration cfg = {0};
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER |RT_SPI_MODE_0 | RT_SPI_MSB ;
    cfg.max_hz = 8000000;
    rt_spi_configure(VS1003_dev->spi_dev, &cfg);

    VS1003_printf("VS1003 %s init ok\n", VS1003_dev->config->name);
#endif

    return res;
}

#endif /* BSP_USING_SPI1 || BSP_USING_SPI2 || BSP_USING_SPI3 || BSP_USING_SPI4 || BSP_USING_SPI5 */

#endif  // PKG_USING_VS1003

