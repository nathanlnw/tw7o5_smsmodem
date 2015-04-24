#ifndef Flash_Nand
#define  Flash_Nand
#include "App_moduleConfig.h"

//-----------------------------------------------------------------------------------------------------------------------------
//==============================================================================================================================
//   Add Type define
#define   Type_Idle                            0                     // ����   
#define   TYPE_CycleAdd                        1                     // ѭ��
#define   TYPE_PhotoAdd                        2                     // ͼƬ 
#define   TYPE_ExpSpdAdd                       4                     // ���ٱ�����¼ƫ�Ƶ�ַ
#define   TYPE_15minSpd                        5                      // ͣ��ǰ15����ƽ���ٶ�
#define   TYPE_DayDistancAdd                   6                    // ÿ�������ʼ��Ŀ 
//-----------------------------------------------------------------------------------------------------------------------------

//---------  ˳���ȡ���� ��� define  -----------
#define   RdCycle_Idle                 0     // ����
#define   RdCycle_RdytoSD              1     // ׼������
#define   RdCycle_SdOver               2     // ������ϵȴ�����Ӧ��


//--------   ˳���ȡ�������  ------------
extern u8       ReadCycle_status;

extern u32    cycle_write, cycle_read, delta_0704_rd, mangQu_read_reg; // ѭ���洢��¼  delta �������������ϴ�
#ifdef SPD_WARN_SAVE
extern u32    ExpSpdRec_write, ExpSpdRec_read;  // ���ٱ����洢��¼
#endif
extern u32    pic_current_page, pic_PageIn_offset, pic_size;     // ͼƬ�洢��¼
extern u32    Distance_m_u32;	 // �г���¼�����о���	  ��λ��
extern u32    DayStartDistance_32; //ÿ����ʼ�����Ŀ



extern u8  SaveCycleGPS(u32 cycle_wr, u8 *content , u16 saveLen);
extern u8  ReadCycleGPS(u32 cycleread, u8 *content , u16 ReadLen);
#ifdef SPD_WARN_SAVE
extern u8  Common_WriteContent(u32 In_write, u8 *content , u16 saveLen, u8 Type);
extern u8  Common_ReadContent(u32 In_read, u8 *content , u16 ReadLen, u8 Type);
#endif


#endif

