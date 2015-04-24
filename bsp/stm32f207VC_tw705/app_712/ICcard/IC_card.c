/*
       IC  card
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
#include  <ctype.h>
#include  <stdlib.h>
#include  <stdarg.h>
#include "App_moduleConfig.h"
#include "Vdr.h"
#include"IS2401.h"


u8   IC_loginState = 0; //  IC  ��������ȷʶ��:   1    ����   0
unsigned char IC_CardInsert = 0; //1:IC��������ȷ  2:IC���������
unsigned char IC_Check_Count = 0;
unsigned int      read_counter = 0, flag_8024off = 1;
unsigned char   Init8024Flag = 0;
unsigned int      DelayCheckIc = 0;
unsigned char administrator_card = 0;
u8        powerOn_first = 0; //    �״��ϵ���жϰο�

// -----IC  ����Ϣ���---
DRV_INFO    Read_ICinfo_Reg;   // ��ʱ��ȡ��IC ����Ϣ
DRIVE_STRUCT     Drivers_struct[MAX_DriverIC_num]; // Ԥ��5 ����ʻԱ�Ĳ忨�Ա�




void KeyBuzzer(unsigned char num)
{

    if(num == 1)
    {
        BuzzerFlag++;
        if(BuzzerFlag == 2)
            buzzer_onoff(1);
        if(BuzzerFlag == 4)
        {
            buzzer_onoff(0);
            BuzzerFlag = 0;
            IC_CardInsert = 0;
        }

    }
    else if(num == 2)
    {
        BuzzerFlag++;
        if((BuzzerFlag == 12) || (BuzzerFlag == 16))
            buzzer_onoff(1);
        if((BuzzerFlag == 14) || (BuzzerFlag == 18))
            buzzer_onoff(0);
        if(BuzzerFlag == 18)
        {
            BuzzerFlag = 0;
            IC_CardInsert = 0;
        }
    }
}

void IC_info_default(void)
{
    memcpy(Read_ICinfo_Reg.DriverCard_ID, "000000000000000000", 18);
    memset(Read_ICinfo_Reg.Effective_Date, 0, sizeof(Read_ICinfo_Reg.Effective_Date));
    Read_ICinfo_Reg.Effective_Date[0] = 0x20;
    Read_ICinfo_Reg.Effective_Date[1] = 0x08;
    Read_ICinfo_Reg.Effective_Date[2] = 0x08;
    memcpy(Read_ICinfo_Reg.Drv_CareerID, "000000000000000000", 18);
}
//   �ж϶����ʻԱ���忨����Ϣ�ĸ���
void  Different_DriverIC_InfoUpdate(void)
{
    u8  i = 0, j = 0, compare = 0; // compare �Ƿ���ƥ���
    s8  cmp_res = 0; // ƥ��Ϊ0
    u8  selected_num = 0; // ��ʾ�ڼ�����ѡ����

    //  0 .    �ȱ����Ƿ�ͼ��е�ƥ��
    for(i = 0; i < MAX_DriverIC_num; i++)
    {
        if(Drivers_struct[i].Working_state)
        {
            // check compare
            cmp_res = memcmp(Drivers_struct[i].Driver_BASEinfo.DriverCard_ID, Read_ICinfo_Reg.DriverCard_ID, 18);
            if(cmp_res == 0) // ƥ��
            {

                Drivers_struct[i].Working_state = 2;
                selected_num = MAX_DriverIC_num + 2; //   ����ȫƥ��
                compare = 1; //enable
                if(GB19056.workstate == 0)
                    rt_kprintf("\r\n  ����ȫƥ��� i=%d\r\n", i);
            }
            else
            {
                if(Drivers_struct[i].Working_state == 2)
                {
                    Drivers_struct[i].Working_state = 1;
                    selected_num = i; //  ��¼��ǰΪ2 �� �±�
                    if(GB19056.workstate == 0)
                        rt_kprintf("\r\n  �ϴβ���� 2bian1  i=%d\r\n", i);
                }
            }
        }
    }

    if(compare)         //  �ҵ�ƥ����ˣ��Ҹ���״̬��
        return;
    //--------------------------------------------------------------
    // 1. ������ʻԱ״̬���ҳ���ǰҪ�õ�λ��
    for(i = 0; i < MAX_DriverIC_num; i++)
    {
        // ����е�ǰ�ģ���ô������ǰ״̬ת���ɼ���״̬

        //    ״̬Ϊ2 ����һ����Ϊ ��ǰλ��
        if((Drivers_struct[i].Working_state == 1) && (selected_num == 0x0F))
        {
            selected_num = i;
            break;
        }
        if(Drivers_struct[i].Working_state == 1)
        {
            if(selected_num == i) // �жϵ�ǰ��¼�Ƿ����ϴ�Ϊ2 �ļ�¼
            {
                Drivers_struct[i].Working_state = 1;
                selected_num = 0x0F;
            }
        }
        if(Drivers_struct[i].Working_state == 0)
        {
            selected_num = i;
            break;
        }

    }

    if(i >= MAX_DriverIC_num) //   ȫ���ˣ������һ����2����ô��һ�����ǵ�ǰ�µ�
    {
        selected_num = 0;
    }

    // 2. update new  info
    memcpy((u8 *)(&Drivers_struct[selected_num].Driver_BASEinfo), (u8 *)(&Read_ICinfo_Reg), sizeof(Read_ICinfo_Reg));
    Drivers_struct[selected_num].Working_state = 2; //
    Drivers_struct[selected_num].Running_counter = 0;
    Drivers_struct[selected_num].Stopping_counter = 0;

    if(GB19056.workstate == 0)
    {
        rt_kprintf("\r\n  ���������� i=%d  ID=%s\r\n", selected_num, Drivers_struct[selected_num].Driver_BASEinfo.DriverCard_ID);


        rt_kprintf("\r\n  1--���������� i=%d  ID=%s\r\n", 0, Drivers_struct[0].Driver_BASEinfo.DriverCard_ID);
        rt_kprintf("\r\n  2--���������� i=%d  ID=%s\r\n", 1, Drivers_struct[1].Driver_BASEinfo.DriverCard_ID);
        rt_kprintf("\r\n  3--���������� i=%d  ID=%s\r\n", 2, Drivers_struct[2].Driver_BASEinfo.DriverCard_ID);
    }
}



//  �ж� ��ͬ��ʻԱ ,������ʻ�Ŀ�ʼʱ��
void  Different_DriverIC_Start_Process(void)
{
    u8  i = 0;
    // �жϸ�����ʻԱ��������ʻ�Ŀ�ʼʱ��
    if((Spd_Using > 60) && (Sps_larger_5_counter > 10))
    {
        for(i = 0; i < MAX_DriverIC_num; i++)
        {
            if((Drivers_struct[i].Working_state == 2) && (Drivers_struct[i].Running_counter == 0))
            {
                time_now = Get_RTC();   //  RTC  ���
                Time2BCD(Drivers_struct[i].Start_Datetime);
                //   3.  ��ʼλ��
                memcpy(Drivers_struct[i].Longi, VdrData.Longi, 4); // ����
                memcpy(Drivers_struct[i].Lati, VdrData.Lati, 4); //γ��
                Drivers_struct[i].Hight = GPS_Hight;
                Drivers_struct[i].H_11_start = 1; //  start
                Drivers_struct[i].H_11_lastSave_state = 0;
            }
        }
    }

}

//  �жϲ�ͬ��ʻԱ��������ʻ�Ľ���ʱ�� // �ٶ�Ϊ0  ʱ��ִ��
void  Different_DriverIC_End_Process(void)
{
    u8 i = 0;
    u8  value = 0;

    for(i = 0; i < MAX_DriverIC_num; i++)
    {
        if((Drivers_struct[i].Working_state) && (Drivers_struct[i].Running_counter >= TiredConf_struct.TiredDoor.Door_DrvKeepingSec))
        {
            if((TiredConf_struct.TiredDoor.Door_DrvKeepingSec <= 0) && (Drivers_struct[i].H_11_start == 0))
                break;

            //  1.   ��������ʻ֤��
            memcpy(VdrData.H_11, Drivers_struct[i].Driver_BASEinfo.DriverCard_ID, 18);
            if(GB19056.workstate == 0)
                rt_kprintf("\r\n    drivernum=%d drivercode-=%s\r\n", i, Drivers_struct[i].Driver_BASEinfo.DriverCard_ID);
            //   2.   ��ʼʱ��
            memcpy(VdrData.H_11 + 18, Drivers_struct[i].Start_Datetime, 6); // ��ʼʱ��
            time_now = Get_RTC();   //  RTC  ���
            Time2BCD(Drivers_struct[i].End_Datetime);
            memcpy(VdrData.H_11 + 24, Drivers_struct[i].End_Datetime, 6); // ����ʱ��
            //   3.  ��ʼλ��
            memcpy( VdrData.H_11 + 30, Drivers_struct[i].Longi, 4); // ����
            memcpy( VdrData.H_11 + 30 + 4, Drivers_struct[i].Lati, 4); //γ��
            VdrData.H_11[30 + 8] = (Drivers_struct[i].Hight >> 8);
            VdrData.H_11[30 + 9] = Drivers_struct[i].Hight;
            //VdrData.H_11_start=1; //  start
            //VdrData.H_11_lastSave=0;

            //   4.   ����λ��
            memcpy( VdrData.H_11 + 40, VdrData.Longi, 4); // ����
            memcpy( VdrData.H_11 + 40 + 4, VdrData.Lati, 4); //γ��
            VdrData.H_11[40 + 8] = (GPS_Hight >> 8);
            VdrData.H_11[40 + 9] = GPS_Hight;

            value = 2;

            VDR_product_11H_End(2);

            Drivers_struct[i].H_11_start = 2; // end over

            if(value == 2)
            {
                Drivers_struct[i].H_11_lastSave_state = 1;
            }
        }

    }
}

//  �жϲ�ͬ��ʻԱ ƣ��״̬
void  Different_DriverIC_Checking(void)
{
    u8 i = 0;

    if((Spd_Using > 60 ) && (Sps_larger_5_counter > 10))	// Spd_Using ��λΪ0.1 km/h  �ٶȴ���6 km/h  ��Ϊ����ʻ
    {
        // ��ʻ״̬
        for(i = 0; i < MAX_DriverIC_num; i++)
        {
            if( Drivers_struct[i].Working_state == 2)
            {
                Drivers_struct[i].Running_counter++;

                //       ��ǰ30 min  ������ʾ
                if(TiredConf_struct.TiredDoor.Door_DrvKeepingSec > 1800)
                {
                    if(Drivers_struct[i].Running_counter == ( TiredConf_struct.TiredDoor.Door_DrvKeepingSec - 1800 ) ) //��ǰ5���ӷ�������ʾע��ƣ�ͼ�ʻ 14100
                    {
                        //   ƣ�ͱ���Ԥ��ʾ ��ʼ
                        if(GB19056.SPK_PreTired.Warn_state_Enable == 0)
                            GB19056.SPK_PreTired.Warn_state_Enable = 1;
                    }
                }
                //--   �ж�ƣ�ͼ�ʻ--------------------------------------------------------------------
                if(  Drivers_struct[i].Running_counter == ( TiredConf_struct.TiredDoor.Door_DrvKeepingSec) )//14400
                {
                    if(GB19056.workstate == 0)
                        rt_kprintf( "\r\n   ƣ�ͼ�ʻ������!  on\r\n" );
                    //  TTS_play( "���Ѿ�ƣ�ͼ�ʻ����ע����Ϣ" );


                    //	��ʱ��ʻ ��ʾ������
                    if(GB19056.SPK_DriveExceed_Warn.Warn_state_Enable == 0)
                        GB19056.SPK_DriveExceed_Warn.Warn_state_Enable = 1;
                    //	ƣ�ͱ���Ԥ��ʾ ����
                    GB19056.SPK_PreTired.Warn_state_Enable = 0;



                    //tts_bro_tired_flag    = 1;
                    Warn_Status[3]	   |= 0x04; //BIT(2)  ƣ�ͼ�ʻ
                    //---- ������ʱ�ϱ�����-------
                    PositionSD_Enable( );
                    Current_UDP_sd = 1;
                    //-------------------------------------
                }
                //----------------------------------------------------------------------------------------
                Drivers_struct[i].Stopping_counter = 0;
            }
            else if(Drivers_struct[i].Working_state == 1) //  ״̬Ϊ1  Ҳ����Ϣ
            {
                Drivers_struct[i].Stopping_counter++; // ״̬Ϊ1 �ĳ�����Ϣ

                if(Drivers_struct[i].Stopping_counter >= TiredConf_struct.TiredDoor.Door_MinSleepSec )	//1200	 // ACC ��20������Ϊ��Ϣ
                {
                    if(Drivers_struct[i].Stopping_counter == TiredConf_struct.TiredDoor.Door_MinSleepSec )
                    {
                        Drivers_struct[i].Stopping_counter = 0;
                        Drivers_struct[i].Running_counter = 0;

                        // 	ֻ�д�����ƣ�ͼ�ʻ���ҿ�ʼʱ�丳����ֵ�Ż�洢�ж�
                        if((Drivers_struct[i].Running_counter >= TiredConf_struct.TiredDoor.Door_DrvKeepingSec) && \
                                (Drivers_struct[i].H_11_start == 2));
                        {
                            Drivers_struct[i].Working_state = 0; //
                            Warn_Status[3] &= ~0x04; //BIT(2)	 ƣ�ͼ�ʻ

                            //---- ������ʱ�ϱ�����-------
                            PositionSD_Enable( );
                            Current_UDP_sd = 1;
#if 0
                            //-------------------------------------
                            //    ��ʱ����  �洢�������ۼ�
                            //  1.   ��������ʻ֤��
                            memcpy(VdrData.H_11, Drivers_struct[i].Driver_BASEinfo.DriverCard_ID, 18);
                            //   2.   ��ʼʱ��
                            memcpy(VdrData.H_11 + 18, Drivers_struct[i].Start_Datetime, 6); // ��ʼʱ��
                            memcpy(VdrData.H_11 + 24, Drivers_struct[i].End_Datetime, 6);	// ����ʱ��
                            //   3.  ��ʼλ��
                            memcpy( VdrData.H_11 + 30, Drivers_struct[i].Longi, 4); // ����
                            memcpy( VdrData.H_11 + 30 + 4, Drivers_struct[i].Lati, 4); //γ��
                            VdrData.H_11[30 + 8] = (Drivers_struct[i].Hight >> 8);
                            VdrData.H_11[30 + 9] = Drivers_struct[i].Hight;

                            //   4.   ����λ��
                            memcpy( VdrData.H_11 + 40, VdrData.Longi, 4);	// ����
                            memcpy( VdrData.H_11 + 40 + 4, VdrData.Lati, 4); //γ��
                            VdrData.H_11[40 + 8] = (GPS_Hight >> 8);
                            VdrData.H_11[40 + 9] = GPS_Hight;

                            VDR_product_11H_End(1);
#endif

                            //	ֻ�в�Ϊ��ǰ���������ǰΪ2 �����
                            if(Drivers_struct[i].Working_state == 1)
                                memset((u8 *)&Drivers_struct[i], 0, sizeof(DRIVE_STRUCT)); // clear


                            //  ��ʱ��ʻ����
                            GB19056.SPK_DriveExceed_Warn.Warn_state_Enable = 0;
                            if(GB19056.workstate == 0)
                                rt_kprintf( "\r\n	i=%d ����ƣ�ͼ�ʻ�����Ѿ����-on 1 \r\n", i);
                        }
                    }
                }
            }

        }

    }
    else
    {
        // ֹͣ״̬
        for(i = 0; i < MAX_DriverIC_num; i++)
        {
            if( Drivers_struct[i].Working_state)
            {
                if(Drivers_struct[i].Running_counter)
                {
                    //-- ACC û����Ϣǰ����AccON ��״̬  ---------------
                    Drivers_struct[i].Running_counter++;

                    //---------------------------------------------------------------
                    //       ��ǰ30 min  ������ʾ
                    if(TiredConf_struct.TiredDoor.Door_DrvKeepingSec > 1800)
                    {
                        if(Drivers_struct[i].Running_counter == ( TiredConf_struct.TiredDoor.Door_DrvKeepingSec - 1800 ) ) //��ǰ5���ӷ�������ʾע��ƣ�ͼ�ʻ 14100
                        {
                            //   ƣ�ͱ���Ԥ��ʾ ��ʼ
                            if(GB19056.SPK_PreTired.Warn_state_Enable == 0)
                                GB19056.SPK_PreTired.Warn_state_Enable = 1;
                        }
                    }


                    //      ACC off    but     Acc on  conintue  run
                    if( Drivers_struct[i].Running_counter >= ( TiredConf_struct.TiredDoor.Door_DrvKeepingSec ) )       //14400 // ������ʻ����4Сʱ��ƣ�ͼ�ʻ
                    {
                        if(Drivers_struct[i].Running_counter == ( TiredConf_struct.TiredDoor.Door_DrvKeepingSec  ) )  //14400
                        {
                            if(GB19056.workstate == 0)
                                rt_kprintf( "\r\n	 �ٶ�С����δ������Ϣ����ʱ�� ƣ�ͼ�ʻ������! \r\n");
                            //TTS_play( "���Ѿ�ƣ�ͼ�ʻ����ע����Ϣ" );




                            //	 ��ʱ��ʻ ������
                            if(GB19056.SPK_DriveExceed_Warn.Warn_state_Enable == 0)
                                GB19056.SPK_DriveExceed_Warn.Warn_state_Enable = 1;
                            //   ƣ�ͱ���Ԥ��ʾ ����
                            GB19056.SPK_PreTired.Warn_state_Enable = 0;


                            Warn_Status[3]		|= 0x04;                                                                                    //BIT(2)  ƣ�ͼ�ʻ
                            //---- ������ʱ�ϱ�����-------
                            PositionSD_Enable( );
                            Current_UDP_sd = 1;
                        }
                        Warn_Status[3] |= 0x04;                                                                                             //BIT(2)  ƣ�ͼ�ʻ
                    }


                    //     ACC  off   counter
                    Drivers_struct[i].Stopping_counter++;
                    if(Drivers_struct[i].Stopping_counter >= TiredConf_struct.TiredDoor.Door_MinSleepSec ) //1200	// ACC ��20������Ϊ��Ϣ
                    {
                        if(Drivers_struct[i].Stopping_counter == TiredConf_struct.TiredDoor.Door_MinSleepSec )
                        {
                            Drivers_struct[i].Stopping_counter = 0;
                            Drivers_struct[i].Running_counter = 0;

                            //     ֻ�д�����ƣ�ͼ�ʻ���ҿ�ʼʱ�丳����ֵ�Ż�洢�ж�
                            if((Drivers_struct[i].Running_counter >= TiredConf_struct.TiredDoor.Door_DrvKeepingSec) && \
                                    (Drivers_struct[i].H_11_start == 2));
                            {
                                Drivers_struct[i].Working_state = 0; //
                                Warn_Status[3] &= ~0x04; //BIT(2)	ƣ�ͼ�ʻ

                                //---- ������ʱ�ϱ�����-------
                                PositionSD_Enable( );
                                Current_UDP_sd = 1;
                                //-------------------------------------
                                //    ��ʱ����  �洢�������ۼ�
                                //  1.   ��������ʻ֤��
                                memcpy(VdrData.H_11, Drivers_struct[i].Driver_BASEinfo.DriverCard_ID, 18);
                                //   2.   ��ʼʱ��
                                memcpy(VdrData.H_11 + 18, Drivers_struct[i].Start_Datetime, 6); // ��ʼʱ��
                                memcpy(VdrData.H_11 + 24, Drivers_struct[i].End_Datetime, 6); // ����ʱ��
                                //   3.  ��ʼλ��
                                memcpy( VdrData.H_11 + 30, Drivers_struct[i].Longi, 4); // ����
                                memcpy( VdrData.H_11 + 30 + 4, Drivers_struct[i].Lati, 4); //γ��
                                VdrData.H_11[30 + 8] = (Drivers_struct[i].Hight >> 8);
                                VdrData.H_11[30 + 9] = Drivers_struct[i].Hight;

                                //   4.   ����λ��
                                memcpy( VdrData.H_11 + 40, VdrData.Longi, 4); // ����
                                memcpy( VdrData.H_11 + 40 + 4, VdrData.Lati, 4); //γ��
                                VdrData.H_11[40 + 8] = (GPS_Hight >> 8);
                                VdrData.H_11[40 + 9] = GPS_Hight;

                                if((Drivers_struct[i].End_Datetime[0] == 0x00) && (Drivers_struct[i].End_Datetime[1] == 0x00) && (Drivers_struct[i].End_Datetime[2] == 0x00))
                                    ; // ������ Ϊ0  ����
                                else
                                    VDR_product_11H_End(1);

                                //  ֻ�в�Ϊ��ǰ���������ǰΪ2 �����
                                if(Drivers_struct[i].Working_state == 1)
                                    memset((u8 *)&Drivers_struct[i], 0, sizeof(DRIVE_STRUCT)); // clear
                                else



                                    //  ��ʱ��ʻ����
                                    GB19056.SPK_DriveExceed_Warn.Warn_state_Enable = 0;
                                if(GB19056.workstate == 0)
                                    rt_kprintf( "\r\n	����ƣ�ͼ�ʻ�����Ѿ���� \r\n");
                            }

                        }
                    }
                    //-------------------------------------------------------------------------------------
                }
            }
        }


    }
}



#if 1
//  orginal setting
void CheckICInsert(void)
{
    unsigned char write_flag = 0;
    u8 result0 = 0, result1 = 1, result2 = 2, result3 = 4, result4 = 5; //i=0;
    u8 reg_record_liu[13];
    u32 DriveCode32 = 0;
    u8  red_byte[128];
    u8  fcs = 0, i = 0;


    //===================����IC����д==================================================
    if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_7))
    {
        IC_Check_Count++;
        if(IC_Check_Count >= 10)
        {
            IC_Check_Count = 0;
            //�����ϵ翪8024�ĵ�
            if(flag_8024off == 1)     //  �ϵ粻��ȡIC ��
            {
                R_Flag |= b_CardEdge;
                Init8024Flag = 2;
                flag_8024off = 0;
            }

            //8024��off�ӵͱ��
            if(Init8024Flag == 1)
            {
                Init8024Flag = 2;
                R_Flag |= b_CardEdge;
            }
            //��⵽�����ʼ��ic��
            if((R_Flag & b_CardEdge) && (Init8024Flag == 2))
            {
                Init8024Flag = 3;
                _CardCMDVCC_LOW;
                for(DelayCheckIc = 0; DelayCheckIc < 500; DelayCheckIc++)
                    DELAY5us();
                _CardSetPower_HIGH;
                _CardSetRST_LOW;
                for(DelayCheckIc = 0; DelayCheckIc < 15; DelayCheckIc++)
                {
                    _CardSetCLK_LOW;
                    DELAY5us();
                    DELAY5us();
                    DELAY5us();
                    _CardSetCLK_HIGH;
                    DELAY5us();
                    DELAY5us();
                    DELAY5us();
                    _CardSetCLK_LOW;
                }
                R_Flag &= ~b_CardEdge;
                write_flag = 1;
                //----------------------------------------------------
            }
        }
    }
    else
    {

        IC_Check_Count = 0;
        _CardSetRST_HIGH;
        _CardSetPower_LOW;
        _CardCMDVCC_HIGH;
        if(Init8024Flag == 0)
        {
            Init8024Flag = 1;
            //if(powerOn_first==0)
            // powerOn_first=1;
            //else
            {
                //---------------------------------------------------
                // rt_kprintf("   �ο� pc7  Ϊ ��---����\r\n");
                if((Spd_Using < 40) && (IC_loginState == 1)) // ����ʻ����״̬�²ſ�ʼ �жϲ�ο�
                {
                    VDR_product_12H(0x02);  //  �ǳ�
                    TTS_play("��ʻԱ�˳�");
                    IC_loginState = 0;
                }
                //---------------------------------------------------

            }
        }
    }

    if(write_flag == 1)
    {
        write_flag = 0;
        if((Spd_Using < 40) && (Sps_larger_5_counter == 0)) // ����ʻ����״̬�²ſ�ʼ �жϲ�ο�
        {
            /*  result2=Rx_4442(0,32,red_byte);  //����ʻ֤����
            	  rt_kprintf("\r\n��������ʻ֤����:result1=%d",result2);
            	{
            		OutPrint_HEX("����IC��Ϣ 1",red_byte,32);

            	}
            */
            // 0  ------------- ����Ա��---
            Rx_4442(241, 13, red_byte);	//����Ա��
            if(strncmp((char *)red_byte, "administrator", 13) == 0)
            {
                rt_kprintf("\r\n����Ա��");
                administrator_card = 1;

                pMenuItem = &Menu_0_loggingin; // ����Ա���������
                pMenuItem->show();
                BuzzerFlag = 1; //��һ����ʾ
                Init8024Flag = 0;
                GpsIo_Init();
                return;
            }

            // 1 .---------��ȡ������Ϣ-----------
            result2 = Rx_4442(32, 96, red_byte); //����ʻ֤����
            if(result2 == 0)
            {
                //OutPrint_HEX("����IC��Ϣ 2",red_byte,96);
                fcs = 0;
                for(i = 0; i < 95; i++)
                {
                    fcs ^= red_byte[i];
                }
            }
            else
            {
                //  ����ʧ��
                BuzzerFlag = 11; //��һ����ʾ
                IC_CardInsert = 2; //IC	���������
                Init8024Flag = 0;
                GpsIo_Init();
                Menu_txt_state = 3;
                pMenuItem = &Menu_TXT;
                pMenuItem->show();
                pMenuItem->timetick( 10 );
                pMenuItem->keypress( 10 );
                return;
            }

            // 2.����FCS
            if((red_byte[95] == fcs) && (result2 == 0))
            {
                memset(Read_ICinfo_Reg.DriverCard_ID, 0, sizeof(Read_ICinfo_Reg.DriverCard_ID));
                result1 = Rx_4442(32, 18, (unsigned char *)Read_ICinfo_Reg.DriverCard_ID);	//����ʻ֤����
                //if( GB19056.workstate==0)
                //{
                //rt_kprintf("\r\n��������ʻ֤����:%s,result1=%d",Read_ICinfo_Reg.DriverCard_ID,result1);
                //OutPrint_HEX("��������ʻ֤����HEX",Read_ICinfo_Reg.DriverCard_ID,18);
                //}

                memset(Read_ICinfo_Reg.Effective_Date, 0, sizeof(Read_ICinfo_Reg.Effective_Date));
                result2 = Rx_4442(50, 3, (unsigned char *)Read_ICinfo_Reg.Effective_Date);	//����ʻԱ����
                //if( GB19056.workstate==0)
                //rt_kprintf("\r\n��Ч����:%2X-%2X-%2X,result2=%d",Read_ICinfo_Reg.Effective_Date[0],Read_ICinfo_Reg.Effective_Date[1],Read_ICinfo_Reg.Effective_Date[2],result2);

                memset(Read_ICinfo_Reg.Drv_CareerID, 0, sizeof(Read_ICinfo_Reg.Drv_CareerID));
                result4 = Rx_4442(53, 18, (unsigned char *)Read_ICinfo_Reg.Drv_CareerID);	//��ҵ�ʸ�֤
                //if( GB19056.workstate==0)
                //rt_kprintf("\r\n��ҵ�ʸ�֤:%s,result4=%d",Read_ICinfo_Reg.Drv_CareerID,result4);

                //------  ��ʻԱ����(ѡ��)
                memcpy(red_byte, 0, sizeof(red_byte));
                result0 = Rx_4442(128, 10, (unsigned char *)red_byte); //����ʻԱ����
                if(result0 == 0)
                {
                    if(strlen(red_byte))
                    {
                        if(GB19056.workstate == 0)
                            rt_kprintf("\r\n��ʻԱ����:%s,result0=%d", red_byte, result0);
                        memset(JT808Conf_struct.Driver_Info.DriveName, 0, sizeof(JT808Conf_struct.Driver_Info.DriveName));
                        memcpy(JT808Conf_struct.Driver_Info.DriveName, red_byte, strlen(red_byte));
                    }
                }
                //-----------  ͣ���忨 ��ʶ�� -------
                if((result1 == 0) && (result2 == 0) && (result4 == 0)) //�������ȷ
                {
                    Different_DriverIC_InfoUpdate();  // ����IC ��ʻԱ��Ϣ
                    IC_loginState = 1;
                    VDR_product_12H(0x01);	//	��¼
                    if(powerOn_first)
                        TTS_play("��ʻԱ��¼"); // ��һ���ϵ粥������Ҫ��¼
                    powerOn_first = 1;
                    GB19056.SPK_UnloginWarn.Warn_state_Enable = 0; // clear
                }
            }
            else
            {
                // У�����
                BuzzerFlag = 11; //��һ����ʾ
                IC_CardInsert = 2; //IC	���������
                Menu_txt_state = 2;
                pMenuItem = &Menu_TXT;
                pMenuItem->show();
                pMenuItem->timetick( 10 );
                pMenuItem->keypress( 10 );

            }
            //------------------------------------------
        }
        else
        {
            result1 = 1; // ��ȫΪ0  ���жϴ���
            // if( GB19056.workstate==0)
            // rt_kprintf("\r\n��ʻ�в忨����¼!");
        }


        //--------- IC ��������ʾ-----------------
        if((result1 == 0) && (result2 == 0) && (result4 == 0)) //�������ȷ
        {
            IC_CardInsert = 1; //IC	��������ȷ
            BuzzerFlag = 1; //��һ����ʾ
            pMenuItem = &Menu_3_4_DriverInfor;
            pMenuItem->show();
        }
        else
        {
            BuzzerFlag = 11; //��һ����ʾ
            IC_CardInsert = 2; //IC	���������
        }
        //-------------------------------------------------------------------
        Init8024Flag = 0;
        GpsIo_Init();
    }

    //===================����IC����д���==================================================

}
#endif


