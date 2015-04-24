/*
     GB19056.C
*/

#include <rtthread.h>
#include <rthw.h>
#include "stm32f2xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>
#include <stm32f2xx_usart.h>

#include  <stdlib.h>//����ת�����ַ���
#include  <stdio.h>
#include  <string.h>
#include "App_moduleConfig.h"
#include "App_gsm.h"
#include "GB19056.h"



u8  DOUBT3 = 0;
u8  Doubt_u8[6];


#define   OUT_PUT        SERIAL_OUTput(type)
#define   USB_OUT        USB_OUTput(CMD,add_fcs_byte)



//    GB  ���ݵ���
#define GB_USB_OUT_idle          0
#define GB_USB_OUT_start         1
#define GB_USB_OUT_running       2
#define GB_USB_OUT_end           3

#define GB_OUT_TYPE_Serial         1
#define GB_OUT_TYPE_USB            2


struct rt_semaphore GB_RX_sem;  //  gb  ����AA 75

rt_device_t   Udisk_device = RT_NULL;

GB_STRKT GB19056;
int  usb_fd = 0;
u8 usb_outbuf[1500];
u16 usb_outbuf_Wr = 0;
u8    Warn_Play_controlBit = 0x16;
/*
                                      ʹ�ܲ���״̬����BIT
                                    BIT   0:    ��ʻԱδ��¼״̬��ʾ(����Ĭ��ȡ��)
                                    BIT   1:    ��ʱԤ��
                                    BIT   2:    ��ʱ����
                                    BIT   3:    �ٶ��쳣����(����Ĭ��ȡ��)
                                    BIT   4:    ���ٱ���
                                    BIT   5:
                             */




void GB_OUT_data(u8  ID, u8 type, u16 IndexNum, u16 total_pkg_num);
//void  out_plus(u8  state);
//void  output_rtcplus(u8 value);
//void  output_spd(u16 speed);



u8 GB_fcs(u8 *instr, u16 infolen, u8 fcs_value)
{
    u16 i = 0;
    u8  FCS = fcs_value;

    for(i = 0; i < infolen; i++)
        FCS ^= instr[i];
    return  FCS;
}

void  SERIAL_OUTput(u8 type)
{
    if(type == GB_OUT_TYPE_Serial)
    {
        usb_outbuf[usb_outbuf_Wr++] = GB_fcs(usb_outbuf, usb_outbuf_Wr, 0);
        dev_vuart.flag &= ~RT_DEVICE_FLAG_STREAM;
        rt_device_write( &dev_vuart,  0,  usb_outbuf, usb_outbuf_Wr);
        dev_vuart.flag |= RT_DEVICE_FLAG_STREAM;
    }
}

void USB_OUTput(u8  CMD, u8 fcsAdd)
{
    GB19056.usb_xor_fcs = GB_fcs(usb_outbuf, usb_outbuf_Wr, GB19056.usb_xor_fcs);
    if(fcsAdd == 1)
        usb_outbuf[usb_outbuf_Wr++] = GB19056.usb_xor_fcs;
    write(usb_fd, usb_outbuf, usb_outbuf_Wr);

}

u16  GB_00_07_info_only(u8 *dest, u8 ID)
{
    u32	regdis			   = 0, reg2 = 0;
    u16  i = 0;
    u16  return_len = 0;


    return_len = 0;
    switch(ID)
    {
    case 0x00:
        dest[return_len++]	 = 0x12;		   //  12  ���׼
        dest[return_len++]	 = 0x00;
        break;
    case 0x01:
        memcpy( dest + return_len, JT808Conf_struct.Driver_Info.DriverCard_ID, 18 );  //��Ϣ����
        return_len				   += 18;
        break;
    case 0x02: //  02  �ɼ���¼�ǵ�ʵʱʱ��
        dest[return_len++]	= ( ( time_now.year / 10 ) << 4 ) + ( ( time_now.year) % 10 );
        dest[return_len++]	= ( ( time_now.month / 10 ) << 4 ) + (time_now.month % 10 );
        dest[return_len++]	= ( ( time_now.day / 10 ) << 4 ) + ( time_now.day % 10 );
        dest[return_len++] = ( ( time_now.hour / 10 ) << 4 ) + ( time_now.hour % 10 );
        dest[return_len++] = ( ( time_now.min / 10 ) << 4 ) + ( time_now.min % 10 );
        dest[return_len++] = ( ( time_now.sec / 10 ) << 4 ) + ( time_now.sec % 10 );
        break;

    case 0x03:	   // 03 �ɼ�360h�����
        dest[return_len++]   = ( ( time_now.year / 10 ) << 4 ) + ( ( time_now.year) % 10 );
        dest[return_len++]   = ( ( time_now.month / 10 ) << 4 ) + (time_now.month % 10 );
        dest[return_len++]   = ( ( time_now.day / 10 ) << 4 ) + ( time_now.day % 10 );
        dest[return_len++] = ( ( time_now.hour / 10 ) << 4 ) + ( time_now.hour % 10 );
        dest[return_len++] = ( ( time_now.min / 10 ) << 4 ) + ( time_now.min % 10 );
        dest[return_len++] = ( ( time_now.sec / 10 ) << 4 ) + ( time_now.sec % 10 );

        dest[return_len++] = JT808Conf_struct.FirstSetupDate[0];						 // ���ΰ�װʱ��
        dest[return_len++] = JT808Conf_struct.FirstSetupDate[1];
        dest[return_len++] = JT808Conf_struct.FirstSetupDate[2];
        dest[return_len++] = JT808Conf_struct.FirstSetupDate[3];
        dest[return_len++] = JT808Conf_struct.FirstSetupDate[4];
        dest[return_len++] = JT808Conf_struct.FirstSetupDate[5];
        //--- ��ʼ���
        dest[return_len++] = 0x00;
        dest[return_len++] = 0x00;
        dest[return_len++] = 0x00;
        dest[return_len++] = 0x00;
        // -- �ۻ���� 3���ֽ� ��λ0.1km	 6λ
        regdis 							 = JT808Conf_struct.Distance_m_u32 / 100; //��λ0.1km
        reg2								 = regdis / 100000;
        dest[return_len++] = 0x00;
        dest[return_len++] = ( reg2 << 4 ) + ( regdis % 100000 / 10000 );
        dest[return_len++] = ( ( regdis % 10000 / 1000 ) << 4 ) + ( regdis % 1000 / 100 );
        dest[return_len++] = ( ( regdis % 100 / 10 ) << 4 ) + ( regdis % 10 );
        break;

    case 0x04:																   // 04  �ɼ���¼������ϵ��
        dest[return_len++]	= ( ( time_now.year / 10 ) << 4 ) + ( ( time_now.year) % 10 );
        dest[return_len++]	= ( ( time_now.month / 10 ) << 4 ) + (time_now.month % 10 );
        dest[return_len++]	= ( ( time_now.day / 10 ) << 4 ) + ( time_now.day % 10 );
        dest[return_len++] = ( ( time_now.hour / 10 ) << 4 ) + ( time_now.hour % 10 );
        dest[return_len++] = ( ( time_now.min / 10 ) << 4 ) + ( time_now.min % 10 );
        dest[return_len++] = ( ( time_now.sec / 10 ) << 4 ) + ( time_now.sec % 10 );

        dest[return_len++]	 = (u8)( JT808Conf_struct.Vech_Character_Value >> 8 );
        dest[return_len++]	 = (u8)( JT808Conf_struct.Vech_Character_Value );
        break;
    case 0x05:																   //05 	 ������Ϣ�ɼ�
        //-----------   ��Ϣ����  --------------
        memcpy( dest + return_len, Vechicle_Info.Vech_VIN, 17 ); 	  //��Ϣ����
        return_len += 17;
        memcpy( dest + return_len, Vechicle_Info.Vech_Num, 12 );			 // ���ƺ�
        return_len				   += 12;
        //��������
        memcpy( dest + return_len, Vechicle_Info.Vech_Type, 12 );
        return_len				   += 12;
        break;
    case   0x06: 												   // 06-09
        dest[return_len++]	 = ( ( time_now.year / 10 ) << 4 ) + ( ( time_now.year) % 10 );
        dest[return_len++]	 = ( ( time_now.month / 10 ) << 4 ) + (time_now.month % 10 );
        dest[return_len++]	 = ( ( time_now.day / 10 ) << 4 ) + ( time_now.day % 10 );
        dest[return_len++] = ( ( time_now.hour / 10 ) << 4 ) + ( time_now.hour % 10 );
        dest[return_len++] = ( ( time_now.min / 10 ) << 4 ) + ( time_now.min % 10 );
        dest[return_len++] = ( ( time_now.sec / 10 ) << 4 ) + ( time_now.sec % 10 );

        //-------  ״̬�ָ���----------------------
        dest[return_len++] = 0;// 8//�޸�Ϊ0
        //---------- ״̬������-------------------


        /*
          -------------------------------------------------------------
        		 F4  �г���¼�� TW705	�ܽŶ���
          -------------------------------------------------------------
          ��ѭ	GB10956 (2012)	Page26	��A.12	�涨
          -------------------------------------------------------------
        | Bit  |	  Note		 |	�ر�|	MCUpin	|	PCB pin  |	 Colour | ADC
          ------------------------------------------------------------
           D7	   ɲ�� 		  * 		   PE11 			9				 ��
           D6	   ��ת��	  * 			PE10			10				 ��
           D5	   ��ת��	  * 			PC2 			 8				  ��
           D4	   Զ���	  * 			PC0 			 4				  ��
           D3	   �����	  * 			PC1 			 5				  ��
           D2	   ��� 		 add		  PC3			   7				��		*
           D1	   ���� 		 add		  PA1			   6				��		*
           D0	   Ԥ��
        */
        memcpy( dest + return_len, "Ԥ��	", 10 );	   // D0
        return_len += 10;
        memcpy( dest + return_len, "Ԥ��	", 10 );	   // D1
        return_len += 10;
        memcpy( dest + return_len, "Ԥ��	", 10 );	   // D2
        return_len += 10;
        memcpy( dest + return_len, "�����	  ", 10 );	   // D3
        return_len += 10;
        memcpy( dest + return_len, "Զ���	  ", 10 );	   // D4
        return_len += 10;
        memcpy( dest + return_len, "��ת��	  ", 10 );	   // D5
        return_len += 10;
        memcpy( dest + return_len, "��ת��	  ", 10 );	   // D6
        return_len += 10;
        memcpy( dest + return_len, "ɲ��	", 10 );	   // D7
        return_len += 10;
        break;

    case 0x07:															   //07  ��¼��Ψһ���
        //------- ��Ϣ���� ------
        memcpy( dest + return_len, "7654321", 7 );		   // 3C ��֤����
        return_len += 7;
        memcpy( dest + return_len, "TW705", 5 ); // ��Ʒ�ͺ�
        return_len				   += 5;
        for(i = 0; i < 11; i++)
            dest[return_len++]	= 0x00;

        dest[return_len++]	 = 0x14;					   // ��������
        dest[return_len++]	 = 0x03;
        dest[return_len++]	 = 0x01;
        dest[return_len++]	 = SimID_12D[8];					   // ������ˮ��
        dest[return_len++]	 = SimID_12D[9];
        dest[return_len++]	 = SimID_12D[10];
        dest[return_len++]	 = SimID_12D[11];
        //  31--35  ����
        dest[return_len++]	 = 0x00;
        dest[return_len++]	 = 0x00;
        dest[return_len++]	 = 0x00;
        dest[return_len++]	 = 0x00;
        dest[return_len++]	 = 0x00;
        break;
    }
    return return_len;
}


