#include <rtthread.h>
#include <stm32f2xx.h>
#include "rtc.h"


__IO uint32_t AsynchPrediv = 0, SynchPrediv = 0;
RTC_TimeTypeDef RTC_TimeStructure;
RTC_InitTypeDef RTC_InitStructure;
RTC_DateTypeDef RTC_DateStructure;

TDateTime time_now;



static struct rt_device rtc;
static rt_err_t rt_rtc_open(rt_device_t dev, rt_uint16_t oflag)
{
    if (dev->rx_indicate != RT_NULL)
    {
        /* Open Interrupt */
    }

    return RT_EOK;
}

static rt_size_t rt_rtc_read(rt_device_t dev, rt_off_t pos, void *buffer, rt_size_t size)
{
    return 0;
}

static rt_err_t rt_rtc_control(rt_device_t dev, rt_uint8_t cmd, void *args)
{
    //rt_time_t *time;
    RT_ASSERT(dev != RT_NULL);

    switch (cmd)
    {
    case RT_DEVICE_CTRL_RTC_GET_TIME:
        // time = (rt_time_t *)args;
        /* read device */
        RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
        break;

    case RT_DEVICE_CTRL_RTC_SET_TIME:
    {
        //        time = (rt_time_t *)args;

        /* Enable PWR and BKP clocks */
        //RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
        /* Allow access to BKP Domain */
        //PWR_BackupAccessCmd(ENABLE);
        PWR_BackupAccessCmd(ENABLE);
        /* Wait until last write operation on RTC registers has finished */
        //RTC_WaitForLastTask();
        RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);

        RTC_WaitForSynchro();

        /* Change the current time */
        //RTC_SetCounter(*time);

        /* Wait until last write operation on RTC registers has finished */
        //RTC_WaitForLastTask();

    }
    break;
    case RT_DEVICE_CTRL_RTC_SET_DATE:
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
        PWR_BackupAccessCmd(ENABLE);
        RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure);

        RTC_WaitForSynchro();
    }
    break;
    }

    return RT_EOK;
}


/*******************************************************************************
* Function Name  : RTC_Configuration
* Description    : Configures the RTC.
* Input          : None
* Output         : None
* Return         : 0 reday,-1 error.
*******************************************************************************/
int RTC_Config(void)
{
    u32 count = 0x250000;
    /* Enable the PWR clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    /* Allow access to RTC */
    PWR_BackupAccessCmd(ENABLE);

    RCC_LSEConfig(RCC_LSE_ON);

    /* Wait till LSE is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET && (--count) );
    if ( count == 0 )
    {
        return -1;
    }

    /* Select the RTC Clock Source */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

    SynchPrediv = 0xFF;
    AsynchPrediv = 0x7F;

    /* Enable the RTC Clock */
    RCC_RTCCLKCmd(ENABLE);

    /* Wait for RTC APB registers synchronisation */
    RTC_WaitForSynchro();
    return 0;
}

u8 RT_Total_Config(void)
{
    u8  Resualt = 0;
    //����RTC���ݶ�ʧ����Ҫ��������
    if (RTC_ReadBackupRegister(RTC_BKP_DR0) != RTC_First)
    {
        rt_kprintf("rtc is not configured\n");
        //��������RTC
        RTC_Config();
        // Check  resualt
        if ( RTC_Config() != 0)
        {
            RTC_Config();
            rt_kprintf("rtc configure fail...Reconfig once\r\n");
            return  0 ;
        }
        else
        {
            /* Configure the RTC data register and RTC prescaler */
            RTC_InitStructure.RTC_AsynchPrediv = AsynchPrediv;
            RTC_InitStructure.RTC_SynchPrediv = SynchPrediv;
            RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;

            /* Check on RTC init */
            if (RTC_Init(&RTC_InitStructure) == ERROR)
            {
                rt_kprintf("\n\r  /!\\***** RTC Prescaler Config failed ********/!\\ \n\r");
            }
        }
        //������ɺ���󱸼Ĵ�����д�����ַ�0xA5A5
        RTC_WriteBackupRegister(RTC_BKP_DR0, RTC_First);

        Resualt = 1;
    }
    else
    {

        /* Check if the Power On Reset flag is set */
        if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != 0)
        {
            ;// rt_kprintf("\r\n Power On Reset occurred....\n\r");
        }
        /* Check if the Pin Reset flag is set */
        else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != 0)
        {
            ;//rt_kprintf("\r\n External Reset occurred....\n\r");
        }

        // rt_kprintf("\r\n No need to configure RTC....\n\r");

        /* Enable the PWR clock */
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

        /* Allow access to RTC */
        PWR_BackupAccessCmd(ENABLE);

        /* Wait for RTC APB registers synchronisation */
        RTC_WaitForSynchro();

        /* Clear the RTC Alarm Flag */
        RTC_ClearFlag(RTC_FLAG_ALRAF);

        /* Wait for RTC registers synchronization */
        RTC_WaitForSynchro();
        Resualt = 0;
    }
    return Resualt;
}

