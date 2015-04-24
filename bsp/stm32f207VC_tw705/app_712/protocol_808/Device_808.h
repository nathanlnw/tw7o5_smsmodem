/*
     Device  808 .h
*/
#ifndef  _DEVICE808
#define   _DEVICE808

#include <rtthread.h>
#include <rthw.h>
#include "stm32f2xx.h"

#include  <stdlib.h>
#include  <stdio.h>
#include  <string.h>
#include "App_moduleConfig.h"




//--------  Protocol IO define -------------
//----- in pins  -------
#define  ACC_IO_Group          GPIOE               // ACC �ܽ�����
#define  ACC_Group_NUM         GPIO_Pin_9

#define  WARN_IO_Group         GPIOE               // �������� �ܽ�����
#define  WARN_Group_NUM        GPIO_Pin_8


//---- ����״̬  ------------------
/*
  -------------------------------------------------------------
           F4  �г���¼�� TW705   �ܽŶ���
  -------------------------------------------------------------
  ��ѭ  GB10956 (2012)  Page26  ��A.12  �涨
 -------------------------------------------------------------
 | Bit  |      Note       |  �ر�|   MCUpin  |   PCB pin  |   Colour | ADC
 ------------------------------------------------------------
     D7      ɲ��           *            PE11             9                ��
     D6      ��ת��     *             PE10            10               ��
     D5      ��ת��     *             PC2              8                ��
     D4      Զ���     *             PC0              4                ��
     D3      �����     *             PC1              5                ��
     D2      ���          add          PC3              7                ��      *
     D1      ����          add          PA1              6                ��      *
     D0      Ԥ��
*/

#define BREAK_IO_Group                GPIOE                 //  ɲ����
#define BREAK_Group_NUM             GPIO_Pin_11

#define LEFTLIGHT_IO_Group         GPIOE                // ��ת��
#define LEFTLIGHT_Group_NUM       GPIO_Pin_10

#define RIGHTLIGHT_IO_Group       GPIOC               // ��ת��
#define RIGHTLIGHT_Group_NUM      GPIO_Pin_2

#define FARLIGHT_IO_Group         GPIOC              // Զ���
#define FARLIGHT_Group_NUM        GPIO_Pin_0

#define NEARLIGHT_IO_Group        GPIOA             // �����
#define NEARLIGHT_Group_NUM       GPIO_Pin_6

#define FOGLIGHT_IO_Group          GPIOA          //  ���
#define FOGLIGHT_Group_NUM         GPIO_Pin_7

#define DOORLIGHT_IO_Group        GPIOA             // ���ŵ�   Ԥ��
#define DOORLIGHT_Group_NUM       GPIO_Pin_1


//------  out pins ---
#define RELAY_IO_Group           GPIOB               //�̵���
#define RELAY_Group_NUM          GPIO_Pin_1

#define Buzzer_IO_Group          GPIOB               //������ 
#define Buzzer_Group_Num         GPIO_Pin_6


typedef  struct  _AD_POWER
{
    u16   ADC_ConvertedValue; //��ص�ѹAD��ֵ
    u16   AD_Volte;      // �ɼ�����ʵ�ʵ�ѹ��ֵ
    u16   Classify_Door;   //  ���ִ�С�����ͣ�  >16V  ���ͳ�17V Ƿѹ            <16V С�ͳ�   10V Ƿѹ
    u16   LowWarn_Limit_Value;  //  Ƿѹ��������ֵ
    u16   LowPowerCounter;
    u16   CutPowerCounter;
    u16   PowerOK_Counter;
    u8    Battery_Flag;

} AD_POWER;



//-----  WachDog related----
extern u8    wdg_reset_flag;    //  Task Idle Hook ���

//----------AD  ��ѹ�ɼ�-----
extern AD_POWER  Power_AD;
extern  u16   ADC_ConValue[3];   //   3  ��ͨ��ID
extern  u16   AD_2through[2]; //  ����2 ·AD ����ֵ