u16  GB_557A_protocol_00_07_stuff(u8 *dest, u8 ID)
{
    u32	regdis			   = 0, reg2 = 0;
    u16	SregLen = 0;
    u16  i = 0, subi = 0, continue_counter = 0;
    u16   get_indexnum = 0; // ��ȡУ��
    //u16  Sd_DataBloknum=0; //   �������ݿ�  ��Ŀ <GB19056.Max_dataBlocks
    u8   Not_null_flag = 0; // NO  data ACK null once
    u8   return_value = 0;
    u32  Rd_Time = 0;

    u16  return_len = 0;


    return_len = 0;
    switch(ID)
    {
    case 0x00:
        dest[return_len++]	 = 0x55;		   // ��ʼͷ
        dest[return_len++]	 = 0x7A;
        dest[return_len++]	 = 0x00;		   //������
        SregLen							   = 0x02;			   // ��Ϣ����
        dest[return_len++]	 = 0x00;		   // Hi
        dest[return_len++]	 = 2;				   // Lo

        dest[return_len++] = 0x00;						   // ������
        break;
    case 0x01:
        return_len = 0;
        dest[return_len++]	 = 0x55;					   // ��ʼͷ
        dest[return_len++]	 = 0x7A;
        dest[return_len++]	 = 0x01;					   //������

        dest[return_len++]	 = 0x00;					   // Hi
        dest[return_len++]	 = 18;						   // Lo

        dest[return_len++] = 0x00;						   // ������
        break;
    case 0x02: //  02  �ɼ���¼�ǵ�ʵʱʱ��
        return_len = 0;

        dest[return_len++]	 = 0x55;					   // ��ʼͷ
        dest[return_len++]	 = 0x7A;
        dest[return_len++]	 = 0x02;					   //������

        dest[return_len++]	 = 0x00;					   // Hi
        dest[return_len++]	 = 6;							   // Lo

        dest[return_len++] = 0x00;						   // ������
        break;

    case 0x03:	   // 03 �ɼ�360h�����
        return_len = 0;												 // 03 �ɼ�360h�����

        dest[return_len++] = 0x55;						 // ��ʼͷ
        dest[return_len++] = 0x7A;
        dest[return_len++] = 0x03;						 //������

        dest[return_len++] = 0x00;						 // Hi
        dest[return_len++] = 20;							 // Lo

        dest[return_len++] = 0x00;						   // ������
        break;

    case 0x04:																   // 04  �ɼ���¼������ϵ��
        return_len = 0;
        dest[return_len++]	 = 0x55;						   // ��ʼͷ
        dest[return_len++]	 = 0x7A;

        dest[return_len++] = 0x04;							   //������

        dest[return_len++]	 = 0x00;						   // Hi
        dest[return_len++]	 = 8;								   // Lo

        dest[return_len++] = 0x00;							   // ������
        break;
    case 0x05:																   //05 	 ������Ϣ�ɼ�
        return_len = 0;
        dest[return_len++]	 = 0x55;						   // ��ʼͷ
        dest[return_len++]	 = 0x7A;

        dest[return_len++] = 0x05;							   //������

        SregLen							   = 41;							   // ��Ϣ����
        dest[return_len++]	 = (u8)( SregLen >> 8 );		   // Hi
        dest[return_len++]	 = (u8)SregLen; 				   // Lo	 65x7

        dest[return_len++] = 0x00;							   // ������
        //-----------   ��Ϣ����  --------------
        break;
    case   0x06: 												   // 06-09
        return_len = 0;
        //  06 �ź�������Ϣ
        dest[return_len++]	 = 0x55;			   // ��ʼͷ
        dest[return_len++]	 = 0x7A;

        dest[return_len++] = 0x06;				   //������

        SregLen							   = 87;				   // ��Ϣ����
        dest[return_len++]	 = (u8)( SregLen >> 8 ); // Hi
        dest[return_len++]	 = (u8)SregLen; 	   // Lo
        dest[return_len++] = 0x00;				   // ������
        break;

    case 0x07:															   //07  ��¼��Ψһ���
        return_len = 0;
        dest[return_len++]	 = 0x55;					   // ��ʼͷ
        dest[return_len++]	 = 0x7A;

        dest[return_len++] = 0x07;						   //������

        SregLen							   = 35;						   //206;		// ��Ϣ����
        dest[return_len++]	 = (u8)( SregLen >> 8 );	   // Hi
        dest[return_len++]	 = (u8)SregLen; 			   // Lo

        dest[return_len++] = 0x00;						   // ������
        break;
    }

    return_len += GB_00_07_info_only(dest + return_len, ID);
    return return_len;

}

void  GB_null_content(u8 ID, u8 type)
{
    usb_outbuf_Wr = 0;
    usb_outbuf[usb_outbuf_Wr++]   = 0x55;					// ��ʼͷ
    usb_outbuf[usb_outbuf_Wr++]   = 0x7A;
    usb_outbuf[usb_outbuf_Wr++]   = ID;					//������
    usb_outbuf[usb_outbuf_Wr++]   = 0;			// Hi
    usb_outbuf[usb_outbuf_Wr++]   = 0;				// Lo

    usb_outbuf[usb_outbuf_Wr++] = 0x00; 					// ������
    OUT_PUT;
}

void GB_SPK_DriveExceed_clear(void)
{
    GB19056.SPK_DriveExceed_Warn.Warn_state_Enable = 0;
    GB19056.SPK_DriveExceed_Warn.group_playTimes = 0;
    GB19056.SPK_DriveExceed_Warn.FiveMin_sec_counter = 0;
}
void GB_SPK_SpeedStatus_Abnormal_clear(void)
{
    GB19056.SPK_SpeedStatus_Abnormal.Warn_state_Enable = 0;
    GB19056.SPK_SpeedStatus_Abnormal.group_playTimes = 0;
    GB19056.SPK_SpeedStatus_Abnormal.FiveMin_sec_counter = 0;
}

void GB_SPK_Speed_Warn_clear(void)
{
    GB19056.SPK_Speed_Warn.Warn_state_Enable = 0;
    GB19056.SPK_Speed_Warn.group_playTimes = 0;
    GB19056.SPK_Speed_Warn.FiveMin_sec_counter = 0;
}

void GB_SPK_UnloginWarn_clear(void)
{
    GB19056.SPK_UnloginWarn.Warn_state_Enable = 0;
    GB19056.SPK_UnloginWarn.group_playTimes = 0;
    GB19056.SPK_UnloginWarn.FiveMin_sec_counter = 0;
}

void  GB_SPK_preTired_clear(void)
{
    GB19056.SPK_PreTired.Warn_state_Enable = 0;
    GB19056.SPK_PreTired.group_playTimes = 0;
    GB19056.SPK_PreTired.FiveMin_sec_counter = 0;
}

void GB_serial_get_clear(void)
{
    Vdr_Wr_Rd_Offset.V_08H_get = 0;
    Vdr_Wr_Rd_Offset.V_09H_get = 0;
    Vdr_Wr_Rd_Offset.V_10H_get = 0;
    Vdr_Wr_Rd_Offset.V_11H_get = 0;
    Vdr_Wr_Rd_Offset.V_12H_get = 0;
    Vdr_Wr_Rd_Offset.V_13H_get = 0;
    Vdr_Wr_Rd_Offset.V_14H_get = 0;
    Vdr_Wr_Rd_Offset.V_15H_get = 0;
}

void GB_Drv_init(void)
{
    u8 i = 0;

    GB19056.workstate = 2;
    GB19056.RX_CMD = 0;
    GB19056.TX_CMD = 0;
    GB19056.RX_FCS = 0;
    GB19056.TX_FCS = 0;
    GB19056.rx_wr = 0;
    GB19056.rx_flag = 0;
    GB19056.u1_rx_timer = 0;
    GB19056.usb_write_step = GB_USB_OUT_idle;
    GB19056.usb_xor_fcs = 0;
    GB19056.speed_larger40km_counter = 0;
    GB19056.speedlog_Day_save_times = 0;
    GB19056.delataSpd_largerThan11_counter = 0;
    GB19056.gps_largeThan40_counter = 0;
    GB19056.start_record_speedlog_flag = 0;

    GB19056.DoubtData3_counter = 0;
    GB19056.DoubtData3_triggerFlag = 0;
    GB19056.Checking_status = 0;

    //  init 09  data
    for(i = 0; i < 60; i++)
    {
        VdrData.H_09[6 + i * 11] = 0x7F;
        VdrData.H_09[7 + i * 11] = 0xFF;
        VdrData.H_09[8 + i * 11] = 0xFF;
        VdrData.H_09[9 + i * 11] = 0xFF;
        VdrData.H_09[10 + i * 11] = 0x7F;
        VdrData.H_09[11 + i * 11] = 0xFF;
        VdrData.H_09[12 + i * 11] = 0xFF;
        VdrData.H_09[13 + i * 11] = 0xFF;
        VdrData.H_09[14 + i * 11] = 0x7F;
        VdrData.H_09[15 + i * 11] = 0xFF;
        VdrData.H_09[16 + i * 11] = 0x00;
    }

    //   ��ȫ��ʾ ��س�ʼ��
    GB_SPK_DriveExceed_clear();
    GB_SPK_SpeedStatus_Abnormal_clear();
    GB_SPK_Speed_Warn_clear();
    GB_SPK_UnloginWarn_clear();
    GB_SPK_preTired_clear();
}

//  ���갲ȫ��ʾ ����                                    Note:  ���� 1s ��ʱ��������
void  GB_Warn_Running(void)
{
    //   5. ��ʻԱδ��¼�����ٶ�
    if((GB19056.SPK_UnloginWarn.Warn_state_Enable) && (Spd_Using > 50) && (Sps_larger_5_counter > 10))
    {
        //    ��ǰ3  �� ���ȼ� ���е�����¿�ʼ
        if((GB19056.SPK_Speed_Warn.group_playTimes == 0) && (GB19056.SPK_SpeedStatus_Abnormal.group_playTimes == 0) && \
                (GB19056.SPK_DriveExceed_Warn.group_playTimes == 0) && (GB19056.SPK_PreTired.group_playTimes == 0))
        {
            //  5.0.	firs trigger
            if(GB19056.SPK_UnloginWarn.Warn_state_Enable == 1)
            {
                GB19056.SPK_UnloginWarn.Warn_state_Enable = 2;
                GB19056.SPK_UnloginWarn.group_playTimes = 1;
            }
            //5.1  play  operate
            if((GB19056.SPK_UnloginWarn.group_playTimes) && (TTS_Var.Playing == 0))
            {
                GB19056.SPK_UnloginWarn.group_playTimes++;
                if(Warn_Play_controlBit & 0x01)
                    TTS_play( "��ʻԱδ��¼��忨��¼");
                //rt_kprintf("\r\n-----��ʻԱδ��¼��忨��¼\r\n");
                if(GB19056.SPK_UnloginWarn.group_playTimes >= 4)
                    GB19056.SPK_UnloginWarn.group_playTimes = 0;
                return;
            }
            // 5.2	timer
            GB19056.SPK_UnloginWarn.FiveMin_sec_counter++;
            if(GB19056.SPK_UnloginWarn.FiveMin_sec_counter >= 300)
            {
                GB19056.SPK_UnloginWarn.group_playTimes = 1;
                GB19056.SPK_UnloginWarn.FiveMin_sec_counter = 0;
            }

        }

    }
    else
        GB_SPK_UnloginWarn_clear();
    //   4.  ƣ�ͼ�ʻ�����ѹ���
    if(GB19056.SPK_PreTired.Warn_state_Enable)
    {

        //	 ��ǰ3	�� ���ȼ� ���е�����¿�ʼ
        if((GB19056.SPK_Speed_Warn.group_playTimes == 0) && (GB19056.SPK_SpeedStatus_Abnormal.group_playTimes == 0) && \
                (GB19056.SPK_DriveExceed_Warn.group_playTimes == 0))
        {
            //  4.0.  firs trigger
            if(GB19056.SPK_PreTired.Warn_state_Enable == 1)
            {
                GB19056.SPK_PreTired.Warn_state_Enable = 2;
                GB19056.SPK_PreTired.group_playTimes = 1;
            }
            //  4.1	play  operate
            if((GB19056.SPK_PreTired.group_playTimes) && (TTS_Var.Playing == 0))
            {
                GB19056.SPK_PreTired.group_playTimes++;
                if(Warn_Play_controlBit & 0x02)
                    TTS_play( "��������ʱ��ʻ,��ע����Ϣ" );
                if(GB19056.SPK_PreTired.group_playTimes >= 4)
                    GB19056.SPK_PreTired.group_playTimes = 0;
                return;
            }
            // 4.2	 timer
            GB19056.SPK_PreTired.FiveMin_sec_counter++;
            if(GB19056.SPK_PreTired.FiveMin_sec_counter >= 300)
            {
                GB19056.SPK_PreTired.group_playTimes = 1;
                GB19056.SPK_PreTired.FiveMin_sec_counter = 0;
            }

        }


    }
    else
        GB_SPK_preTired_clear();
    //   3.     ��ʱ��ʻ
    if(GB19056.SPK_DriveExceed_Warn.Warn_state_Enable)
    {
        //   ��ǰ2��  ���ȼ����е������
        if((GB19056.SPK_Speed_Warn.group_playTimes == 0) && (GB19056.SPK_SpeedStatus_Abnormal.group_playTimes == 0))
        {

            //  3.0.  firs trigger
            if(GB19056.SPK_DriveExceed_Warn.Warn_state_Enable == 1)
            {
                GB19056.SPK_DriveExceed_Warn.Warn_state_Enable = 2;
                GB19056.SPK_DriveExceed_Warn.group_playTimes = 1;
            }
            //  3.1  play  operate
            if((GB19056.SPK_DriveExceed_Warn.group_playTimes) && (TTS_Var.Playing == 0))
            {
                GB19056.SPK_DriveExceed_Warn.group_playTimes++;
                if(Warn_Play_controlBit & 0x04)
                    TTS_play( "���ѳ�ʱ��ʻ����ע����Ϣ" );
                if(GB19056.SPK_DriveExceed_Warn.group_playTimes >= 4)
                    GB19056.SPK_DriveExceed_Warn.group_playTimes = 0;
                return;
            }
            // 3.2    timer
            GB19056.SPK_DriveExceed_Warn.FiveMin_sec_counter++;
            if(GB19056.SPK_DriveExceed_Warn.FiveMin_sec_counter >= 300)
            {
                GB19056.SPK_DriveExceed_Warn.group_playTimes = 1;
                GB19056.SPK_DriveExceed_Warn.FiveMin_sec_counter = 0;
            }

        }

    }
    else
        GB_SPK_DriveExceed_clear();
    //  2.   �ٶ��쳣
    if(GB19056.SPK_SpeedStatus_Abnormal.Warn_state_Enable)
    {
        //   ��ǰ1�����ȼ� ��������½���
        if(GB19056.SPK_Speed_Warn.group_playTimes == 0)
        {
            //  2.0.  firs trigger
            if(GB19056.SPK_SpeedStatus_Abnormal.Warn_state_Enable == 1)
            {
                GB19056.SPK_SpeedStatus_Abnormal.Warn_state_Enable = 2;
                GB19056.SPK_SpeedStatus_Abnormal.group_playTimes = 1;
            }
            //  2.1  play  operate
            if((GB19056.SPK_SpeedStatus_Abnormal.group_playTimes) && (TTS_Var.Playing == 0))
            {
                GB19056.SPK_SpeedStatus_Abnormal.group_playTimes++;
                if(Warn_Play_controlBit & 0x08)
                    TTS_play( "�ٶ�״̬�쳣���밲ȫ��ʻ" );

                if(GB19056.SPK_SpeedStatus_Abnormal.group_playTimes >= 4)
                {
                    GB19056.SPK_SpeedStatus_Abnormal.group_playTimes = 0;
                    GB19056.SPK_SpeedStatus_Abnormal.Warn_state_Enable = 0; // clear     �ٶ��쳣һ��һ��
                }
                return;
            }
            // 2.2    timer
            GB19056.SPK_SpeedStatus_Abnormal.FiveMin_sec_counter++;
            if(GB19056.SPK_SpeedStatus_Abnormal.FiveMin_sec_counter >= 300)
            {
                GB19056.SPK_SpeedStatus_Abnormal.group_playTimes = 1;
                GB19056.SPK_SpeedStatus_Abnormal.FiveMin_sec_counter = 0;
            }


        }
    }
    else
        GB_SPK_SpeedStatus_Abnormal_clear();
    //   1.���ٱ���
    if(GB19056.SPK_Speed_Warn.Warn_state_Enable)
    {
        //  1.0.  firs trigger
        if(GB19056.SPK_Speed_Warn.Warn_state_Enable == 1)
        {
            GB19056.SPK_Speed_Warn.Warn_state_Enable = 2;
            GB19056.SPK_Speed_Warn.group_playTimes = 1;
        }
        //  1.1  play  operate
        if((GB19056.SPK_Speed_Warn.group_playTimes) && (TTS_Var.Playing == 0))
        {
            GB19056.SPK_Speed_Warn.group_playTimes++;
            if(Warn_Play_controlBit & 0x10)
                TTS_play( "���ѳ����밲ȫ��ʻ" );
            // rt_kprintf("\r\n-----���ѳ����밲ȫ��ʻ\r\n");
            if(GB19056.SPK_Speed_Warn.group_playTimes >= 4)
                GB19056.SPK_Speed_Warn.group_playTimes = 0;
            return;
        }
        // 1.2    timer
        GB19056.SPK_Speed_Warn.FiveMin_sec_counter++;
        if(GB19056.SPK_Speed_Warn.FiveMin_sec_counter >= 300)
        {
            GB19056.SPK_Speed_Warn.group_playTimes = 1;
            GB19056.SPK_Speed_Warn.FiveMin_sec_counter = 0;
        }

    }
    else
        GB_SPK_Speed_Warn_clear();



}

