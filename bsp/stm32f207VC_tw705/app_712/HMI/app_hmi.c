/*
     HMI  :  Process  LCD  ��Printer �� KeyChecking
*/

#include <rtthread.h>
#include <rthw.h>
#include "stm32f2xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>

#include  <stdlib.h>//����ת�����ַ���
#include  <stdio.h>
#include  <string.h>
#include "App_moduleConfig.h"
#include "spi_sd.h"
#include "Usbh_conf.h"
#include <dfs_posix.h>


unsigned char dayin_data_time[35] = {"\r\n��ӡʱ��:                    \r\n"};
unsigned char dayin_chepaihaoma[26] = {"\r\n���������ƺ���:00000000"};
unsigned char dayin_chepaifenlei[25] = {"\r\n���������Ʒ���:000000"};
unsigned char dayin_cheliangVIN[32] = {"\r\n����VIN:00000000000000000"};
//unsigned char dayin_driver_NUM[40]={"��ʻԱ����:000000000000000000000"};
unsigned char dayin_driver_NUM[40] = {"\r\n��ʻԱ����:000000"};
unsigned char dayin_driver_card[50] = {"\r\n��������ʻ֤����:\r\n   000000000000000000"};
static struct rt_messagequeue	HMI_MsgQue;

void Dayin_Fun(u8 dayin_par)
{
    u8 datatime_str[6];
    u8  drive_illegalstr[666];
    u32  current_u32time = 0; //  ��ǰ��ʱ��
    u32  old2day_u32time = 0; //  ǰ2���������ʱ��    current-2x86400 (172800)
    u32  read_u32time = 0;
    u32  read_u32_ENDTime = 0; // ��ȡ��¼�н���ʱ��
    u8  i = 0, efftive_counter = 0, check_limit = 0; // check_limit   ��ʾ��Ҫ������¼����Ŀ
    u8  print_buf[70];
    u8  leftminute = 0; // ��ǰ������ֵ
    u8  find_chaoshi_record = 0; //

    if(dayin_par == 1)
    {
        memcpy(dayin_chepaihaoma + 17, Vechicle_Info.Vech_Num, 8); //  2
        memcpy(dayin_chepaifenlei + 17, Vechicle_Info.Vech_Type, strlen(Vechicle_Info.Vech_Type)); //  3
        memcpy(dayin_cheliangVIN + 10, Vechicle_Info.Vech_VIN, 17); //  4
        memcpy(dayin_driver_NUM + 13, JT808Conf_struct.Driver_Info.DriveName, 21); //5
        memcpy(dayin_driver_card + 24, Read_ICinfo_Reg.DriverCard_ID, 18); //6
        memcpy((char *)dayin_data_time + 11, (char *)Dis_date, 20); //7
        switch(DaYin)
        {
        case 1:
            if(step(50, 1000))
            {
                Menu_txt_state = 1;
                pMenuItem = &Menu_TXT;
                pMenuItem->show();
                pMenuItem->timetick( 10 );
                pMenuItem->keypress( 10 );

                //-------------------------
                DaYin = 0;
                print_rec_flag = 0;
                GPIO_ResetBits(GPIOB, GPIO_Pin_7); //��ӡ�ص�

                //-----------------------------------------------------
                print_workingFlag = 0; // ��ӡ״̬������

                //-----------------------------------------------
            }
            else
            {
                Menu_txt_state = 5;
                pMenuItem = &Menu_TXT;
                pMenuItem->show();
                pMenuItem->timetick( 10 );
                pMenuItem->keypress( 10 );
                //--------------------------------------------
                DaYin++;
                GPIO_SetBits( GPIOB, GPIO_Pin_7 );
            }
            break;
        case 2://���ƺ��� 9
            printer((const char *)dayin_chepaihaoma);
            printer((const char *)dayin_chepaifenlei);
            printer((const char *)dayin_cheliangVIN);
            printer((const char *)dayin_driver_card);
            if((VdrData.H_15[0] == 0x02) && (Limit_max_SateFlag == 0))
                printer("\r\n�ٶ�״̬: �쳣");
            else
                printer("\r\n�ٶ�״̬: ����");
            printer((const char *)dayin_data_time);//00/00/00 00:00:00
            printer("2���������ڳ�ʱ��ʻ��¼:\r\n");
            time_now = Get_RTC(); 	//	RTC  ���
            Time2BCD(datatime_str);
            current_u32time = Time_sec_u32(datatime_str, 6);
            old2day_u32time = current_u32time - 172800; // 2���������ڵ�ʱ��
            if(Vdr_Wr_Rd_Offset.V_11H_full)
                check_limit = VDR_11_MAXindex;
            else
                check_limit = Vdr_Wr_Rd_Offset.V_11H_Write;
            if(check_limit)
            {
                for(i = 0; i < check_limit; i++)
                {
                    memset(drive_illegalstr, 0, sizeof(drive_illegalstr));
                    if(get_11h(i, drive_illegalstr) == 0)							//50  packetsize	  num=100
                        continue;
                    read_u32time = Time_sec_u32(drive_illegalstr + 18, 6); // ������ʻ��ʼʱ��
                    read_u32_ENDTime = Time_sec_u32(drive_illegalstr + 24, 6); // ������ʻ����ʱ��
                    if(read_u32time >= old2day_u32time)
                    {
                        if((read_u32_ENDTime > read_u32time) && ((read_u32_ENDTime - read_u32time) > (4 * 60 * 60)))
                        {
                            //  ����ʱ�������ʼʱ�䣬�Ҳ�ֵ����4��Сʱ
                            efftive_counter++;
                            memset(print_buf, 0, sizeof(print_buf));
                            sprintf(print_buf, "\r\n��¼ %d:", efftive_counter);
                            printer(print_buf);
                            memcpy(dayin_driver_card + 24, drive_illegalstr, 18); //6
                            printer((const char *)dayin_driver_card);
                            memset(print_buf, 0, sizeof(print_buf));
                            sprintf(print_buf, "\r\n ������ʻ��ʼʱ��: \r\n  20%2d-%d%d-%d%d %d%d:%d%d:%d%d", BCD2HEX(drive_illegalstr[18]), \
                                    BCD2HEX(drive_illegalstr[19]) / 10, BCD2HEX(drive_illegalstr[19]) % 10, BCD2HEX(drive_illegalstr[20]) / 10, BCD2HEX(drive_illegalstr[20]) % 10, \
                                    BCD2HEX(drive_illegalstr[21]) / 10, BCD2HEX(drive_illegalstr[21]) % 10, BCD2HEX(drive_illegalstr[22]) / 10, BCD2HEX(drive_illegalstr[22]) % 10, \
                                    BCD2HEX(drive_illegalstr[23]) / 10, BCD2HEX(drive_illegalstr[23]) % 10);
                            printer(print_buf);
                            memset(print_buf, 0, sizeof(print_buf));
                            sprintf(print_buf, "\r\n ������ʻ����ʱ��: \r\n  20%2d-%d%d-%d%d %d%d:%d%d:%d%d", BCD2HEX(drive_illegalstr[24]), \
                                    BCD2HEX(drive_illegalstr[25]) / 10, BCD2HEX(drive_illegalstr[25]) % 10, BCD2HEX(drive_illegalstr[26]) / 10, BCD2HEX(drive_illegalstr[26]) % 10, \
                                    BCD2HEX(drive_illegalstr[27]) / 10, BCD2HEX(drive_illegalstr[27]) % 10, BCD2HEX(drive_illegalstr[28]) / 10, BCD2HEX(drive_illegalstr[28]) % 10, \
                                    BCD2HEX(drive_illegalstr[29]) / 10, BCD2HEX(drive_illegalstr[29]) % 10);
                            printer(print_buf);
                            find_chaoshi_record = enable; // find  record
                        }

                    }
                }

                if(find_chaoshi_record == 0)
                    printer("\r\n�޳�ʱ��ʻ��¼\r\n");
            }
            else
                printer("\r\n�޳�ʱ��ʻ��¼\r\n");
            // ���15���� ƽ���ٶ�
            printer("\r\nͣ��ǰ15����ÿ����ƽ���ٶ�:");
            memset(drive_illegalstr, 0, sizeof(drive_illegalstr));
            leftminute = Api_avrg15minSpd_Content_read(drive_illegalstr);
            if(leftminute == 0)
                printer("\r\n �������15����ͣ����¼");
            else if(leftminute == 105)
            {
                for(i = 0; i < 15; i++)
                {
                    memset(print_buf, 0, sizeof(print_buf));
                    if((drive_illegalstr[i * 7 + 0] == 0) && (drive_illegalstr[i * 7 + 1] == 0) &&
                            (drive_illegalstr[i * 7 + 2] == 0) && (drive_illegalstr[i * 7 + 3] == 0) &&
                            (drive_illegalstr[i * 7 + 4] == 0) && (drive_illegalstr[i * 7 + 5] == 0))
                    {
                        //U3_PutStr("\r\n   NULL");
                        continue;
                    }

                    if((drive_illegalstr[i * 7 + 0] == 0xFF) && (drive_illegalstr[i * 7 + 1] == 0xFF) &&
                            (drive_illegalstr[i * 7 + 2] == 0xFF) && (drive_illegalstr[i * 7 + 3] == 0xFF) &&
                            (drive_illegalstr[i * 7 + 4] == 0xFF) && (drive_illegalstr[i * 7 + 5] == 0xFF))
                    {
                        //U3_PutStr("\r\n   NULL");
                        continue;
                    }

                    sprintf(print_buf, "\r\n  20%02d-%02d-%02d %02d:%02d  %d km/h", drive_illegalstr[i * 7], \
                            drive_illegalstr[i * 7 + 1], drive_illegalstr[i * 7 + 2], drive_illegalstr[i * 7 + 3], drive_illegalstr[i * 7 + 4], \
                            drive_illegalstr[i * 7 + 6]);
                    printer(print_buf);
                }
            }
            printer("\r\n                        \r\n");
            printer("\r\n\r\n ǩ�� :   ______________\r\n");
            DaYin++;
            break;
        case 3:
            step(50, 1000);
            DaYin++;
            break;
        case 4:
            step(50, 1000);
            DaYin++;
            break;
        case 5:
            DaYin = 0;
            print_rec_flag = 0;
            GPIO_ResetBits( GPIOB, GPIO_Pin_7 );
            print_workingFlag = 0; // ��ӡ״̬������

            pMenuItem = &Menu_1_Idle;
            pMenuItem->show();
            break;
        }

    }
}