/***********************************************RTCע��******************************************************/
u8  rt_hw_rtc_init(void)
{
    u8  Resualt = 0;

    rtc.type	= RT_Device_Class_RTC;

    //����RTC���ݶ�ʧ����Ҫ��������
    Resualt = RT_Total_Config();

    /* register rtc device */
    rtc.init 	= RT_NULL;
    rtc.open 	= rt_rtc_open;
    rtc.close	= RT_NULL;
    rtc.read 	= rt_rtc_read;
    rtc.write	= RT_NULL;
    rtc.control = rt_rtc_control;

    /* no private */
    //rtc.user_data = RT_NULL;

    rt_device_register(&rtc, "rtc", RT_DEVICE_FLAG_RDWR);

    return  Resualt;
}

/**********************************************************�ɵ��ñ�׼�����ӿ�*****************************************/

void set_time(rt_uint32_t hour, rt_uint32_t minute, rt_uint32_t second)
{
    rt_device_t device;

    RTC_TimeStructure.RTC_Hours = hour;
    RTC_TimeStructure.RTC_Minutes 	= minute;
    RTC_TimeStructure.RTC_Seconds 	= second;


    device = rt_device_find("rtc");
    if (device != RT_NULL)
    {
        rt_rtc_control(device, RT_DEVICE_CTRL_RTC_SET_TIME, &RTC_TimeStructure);
    }
}

void set_date(rt_uint32_t year, rt_uint32_t month, rt_uint32_t date)
{

    rt_device_t device;

    RTC_DateStructure.RTC_Year = year;
    RTC_DateStructure.RTC_Month 	= month;
    RTC_DateStructure.RTC_Date 	= date;


    device = rt_device_find("rtc");
    if (device != RT_NULL)
    {
        rt_rtc_control(device, RT_DEVICE_CTRL_RTC_SET_DATE, &RTC_TimeStructure);
    }

}

u8  Device_RTC_set(TDateTime now)
{
    rt_device_t device;

    RTC_DateStructure.RTC_Year = now.year;
    RTC_DateStructure.RTC_Month = now.month;
    RTC_DateStructure.RTC_Date = now.day;
    RTC_DateStructure.RTC_WeekDay = now.week;



    RTC_TimeStructure.RTC_Hours = now.hour;
    RTC_TimeStructure.RTC_Minutes = now.min;
    RTC_TimeStructure.RTC_Seconds 	= now.sec;

    device = rt_device_find("rtc");
    if (device != RT_NULL)
    {
        rt_rtc_control(device, RT_DEVICE_CTRL_RTC_SET_DATE, &RTC_DateStructure);

        rt_rtc_control(device, RT_DEVICE_CTRL_RTC_SET_TIME, &RTC_TimeStructure);

        return 1;

    }

    return 0;
}


TDateTime  Get_RTC(void)
{
    TDateTime  tm;
    RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
    tm.hour = RTC_TimeStructure.RTC_Hours;
    tm.min = RTC_TimeStructure.RTC_Minutes;
    tm.sec = RTC_TimeStructure.RTC_Seconds;
    RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
    tm.year = RTC_DateStructure.RTC_Year;
    tm.month = RTC_DateStructure.RTC_Month;
    tm.day = RTC_DateStructure.RTC_Date;
    tm.week = RTC_DateStructure.RTC_WeekDay;

    return tm;
}

u8  Set_RTC( TDateTime now)
{

    RTC_TimeStructure.RTC_Hours = now.hour;
    RTC_TimeStructure.RTC_Minutes = now.min;
    RTC_TimeStructure.RTC_Seconds = now.sec;
    if( RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure) == 0)
        return  0;
    RTC_DateStructure.RTC_Year = now.year;
    RTC_DateStructure.RTC_Month = now.month;
    RTC_DateStructure.RTC_Date = now.day;
    RTC_DateStructure.RTC_WeekDay = now.week;

    if( RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure) == 0)
        return 0;
    else
        return 1;

}