//    �¹��ɵ�3 �����о�: ���������ٶȣ�����λ��10s  ���仯
void GB_doubt_Data3_Trigger(u32  lati_old, u32 longi_old, u32  lati_new, u32 longi_new)
{
    u32   Delta_lati = 0, Delta_longi = 0;

    Delta_lati = abs(lati_old - lati_new);
    Delta_longi = abs(longi_old - longi_new);

    // rt_kprintf("\r\ndeltalati: %d    deltati_longi: %d\r\n",Delta_lati,Delta_longi);

    if((Spd_Using > 50) && (Sps_larger_5_counter > 10) && (Delta_lati < 20) && (Delta_longi < 20) && (GB19056.DoubtData3_triggerFlag == 0))
    {
        GB19056.DoubtData3_counter++;
        if(GB19056.DoubtData3_counter == 21)
        {
            time_now = Get_RTC();	 //  RTC  ���
            Time2BCD(VdrData.H_10);
        }
        if(GB19056.DoubtData3_counter == 23)
        {
            time_now = Get_RTC();	 //  RTC  ���
            Time2BCD(Doubt_u8);
        }
        if(GB19056.DoubtData3_counter >= 30)
        {
            GB19056.DoubtData3_counter = 0;
            GB19056.DoubtData3_triggerFlag = 1; // enable
            moni_drv(0x10, 10);
            DOUBT3 = 1;
        }
    }
    else if(!((Spd_Using > 50) && (Delta_lati < 10) && (Delta_longi < 10)))
    {
        /*  if(DOUBT3)
          	{
          	     memcpy(VdrData.H_10,Doubt_u8,6);
        		 moni_drv(0x10,10);
        		 DOUBT3=0;
        		 //  rt_kprintf("\r\n  --2");
          	}*/
        GB19056.DoubtData3_counter = 0;
        GB19056.DoubtData3_triggerFlag = 0; // clear
    }

}

//    �ٶ�״̬��־�ж�
void  GB_SpeedSatus_Judge(void)
{
    u16  deltaSpd = 0;
    u8  spd_reg = 0;

    // ֻҪGPS �ٶ�(�ο��ٶȴ���40 �� �ж�)
    if(Speed_gps > 400)
    {
        //0 . �ο��ٶȴ���40 ��ʼ����
        GB19056.gps_largeThan40_counter++;
        //1.  �����ٶȲ�ֵ
        deltaSpd = abs(Speed_gps - Speed_cacu) * 100; // �ٶȲ�ֵ����100��
        // 2. ����11%  ������
        if((deltaSpd / Speed_gps) >= 11)
        {
            GB19056.delataSpd_largerThan11_counter++;
            //  ����ٶ�ƫ��һֱС��11% ,��ô�״δ���11%
            if(GB19056.delataSpd_largerThan11_counter == 0)
            {
                GB19056.gps_largeThan40_counter = 0; // clear
                GB19056.speed_larger40km_counter = 0; // clear
            }
        }
        else
        {
            //  ����ٶ�ƫ��һֱ����11% ,��ô�״�С��11%
            if(GB19056.delataSpd_largerThan11_counter)
            {
                GB19056.gps_largeThan40_counter = 0; // clear
                GB19056.speed_larger40km_counter = 0; // clear
            }
            GB19056.delataSpd_largerThan11_counter = 0;
        }
        // ��ʼ���������ж�
        if(GB19056.gps_largeThan40_counter)
        {
            if(GB19056.gps_largeThan40_counter == 2) // ��0�뿪ʼ
                GB19056.start_record_speedlog_flag = 1;

            if( (GB19056.start_record_speedlog_flag) && (VdrData.H_15_saveFlag == 0))
            {
                if((GB19056.speed_larger40km_counter == 0) && (UDP_dataPacket_flag == 0x02)) //
                {
                    //   1. ʱ�䷢����ʱ��
                    Time2BCD(VdrData.H_15 + 1);
                }
                //2.  set  15  speed
                spd_reg = Speed_cacu % 10;
                if(spd_reg >= 5)
                    spd_reg = Speed_cacu / 10 + 1;
                else
                    spd_reg = Speed_cacu / 10;
                VdrData.H_15[13 + 2 * GB19056.speed_larger40km_counter] = spd_reg;
                spd_reg = Speed_gps % 10;
                if(spd_reg >= 5)
                    spd_reg = Speed_gps / 10 + 1;
                else
                    spd_reg = Speed_gps / 10;
                VdrData.H_15[14 + 2 * GB19056.speed_larger40km_counter] = spd_reg;

                GB19056.speed_larger40km_counter++;
                if(GB19056.speed_larger40km_counter >= 60)
                {
                    if(GB19056.speed_larger40km_counter == 60)
                    {
                        VdrData.H_15_saveFlag = 2;
                    }
                }
            }
        }

        //    End Judge     5 �����Ժ�
        if(GB19056.gps_largeThan40_counter >= 299)
        {
            // �쳣�ͼ�����ֵ���
            if(GB19056.delataSpd_largerThan11_counter == GB19056.gps_largeThan40_counter)
            {
                VdrData.H_15[0] = 0x02;	//�쳣
                GB19056.SPK_SpeedStatus_Abnormal.Warn_state_Enable = 1;
            }
            else
                VdrData.H_15[0] = 0x01;

            //   1. ������ʱ��
            Time2BCD(VdrData.H_15 + 7);

            //  ʹ�ܴ洢
            VdrData.H_15_saveFlag = 1;

            GB19056.speedlog_Day_save_times++;
            if(GB19056.workstate == 0)
                rt_kprintf("\r\n  �ٶ�״̬��־����!:%d  total:%d  abnormal:%d\r\n", VdrData.H_15[0], GB19056.gps_largeThan40_counter, GB19056.delataSpd_largerThan11_counter);
            GB19056.speed_larger40km_counter = 0;	 // clear
            GB19056.delataSpd_largerThan11_counter = 0; // clear


            //---------------------------------------
            GB19056.gps_largeThan40_counter = 0; // clear
        }
    }
}

u8  BCD2HEX(u8 BCDbyte)
{
    u8  value = 0;

    value = (BCDbyte >> 4) * 10 + (BCDbyte & 0x0F);
    return  value;
}

u32 Time_sec_u32(u8 *instr, u8 len)
{
    u32  value = 0;

    if(len < 6)
        return  0;
    value = (BCD2HEX(instr[0]) * 356 + BCD2HEX(instr[1]) * 32 + BCD2HEX(instr[2])) * 86400 + \
            BCD2HEX(instr[3]) * 3600 + BCD2HEX(instr[4]) * 60 + BCD2HEX(instr[5]);

    return value;
}

u8  Timezone_judge(u32 Read_Time)
{
    if((Read_Time > GB19056.Query_Start_Time) && (Read_Time <= GB19056.Query_End_Time))
        return 0;
    else if(Read_Time <= GB19056.Query_Start_Time)	 // ��ǰʱ��С��С��
        return 2;  //ֱ��break
    else
        return  1; //��ǰʱ����ڴ�ģ������ң�ֱ��С�ڵ���С��
}


