
//================================================================
/*         ��дAT45DB16������ͷ�ļ�
MSP430 IAR application builder : 2007-04-15 9:00:00
Target : MSP430F149
Crystal: 3.6864Mhz
*/
//================================================================
//#include "common.h"
//#include "71x_type.h"

#ifndef _H_AT45
#define _H_AT45

#define    PageSIZE     512
#define  DFBakSize   150//50 

//================================================================
/*
  Flash Chip : SSST25VF032B-50-4I-S2AF
  ChipSize       : 4MBytes       PageSize(vitual): 512Bytes  SectorSize:4K<=>8 Pages    Chip: 1024Sectors<=>8192Pages

  Regulation :



<һ>   ϵͳ���� �Լ� Ӧ�ò���  �г���¼��ص�ַ�洢����
*/


/*               Dataflash Page  �滮   ------->    Start          */

/*  0. Page 0~9     Producet Info */
#define DF_ProInfo_Page      0

/*  1. page 10 -903	  ISP	*/
#define ISP_StartArea                                 0x1000        // ��ʼ��ַ 
#define DF_APP1_PageNo		                          8             /*
start :   0x1000---- ��Boot �����Ӧ  8 page
size        60 sector     480 page

DF_APP_flah run PageNo:   50  ~ 903  page        */
/* 512K  -->1072 Page */


//   2.     config   information
#define    ConfigStart_offset                         808        //   Block   ��ʼλ��  Conifg  Struct Save      Sector 1 
#define    TiredCondifg_offset                        864        //   Block   ��ʼλ��  Conifg  Struct Save      Sector 3  
#define    JT808_BakSetting_offset                    960        //   Block   ��ʼλ?
#define    JT808_Bak2Setting_offset                   6240        //   Block   ��ʼλ?
#define    JT808Start_offset                          1000        //   Block   ��ʼλ��  Conifg  Struct Save    Sector 2 





/*  2. Page  904 - 912           ״̬��Ϣ    */


/*  3. Page  920 - 943        	GPSӦ�� ���������    */   //���ٶ�д
#define  DF_socket_all                          1080     // Block��ʼ    socket 1 ,2, 3 
#define  DF_APN_page			                 1088	 //
#define  DF_ACC_ON_dur_Page                     1092     // ACC ��ʱ���ͼ��     
#define  DF_ACC_OFF_dur_Page					 1093 	 // ACC ��ʱ���ͼ�� 		   
#define  DF_TCP_sd_Dur_Page                     1094     // TCP ���ͼ��
#define  DF_TrigSDStatus_Page                   1095     // �����������ϱ�״̬
#define  DF_CycleAdd_Page                       1096     // Block ��ʼ-- ��¼ѭ���洢��дƫ�Ƶ�ַ��page
#define  DF_PhotoAdd_Page                       1104     // Block ��ʼ--��¼��Ƭ�洢��дƫ�Ƶ�ַ��page 


/*  4. ��ͬ�ͻ���ƷӦ�����й��ܲ���           */
#define  DF_DevConfirmCode_Page                 1112     // Block ��ʼ-- ����αIP
#define  DF_ListenNum_Page                      1113     // ���ļ�������    
#define  DF_Distance_Page                       1120     // Block ��ʼ-- �����ۼ���ʻ���
#define  DF_LoadState_Page                      1128     // Block ��ʼ-- ��������״̬ 
#define  DF_Speed_GetType_Page               1136     // Block ��ʼ--�洢�ٶȻ�ȡ��ʽ 1Ϊ �ٶȴ����� 0ΪGPS
#define  DF_K_adjust_Page                          1144     // Block ��ʼ--�洢��ʶ�Ƿ�����ϵ���Ѿ����Զ�У׼   1.У׼��  0:��δУ׼
#define  DF_ACCONFFcounter_Page              1152     // Block ��ʼ--�쳣��λʱ�洢ACCON_Off�ļ�����ֵ
/*
               Byte1 Flag :   0 :ͣ��  1:ͣ����û���� 2:�����˻�û����
               Byte2 TiredDrvStatus  Tired_drive.Tireddrv_status
               Byte3 On->off Flag
               Byte4~8: starttimeBCD
         */
#define  DF_OutGPS_Page                          1168    // Block ��ʼ -- ���ⲿGPS�ź�Դ״̬��־   
#define  DF_BD_Extend_Page                    1176   //  ������չ

//��������� �� 1023  ( ��1024Page)


/*  5.�г���¼����ز���  */
#define       DF_VehicleID_Page                        1192                           // Block ��ʼ-���ƺ���
#define       DF_VehicleType_Page                      1200                           // Block ��ʼ-��������
#define       DF_PropertiValue_Page                    1208                           // Block ��ʼ-- ����ϵ��
#define       DF_DriverID_Page                         1216                           // Block ��ʼ--��ʻԱID ������
#define       DF_ExpSpdAdd_Page                        1232                           // Block ��ʼ--���ٱ�����¼ƫ�Ƶ�ַ
#define       DF_DayDistance_Page                      1288                           // Block ��ʼ--�������  
#define       DF_Minpos_Page                           1328                           // Block ��ʼ-ÿ����λ�ô洢
#define       DF_WARN_PLAY_Page                        1332                           // Block ��ʼ--����ʹ��״̬--2014  TW705 add 
/*
                ������ʻʱ�䡢�����ۼƼ�ʻʱ�䡢��С��Ϣʱ�䡢�ͣ��ʱ��
               */
