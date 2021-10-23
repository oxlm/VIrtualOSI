/*
 * Copyright (c) 2020-2021, Bluetrum Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021/10/23     oxlm         adc + uart OK
 */

/**
 * Notice!
 * All functions or data that are called during an interrupt need to be in RAM.
 * You can do it the way exception_isr() does.
 */

#include <rtthread.h>
#include "board.h"
#include "rtdevice.h"
#include <string.h>
#include <stdio.h>

#define ADC_DEV_NAME        "adc0"      /* ADC 设备名称 */
#define ADC_DEV_CHANNEL     0           /* ADC 通道 */
#define REFER_VOLTAGE       330         /* 参考电压 3.3V,数据精度乘以100保留2位小数*/
#define CONVERT_BITS        (1 << 10)   /* 转换位数为10位 */

static rt_adc_device_t adc_dev = RT_NULL;                /* ADC 设备句柄 */


#define SAMPLE_UART_NAME       "uart1"    /* 串口设备名称 */
#define UART_BUF_SIZE          (128)

static rt_device_t serial;                /* 串口设备句柄 */
static struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;  /* 初始化配置参数 */

static void adc_init(void)
{
    adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
    if(adc_dev == RT_NULL)
    {
        rt_kprintf("No device named %s\n", ADC_DEV_NAME);
        return;
    }

    if(rt_adc_enable(adc_dev, ADC_DEV_CHANNEL) != RT_EOK)
    {
        rt_kprintf("ADC channel %d didn't exist\n", ADC_DEV_CHANNEL);
        return;
    }
}

void uart_init(void)
{
    serial = rt_device_find(SAMPLE_UART_NAME);

    config.baud_rate = BAUD_RATE_115200;
    config.data_bits = DATA_BITS_8;
    config.stop_bits = STOP_BITS_1;
    config.bufsz     = UART_BUF_SIZE;
    config.parity    = PARITY_NONE;

    rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config);

    rt_device_open(serial, RT_DEVICE_FLAG_INT_RX);
}

int main(void)
{
    rt_uint32_t value, vol;
    char str[UART_BUF_SIZE];

    adc_init();
    uart_init();
    while(1)
    {
        /* 读取采样值 */
        value = rt_adc_read(adc_dev, ADC_DEV_CHANNEL);

        /* 转换为对应电压值 */
        vol = value * REFER_VOLTAGE / CONVERT_BITS;
        //rt_kprintf("the voltage is :%d.%02d \n", vol / 100, vol % 100);
        memset(str, 0x00, UART_BUF_SIZE);
        snprintf(str, UART_BUF_SIZE, "$%d;", value);
        rt_device_write(serial, 0, str, (sizeof(str) - 1));
        rt_thread_mdelay(5);
    }
}