extern u32  IC2Value;   //
extern u32  DutyCycle;



/*
     -----------------------------
    1.    ����ܽ�״̬���
     -----------------------------
*/
extern void  WatchDog_Feed(void);
extern void  WatchDogInit(void);
extern void  APP_IOpinInit(void) ;
//  INPUT
extern u8    ACC_StatusGet(void);
extern u8    WARN_StatusGet(void);
extern u8    MainPower_cut(void);
extern u8   FarLight_StatusGet(void);
extern u8  NearLight_StatusGet(void);
extern u8  FogLight_StatusGet(void);
extern u8   WarnLight_StatusGet(void);
extern u8  Speaker_StatusGet(void);
extern u8  LeftLight_StatusGet(void);
extern u8  DoorLight_StatusGet(void);
extern u8  RightLight_StatusGet(void);
extern u8  BreakLight_StatusGet(void);
extern u8 RainBrush_StatusGet(void);
extern u8  Get_SensorStatus(void);
extern u8  HardWareGet(void);

//   OUTPUT
extern void  Enable_Relay(void);
extern void  Disable_Relay(void);
extern void  IO_statusCheck(void);
extern void  ACC_status_Check(void);


/*
     -----------------------------
    2.  Ӧ�����
     -----------------------------
*/
extern   void Init_ADC(void);
extern void pulse_init( void );
extern void TIM5_IRQHandler( void );
extern void TIM3_Config( void );
extern void TIM3_IRQHandler(void);
/*
     -----------------------------
    3.  RT �������
     -----------------------------
*/

/*
        ----------------------------
       �������������
        ----------------------------
*/
extern	void GPIO_Config_PWM(void);
extern  void TIM_Config_PWM(void);


/*
       -----------------------------
       ������Ӧ��
      -----------------------------
*/
//  1 .  ѭ���洢
extern   u8       Api_cycle_write(u8 *buffer, u16 len);
extern   u8       Api_cycle_read(u8 *buffer, u16 len);
extern   u8       Api_cycle_Update(void);
extern   u8       Api_CHK_ReadCycle_status(void);

// 2. Config
extern   u8    Api_Config_write(u8 *name, u16 ID, u8 *buffer, u16 wr_len);

//  3.  ����

extern   void   Api_MediaIndex_Init(void);
extern   u32  Api_DFdirectory_Query(u8 *name, u8  returnType);
extern   u8   Api_DFdirectory_Write(u8 *name, u8 *buffer, u16 len);
extern   u8    Api_DFdirectory_Read(u8 *name, u8 *buffer, u16 len, u8  style , u16 numPacket); // style  1. old-->new   0 : new-->old
extern   u8   Api_DFdirectory_Delete(u8 *name);
extern   u8   API_List_Directories(void );
extern  void  Api_WriteInit_var_rd_wr(void);    //   д��ʼ���������Ͷ�д��¼��ַ
extern  void  Api_Read_var_rd_wr(void);    //   ����ʼ���������Ͷ�д��¼��ַ



extern   u8     Api_Config_Recwrite_Large(u8 *name, u16 ID, u8 *buffer, u16 wr_len);
extern  u8      Api_Config_read(u8 *name, u16 ID, u8 *buffer, u16 Rd_len);
extern   u8     Api_RecordNum_Write( u8 *name, u8 Rec_Num, u8 *buffer, u16 len);
extern    u8    Api_RecordNum_Read( u8 *name, u8 Rec_Num, u8 *buffer, u16 len);

extern u8	DF_Write_RecordAdd(u32 Wr_Address, u32 Rd_Address, u8 Type);
extern u8	DF_Read_RecordAdd(u32 Wr_Address, u32 Rd_Address, u8 Type);


//------------  AD    ��ѹ���  --------------------
extern void  AD_PowerInit(void);
extern void  Voltage_Checking(void);



extern u8     TF_Card_Status(void);
extern  void Socket_main_Set(u8 *str);
extern  void  debug_relay(u8 *str);
extern  void q7_relay(u8 value);


#endif