#define       DF_SDTime_Page                           1392                           // Block ��ʼ-��ʱ��ʽ���                
#define       DF_SDDistance_Page                       1400                           // Block ��ʼ-���෢�;���
#define       DF_SDMode_Page                           1408                           // Block ��ʼ- �ն����ݷ��ͷ�ʽ
#define       DF_RTLock_Page                           1416                           // Block ��ʼ- ʵʱ�ϱ� --
#define       DF_Event_Page                            1424                           // Block ��ʼ- �¼����  
#define       DF_Msg_Page                              1432                           // Block ��ʼ- ��Ϣ���  
#define       DF_PhoneBook_Page                        1440                           // Block ��ʼ- �绰�����
#define       DF_CircleRail_Page                       1448                           // Block ��ʼ- Բ��Χ��

#define       DF_RectangleRail_Page                    4056                           // Block ��ʼ- ����Χ��  1288  --���������7000 �� 24�� 

#define       DF_PolygenRail_Page                      1464                           // Block ��ʼ- �����Χ��
#define       DF_PicIndex_Page                         1480                           // Block ��ʼ- ͼ�����
#define       DF_SoundIndex_Page                       1488                           // Block ��ʼ- ��Ƶ����        
#define       DF_FlowNum_Page                          1496                           // Block ��ʼ- ��ˮ��

// 16  �ı���Ϣ
#define       TextStart_offdet                         1504

//17.  ����
#define       DF_DomainName_Page                        1512


//���������

/*
<��>   ѭ���洢�ϱ�  �г���¼����ع��� ���ݴ洢��
*/

/*  I.  Function App Area                  ע: ����Page �滮����   W25Q64FVSSIP     ������ʱTest      */

//                     Name                                     PageNum                	 	                     Description
//     2048*3  �� Page         8192+2048*3=14336          3*2048*4=24567   ����¼
// 1.  Cycle Save Send Area
#define       CycleStart_offset                       8192  //1768                          // ѭ���洢�ϱ��洢����(Basic �����ر�)        1 record=128 Bytes

// 5. Exp  Speed  Record
#define       ExpSpdStart_offset                      4016                          //  ���ٱ���ƫ�� 

// 6. Average 15 min  spd  recrod
#define      Avrg15minSpeedt_offset                     4032                         // ����ÿСʱ��ÿ����λ�ü�¼   1 record =512 Bytes

// 14. Picture   Area
/*
                                 filename            cameraNum    size
                                    19                         1             4
                      */
#define       PicStart_offset                          4096                          // Block ��ʼλ�� ͼƬ�洢����(Current Save) ����Ҫ�ŵ�TF����
#define       PicStart_offset2                        4424                          // Block ��ʼλ�� ͼƬ2���� 
#define       PicStart_offset3                        4752                          // Block ��ʼλ�� ͼƬ3���� 
#define       PicStart_offset4                        5080                          // Block ��ʼλ�� ͼƬ4����  



// 15  Sound  Area
#define       SoundStart_offdet                      5248      //4200                 32K �ռ�        // Block ��ʼλ�� 15s�����洢����(Current Save) ����Ҫ�ŵ�TF����
/*
             filesize              filename
                4  Bytes          5thstart
*/
#define       SoundFileMax_Sectors                   5                              //  5 sect=5*8 pages =20s data



#define       DF_DeviceID_offset                      5400                 // Block ��ʼλ��   ����ID  12  λ BCD   
#define       DF_License_effect                       5416
#define       DF_Vehicle_Struct_offset                5424                 // block ��ʼλ��   
#define       DF_VehicleBAK_Struct_offset             6200                 // block ��ʼλ��   
#define       DF_VehicleBAK2_Struct_offset            6216                 // block ��ʼλ��      

#define       DF_SIMID_12D                            6000                 // Block  	��ʼλ��     
#define       DF_LOGIIN_Flag_offset                          6040                 // Block      ��ʼλ�� 
#define       DF_LimitSPEED_offset                    6048                 // Block   ��ʼλ��    




//----  ����
#define    DF_Broadcast_offset                      5300       //  Block   ��ʼλ��  ������ʼ��ַ
#define    DF_Route_Page                               5400      // 1304                           // Block ��ʼ- ·��
#define    DF_turnPoint_Page                         5500       //  �յ�
#define   DF_AskQuestion_Page                    5600       //  ��������    
/*                Dataflash     <------------   End              */



//  =================  �г���¼�� ��� ============================
/*
    StartPage :    6320          Start Address offset :   0x316000

    Area Size :
                          213   Sector       = 1704  pages
                           ----------------------

				����
				1                                      00-07H
				135                                   08H
				64                                     09H
				7                                      10H
				2                                      11H
				2                                      12H
				1                                      13H
				1                                      14H
				1                                      15H

          ----------  ֻ������������---  ע�� ����������� Vdr.C

*/



//-------------------------------------------------------


extern  u8   DF_initOver;    //     Dataflash  Lock


extern void DF_delay_us(u16 j);
extern void DF_delay_ms(u16 j);
extern void DF_ReadFlash(u16 page_counter, u16 page_offset, u8 *p, u16 length);
extern void DF_WriteFlash(u16 page_counter, u16 page_offset, u8 *p, u16 length);
extern void DF_ReadFlash(u16 page_counter, u16 page_offset, u8 *p, u16 length);
extern void DF_WriteFlashSector(u16 page_counter, u16 page_offset, u8 *p, u16 length); //512bytes ֱ�Ӵ洢
extern void DF_WriteFlashDirect(u16 page_counter, u16 page_offset, u8 *p, u16 length);
extern void DF_Erase(void);
extern void DF_init(void);

#endif