/* HMI  thread */
ALIGN(RT_ALIGN_SIZE)
char HMI_thread_stack[2048]; // 4096
struct rt_thread HMI_thread;

static void HMI_thread_entry(void *parameter)
{
    u8 counter_printer = 0;

    TIRED_DoorValue_Init();
    Init_4442();
    IC_info_default();



    //--------�Լ�������DF  ��ȡ-----
    DF_init();
    SysConfiguration();    // system config
  //  total_ergotic();
    DF_initOver = 1;
    Gsm_RegisterInit(); 	//	init register states	,then  it  will  power on  the module
    SIMID_Convert_SIMCODE(); //   translate
    
	pMenuItem = &Menu_0_0_self_Checking;
	pMenuItem->show();
	pMenuItem->timetick(10);
	pMenuItem->keypress(10);
	
    while (1)
    {

        // 6.   ACC ״̬���
        ACC_status_Check();
        //--------------------------------------------
        rt_thread_delay(25);     //25
    }
}



/* init HMI  */
void HMI_app_init(void)
{
    rt_err_t result;


    result = rt_thread_init(&HMI_thread, "HMI",
                            HMI_thread_entry, RT_NULL,
                            &HMI_thread_stack[0], sizeof(HMI_thread_stack),
                            Prio_HMI, 6);

    if (result == RT_EOK)
    {
        rt_thread_startup(&HMI_thread);
    }
}



