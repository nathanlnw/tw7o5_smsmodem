/*
     IC  card .h
*/


#ifndef    IC_COMMON
#define    IC_COMMON


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



#define b_CardEdge   0x0001

#define  MAX_DriverIC_num   3

typedef struct _DRIVE_STRUCT
{
    DRV_INFO     Driver_BASEinfo;  //  ��ʻԱ������Ϣ
    u8           Working_state;  //  0:  ��δ������   1:   �����õ��ǲ��ǵ�ǰ��ʻԱ  2 :  ��������Ϊ��ǰ��ʻԱ
    u8           Start_Datetime[6];  // ��ʼʱ�� BCD
    u8           End_Datetime[6];  // ��ʼʱ�� BCD
    u32          Running_counter; // ��ʻʱ��
    u32          Stopping_counter; // ֹͣ��ʻ����ʱ��
    u8           H_11_start;  // 1 ��ʼ��� 2 ���������  3 ���洢��� clear
    u8           H_11_lastSave_state; // �Ƿ�洢��
    u8           Lati[4];  //    γ��
    u8           Longi[4]; //	 ����
    u16          Hight;  //  �߶�

} DRIVE_STRUCT;

extern DRV_INFO	Read_ICinfo_Reg;   // ��ʱ��ȡ��IC ����Ϣ
extern DRIVE_STRUCT     Drivers_struct[MAX_DriverIC_num]; // Ԥ��5 ����ʻԱ�Ĳ忨�Ա�

extern unsigned char IC_CardInsert;//1:IC��������ȷ  2:IC���������
extern unsigned char IC_Check_Count;
extern unsigned char administrator_card;
extern 	u8		  powerOn_first;	 //    �״��ϵ���жϰο�


extern void CheckICInsert(void);
extern void KeyBuzzer(unsigned char num);
extern void IC_info_default(void);

extern void  Different_DriverIC_InfoUpdate(void);
extern void  Different_DriverIC_Start_Process(void);
extern void  Different_DriverIC_End_Process(void);
extern void  Different_DriverIC_Checking(void);


#endif