void  GB19056_Decode(u8 *instr, u8  len)
{
    TDateTime now;
    u32  reg_dis = 0;
    // AA 75   already   check     AA 75 08 00 01 00 BB
    //  cmd  ID
    switch(instr[2])
    {
    case  0x08:     //  �ɼ���¼��ָ������ʻ�ٶȼ�¼
    case  0x09:    //  �ɼ�ָ����ָ��λ����Ϣ��¼
    case  0x10:   // �ɼ��¹��ɵ��¼
    case  0x11:    //  �ɼ�ָ���ĳ�ʱ��ʻ��¼
    case  0x12:    //  �ɼ�ָ���ļ�ʻ����ݼ�¼
    case  0x13:    // �ɼ�ָ�����ⲿ�����¼
    case  0x14:    //  �ɼ�ָ���Ĳ����޸ļ�¼
    case  0x15:    // �ɼ�ָ�����ٶ�״̬��־
        GB19056.Query_Start_Time = Time_sec_u32(instr + 6, 6);
        GB19056.Query_End_Time = Time_sec_u32(instr + 12, 6);
        GB19056.Max_dataBlocks = (instr[18] << 8) + instr[19];



    case  0x00:   //�ɼ���¼��ִ�а汾
    case  0x01:   //  �ɼ���ʻ����Ϣ
    case  0x02:   // �ɼ���¼��ʱ��
    case  0x03:   //  �ɼ��ۼ���ʻ���
    case  0x04:   // �ɼ���¼������ϵ��
    case  0x05:   // �ɼ�������Ϣ
    case  0x06:    // �ɼ���¼���ź�������Ϣ
    case  0x07:    //  �ɼ���¼��Ψһ�Ա��
    case  0xFE:    // ALL
    case  0xFB:	   //  Recommond
        GB_OUT_data(instr[2], GB_OUT_TYPE_Serial, 1, 1);
        break;
        //  ���ü�¼�����
    case 0x82: //	  �������ó��ƺ�
        memset(Vechicle_Info.Vech_VIN, 0, sizeof(Vechicle_Info.Vech_VIN));
        memset(Vechicle_Info.Vech_Num, 0, sizeof(Vechicle_Info.Vech_Num));
        memset(Vechicle_Info.Vech_Type, 0, sizeof(Vechicle_Info.Vech_Type));

        //-----------------------------------------------------------------------
        memcpy(Vechicle_Info.Vech_VIN, instr + 6, 17);
        memcpy(Vechicle_Info.Vech_Num, instr + 23, 12);
        memcpy(Vechicle_Info.Vech_Type, instr + 25, 12);

        DF_WriteFlashSector(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
        WatchDog_Feed();
        DF_WriteFlashSector(DF_VehicleBAK_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
        WatchDog_Feed();
        DF_WriteFlashSector(DF_VehicleBAK2_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
        GB_OUT_data(instr[2], GB_OUT_TYPE_Serial, 1, 1);
        VDR_product_14H(instr[2]);
        break;

    case 0xC2: //���ü�¼��ʱ��
        // ûɶ�ã������ظ����У�����GPSУ׼�͹���
        //  ������ ʱ���� BCD
        now.year = (instr[6] >> 4) * 10 + (instr[6] & 0x0F);
        now.month = (instr[7] >> 4) * 10 + (instr[7] & 0x0F);
        now.day = (instr[8] >> 4) * 10 + (instr[8] & 0x0F);
        now.hour = (instr[9] >> 4) * 10 + (instr[9] & 0x0F);
        now.min = (instr[10] >> 4) * 10 + (instr[10] & 0x0F);
        now.sec = (instr[11] >> 4) * 10 + (instr[11] & 0x0F);
        now.week = 1;
        Device_RTC_set(now);
        time_now = Get_RTC();
        GB_OUT_data(instr[2], GB_OUT_TYPE_Serial, 1, 1);
        VDR_product_14H(instr[2]);
        break;


    case 0xC3: //�����ٶ�����ϵ��������ϵ����
        // ǰ6 ���ǵ�ǰʱ��
        now.year = (instr[6] >> 4) * 10 + (instr[6] & 0x0F);
        now.month = (instr[7] >> 4) * 10 + (instr[7] & 0x0F);
        now.day = (instr[8] >> 4) * 10 + (instr[8] & 0x0F);
        now.hour = (instr[9] >> 4) * 10 + (instr[9] & 0x0F);
        now.min = (instr[10] >> 4) * 10 + (instr[10] & 0x0F);
        now.sec = (instr[11] >> 4) * 10 + (instr[11] & 0x0F);
        now.week = 1;
        Device_RTC_set(now);
        JT808Conf_struct.Vech_Character_Value = (instr[12] << 8) + (u32)instr[13]; // ����ϵ��  �ٶ�����ϵ��
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        time_now = Get_RTC();
        GB_OUT_data(instr[2], GB_OUT_TYPE_Serial, 1, 1);
        VDR_product_14H(instr[2]);
        break;
    case 0x83: //  ��¼�ǳ��ΰ�װʱ��

        memcpy(JT808Conf_struct.FirstSetupDate, instr + 6, 6);
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        GB_OUT_data(instr[2], GB_OUT_TYPE_Serial, 1, 1);
        VDR_product_14H(instr[2]);
        break;
    case 0x84: // �����ź���������Ϣ
        //memcpy(Setting08,instr+6,80);
        GB_OUT_data(instr[2], GB_OUT_TYPE_Serial, 1, 1);
        VDR_product_14H(instr[2]);
        break;

    case 0xC4: //   ���ó�ʼ���

        now.year = (instr[6] >> 4) * 10 + (instr[6] & 0x0F);
        now.month = (instr[7] >> 4) * 10 + (instr[7] & 0x0F);
        now.day = (instr[8] >> 4) * 10 + (instr[8] & 0x0F);
        now.hour = (instr[9] >> 4) * 10 + (instr[9] & 0x0F);
        now.min = (instr[10] >> 4) * 10 + (instr[10] & 0x0F);
        now.sec = (instr[11] >> 4) * 10 + (instr[11] & 0x0F);
        now.week = 1;
        Device_RTC_set(now);



        memcpy(JT808Conf_struct.FirstSetupDate, instr + 12, 6);
        //Api_Config_Recwrite_Large(jt808,0,(u8*)&JT808Conf_struct,sizeof(JT808Conf_struct));
        // instr     + 12      ���ó�ʼ��̵�λ��   BCD

        reg_dis = (instr[18] >> 4) * 10000000 + (instr[18] & 0x0F) * 1000000 + (instr[19] >> 4) * 100000 + (instr[19] & 0x0F) * 10000 \
                  +(instr[20] >> 4) * 1000 + (instr[20] & 0x0F) * 100 + (instr[21] >> 4) * 10 + (instr[21] & 0x0F);

        Distance_m_u32 = reg_dis * 100;
        DF_Write_RecordAdd(Distance_m_u32, DayStartDistance_32, TYPE_DayDistancAdd);
        JT808Conf_struct.DayStartDistance_32 = DayStartDistance_32;
        JT808Conf_struct.Distance_m_u32 = Distance_m_u32;
        time_now = Get_RTC();
        GB_OUT_data(instr[2], GB_OUT_TYPE_Serial, 1, 1);
        VDR_product_14H(instr[2]);
        break;
    case 0xE0:   //pMenuItem=&Menu_8_statechecking;  //  ��ʽ����û�м춨״̬
        //pMenuItem->show();
        // out_plus(0);
        GB_OUT_data(instr[2], GB_OUT_TYPE_Serial, 1, 1);
        break;

    case 0xE4:
        pMenuItem = &Menu_1_Idle;
        pMenuItem->show();
        // out_plus(0);
        GB_OUT_data(instr[2], GB_OUT_TYPE_Serial, 1, 1);
        break;

    case 0xE1:  //GB19056.Deltaplus_outEnble=3;
        GB_OUT_data(instr[2], GB_OUT_TYPE_Serial, 1, 1);
        break;
    case 0xE2: // output_spd(Speed_cacu);
        GB_OUT_data(instr[2], GB_OUT_TYPE_Serial, 1, 1);
        break;
    case 0xE3: // output_rtcplus(1);
        GB_OUT_data(instr[2], GB_OUT_TYPE_Serial, 1, 1);

        GB_OUT_data(instr[2], GB_OUT_TYPE_Serial, 1, 1);
        break;

    case  0xFF:    //��ԭ��ʾ״̬
        GB19056.workstate = 0;
        rt_kprintf(" GB_Data_disable\r\n");
        break;
    default:
        break;




    }



}

void  gb_usb_out(u8 ID)
{
    /*
               ������Ϣ��USB device
      */
    if(USB_Disk_RunStatus() == USB_FIND)
    {
        if(GB19056.usb_exacute_output == 2)
        {

            if(GB19056.workstate == 0)
                rt_kprintf("\r\n GB USB ���ݵ����� \r\n");
            return;
        }
        else
        {

            switch(ID)
            {
            case 0x00:
            case 0x01:
            case 0x02:
            case 0x03:
            case 0x04:
            case 0x05:
            case 0x06:
            case 0x07:
            case 0x08:
            case 0x09:
            case 0x10:
            case 0x11:
            case 0x12:
            case 0x13:
            case 0x14:
            case 0x15:

            case 0xFE:  // output all
            case 0xFB:  // recommond output
            case 0xFF:  //idle
                break;
            default:
                if(GB19056.workstate == 0)
                    rt_kprintf("\r\n ����ID ����!  idle:  FF    0x00~0x15     0xFE :out put all        0xFB : output  recommond info   \r\n");
                return;


            }
            GB19056.usb_write_step = GB_USB_OUT_start;
            GB19056.usb_exacute_output = 1;
            GB19056.usb_out_selectID = ID;


            if(GB19056.workstate == 0)
                rt_kprintf("\r\n USB ���ݵ���\r\n");
        }
    }
    else
    {
        if(GB19056.workstate == 0)
            rt_kprintf("\r\n û�м�⵽ USB device \r\n");
    }

}
FINSH_FUNCTION_EXPORT(gb_usb_out, gb_usb_out(ID) idle:  FF    0x00~0x15     0xFE : out put all        0xFB : output  recommond info   );
/*
    ( ����: CMD,
      ����: u8*discrip_str,
      ��ǰ�洢��¼��Ŀ:u16 Ownindex_num,
      ������¼�Ĵ�Сu16 indexSize)
*/
void GB_USBfile_stuff(u8 CMD, u8 *discrip_str, u16 Ownindex_num, u16 indexSize)
{
    u8  discr_str_local[18];
    u16  i = 0, pkg_counter = 0, get_indexnum = 0;
    u16  pakge_num = 0;
    u16 distr_len = strlen(discrip_str);
    u32  content_len = 0; // �����Ϣ����
    u32  regdis 			= 0, reg2 = 0;
    u8  pos_V = 0;
    u8   add_fcs_byte = 0;

    memset(discr_str_local, 0, sizeof(discr_str_local));
    memcpy(discr_str_local, discrip_str, distr_len);
    usb_outbuf_Wr = 0; // star from here
    if(CMD == 0x00)
    {
        // ��һ�� �����ݿ����
        usb_outbuf[usb_outbuf_Wr++] = 0x00;
        usb_outbuf[usb_outbuf_Wr++] = 0x10;
    }

    // cmd
    usb_outbuf[usb_outbuf_Wr++] = CMD;
    // discription
    memcpy(usb_outbuf + usb_outbuf_Wr, discr_str_local, 18);
    usb_outbuf_Wr += 18;
    //    length
    if(CMD != 0x09)
    {
        content_len = indexSize * Ownindex_num;
        pakge_num = Ownindex_num;
    }
    else
    {
        pos_V = 0;
        for(i = 0; i < 60; i++)
        {
            if((VdrData.H_09[6 + i * 11] == 0x7F) && (VdrData.H_09[6 + i * 11 + 4] == 0x7F))
                pos_V++;   // λ����Ч���ۼ�
        }
        if(pos_V != 60)
        {
            content_len = indexSize * (Ownindex_num + 1); // 09 ��һ���ǵ�ǰ��
            pakge_num = Ownindex_num + 1;
        }
        else
        {
            content_len = indexSize * (Ownindex_num);
            pakge_num = Ownindex_num + 1;
        }
    }

    usb_outbuf[usb_outbuf_Wr++]   = (u8)(content_len >> 24);
    usb_outbuf[usb_outbuf_Wr++]   = (u8)(content_len >> 16);
    usb_outbuf[usb_outbuf_Wr++]   = (u8)(content_len >> 8);
    usb_outbuf[usb_outbuf_Wr++]   = (u8)(content_len);


    // content  stuff
    switch(CMD)
    {
    case 0x00:
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
    case 0x07:
        usb_outbuf_Wr += GB_00_07_info_only(usb_outbuf + usb_outbuf_Wr, CMD);
        USB_OUT;
        break;

    case 0x08:	    //  д������ͷ
        USB_OUT;
        // д������
        for(pkg_counter = 0; pkg_counter < pakge_num; pkg_counter++)
        {
            //  ÿ����¼дһ��
            usb_outbuf_Wr = 0;
            if(Vdr_Wr_Rd_Offset.V_08H_Write >= (pkg_counter + 1)) //û����
                get_indexnum = Vdr_Wr_Rd_Offset.V_08H_Write - 1 - pkg_counter;
            else
                get_indexnum = VDR_08_MAXindex + Vdr_Wr_Rd_Offset.V_08H_Write - 1 - pkg_counter;

            get_08h(get_indexnum, usb_outbuf + usb_outbuf_Wr);
            usb_outbuf_Wr += indexSize;
            USB_OUT;
        }
        break;
    case 0x09:
        //  д������ͷ
        USB_OUT;
        // д������
        for(pkg_counter = 0; pkg_counter < pakge_num; pkg_counter++)
        {
            //  ÿ����¼дһ��
            usb_outbuf_Wr = 0;
            if(Vdr_Wr_Rd_Offset.V_09H_Write >= pkg_counter) //û����
                get_indexnum = Vdr_Wr_Rd_Offset.V_09H_Write - pkg_counter;
            else
                get_indexnum = VDR_09_MAXindex + Vdr_Wr_Rd_Offset.V_09H_Write - pkg_counter;
            //------------------------------------------------------------------------------
            if(pkg_counter == 0)
            {
                // ��ǰСʱ������
                VdrData.H_09[0] = ((time_now.year / 10) << 4) + (time_now.year % 10);
                VdrData.H_09[1] = ((time_now.month / 10) << 4) + (time_now.month % 10);
                VdrData.H_09[2] = ((time_now.day / 10) << 4) + (time_now.day % 10);
                VdrData.H_09[3] = ((time_now.hour / 10) << 4) + (time_now.hour % 10);
                VdrData.H_09[4] = 0;
                VdrData.H_09[5] = 0;

                pos_V = 0;
                for(i = 0; i < 60; i++)
                {
                    if((VdrData.H_09[6 + i * 11] == 0x7F) && (VdrData.H_09[6 + i * 11 + 4] == 0x7F))
                        pos_V++;   // λ����Ч���ۼ�
                }
                if(pos_V != 60)
                {
                    memcpy(usb_outbuf + usb_outbuf_Wr, VdrData.H_09, 666);
                    usb_outbuf_Wr += indexSize;
                }
            }

            else
            {
                get_09h(get_indexnum, usb_outbuf + usb_outbuf_Wr);
                usb_outbuf_Wr += indexSize;
            }
            USB_OUT;
        }
        break;
    case 0x10:
        USB_OUT;
        // д������
        for(pkg_counter = 0; pkg_counter < pakge_num; pkg_counter++)
        {
            //  ÿ����¼дһ��
            usb_outbuf_Wr = 0;
            if(Vdr_Wr_Rd_Offset.V_10H_Write >= (pkg_counter + 1)) //û����
                get_indexnum = Vdr_Wr_Rd_Offset.V_10H_Write - 1 - pkg_counter;
            else
                get_indexnum = VDR_10_MAXindex + Vdr_Wr_Rd_Offset.V_10H_Write - 1 - pkg_counter;

            get_10h(get_indexnum, usb_outbuf + usb_outbuf_Wr);
            usb_outbuf_Wr += indexSize;
            USB_OUT;
        }
        break;
    case 0x11:
        USB_OUT;
        // д������
        for(pkg_counter = 0; pkg_counter < pakge_num; pkg_counter++)
        {
            //  ÿ����¼дһ��
            usb_outbuf_Wr = 0;
            if(Vdr_Wr_Rd_Offset.V_11H_Write >= (pkg_counter + 1)) //û����
                get_indexnum = Vdr_Wr_Rd_Offset.V_11H_Write - 1 - pkg_counter;
            else
                get_indexnum = VDR_11_MAXindex + Vdr_Wr_Rd_Offset.V_11H_Write - 1 - pkg_counter;

            get_11h(get_indexnum, usb_outbuf + usb_outbuf_Wr);
            usb_outbuf_Wr += indexSize;
            USB_OUT;
        }
        break;
    case 0x12:
        USB_OUT;
        // д������
        for(pkg_counter = 0; pkg_counter < pakge_num; pkg_counter++)
        {
            //  ÿ����¼дһ��
            usb_outbuf_Wr = 0;
            if(Vdr_Wr_Rd_Offset.V_12H_Write >= (pkg_counter + 1)) //û����
                get_indexnum = Vdr_Wr_Rd_Offset.V_12H_Write - 1 - pkg_counter;
            else
                get_indexnum = VDR_12_MAXindex + Vdr_Wr_Rd_Offset.V_12H_Write - 1 - pkg_counter;

            get_12h(get_indexnum, usb_outbuf + usb_outbuf_Wr);
            usb_outbuf_Wr += indexSize;
            USB_OUT;
        }
        break;
    case 0x13:
        USB_OUT;
        // д������
        for(pkg_counter = 0; pkg_counter < pakge_num; pkg_counter++)
        {
            //  ÿ����¼дһ��
            usb_outbuf_Wr = 0;
            if(Vdr_Wr_Rd_Offset.V_13H_Write >= (pkg_counter + 1)) //û����
                get_indexnum = Vdr_Wr_Rd_Offset.V_13H_Write - 1 - pkg_counter;
            else
                get_indexnum = VDR_13_MAXindex + Vdr_Wr_Rd_Offset.V_13H_Write - 1 - pkg_counter;

            get_13h(get_indexnum, usb_outbuf + usb_outbuf_Wr);
            usb_outbuf_Wr += indexSize;
            USB_OUT;
        }
        break;
    case 0x14:
        USB_OUT;
        // д������
        for(pkg_counter = 0; pkg_counter < pakge_num; pkg_counter++)
        {
            //  ÿ����¼дһ��
            usb_outbuf_Wr = 0;
            if(Vdr_Wr_Rd_Offset.V_14H_Write >= (pkg_counter + 1)) //û����
                get_indexnum = Vdr_Wr_Rd_Offset.V_14H_Write - 1 - pkg_counter;
            else
                get_indexnum = VDR_14_MAXindex + Vdr_Wr_Rd_Offset.V_14H_Write - 1 - pkg_counter;

            get_14h(get_indexnum, usb_outbuf + usb_outbuf_Wr);
            usb_outbuf_Wr += indexSize;
            USB_OUT;
        }
        break;
    case 0x15:
        USB_OUT;
        // д������
        if(pakge_num == 0)
        {
            add_fcs_byte = 1;
            USB_OUT;
            break;
        }
        for(pkg_counter = 0; pkg_counter < pakge_num; pkg_counter++)
        {
            //  ÿ����¼дһ��
            usb_outbuf_Wr = 0;
            if(Vdr_Wr_Rd_Offset.V_15H_Write >= (pkg_counter + 1)) //û����
                get_indexnum = Vdr_Wr_Rd_Offset.V_15H_Write - 1 - pkg_counter;
            else
                get_indexnum = VDR_15_MAXindex + Vdr_Wr_Rd_Offset.V_15H_Write - 1 - pkg_counter;

            get_15h(get_indexnum, usb_outbuf + usb_outbuf_Wr);
            usb_outbuf_Wr += indexSize;

            if((pkg_counter + 1) == pakge_num)
                add_fcs_byte = 1;
            USB_OUT;
        }
        break;

    }

}


#if 0
void GB_serial_OUT(void )
{
    if(type == GB_OUT_TYPE_Serial)
    {
        //---------------------------------------------------------------------
        if(Vdr_Wr_Rd_Offset.V_08H_get == 0)
        {
            GB_serial_get_clear();
            Vdr_Wr_Rd_Offset.V_08H_get = 1;



            GB19056.Send_add = 0; // clear
        }
        //--------------------------------------------------------------------
        usb_outbuf_Wr = 0;
        usb_outbuf[usb_outbuf_Wr++]   = 0x55;					// ��ʼͷ
        usb_outbuf[usb_outbuf_Wr++]   = 0x7A;
        usb_outbuf[usb_outbuf_Wr++]   = 0x08;					//������


        SregLen = GB19056.Max_dataBlocks * 126; //

        usb_outbuf[usb_outbuf_Wr++]   = (SregLen >> 8);			// Hi
        usb_outbuf[usb_outbuf_Wr++]   = SregLen;				// Lo

        usb_outbuf[usb_outbuf_Wr++] = 0x00; 					// ������
        //	��Ϣ����
        WatchDog_Feed( );
        Not_null_flag = 0;
        continue_counter = 0;

        if(GB19056.Send_add < GB19056.Total_dataBlocks)
        {

            //---------------------------------------------------------------------
            for(subi = GB19056.Send_add; subi < GB19056.Total_dataBlocks; subi++)
            {
                if(Vdr_Wr_Rd_Offset.V_08H_Write >= (subi + 1)) //û����
                    get_indexnum = Vdr_Wr_Rd_Offset.V_08H_Write - 1 - subi;
                else
                    get_indexnum = VDR_08_MAXindex + Vdr_Wr_Rd_Offset.V_08H_Write - 1 - subi;

                get_08h(get_indexnum, usb_outbuf + usb_outbuf_Wr); //126*5=630		 num=576  packet
                //-----time  filter ------------
                Rd_Time = Time_sec_u32(usb_outbuf + usb_outbuf_Wr, 6);
                return_value = Timezone_judge(Rd_Time);

                if(return_value == 2) // end
                {
                    GB_serial_get_clear();// clear
                    GB_null_content(0x08, type);
                    return;
                }
                if(return_value == 1) // ����
                {
                    continue;
                }
                else
                    usb_outbuf_Wr += 126;
                continue_counter++;
                if(continue_counter == GB19056.Max_dataBlocks)
                    break;
            }
            //---------------------------------------------------
            if(continue_counter != GB19056.Max_dataBlocks) // �����Ϣ��Ϊ�գ�����
            {
                // ---caculate  infolen
                SregLen = continue_counter * 126; // ��û���õ���Ϣȥ��
                usb_outbuf[3]   = (SregLen >> 8);			// Hi
                usb_outbuf[4]   = SregLen;				// Lo
            }
            // -----update---
            GB19056.Send_add += continue_counter;
            OUT_PUT;


        }
        else
        {
            GB_serial_get_clear();// clear
            GB_null_content(0x08, type);
        }




        //----------------------------------------------------------------------
    }





}
#endif

void GB_OUT_data(u8  ID, u8 type, u16 IndexNum, u16 total_pkg_num)
{


    u32	regdis			   = 0, reg2 = 0;
    u16	SregLen = 0;
    u16  i = 0, subi = 0, continue_counter = 0;
    u16   get_indexnum = 0; // ��ȡУ��
    //u16  Sd_DataBloknum=0; //   �������ݿ�  ��Ŀ <GB19056.Max_dataBlocks
    u8   Not_null_flag = 0; // NO  data ACK null once
    u8   return_value = 0;
    u32  Rd_Time = 0;

    switch(ID)
    {
    case 0x00:  //  ִ�б�׼�汾���
        if(type == GB_OUT_TYPE_USB)
        {
            GB_USBfile_stuff(ID, "ִ�б�׼�汾���", 1, 2);
            return;
        }
        //------------------------------------------------------------
        usb_outbuf_Wr = 0;
        usb_outbuf_Wr += GB_557A_protocol_00_07_stuff(usb_outbuf, ID);
        break;
    case 0x01:

        //---------------- �ϴ�������  -------------------------------------
        //	01	��ǰ��ʻԱ���뼰��Ӧ�Ļ�������ʻ֤��
        if(type == GB_OUT_TYPE_USB)
        {
            GB_USBfile_stuff(ID, "��ǰ��ʻ����Ϣ", 1, 18);
            return;
        }
        //------------------------------------------------------------
        usb_outbuf_Wr = 0;
        usb_outbuf_Wr += GB_557A_protocol_00_07_stuff(usb_outbuf, ID);
        break;
    case 0x02: //  02  �ɼ���¼�ǵ�ʵʱʱ��
        if(type == GB_OUT_TYPE_USB)
        {
            GB_USBfile_stuff(ID, "ʵʱʱ��", 1, 6);
            return;
        }
        //------------------------------------------------------------
        usb_outbuf_Wr = 0;
        usb_outbuf_Wr += GB_557A_protocol_00_07_stuff(usb_outbuf, ID);
        break;

    case 0x03:		// 03 �ɼ�360h�����
        if(type == GB_OUT_TYPE_USB)
        {
            GB_USBfile_stuff(ID, "�ۼ���ʻ���", 1, 20);
            return;
        }
        //------------------------------------------------------------
        usb_outbuf_Wr = 0;												 // 03 �ɼ�360h�����
        usb_outbuf_Wr += GB_557A_protocol_00_07_stuff(usb_outbuf, ID);
        break;

    case 0x04:																	// 04  �ɼ���¼������ϵ��
        if(type == GB_OUT_TYPE_USB)
        {
            GB_USBfile_stuff(ID, "����ϵ��", 1, 8);
            return;
        }
        //------------------------------------------------------------

        usb_outbuf_Wr = 0;
        usb_outbuf_Wr += GB_557A_protocol_00_07_stuff(usb_outbuf, ID);
        break;
    case 0x05:																	//05	  ������Ϣ�ɼ�
        if(type == GB_OUT_TYPE_USB)
        {
            GB_USBfile_stuff(ID, "������Ϣ", 1, 41);
            return;
        }
        //------------------------------------------------------------

        usb_outbuf_Wr = 0;
        usb_outbuf_Wr += GB_557A_protocol_00_07_stuff(usb_outbuf, ID);

        break;
    case   0x06:													// 06-09
        if(type == GB_OUT_TYPE_USB)
        {
            GB_USBfile_stuff(ID, "״̬�ź�������Ϣ", 1, 87);
            return;
        }
        //------------------------------------------------------------
        usb_outbuf_Wr = 0;
        usb_outbuf_Wr += GB_557A_protocol_00_07_stuff(usb_outbuf, ID);
        break;

    case 0x07:																//07  ��¼��Ψһ���
        if(type == GB_OUT_TYPE_USB)
        {
            GB_USBfile_stuff(ID, "��¼��Ψһ�Ա��", 1, 35);
            return;
        }
        //------------------------------------------------------------
        usb_outbuf_Wr = 0;
        usb_outbuf_Wr += GB_557A_protocol_00_07_stuff(usb_outbuf, ID);
        break;
    case 0x08:															//	08	 �ɼ�ָ������ʻ�ٶȼ�¼
        //  ��ȡ��ǰ�ܰ���
        if(Vdr_Wr_Rd_Offset.V_08H_full)
        {
            GB19056.Total_dataBlocks = VDR_08_MAXindex;
        }
        else
            GB19056.Total_dataBlocks = Vdr_Wr_Rd_Offset.V_08H_Write;
        //   USB
        if(type == GB_OUT_TYPE_USB)
        {
            GB_USBfile_stuff(ID, "��ʻ�ٶȼ�¼", GB19056.Total_dataBlocks, 126);
            return;
        }
        // Serial
#if 1
        if(type == GB_OUT_TYPE_Serial)
        {
            //---------------------------------------------------------------------
            if(Vdr_Wr_Rd_Offset.V_08H_get == 0)
            {
                GB_serial_get_clear();
                Vdr_Wr_Rd_Offset.V_08H_get = 1;



                GB19056.Send_add = 0; // clear
            }
            //--------------------------------------------------------------------
            usb_outbuf_Wr = 0;
            usb_outbuf[usb_outbuf_Wr++]   = 0x55;					// ��ʼͷ
            usb_outbuf[usb_outbuf_Wr++]   = 0x7A;
            usb_outbuf[usb_outbuf_Wr++]   = 0x08;					//������


            SregLen = GB19056.Max_dataBlocks * 126; //

            usb_outbuf[usb_outbuf_Wr++]   = (SregLen >> 8);			// Hi
            usb_outbuf[usb_outbuf_Wr++]   = SregLen;				// Lo

            usb_outbuf[usb_outbuf_Wr++] = 0x00; 					// ������
            //	��Ϣ����
            WatchDog_Feed( );
            Not_null_flag = 0;
            continue_counter = 0;

            if(GB19056.Send_add < GB19056.Total_dataBlocks)
            {

                //---------------------------------------------------------------------
                for(subi = GB19056.Send_add; subi < GB19056.Total_dataBlocks; subi++)
                {
                    if(Vdr_Wr_Rd_Offset.V_08H_Write >= (subi + 1)) //û����
                        get_indexnum = Vdr_Wr_Rd_Offset.V_08H_Write - 1 - subi;
                    else
                        get_indexnum = VDR_08_MAXindex + Vdr_Wr_Rd_Offset.V_08H_Write - 1 - subi;

                    get_08h(get_indexnum, usb_outbuf + usb_outbuf_Wr); //126*5=630		 num=576  packet
                    //-----time  filter ------------
                    Rd_Time = Time_sec_u32(usb_outbuf + usb_outbuf_Wr, 6);
                    return_value = Timezone_judge(Rd_Time);

                    if(return_value) // end
                    {
                        continue;
                    }
                    else
                        usb_outbuf_Wr += 126;
                    continue_counter++;
                    if(continue_counter == GB19056.Max_dataBlocks)
                        break;
                }
                //---------------------------------------------------
                if(continue_counter != GB19056.Max_dataBlocks) // �����Ϣ��Ϊ�գ�����
                {
                    // ---caculate  infolen
                    SregLen = continue_counter * 126; // ��û���õ���Ϣȥ��
                    usb_outbuf[3]   = (SregLen >> 8);			// Hi
                    usb_outbuf[4]   = SregLen;				// Lo
                }
                // -----update---
                GB19056.Send_add += continue_counter;
                OUT_PUT;

                if(GB19056.Send_add >= GB19056.Total_dataBlocks)
                    GB_serial_get_clear();// clear


            }
            else
                GB_null_content(0x08, type);




            //----------------------------------------------------------------------
        }
#endif

        break;


    case   0x09:															// 09	ָ����λ����Ϣ��¼
        //  Get total effective num
        if(Vdr_Wr_Rd_Offset.V_09H_full)
        {
            GB19056.Total_dataBlocks = VDR_09_MAXindex;
        }
        else
            GB19056.Total_dataBlocks = Vdr_Wr_Rd_Offset.V_09H_Write;

        //   USB
        if(type == GB_OUT_TYPE_USB)
        {
            GB_USBfile_stuff(ID, "λ����Ϣ��¼", GB19056.Total_dataBlocks, 666);
            return;
        }
        //   Serial
#if 1
        if(type == GB_OUT_TYPE_Serial)
        {
            //---------------------------------------------------------------------
            if(Vdr_Wr_Rd_Offset.V_09H_get == 0)
            {
                GB_serial_get_clear();
                Vdr_Wr_Rd_Offset.V_09H_get = 1;

                GB19056.Send_add = 0; // clear
            }
            //--------------------------------------------------------------------
            usb_outbuf_Wr = 0;
            usb_outbuf[usb_outbuf_Wr++]   = 0x55;					// ��ʼͷ
            usb_outbuf[usb_outbuf_Wr++]   = 0x7A;
            usb_outbuf[usb_outbuf_Wr++]   = 0x09;					//������


            SregLen = 666;  //

            usb_outbuf[usb_outbuf_Wr++]   = (SregLen >> 8);			// Hi
            usb_outbuf[usb_outbuf_Wr++]   = SregLen;				// Lo

            usb_outbuf[usb_outbuf_Wr++] = 0x00; 					// ������
            //	��Ϣ����
            WatchDog_Feed( );
            Not_null_flag = 0;
            continue_counter = 0;

            if(GB19056.Send_add < (GB19056.Total_dataBlocks + 1))
            {

                for(subi = GB19056.Send_add; subi < (GB19056.Total_dataBlocks + 1); subi++)
                {
                    if(subi == 0)
                    {
                        // ��ǰСʱ������
                        VdrData.H_09[0] = ((time_now.year / 10) << 4) + (time_now.year % 10);
                        VdrData.H_09[1] = ((time_now.month / 10) << 4) + (time_now.month % 10);
                        VdrData.H_09[2] = ((time_now.day / 10) << 4) + (time_now.day % 10);
                        VdrData.H_09[3] = ((time_now.hour / 10) << 4) + (time_now.hour % 10);
                        VdrData.H_09[4] = 0;
                        VdrData.H_09[5] = 0;

                        memcpy(usb_outbuf + usb_outbuf_Wr, VdrData.H_09, 666);
                    }
                    else
                    {
                        if(Vdr_Wr_Rd_Offset.V_09H_Write >= subi) //û����
                            get_indexnum = Vdr_Wr_Rd_Offset.V_09H_Write - subi;
                        else
                            get_indexnum = VDR_09_MAXindex + Vdr_Wr_Rd_Offset.V_09H_Write - subi;

                        get_09h(get_indexnum, usb_outbuf + usb_outbuf_Wr);
                    }
                    //-----time  filter ------------
                    Rd_Time = Time_sec_u32(usb_outbuf + usb_outbuf_Wr, 6);
                    return_value = Timezone_judge(Rd_Time);

                    if(return_value) // end
                    {
                        continue;
                    }
                    else
                        usb_outbuf_Wr += 666;
                    continue_counter++;
                    break;
                }
                //---------------------------------------------------
                if(continue_counter == 0) // �����Ϣ��Ϊ�գ�����
                {
                    // ---caculate  infolen
                    SregLen = continue_counter * 666; // ��û���õ���Ϣȥ��
                    usb_outbuf[3]   = (SregLen >> 8);			// Hi
                    usb_outbuf[4]   = SregLen;				// Lo
                }
                // -----update---
                GB19056.Send_add += continue_counter;
                OUT_PUT;

                if(GB19056.Send_add >= GB19056.Total_dataBlocks)
                    GB_serial_get_clear();// clear

            }
            else
                GB_null_content(0x09, type);




            //----------------------------------------------------------------------
        }
#endif
        break;

    case   0x10:															// 10-13	 10   �¹��ɵ�ɼ���¼
        //�¹��ɵ�����   ------    �¹��ɵ�״̬�ǵ������е�
        //  Get total effective num
        if(Vdr_Wr_Rd_Offset.V_10H_full)
        {
            GB19056.Total_dataBlocks = VDR_10_MAXindex;
        }
        else
            GB19056.Total_dataBlocks = Vdr_Wr_Rd_Offset.V_10H_Write;

        //   USB
        if(type == GB_OUT_TYPE_USB)
        {
            GB_USBfile_stuff(ID, "�¹��ɵ��¼", GB19056.Total_dataBlocks, 234);
            return;
        }
        //  serial
#if 1
        if(type == GB_OUT_TYPE_Serial)
        {
            //---------------------------------------------------------------------
            if(Vdr_Wr_Rd_Offset.V_10H_get == 0)
            {
                GB_serial_get_clear();
                Vdr_Wr_Rd_Offset.V_10H_get = 1;

                GB19056.Send_add = 0; // clear
            }


            if(GB19056.Max_dataBlocks >= 5)
                GB19056.Max_dataBlocks = 4;
            //--------------------------------------------------------------------
            usb_outbuf_Wr = 0;
            usb_outbuf[usb_outbuf_Wr++]   = 0x55;					// ��ʼͷ
            usb_outbuf[usb_outbuf_Wr++]   = 0x7A;
            usb_outbuf[usb_outbuf_Wr++]   = 0x10;					//������


            SregLen = GB19056.Max_dataBlocks * 234; //

            usb_outbuf[usb_outbuf_Wr++]   = (SregLen >> 8);			// Hi
            usb_outbuf[usb_outbuf_Wr++]   = SregLen;				// Lo

            usb_outbuf[usb_outbuf_Wr++] = 0x00; 					// ������
            //	��Ϣ����
            WatchDog_Feed( );
            Not_null_flag = 0;
            continue_counter = 0;


            if(GB19056.Send_add < (GB19056.Total_dataBlocks))
            {

                for(subi = GB19056.Send_add; subi < GB19056.Total_dataBlocks; subi++)
                {

                    if(Vdr_Wr_Rd_Offset.V_10H_Write >= (subi + 1)) //û����
                        get_indexnum = Vdr_Wr_Rd_Offset.V_10H_Write - 1 - subi;
                    else
                        get_indexnum = VDR_10_MAXindex + Vdr_Wr_Rd_Offset.V_10H_Write - 1 - subi;

                    get_10h(get_indexnum, usb_outbuf + usb_outbuf_Wr);
                    //-----time  filter ------------
                    Rd_Time = Time_sec_u32(usb_outbuf + usb_outbuf_Wr, 6);
                    return_value = Timezone_judge(Rd_Time);
                    // rt_kprintf("\r\n return_value=%d \r\n",return_value);
                    if(return_value) // end
                    {
                        continue;
                    }
                    else
                        usb_outbuf_Wr += 234;
                    continue_counter++;
                    if(continue_counter == GB19056.Max_dataBlocks)
                        break;
                }
                //---------------------------------------------------
                if(continue_counter != GB19056.Max_dataBlocks) // �����Ϣ��Ϊ�գ�����
                {
                    // ---caculate  infolen
                    SregLen = continue_counter * 234; // ��û���õ���Ϣȥ��
                    usb_outbuf[3]   = (SregLen >> 8);			// Hi
                    usb_outbuf[4]   = SregLen;				// Lo
                }
                // -----update---
                GB19056.Send_add += continue_counter;
                OUT_PUT;

                if(GB19056.Send_add >= GB19056.Total_dataBlocks)
                    GB_serial_get_clear();// clear

            }
            else
                GB_null_content(0x10, type);




            //----------------------------------------------------------------------
        }
#endif

        break;


    case  0x11: 															// 11 �ɼ�ָ���ĵĳ�ʱ��ʻ��¼
        //  Get total effective num
        if(Vdr_Wr_Rd_Offset.V_11H_full)
        {
            GB19056.Total_dataBlocks = VDR_11_MAXindex;
        }
        else
            GB19056.Total_dataBlocks = Vdr_Wr_Rd_Offset.V_11H_Write;

        //   USB
        if(type == GB_OUT_TYPE_USB)
        {
            GB_USBfile_stuff(ID, "��ʱ��ʻ��¼", GB19056.Total_dataBlocks, 50);
            return;
        }
        //  serial
#if 1
        if(type == GB_OUT_TYPE_Serial)
        {
            if(VdrData.H_11_lastSave)
            {
                GB19056.Total_dataBlocks = 1 + Vdr_Wr_Rd_Offset.V_11H_Write;
                GB_serial_get_clear();
            }
            //---------------------------------------------------------------------
            if(Vdr_Wr_Rd_Offset.V_11H_get == 0)
            {
                GB_serial_get_clear();
                Vdr_Wr_Rd_Offset.V_11H_get = 1;

                GB19056.Send_add = 0; // clear
            }
            //--------------------------------------------------------------------
            usb_outbuf_Wr = 0;
            usb_outbuf[usb_outbuf_Wr++]   = 0x55;					// ��ʼͷ
            usb_outbuf[usb_outbuf_Wr++]   = 0x7A;
            usb_outbuf[usb_outbuf_Wr++]   = 0x11;					//������

            if(VdrData.H_11_lastSave)
                SregLen = GB19056.Max_dataBlocks * 50;
            else
                SregLen = 50;  //

            usb_outbuf[usb_outbuf_Wr++]   = (SregLen >> 8);			// Hi
            usb_outbuf[usb_outbuf_Wr++]   = SregLen;				// Lo

            usb_outbuf[usb_outbuf_Wr++] = 0x00; 					// ������
            //	��Ϣ����
            WatchDog_Feed( );
            Not_null_flag = 0;
            continue_counter = 0;

            if(GB19056.Send_add < GB19056.Total_dataBlocks)
            {

                for(subi = GB19056.Send_add; subi < GB19056.Total_dataBlocks; subi++)
                {
                    if(VdrData.H_11_lastSave)
                        get_indexnum = Vdr_Wr_Rd_Offset.V_11H_Write;
                    else
                    {
                        if(Vdr_Wr_Rd_Offset.V_11H_Write >= (subi + 1)) //û����
                            get_indexnum = Vdr_Wr_Rd_Offset.V_11H_Write - 1 - subi;
                        else
                            get_indexnum = VDR_11_MAXindex + Vdr_Wr_Rd_Offset.V_11H_Write - 1 - subi;
                    }

                    get_11h(get_indexnum, usb_outbuf + usb_outbuf_Wr);
                    //-----time  filter ------------
                    Rd_Time = Time_sec_u32(usb_outbuf + usb_outbuf_Wr + 18, 6);
                    return_value = Timezone_judge(Rd_Time);

                    if(return_value) // end
                    {
                        continue;
                    }
                    else
                        usb_outbuf_Wr += 50;
                    continue_counter++;
                    if(VdrData.H_11_lastSave)
                        break;
                    else if(continue_counter == GB19056.Max_dataBlocks)
                        break;

                }
                //---------------------------------------------------
                if(continue_counter != GB19056.Max_dataBlocks) // �����Ϣ��Ϊ�գ�����
                {
                    // ---caculate  infolen
                    SregLen = continue_counter * 50; // ��û���õ���Ϣȥ��
                    usb_outbuf[3]   = (SregLen >> 8);			// Hi
                    usb_outbuf[4]   = SregLen;				// Lo
                }
                // -----update---
                if(VdrData.H_11_lastSave == 0)
                    GB19056.Send_add += continue_counter;
                OUT_PUT;

                if(GB19056.Send_add >= GB19056.Total_dataBlocks)
                    GB_serial_get_clear();// clear

            }
            else
                GB_null_content(0x11, type);




            //----------------------------------------------------------------------
        }
#endif

        break;
    case  0x12: 															// 12 �ɼ�ָ����ʻ����ݼ�¼  ---Devide
        //	Get total effective num
        if(Vdr_Wr_Rd_Offset.V_12H_full)
        {
            GB19056.Total_dataBlocks = VDR_12_MAXindex;
        }
        else
            GB19056.Total_dataBlocks = Vdr_Wr_Rd_Offset.V_12H_Write;

        //   USB
        if(type == GB_OUT_TYPE_USB)
        {
            GB_USBfile_stuff(ID, "��ʻ����ݼ�¼", GB19056.Total_dataBlocks, 25);
            return;
        }
        //  serial

#if 1
        if(type == GB_OUT_TYPE_Serial)
        {
            //---------------------------------------------------------------------
            if(Vdr_Wr_Rd_Offset.V_12H_get == 0)
            {
                GB_serial_get_clear();
                Vdr_Wr_Rd_Offset.V_12H_get = 1;

                GB19056.Send_add = 0; // clear
            }
            //--------------------------------------------------------------------
            usb_outbuf_Wr = 0;
            usb_outbuf[usb_outbuf_Wr++]   = 0x55;					 // ��ʼͷ
            usb_outbuf[usb_outbuf_Wr++]   = 0x7A;
            usb_outbuf[usb_outbuf_Wr++]   = 0x12;					 //������

            SregLen = GB19056.Max_dataBlocks * 25;	//

            usb_outbuf[usb_outbuf_Wr++]   = (SregLen >> 8);		 // Hi
            usb_outbuf[usb_outbuf_Wr++]   = SregLen;				 // Lo

            usb_outbuf[usb_outbuf_Wr++] = 0x00;					 // ������
            //  ��Ϣ����
            WatchDog_Feed( );
            Not_null_flag = 0;
            continue_counter = 0;

            if(GB19056.Send_add < (GB19056.Total_dataBlocks))
            {

                for(subi = GB19056.Send_add; subi < GB19056.Total_dataBlocks; subi++)
                {

                    if(Vdr_Wr_Rd_Offset.V_12H_Write >= (subi + 1)) //û����
                        get_indexnum = Vdr_Wr_Rd_Offset.V_12H_Write - 1 - subi;
                    else
                        get_indexnum = VDR_12_MAXindex + Vdr_Wr_Rd_Offset.V_12H_Write - 1 - subi;

                    get_12h(get_indexnum, usb_outbuf + usb_outbuf_Wr);
                    //-----time  filter ------------
                    Rd_Time = Time_sec_u32(usb_outbuf + usb_outbuf_Wr, 6);
                    return_value = Timezone_judge(Rd_Time);

                    if(return_value) // end
                    {
                        continue;
                    }
                    else
                        usb_outbuf_Wr += 25;
                    continue_counter++;
                    if(continue_counter == GB19056.Max_dataBlocks)
                        break;
                }
                //---------------------------------------------------
                if(continue_counter != GB19056.Max_dataBlocks) // �����Ϣ��Ϊ�գ�����
                {
                    // ---caculate  infolen
                    SregLen = continue_counter * 25; // ��û���õ���Ϣȥ��
                    usb_outbuf[3]	= (SregLen >> 8);			// Hi
                    usb_outbuf[4]	= SregLen;				// Lo
                }
                // -----update---
                GB19056.Send_add += continue_counter;
                OUT_PUT;


                if(GB19056.Send_add >= GB19056.Total_dataBlocks)
                    GB_serial_get_clear();// clear
            }
            else
                GB_null_content(0x12, type);




            //----------------------------------------------------------------------
        }
#endif

        break;


    case  0x13: 													// 13 �ɼ���¼���ⲿ�����¼
        //	Get total effective num
        if(Vdr_Wr_Rd_Offset.V_13H_full)
        {
            GB19056.Total_dataBlocks = VDR_13_MAXindex;
        }
        else
            GB19056.Total_dataBlocks = Vdr_Wr_Rd_Offset.V_13H_Write;

        //   USB
        if(type == GB_OUT_TYPE_USB)
        {
            GB_USBfile_stuff(ID, "�ⲿ�����¼", GB19056.Total_dataBlocks, 7);
            return;
        }
        //  serial
#if 1
        if(type == GB_OUT_TYPE_Serial)
        {
            //---------------------------------------------------------------------
            if(Vdr_Wr_Rd_Offset.V_13H_get == 0)
            {
                GB_serial_get_clear();
                Vdr_Wr_Rd_Offset.V_13H_get = 1;



                GB19056.Send_add = 0; // clear
            }
            //--------------------------------------------------------------------
            usb_outbuf_Wr = 0;
            usb_outbuf[usb_outbuf_Wr++]   = 0x55;					// ��ʼͷ
            usb_outbuf[usb_outbuf_Wr++]   = 0x7A;
            usb_outbuf[usb_outbuf_Wr++]   = 0x13;					//������


            SregLen = GB19056.Max_dataBlocks * 7; //

            usb_outbuf[usb_outbuf_Wr++]   = (SregLen >> 8);			// Hi
            usb_outbuf[usb_outbuf_Wr++]   = SregLen;				// Lo

            usb_outbuf[usb_outbuf_Wr++] = 0x00; 					// ������
            //	��Ϣ����
            WatchDog_Feed( );
            Not_null_flag = 0;
            continue_counter = 0;

            if(GB19056.Send_add < (GB19056.Total_dataBlocks))
            {

                for(subi = GB19056.Send_add; subi < GB19056.Total_dataBlocks; subi++)
                {

                    if(Vdr_Wr_Rd_Offset.V_13H_Write >= (subi + 1)) //û����
                        get_indexnum = Vdr_Wr_Rd_Offset.V_13H_Write - 1 - subi;
                    else
                        get_indexnum = VDR_11_MAXindex + Vdr_Wr_Rd_Offset.V_13H_Write - 1 - subi;

                    get_13h(get_indexnum, usb_outbuf + usb_outbuf_Wr);
                    //-----time  filter ------------
                    Rd_Time = Time_sec_u32(usb_outbuf + usb_outbuf_Wr, 6);
                    return_value = Timezone_judge(Rd_Time);

                    if(return_value) // end
                    {
                        continue;
                    }
                    else
                        usb_outbuf_Wr += 7;
                    continue_counter++;
                    if(continue_counter == GB19056.Max_dataBlocks)
                        break;
                }
                //---------------------------------------------------
                if(continue_counter != GB19056.Max_dataBlocks) // �����Ϣ��Ϊ�գ�����
                {
                    // ---caculate  infolen
                    SregLen = continue_counter * 7; // ��û���õ���Ϣȥ��
                    usb_outbuf[3]	= (SregLen >> 8);			// Hi
                    usb_outbuf[4]	= SregLen;				// Lo
                }
                // -----update---
                GB19056.Send_add += continue_counter;
                OUT_PUT;

                if(GB19056.Send_add >= GB19056.Total_dataBlocks)
                    GB_serial_get_clear();// clear

            }
            else
                GB_null_content(0x13, type);




            //----------------------------------------------------------------------
        }
#endif

        break;



    case   0x14:   //   �����޸ļ�¼
        //	Get total effective num
        if(Vdr_Wr_Rd_Offset.V_11H_full)
        {
            GB19056.Total_dataBlocks = VDR_14_MAXindex;
        }
        else
            GB19056.Total_dataBlocks = Vdr_Wr_Rd_Offset.V_14H_Write;

        //   USB
        if(type == GB_OUT_TYPE_USB)
        {
            GB_USBfile_stuff(ID, "�����޸ļ�¼", GB19056.Total_dataBlocks, 7);
            return;
        }
        //  serial
#if 1
        if(type == GB_OUT_TYPE_Serial)
        {
            //---------------------------------------------------------------------
            if(Vdr_Wr_Rd_Offset.V_14H_get == 0)
            {
                GB_serial_get_clear();
                Vdr_Wr_Rd_Offset.V_14H_get = 1;

                GB19056.Send_add = 0; // clear
            }
            //--------------------------------------------------------------------
            usb_outbuf_Wr = 0;
            usb_outbuf[usb_outbuf_Wr++]   = 0x55;					// ��ʼͷ
            usb_outbuf[usb_outbuf_Wr++]   = 0x7A;
            usb_outbuf[usb_outbuf_Wr++]   = 0x14;					//������


            SregLen = GB19056.Max_dataBlocks * 7; //

            usb_outbuf[usb_outbuf_Wr++]   = (SregLen >> 8);			// Hi
            usb_outbuf[usb_outbuf_Wr++]   = SregLen;				// Lo

            usb_outbuf[usb_outbuf_Wr++] = 0x00; 					// ������
            //	��Ϣ����
            WatchDog_Feed( );
            Not_null_flag = 0;
            continue_counter = 0;

            if(GB19056.Send_add < (GB19056.Total_dataBlocks))
            {

                for(subi = GB19056.Send_add; subi < GB19056.Total_dataBlocks; subi++)
                {

                    if(Vdr_Wr_Rd_Offset.V_14H_Write >= (subi + 1)) //û����
                        get_indexnum = Vdr_Wr_Rd_Offset.V_14H_Write - 1 - subi;
                    else
                        get_indexnum = VDR_14_MAXindex + Vdr_Wr_Rd_Offset.V_14H_Write - 1 - subi;

                    get_14h(get_indexnum, usb_outbuf + usb_outbuf_Wr);
                    //-----time  filter ------------
                    Rd_Time = Time_sec_u32(usb_outbuf + usb_outbuf_Wr, 6);
                    return_value = Timezone_judge(Rd_Time);

                    if(return_value) // end
                    {
                        continue;
                    }
                    else
                        usb_outbuf_Wr += 7;
                    continue_counter++;

                    if(continue_counter == GB19056.Max_dataBlocks)
                        break;
                }
                //---------------------------------------------------
                if(continue_counter != GB19056.Max_dataBlocks) // �����Ϣ��Ϊ�գ�����
                {
                    // ---caculate  infolen
                    SregLen = continue_counter * 7; // ��û���õ���Ϣȥ��
                    usb_outbuf[3]	= (SregLen >> 8);			// Hi
                    usb_outbuf[4]	= SregLen;				// Lo
                }
                // -----update---
                GB19056.Send_add += continue_counter;

                OUT_PUT;

                if(GB19056.Send_add >= GB19056.Total_dataBlocks)
                    GB_serial_get_clear();// clear

            }
            else
                GB_null_content(0x14, type);




            //----------------------------------------------------------------------
        }
#endif
        break;

    case	 0x15:														// 15 �ɼ�ָ�����ٶ�״̬��־	 --------Divde
        //	Get total effective num
        if(Vdr_Wr_Rd_Offset.V_15H_full)
        {
            GB19056.Total_dataBlocks = VDR_15_MAXindex;
        }
        else
            GB19056.Total_dataBlocks = Vdr_Wr_Rd_Offset.V_15H_Write;
        //   USB
        if(type == GB_OUT_TYPE_USB)
        {
            GB_USBfile_stuff(ID, "�ٶ�״̬��־", GB19056.Total_dataBlocks, 133);
            return;
        }
        //  serial
#if 1
        if(type == GB_OUT_TYPE_Serial)
        {
            //---------------------------------------------------------------------
            if(Vdr_Wr_Rd_Offset.V_15H_get == 0)
            {
                GB_serial_get_clear();
                Vdr_Wr_Rd_Offset.V_15H_get = 1;

                GB19056.Send_add = 0; // clear
            }
            //--------------------------------------------------------------------
            usb_outbuf_Wr = 0;
            usb_outbuf[usb_outbuf_Wr++]	 = 0x55;				   // ��ʼͷ
            usb_outbuf[usb_outbuf_Wr++]	 = 0x7A;
            usb_outbuf[usb_outbuf_Wr++]	 = 0x15;				   //������


            SregLen = 133;	 //

            usb_outbuf[usb_outbuf_Wr++]	 = (SregLen >> 8);		   // Hi
            usb_outbuf[usb_outbuf_Wr++]	 = SregLen; 			   // Lo

            usb_outbuf[usb_outbuf_Wr++] = 0x00;					   // ������
            //  ��Ϣ����
            WatchDog_Feed( );
            Not_null_flag = 0;
            continue_counter = 0;

            if(GB19056.Send_add < (GB19056.Total_dataBlocks))
            {

                for(subi = GB19056.Send_add; subi < GB19056.Total_dataBlocks; subi++)
                {

                    if(Vdr_Wr_Rd_Offset.V_15H_Write >= (subi + 1)) //û����
                        get_indexnum = Vdr_Wr_Rd_Offset.V_15H_Write - 1 - subi;
                    else
                        get_indexnum = VDR_15_MAXindex + Vdr_Wr_Rd_Offset.V_15H_Write - 1 - subi;

                    get_15h(get_indexnum, usb_outbuf + usb_outbuf_Wr);
                    //-----time  filter ------------
                    Rd_Time = Time_sec_u32(usb_outbuf + usb_outbuf_Wr + 1, 6);
                    return_value = Timezone_judge(Rd_Time);

                    if(return_value) // end
                    {
                        continue;
                    }
                    else
                        usb_outbuf_Wr += 133;
                    continue_counter++;
                    break;
                }
                //---------------------------------------------------
                if(continue_counter == 0)	// �����Ϣ��Ϊ�գ�����
                {
                    // ---caculate  infolen
                    SregLen = continue_counter * 133; // ��û���õ���Ϣȥ��
                    usb_outbuf[3]   = (SregLen >> 8);		   // Hi
                    usb_outbuf[4]   = SregLen;			   // Lo
                }
                // -----update---
                GB19056.Send_add += continue_counter;
                OUT_PUT;

                if(GB19056.Send_add >= GB19056.Total_dataBlocks)
                    GB_serial_get_clear();// clear

            }
            else
                GB_null_content(0x15, type);

            //----------------------------------------------------------------------
        }
#endif

        break;
    case 0xFE:  // output all
        for(i = 0; i < 8; i++)
            GB_OUT_data(GB_SAMPLE_00H + i, type, 1, 1);

        GB_OUT_data(GB_SAMPLE_08H, type, 1, 720);
        GB_OUT_data(GB_SAMPLE_09H, type, 1, 360);
        GB_OUT_data(GB_SAMPLE_10H, type, 1, 100);
        GB_OUT_data(GB_SAMPLE_11H, type, 1, 100);
        GB_OUT_data(GB_SAMPLE_12H, type, 1, 10);
        GB_OUT_data(GB_SAMPLE_13H, type, 1, 1);
        GB_OUT_data(GB_SAMPLE_14H, type, 1, 1);
        GB_OUT_data(GB_SAMPLE_15H, type, 1, 10);
        break;
    case 0xFB:  // recommond output
        for(i = 0; i < 4; i++)
            GB_OUT_data(GB_SAMPLE_00H + i, type, 1, 1);
        GB_OUT_data(GB_SAMPLE_10H, type, 1, 100);
    case 0xFF:  //idle
        break;
    case 0x82:
    case 0x83:
    case 0x84:
    case 0xC2:
    case 0xC3:
    case 0xC4:
    case 0xE0:
    case 0xE2:
    case 0xE3:
    case 0xE4:
        GB_null_content(ID, type);
        break;
    case 0xE1:  // ���������
        usb_outbuf_Wr = 0;
        usb_outbuf[usb_outbuf_Wr++]   = 0x55;				// ��ʼͷ
        usb_outbuf[usb_outbuf_Wr++]   = 0x7A;

        usb_outbuf[usb_outbuf_Wr++] = 0xE1; 				//������

        usb_outbuf[usb_outbuf_Wr++] = 0x00;              // ����
        usb_outbuf[usb_outbuf_Wr++] = 44;


        usb_outbuf[usb_outbuf_Wr++] = 0x00;					  // ������
        //------- ��Ϣ���� ------
        // 0.    35  ���ֽ�Ψһ���
        usb_outbuf_Wr += GB_00_07_info_only(usb_outbuf + usb_outbuf_Wr, 0x07);
        //  1.  ����ϵ��
        usb_outbuf[usb_outbuf_Wr++]   = (JT808Conf_struct.Vech_Character_Value >> 8);
        usb_outbuf[usb_outbuf_Wr++]   = (u8)JT808Conf_struct.Vech_Character_Value;
        //   2.  �ٶ�
        usb_outbuf[usb_outbuf_Wr++]   = (Speed_cacu >> 8);
        usb_outbuf[usb_outbuf_Wr++]   = (u8)Speed_cacu;
        //   3. �ۼ����
        usb_outbuf[usb_outbuf_Wr++]   = (JT808Conf_struct.Distance_m_u32 >> 24);
        usb_outbuf[usb_outbuf_Wr++]   = (JT808Conf_struct.Distance_m_u32 >> 16);
        usb_outbuf[usb_outbuf_Wr++]   = (JT808Conf_struct.Distance_m_u32 >> 8);
        usb_outbuf[usb_outbuf_Wr++]   = JT808Conf_struct.Distance_m_u32;
        //    4.  ״̬�� 1�� �ֽ�
        usb_outbuf[usb_outbuf_Wr++]   = Get_SensorStatus();
        OUT_PUT;
        break;
    default:
        break;

    }

    if(ID <= 0x07)
    {
        OUT_PUT;
    }

}

void GB_out_E1_ACK(void)
{
    GB_OUT_data(0xE1, GB_OUT_TYPE_Serial, 1, 1);
}


/*��¼�����ݽ���״̬*/
ALIGN(RT_ALIGN_SIZE)

char gbdrv_thread_stack[2048];
struct rt_thread gbdrv_thread;

void thread_GBData_mode( void *parameter )
{
    u8 str_12len[15];
    u8  i = 0;
    /*����һ������ָ�룬�����������	*/

    rt_err_t res;




    GB_Drv_init();



    while( 1 )
    {
        // part1:  serial
        if (rt_sem_take(&GB_RX_sem, 20) == RT_EOK)
        {
            // 0  debug out
            //OutPrint_HEX("��¼����",GB19056.rx_buf,GB19056.rx_wr);
            //rt_device_write( &dev_vuart,  0,GB19056.rx_buf,GB19056.rx_wr);
            if(strncmp(GB19056.rx_buf, "deviceid", 8) == 0) // deviceid("012345678901")
            {
                GB19056.workstate = 0;
                rt_thread_delay(15);
                // for(i=0;i<24;i++)
                //	 rt_kprintf("%c",GB19056.rx_buf[i]);
                //rt_kprintf("%c",0x0a);
                memset(str_12len, 0, sizeof(str_12len));
                memcpy(str_12len, GB19056.rx_buf + 10, 12);
                deviceid(str_12len);
                // tail
                rt_kprintf("0,0x00000000\r\n");
                rt_kprintf("aa mode \r\n");
                rt_thread_delay(10);
                GB19056.workstate = 1;
            }                // 1  process  rx
            else
                GB19056_Decode(GB19056.rx_buf, GB19056.rx_infoLen + 7);


            GB19056.rx_wr = 0;
            GB19056.rx_flag = 0;
            GB19056.rx_infoLen = 0;
            GB19056.u1_rx_timer = 0;
        }
        // part2:  USB
        if(GB19056.usb_write_step == GB_USB_OUT_start)
        {
            //------ U disk
            Udisk_dev = rt_device_find("udisk");
            if (Udisk_dev != RT_NULL)
            {
                res = rt_device_open(Udisk_dev, RT_DEVICE_OFLAG_RDWR);
                if(res == RT_EOK)
                {
                    GB19056.usb_xor_fcs = 0; // clear fcs
                    //  �����ļ�
                    memset(GB19056.Usbfilename, 0, sizeof(GB19056.Usbfilename));
                    //  standard  stytle
                    // sprintf((char*)GB19056.Usbfilename,"/udisk/D%c%c%c%c%c%c_%c%c%c%c_%s.VDR",(time_now.year/10+0x30),(time_now.year%10+0x30),(time_now.month/10+0x30),(time_now.month%10+0x30),\
                    // (time_now.day/10+0x30),(time_now.day%10+0x30),(time_now.hour/10+0x30),(time_now.hour%10+0x30),(time_now.min/10+0x30),(time_now.min%10+0x30),Vechicle_Info.Vech_Num);
                    //  debug  add second
                    sprintf((char *)GB19056.Usbfilename, "/udisk/D%c%c%c%c%c%c_%c%c%c%c_%s.VDR", (time_now.year / 10 + 0x30), (time_now.year % 10 + 0x30), (time_now.month / 10 + 0x30), (time_now.month % 10 + 0x30), \
                            (time_now.day / 10 + 0x30), (time_now.day % 10 + 0x30), (time_now.hour / 10 + 0x30), (time_now.hour % 10 + 0x30), (time_now.min / 10 + 0x30), (time_now.min % 10 + 0x30), Vechicle_Info.Vech_Num);

                    usb_fd = open((const char *)GB19056.Usbfilename, (O_CREAT | O_WRONLY | O_TRUNC), 0 );	 // ����U ���ļ�

                    if(GB19056.workstate == 0)
                        rt_kprintf(" \r\n udiskfile: %s  create res=%d	 \r\n", GB19056.Usbfilename, usb_fd);
                    if(usb_fd >= 0)
                    {

                        if(GB19056.workstate == 0)
                            rt_kprintf("\r\n			  ����Drv����: %s \r\n ", GB19056.Usbfilename);
                        WatchDog_Feed();

                        GB19056.usb_write_step = GB_USB_OUT_running; // goto  next step
                    }
                    else
                    {
                        if(GB19056.workstate == 0)
                            rt_kprintf(" \r\n udiskfile create Fail   \r\n", GB19056.Usbfilename, usb_fd);
                        GB19056.usb_write_step = GB_USB_OUT_idle;
                    }
                }
            }
        }
        if(GB19056.usb_write_step == GB_USB_OUT_running)
        {

            GB_OUT_data(GB19056.usb_out_selectID, GB_OUT_TYPE_USB, 1, 1);
            GB19056.usb_write_step = GB_USB_OUT_end;
        }
        if(GB19056.usb_write_step == GB_USB_OUT_end)
        {

            close(usb_fd);
            GB19056.usb_write_step = GB_USB_OUT_idle;

            if(GB19056.workstate == 0)
                rt_kprintf(" \r\n usboutput   over! \r\n");
            //------------------------------------
            Menu_txt_state = 4;
            pMenuItem = &Menu_TXT;
            pMenuItem->show();
            pMenuItem->timetick( 10 );
            pMenuItem->keypress( 10 );
            //--------------------------------------
        }

        //-------------------------------------------
        rt_thread_delay(5);   // RT_TICK_PER_SECOND / 50
    }

}

void GB_Drv_app_init(void)
{
    rt_err_t result;


    rt_sem_init(&GB_RX_sem, "gbRxSem", 0, 0);

    //------------------------------------------------------
    result = rt_thread_init(&gbdrv_thread,
                            "GB_DRV",
                            thread_GBData_mode, RT_NULL,
                            &gbdrv_thread_stack[0], sizeof(gbdrv_thread_stack),
                            Prio_GBDRV, 10);


    if (result == RT_EOK)
    {
        rt_thread_startup(&gbdrv_thread);
    }
}


void  DB9_7pin_outconfig(void)   //��ʼ�� �͹�����ص�IO �ܽ�
{
    GPIO_InitTypeDef        gpio_init;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_UP;

    //	OUT
    //------------------- PA0-----------------------------
    gpio_init.GPIO_Pin	= GPIO_Pin_1;   //------δ����   ��� ��̬��0
    gpio_init.GPIO_Mode	= GPIO_Mode_OUT;
    GPIO_Init(GPIOB, &gpio_init);


    GPIO_ResetBits(GPIOB, GPIO_Pin_1);	  //�����̬ �� 0

}



void  gb_work(u8 value)
{
    if(value)
    {
        GB19056.workstate = value;
        rt_kprintf(" GB_Data_enable  value:%d\r\n",value);
    }
    else
    {
        GB19056.workstate = 0;
        rt_kprintf(" GB_Data_disable\r\n");
    }

}
FINSH_FUNCTION_EXPORT(gb_work, gb_work(1 : 0));

#if 0
void  out_plus(u8  state)
{
    if(state)
    {
        GB19056.DB9_7pin = 1;
        //rt_kprintf(" DB9 7  Output  Enable!\r\n");
        DB9_7pin_outconfig();
    }
    else
    {
        GB19056.DB9_7pin = 0;
        //rt_kprintf(" DB9 7  Input  Enable!\r\n");
        pulse_init();
        GB19056.Deltaplus_outEnble = 0; // clear
        GB19056.Plus_tick_counter = 0; // clear
    }
}
//FINSH_FUNCTION_EXPORT(out_plus, gb_work(1 :0));

#if 0
void  plus_param_set(u32 value)
{

    JT808Conf_struct.Vech_Character_Value = value;
    Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));

    rt_kprintf(" ����ϵ�� =%d\r\n", JT808Conf_struct.Vech_Character_Value);

}
FINSH_FUNCTION_EXPORT(plus_param_set, plus_param_set(value));
#endif

void  output_spd(u16 speed)
{
    u16  PlusNum_per_10s = 0;
    //	��ͨ����֤ ���ȡ�������ٶ�
    GB19056.Deltaplus_outEnble = 2;
    PlusNum_per_10s = speed * JT808Conf_struct.Vech_Character_Value / 3600; //ÿ10s ���ٸ�����
    GB19056.DeltaPlus_out_Duration = 100000 / PlusNum_per_10s; //  10s/������� �� �������������
    out_plus(1);

}
FINSH_FUNCTION_EXPORT(output_spd, output_spd(speed   0.1km / h));


void  output_rtcplus(u8 value)
{
    if(value)
    {
        GB19056.Deltaplus_outEnble = 1;
        out_plus(1);
        //rt_kprintf(" rtc plus  Enable!\r\n");
    }
    else
    {
        // rt_kprintf(" rtc plus  Disable!\r\n");
        out_plus(0);
    }
}
//FINSH_FUNCTION_EXPORT(output_rtcplus,output_rtcplus(1:0));
//#endif

void jiaspd(u16 s_spd, u16 gps_spd)
{
    if(s_spd && gps_spd)
    {
        Speed_jiade = 1;
        Speed_cacu = s_spd;
        Speed_gps = gps_spd;
        rt_kprintf("\r\n  ��speed enable \r\n");
    }
    else
    {
        Speed_jiade = 0;
        Speed_cacu = 0;
        Speed_gps = 0;
        rt_kprintf("\r\n  �� speed recover \r\n");
    }

}
FINSH_FUNCTION_EXPORT(jiaspd, jiaspd(u16 s_spd, u16 gps_spd));

void jia_gps(u16 gps_spd)
{
    if(gps_spd)
    {
        Speed_jiade = 2;
        Speed_gps = gps_spd;
        rt_kprintf("\r\n  �� gps speed enable =%d\r\n", Speed_gps);
    }
    else
    {
        Speed_jiade = 0;
        Speed_gps = 0;
        rt_kprintf("\r\n  �� gps  speed recover \r\n");
    }

}
FINSH_FUNCTION_EXPORT(jia_gps, jia_gps(u16 gps_spd));

#endif
