#ifndef    _VDR
#define  _VDR

#include "App_moduleConfig.h"


//   �洢�����������
#define  VDR_08_MAXindex         2880
#define  VDR_09_MAXindex         360
#define  VDR_10_MAXindex         100
#define  VDR_11_MAXindex         100
#define  VDR_12_MAXindex         200
#define  VDR_13_MAXindex         100
#define  VDR_14_MAXindex         100
#define  VDR_15_MAXindex         10







typedef  struct
{
    u16  V_08H_Write;	  // 08 H  λ��
    u16  V_09H_Write;	  // 09H  λ��
    u16  V_10H_Write;	  // 10 H  λ��
    u16  V_11H_Write;   // 11 H  λ��
    u16  V_12H_Write;   // 12 H	λ��
    u16  V_13H_Write;   // 13H  λ��
    u16  V_14H_Write;   // 14 H	λ��
    u16  V_15H_Write;	  // 15 H λ��

    u16  V_08H_full;	  // 08 H  λ��
    u16  V_09H_full;	  // 09H  λ��
    u16  V_10H_full;	  // 10 H  λ��
    u16  V_11H_full;   // 11 H  λ��
    u16  V_12H_full;   // 12 H	λ��
    u16  V_13H_full;   // 13H  λ��
    u16  V_14H_full;   // 14 H	λ��
    u16  V_15H_full;	  // 15 H λ��

    u8   V_08H_get; //  ���ڿ�ʼ ��ȡ
    u8   V_09H_get;
    u8   V_10H_get;
    u8   V_11H_get;
    u8   V_12H_get;
    u8   V_13H_get;
    u8   V_14H_get;
    u8   V_15H_get;

} VDR_INDEX;

extern VDR_INDEX      Vdr_Wr_Rd_Offset;



typedef struct
{
    //-----------------------------------
    u8  Lati[4];  //    γ��
    u8  Longi[4]; //    ����

    //-----------------------------------
    u8  H_08[126]; //   08H buf
    u8  H_08_counter; // 08 ������ RTC ��׼
    u8  H_08_BAK[126]; //   08H buf
    u8  H_08_saveFlag; //   �洢��־λ
    u8  H_09[666];//   09H  buf
    u8  H_09_saveFlag; //
    u32 H_09_spd_accumlate;  // �ٶȺ�
    u8  H_09_seconds_counter;// �ٶ�
    u8  H_09_min_couner; // ÿ����
    u8  H_10[234]; //   10H buf
    u8  H_10_saveFlag;
    u8  H_11[50];//   11H  buf
    u8	H_11_start;	//	��ʱ��ʼ״̬ ��¼ δ��ʼ��0  ��ʼ��Ϊ1
    u8  H_11_lastSave; // �ٳ�ʱ��ʽ����ǰ���Ƿ�洢����¼   �洢��Ϊ1: δ�洢��Ϊ 0
    u8  H_11_saveFlag;
    u8	H_12[25]; //	12H buf
    u8  H_12_saveFlag;
    u8  H_13[7];//  13H  buf
    u8  H_13_saveFlag;
    u8  H_14[7]; //   14H buf
    u8  H_14_saveFlag;
    u8  H_15[133];//	 15H  buf
    u8  H_15_Set_StartDateTime[6];  // �������õĲɼ�ʱ��
    u8  H_15_Set_EndDateTime[6];  //  �������õĲɼ�����ʱ��
    u8  H_15_saveFlag;
} VDR_DATA;

extern VDR_DATA   VdrData;  //  ��ʻ��¼�ǵ�databuf



typedef struct
{
    u16  Running_state_enable;   //����״̬
} VDR_TRIG_STATUS;
extern  VDR_TRIG_STATUS  VDR_TrigStatus;

//==================================================================================================
// ���岿�� :   �������г���¼�����Э�� �� ��¼A
//==================================================================================================

extern void total_ergotic(void);
extern void  vdr_erase(void);

extern u16   stuff_drvData(u8 type, u16 Start_recNum, u16 REC_nums, u8 *dest);


extern void  VDR_product_08H_09H_10H(void);
extern void  VDR_product_11H_Start(void);
extern void  VDR_product_11H_End(u8 value);
extern void  VDR_product_12H(u8  value);
extern void  VDR_product_13H(u8  value);
extern void  VDR_product_14H(u8 cmdID);
extern void  VDR_product_15H(u8  value);
extern void  VDR_get_15H_StartEnd_Time(u8  *Start, u8 *End);




extern u16   get_00h(u8  *p_in , u16 inLen, u8 *p_out);
extern u16   get_01h(u8  *p_in , u16 inLen, u8 *p_out);
extern u16   get_02h(u8  *p_in , u16 inLen, u8 *p_out);
extern u16   get_03h(u8  *p_in , u16 inLen, u8 *p_out);
extern u16   get_04h(u8  *p_in , u16 inLen, u8 *p_out);
extern u16   get_05h(u8  *p_in , u16 inLen, u8 *p_out);
extern u16   get_06h(u8  *p_in , u16 inLen, u8 *p_out);
extern u16   get_07h(u8  *p_in , u16 inLen, u8 *p_out);


extern u16 get_08h( u16 indexnum, u8 *p);
extern u16 get_09h( u16 indexnum, u8 *p);
extern u16 get_10h( u16 indexnum, u8 *p);
extern u16 get_11h( u16 indexnum, u8 *p);
extern u16 get_12h( u16 indexnum, u8 *p);
extern u16 get_13h( u16 indexnum, u8 *p);
extern u16 get_14h( u16 indexnum, u8 *p);
extern u16 get_15h( u16 indexnum, u8 *p);





extern u16  vdr_creat_08h( u16 indexnum, u8 *p, u16 inLen);
extern u16  vdr_creat_09h( u16 indexnum, u8 *p, u16 inLen) ;
extern u16  vdr_creat_10h( u16 indexnum, u8 *p, u16 inLen) ;
extern u16  vdr_creat_11h( u16 indexnum, u8 *p, u16 inLen) ;
extern u16  vdr_creat_12h( u16 indexnum, u8 *p, u16 inLen) ;
extern u16  vdr_creat_13h( u16 indexnum, u8 *p, u16 inLen) ;
extern u16  vdr_creat_14h( u16 indexnum, u8 *p, u16 inLen) ;
extern u16  vdr_creat_15h( u16 indexnum, u8 *p, u16 inLen) ;
extern u8   vdr_cmd_writeIndex_save(u8 CMDType, u32 value);
extern u32  vdr_cmd_writeIndex_read(u8 CMDType);


extern void  moni_drv(u8 CMD, u16 delta);
extern void index_write(u8 cmd, u32 value);







#endif
