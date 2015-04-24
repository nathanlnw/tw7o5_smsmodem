/*
     GB19056.h
*/
#ifndef  GB_19056
#define  GB_19056
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
//#include "usbh_usr.h"

//     DRV  CMD
//  �ɼ�����
#define  GB_SAMPLE_00H    0x00
#define  GB_SAMPLE_01H    0x01
#define  GB_SAMPLE_02H    0x02
#define  GB_SAMPLE_03H    0x03
#define  GB_SAMPLE_04H    0x04
#define  GB_SAMPLE_05H    0x05
#define  GB_SAMPLE_06H    0x06
#define  GB_SAMPLE_07H    0x07
#define  GB_SAMPLE_08H    0x08
#define  GB_SAMPLE_09H    0x09
#define  GB_SAMPLE_10H    0x10
#define  GB_SAMPLE_11H    0x11
#define  GB_SAMPLE_12H    0x12
#define  GB_SAMPLE_13H    0x13
#define  GB_SAMPLE_14H    0x14
#define  GB_SAMPLE_15H    0x15
//   ��������
#define  GB_SET_82H      0x82
#define  GB_SET_83H      0x83
#define  GB_SET_84H      0x84
#define  GB_SET_82H      0x82
#define  GB_SET_C2H      0x82
#define  GB_SET_C3H      0x83
#define  GB_SET_C4H      0x84







//    ���갲ȫ��ʾ����
/*
    5  ����һ�飬 ÿ����ʾ3 ��
    a.��ʱ��ʻb.δ��¼ c . ��ʱ���� d. ��¼��״̬�쳣
*/
typedef struct _GB_WARN
{
    u8   Warn_state_Enable;
    u8   group_playTimes;      //  ÿһ�鲥�Ŵ���
    u32  FiveMin_sec_counter; //  5  ���Ӷ�ʱ��
} GB_WARN;






typedef  struct  _GB_STRKT
{
    u8  workstate;  // ��¼�Ǵ��ڹ���ģʽ  1:enable  2:disable
    u8  RX_CMD;   //  ����������
    u8  TX_CMD;   //  ����������
    u8  RX_FCS;   //   ����У��
    u8  TX_FCS;   //   ����У��
    u8  rx_buf[128]; //�����ַ�
    u16  rx_infoLen; //��Ϣ�鳤��
    u8  rx_wr; //
    u8  rx_flag;
    u32 u1_rx_timer;

    //---------usb -----------
    u8  usb_exacute_output;  // ʹ�����     0 : disable     1:  enable    2:  usb output   working
    u8  usb_out_selectID; //  idle:  FF    0x00~0x15     0xFE :out put all        0xFB : output  recommond info
    /*
                                 Note:  Recommond info  include  below :
                                    0x00-0x05 + 10 (latest)
                         */
    u8 Usbfilename[256];
    u8 usb_write_step;  //  д���������
    u8 usb_xor_fcs;

    u8   DB9_7pin;  //    0: instate    1:   outsate
    u16  DeltaPlus_out_Duration;    //  ��ָ������ϵ��ǰ���£���������ļ��
    u8   Deltaplus_outEnble; //  RTC output    1         signal    output     2      3  output ack   idle=0��

    u32  Plus_tick_counter;

    u32  DoubtData3_counter; //  �¹��ɵ�3  ������
    u8   DoubtData3_triggerFlag; //  �¹��ɵ�3    ���� ��־λ
    u32  Delta_lati;
    u32  Delta_longi;

    // �ٶ���־���
    u32  speed_larger40km_counter;  //  �ٶȴ���40 km/h   couter      5 ������ �ٶȲ�ֵ���ٶȲ�ֵƫ�����11%(��ֵ����GPS�ٶ�) ��Ϊ�쳣
    // �ڷ�Χ����Ϊ��������ÿ��������������Ҫ�ж��ٶ�״̬1�Σ�ͬ�洢�ٶ�״̬
    u8   speedlog_Day_save_times; //  ��ǰ�������ڴ洢�Ĵ���(���255��) �� ����״̬�� 0
    u32  gps_largeThan40_counter;
    u32  delataSpd_largerThan11_counter; //
    u8   start_record_speedlog_flag;  // ��ʼ��¼�ٶ���־״̬  ÿ��ֻ�ܼ�¼1 ��

    // ��ʼʱ�����ʱ��
    u32  Query_Start_Time;
    u32  Query_End_Time;
    u16  Max_dataBlocks;
    u16  Total_dataBlocks;
    u16  Send_add;


    //  ��ȫ��ʾ
    //   a.��ʱ��ʻb.δ��¼ c . ���ٶȱ��� d. ��¼��״̬�쳣
    GB_WARN   SPK_DriveExceed_Warn;  	// ��ʱ����
    GB_WARN	 SPK_UnloginWarn;   // δ��¼����
    GB_WARN	 SPK_Speed_Warn;    //   ���ٱ���
    GB_WARN   SPK_SpeedStatus_Abnormal;      // �ٶ��쳣����
    GB_WARN   SPK_PreTired;    //   ƣ�ͼ�ʻԤ����

    // �춨״̬
    u8   Checking_status;   // ����춨״̬��־λ

} GB_STRKT;


extern GB_STRKT GB19056;
extern u8  Warn_Play_controlBit;
/*
									ʹ�ܲ���״̬����BIT
								  BIT	0:	  ��ʻԱδ��¼״̬��ʾ(����Ĭ��ȡ��)
								  BIT	1:	  ��ʱԤ��
								  BIT	2:	  ��ʱ����
								  BIT	3:	  �ٶ��쳣����(����Ĭ��ȡ��)
								  BIT	4:	  ���ٱ���
								  BIT	5:
						   */

extern struct rt_semaphore GB_RX_sem;  //  gb  ����AA 75
extern void GB_Drv_app_init(void);
extern void GB_doubt_Data3_Trigger(u32  lati_old, u32 longi_old, u32  lati_new, u32 longi_new);
extern void  GB_SpeedSatus_Judge(void);
extern void  GB_Warn_Running(void);
extern u8  GB_fcs(u8 *instr, u16 infolen, u8 fcs_value);
extern u32 Time_sec_u32(u8 *instr, u8 len);
extern u8  BCD2HEX(u8 BCDbyte);
extern void  gb_usb_out(u8 ID);
extern void GB_out_E1_ACK(void);
extern u16  GB_00_07_info_only(u8 *dest, u8 ID);
extern u16  GB_557A_protocol_00_07_stuff(u8 *dest, u8 ID);
//extern void  output_spd(u16 speed);





#endif
