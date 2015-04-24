/*
     Protocol_808.C
*/

#include <rtthread.h>
#include <rthw.h>
#include "stm32f2xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>
#include "math.h"
#include  <stdlib.h>
#include  <stdio.h>
#include  <string.h>
#include "App_moduleConfig.h"
#include "math.h"
#include "stdarg.h"
#include "string.h"
#include "SMS.h"
#include "Vdr.h"




#define    ROUTE_DIS_Default            0x3F000000


u8 chushilicheng[4];
u8 Setting08[80] = "Ԥ�� 	    ����  	   ���  	   �����	   Զ���	   ��ת��	   ��ת��	   ɲ�� 	    ";





//----   ��ý�巢��״̬ -------
_Media_SD_state Photo_sdState;   //  ͼƬ����״̬
_Media_SD_state Sound_sdState;	//��������
_Media_SD_state Video_sdState;	//��Ƶ����


//------ Photo -----
u32 PicFileSize = 0; // ͼƬ�ļ���С
u8  PictureName[40];



//------  voice -----



//------  video  --------


/*
             ��
*/
//------ phone
u8       CallState = CallState_Idle; // ͨ��״̬

//   ASCII  to   GB    ---- start
//0-9        10
u8  arr_A3B0[20] = {0xA3, 0xB0, 0xA3, 0xB1, 0xA3, 0xB2, 0xA3, 0xB3, 0xA3, 0xB4, 0xA3, 0xB5, 0xA3, 0xB6, 0xA3, 0xB7, 0xA3, 0xB8, 0xA3, 0xB9};

//@ A-O      16
u8  arr_A3C0[32] = {0xA3, 0xC0, 0xA3, 0xC1, 0xA3, 0xC2, 0xA3, 0xC3, 0xA3, 0xC4, 0xA3, 0xC5, 0xA3, 0xC6, 0xA3, 0xC7, 0xA3, 0xC8, 0xA3, 0xC9, 0xA3, 0xCA, 0xA3, 0xCB, 0xA3, 0xCC, 0xA3, 0xCD, 0xA3, 0xCE, 0xA3, 0xCF};

//P-Z         11��
u8  arr_A3D0[22] = {0xA3, 0xD0, 0xA3, 0xD1, 0xA3, 0xD2, 0xA3, 0xD3, 0xA3, 0xD4, 0xA3, 0xD5, 0xA3, 0xD6, 0xA3, 0xD7, 0xA3, 0xD8, 0xA3, 0xD9, 0xA3, 0xDA};

//.  a-0       16
u8  arr_A3E0[32] = {0xA3, 0xE0, 0xA3, 0xE1, 0xA3, 0xE2, 0xA3, 0xE3, 0xA3, 0xE4, 0xA3, 0xE5, 0xA3, 0xE6, 0xA3, 0xE7, 0xA3, 0xE8, 0xA3, 0xE9, 0xA3, 0xEA, 0xA3, 0xEB, 0xA3, 0xEC, 0xA3, 0xED, 0xA3, 0xEE, 0xA3, 0xEF};

//p-z          11
u8  arr_A3F0[22] = {0xA3, 0xF0, 0xA3, 0xF1, 0xA3, 0xF2, 0xA3, 0xF3, 0xA3, 0xF4, 0xA3, 0xF5, 0xA3, 0xF6, 0xA3, 0xF7, 0xA3, 0xF8, 0xA3, 0xF9, 0xA3, 0xFA};
//-------  ASCII to GB ------



//----------- �г���¼�����  -----------------
u8          Vehicle_sensor = 0; // ����������״̬   0.2s  ��ѯһ��
/*
D7  ɲ��
D6  ��ת��
D5  ��ת��
D4  ����
D3  Զ���
D2  ��ˢ
D1  Ԥ��
D0  Ԥ��
*/

DOUBT_TYPE  Sensor_buf[200];// 20s ״̬��¼
u8          save_sensorCounter = 0, sensor_writeOverFlag = 0;;


u8       DispContent = 0; // ����ʱ�Ƿ���ʾ��������
/*
            1 <->  ������ʾ
            2 <->  ��ʾ������Ϣ��
            3 <->  ��ʾ ������������
            0<-> ����ʾ���������ֻ��ʾЭ������
     */

u8         TextInforCounter = 0; //�ı���Ϣ����

u8 		   FCS_GPS_UDP = 0;						//UDP ��������
u8         FCS_RX_UDP = 0;                     // UDP ���ݽ���У��
u8         FCS_error_counter = 0;              // У����������

u8          Centre_IP_modify = 0;             //  ���޸�IP��
u8          IP_change_counter = 0;           //   �����޸�IP ������
u8          Down_Elec_Flag = 0;              //   ���Ͷϵ�ʹ�ܱ�־λ



//---------74CH595  Q5   control Power----
u8   Print_power_Q5_enable = 0;
u8   Q7_enable = 0;







//------------ ���ٱ���---------------------
SPD_EXP speed_Exd;



GPRMC_PRO GPRMC_Funs =
{
    Time_pro,
    Status_pro,
    Latitude_pro,
    Lat_NS_pro,
    Longitude_pro,
    Long_WE_pro,
    Speed_pro,
    Direction_pro,
    Date_pro
};


//--------  GPS prototcol----------------------------------------------------------------------------------
static u32 	fomer_time_seconds, tmp_time_secnonds, delta_time_seconds;
u8	        UDP_dataPacket_flag = 0x03;			  /*V	   0X03 	 ;		   A	  0X02*/
u8          Year_illigel = 0; //  ��ݲ��Ϸ�
u8	        GPS_getfirst = 0; 		 //  �״��о�γ��
u8          HDOP_value = 99;       //  Hdop ��ֵ
u8          Satelite_num = 0; // ���ǿ���
u8 CurrentTime[3];
u8 BakTime[3];
u8 Sdgps_Time[3];  // GPS ���� ʱ���¼   BCD ��ʽ

//u16  Spd_add_debug=0;

//static u8      UDP_AsciiTx[1800];
ALIGN(RT_ALIGN_SIZE)
u8      GPRS_info[1400];
u16     GPRS_infoWr_Tx = 0;

ALIGN(RT_ALIGN_SIZE)
u8  UDP_HEX_Rx[1024];    // EM310 ��������hex

u16 UDP_hexRx_len = 0;  // hex ���� ����
u16 UDP_DecodeHex_Len = 0; // UDP���պ�808 ���������ݳ���


GPS_RMC GPRMC; // GPMC��ʽ

/*                         pGpsRmc->status,\
						pGpsRmc->latitude_value,\
						pGpsRmc->latitude,\
						pGpsRmc->longtitude_value,\
						pGpsRmc->longtitude,\
						pGpsRmc->speed,\
						pGpsRmc->azimuth_angle);
						*/



//----------808 Э�� -------------------------------------------------------------------------------------
u16	   GPS_Hight = 0;             //   808Э��-> �߳�   m
u16     GPS_direction = 0;         //   808Э��-> ����   ��
u16     Centre_FloatID = 0; //  ������Ϣ��ˮ��
u16     Centre_CmdID = 0; //  ��������ID

u8      Original_info[1400]; // û��ת�崦��ǰ��ԭʼ��Ϣ
u16     Original_info_Wr = 0; // ԭʼ��Ϣд��ַ
//---------- ��GPSУ׼����ϵ����� ----------------------------
u8      Speed_area = 60; // У��Kֵ��Χ
u16	    Spd_Using = 0;			 //   808Э��-> �ٶ�   0.1km/h      ��ǰʹ�õ��ٶȣ��жϳ���ƣ�͵�����
u32     Sps_larger_5_counter = 0;  //   GPS  using  ����   5km/h  ������
u16     Speed_gps = 0; // ͨ��GPS����������ٶ� 0.1km/h
u16     Speed_jiade = 0; //  �ٵ��ٶ�   1: enable 0: disable
u8      Speed_Rec = 0; // �ٶȴ����� У��K�õĴ洢��
u16     Speed_cacu = 0; // ͨ��Kֵ����������ٶ�    ͨ����������ȡ���ٶ�
u16     Speed_cacu_BAK = 0; //  ������  ����
u8      Speed_cacu_Trigger_Flag = 0;
u16     Spd_adjust_counter = 0; // ȷ������״̬������
u16     Spd_Deltacheck_counter = 0; // �������ٶȺ������ٶ����ϴ��ж�
u16     Former_DeltaPlus[K_adjust_Duration]; // ǰ�����������
u8      Former_gpsSpd[K_adjust_Duration];// ǰ������ٶ�
u8      Illeagle_Data_kickOUT = 0; //  �޳��Ƿ�����״̬

//-----  ��̨ע�ᶨʱ��  ----------
DevRegst   DEV_regist;  // ע��
DevLOGIN   DEV_Login;   //  ��Ȩ


//   -------  CAN BD new  --------------
CAN_TRAN     CAN_trans;




//------- �ı���Ϣ�·� -------
TEXT_INFO      TextInfo;    // �ı���Ϣ�·�
//------- �¼� ----
EVENT          EventObj;    // �¼�
EVENT          EventObj_8[8]; // �¼�
//-------�ı���Ϣ-------
MSG_TEXT       TEXT_Obj;
MSG_TEXT       TEXT_Obj_8[8], TEXT_Obj_8bak[8];

//------ ����  --------
CENTRE_ASK     ASK_Centre;  // ��������
//------  ��Ϣ�㲥  ---
MSG_BRODCAST   MSG_BroadCast_Obj;    // ��Ϣ�㲥
MSG_BRODCAST   MSG_Obj_8[8];  // ��Ϣ�㲥
//------  �绰��  -----
PHONE_BOOK    PhoneBook, Rx_PhoneBOOK;  //  �绰��
PHONE_BOOK    PhoneBook_8[8];

//-----  �������� ------
VEHICLE_CONTROL Vech_Control; //  ��������
//-----  ����Χ��  -----
POLYGEN_RAIL Rail_Polygen;   // �����Χ��
RECT_RAIL    Rail_Rectangle; // ����Χ��
RECT_RAIL    Rail_Rectangle_multi[8]; // ����Χ��
CIRCLE_RAIL  Rail_Cycle_multi[8];     // Բ��Χ��
CIRCLE_RAIL  Rail_Cycle;     // Բ��Χ��
//------- ��·���� -----
POINT        POINT_Obj;      // ·�ߵĹյ�
ROUTE        ROUTE_Obj;      // ·�����
//-------    �г���¼��  -----
RECODER      Recode_Obj;     // �г���¼��
//-------  ����  ----
CAMERA        Camera_Obj;     //  �����������
//-----   ¼��  ----
VOICE_RECODE  VoiceRec_Obj;   //  ¼������
//------ ��ý��  --------
MULTIMEDIA    MediaObj;       // ��ý����Ϣ
//-------  ������Ϣ͸��  -------
DATATRANS     DataTrans;      // ������Ϣ͸��
//-------  ����Χ��״̬ --------
INOUT        InOut_Object;    // ����Χ��״̬
//-------- ��ý�����  ------------
MEDIA_INDEX  MediaIndex;  // ��ý����Ϣ
//------- ��������״̬ ---------------
u8  CarLoadState_Flag = 1; //ѡ�г���״̬�ı�־   1:�ճ�   2:���   3:�س�

//------- ��ý����Ϣ����---------------
u8  Multimedia_Flag = 1; //��Ҫ�ϴ��Ķ�ý����Ϣ����   1:��Ƶ   2:��Ƶ   3:ͼ��
u8  SpxBuf[SpxBuf_Size];
u16 Spx_Wr = 0, Spx_Rd = 0;
u8  Duomeiti_sdFlag = 0;

//------- ¼����ʼ���߽���---------------
u8  Recor_Flag = 1; //  1:¼����ʼ   2:¼������


#ifdef AVRG15MIN
//   ͣ��ǰ15  ���ӣ�ÿ���ӵ�ƽ���ٶ�
AVRG15_SPD  Avrg_15minSpd;
#endif


//----------808Э�� -------------------------------------------------------------------------------------
u8		SIM_code[6];							   // Ҫ���͵�IMSI	����
u8		IMSI_CODE[15] = "000000000000000";							//SIM ����IMSI ����
u8		Warn_Status[4]		=
{
    0x00, 0x00, 0x00, 0x00
}; //  ������־λ״̬��Ϣ
u8  Warn_MaskWord[4]		=
{
    0x00, 0x00, 0x00, 0x00
};   //  ����������
u8  Text_MaskWord[4] =
{
    0x00, 0x00, 0x00, 0x00
};	 //  �ı�������
u8  Key_MaskWord[4] =
{
    0x00, 0x00, 0x00, 0x00
};	 //   �ؼ���������



u8		Car_Status[4]		=
{
    0x00, 0x0c, 0x00, 0x00
}; //  ����״̬��Ϣ
T_GPS_Info_GPRS 	Gps_Gprs, Bak_GPS_gprs;
T_GPS_Info_GPRS	Temp_Gps_Gprs;
u8   A_time[6]; // ��λʱ�̵�ʱ��

u8      ReadPhotoPageTotal = 0;
u8      SendPHPacketFlag = 0; ////�յ���������������һ��blockʱ��λ


//-------- �������� --------
u8		warn_flag = 0;
u8		f_Exigent_warning = 0; //0;     //�Ŷ� ��������װ�� (INT0 PD0)
u8		Send_warn_times = 0;    //   �豸�������ϱ��������������3 ��
u32  	fTimer3s_warncount = 0;

// ------  ������Ϣ������ ---------------
VechINFO     Vechicle_Info;     //  ������Ϣ
VechINFO     Vechicle_Info_BAK;  //  ������Ϣ BAK
VechINFO     Vechicle_info_BAK2; //  ������ϢBAK2
u8           Login_Menu_Flag = 0;     //   ��½���� ��־λ
u8           Limit_max_SateFlag = 0;  //   �ٶ������������ָ��


//------  ���ſ������� -------
DOORCamera   DoorOpen;    //  ���س�������

//------- ������չЭ��  ------------
BD_EXTEND     BD_EXT;     //  ������չЭ��
DETACH_PKG   Detach_PKG; // �ְ��ش����
SET_QRY         Setting_Qry; //  �ն˲�����ѯ
u32     CMD_U32ID = 0;
PRODUCT_ATTRIBUTE   ProductAttribute;// �ն�����
HUMAN_CONFIRM_WARN   HumanConfirmWarn;// �˹�ȷ�ϱ���

//-----  ISP    Զ��������� -------
ISP_BD  BD_ISP; //  BD   ������



// ---- �յ� -----
u16  Inflexion_Current = 0;
u16  Inflexion_Bak = 0;
u16  Inflexion_chgcnter = 0; //�仯������
u16  InflexLarge_or_Small = 0;   // �ж�curent �� Bak ��С    0 equql  1 large  2 small
u16  InflexDelta_Accumulate = 0; //  ��ֵ�ۼ�

// ----����״̬  ------------
u8  SleepState = 0; //   0  ������ACC on            1  ����Acc Off
u8  SleepConfigFlag = 0; //  ����ʱ���ͼ�Ȩ��־λ

//---- �̶��ļ���С ---
u32 mp3_fsize = 5616;
u8  mp3_sendstate = 0;
u32 wmv_fsize = 25964;
u8  wmv_sendstate = 0;

//-------------------   ���� ---------------------------------------
static u8 GPSsaveBuf[128];     // �洢GPS buffer
static u8	ISP_buffer[1024];
static u16 GPSsaveBuf_Wr = 0;


POSIT Posit[60];           // ÿ����λ����Ϣ�洢
u8    PosSaveFlag = 0;    // �洢Pos ״̬λ

NANDSVFlag   NandsaveFlg;
A_AckFlag    Adata_ACKflag;  // ����GPRSЭ�� ������� RS232 Э�鷵��״̬�Ĵ���
TCP_ACKFlag  SD_ACKflag;     // ����GPRSЭ�鷵��״̬��־
u32  SubCMD_8103H = 0;          //  02 H���� ���ü�¼�ǰ�װ�����ظ� ������
u32  SubCMD_FF01H = 0;          //  FF02 ������Ϣ��չ
u32  SubCMD_FF03H = 0;   //  FF03  ������չ�ն˲�������1
u8   Fail_Flag = 0;


u8  SubCMD_10H = 0;          //  10H   ���ü�¼�Ƕ�λ�澯����
u8  OutGPS_Flag = 0;   //  0  Ĭ��  1  ���ⲿ��Դ����
u8   Spd_senor_Null = 0; // �ֶ��������ٶ�Ϊ0
u32  Centre_DoubtRead = 0;   //  ���Ķ�ȡ�¹��ɵ����ݵĶ��ֶ�
u32  Centre_DoubtTotal = 0;  //  ���Ķ�ȡ�¹��ɵ�����ֶ�
u8   Vehicle_RunStatus = 0;  //  bit 0: ACC �� ��             1 ��  0��
//  bit 1: ͨ���ٶȴ�������֪    1 ��ʾ��ʻ  0 ��ʾֹͣ
//  bit 2: ͨ��gps�ٶȸ�֪       1 ��ʾ��ʻ  0 ��ʾֹͣ



u32   SrcFileSize = 0, DestFilesize = 0, SrcFile_read = 0;
u8    SleepCounter = 0;

u16   DebugSpd = 0; //������GPS�ٶ�
u8    MMedia2_Flag = 0; // �ϴ�������Ƶ ��ʵʱ��Ƶ  �ı�־λ    0 ������ 1 ��ʵʱ


u8	 reg_128[128];  // 0704 �Ĵ���

unsigned short int FileTCB_CRC16 = 0;
unsigned short int Last_crc = 0, crc_fcs = 0;



//---------  ����Ӧ��  -----------
u8		 ACK_timer = 0;				 //---------	ACK timer ��ʱ��---------------------
u8           Send_Rdy4ok = 0;
unsigned char	Rstart_time = 0;


//---------------  �ٶ��������--------------
u32  Delta_1s_Plus = 0;
u16  Sec_counter = 0;

void  K_AdjustUseGPS(u32  sp_DISP);  // ͨ��GPS У׼  K ֵ  (������ʻ1KM ��������Ŀ)
void  Delta_Speed_judge(void);

u16  Protocol_808_Encode(u8 *Dest, u8 *Src, u16 srclen);
void Protocol_808_Decode(void);  // ����ָ��buffer :  UDP_HEX_Rx
void Photo_send_end(void);
#ifdef REC_VOICE_ENABLE
void Sound_send_end(void);
#endif
//void Video_send_end(void);
unsigned short int  File_CRC_Get(void);
void Spd_ExpInit(void);
u32   Distance_Point2Line(u32 Cur_Lat, u32  Cur_Longi, u32 P1_Lat, u32 P1_Longi, u32 P2_Lat, u32 P2_Longi);
void  RouteRail_Judge(u8 *LatiStr, u8 *LongiStr);

//  A.  Total

void delay_us(u16 j)
{
    u8 i;
    while(j--)
    {
        i = 3;
        while(i--);
    }
}

void delay_ms(u16 j )
{
    while(j--)
    {
        DF_delay_us(2000); // 1000
    }
}

u8  Do_SendGPSReport_GPRS(void)
{
    //unsigned short int crc_file=0;
    u8  packet_type = 0;
    u8  Batch_Value = 0; // �����ϴ��ж�

    // 	  u8 i=0;
    if(DEV_Login.Operate_enable == 1) // !=2
    {
        if((1 == DEV_Login.Enable_sd) && (0 == DEV_regist.Enable_sd))
        {
            Stuff_DevLogin_0102H();   //  ��Ȩ   ==2 ʱ��Ȩ���
            DEV_Login.Enable_sd = 0;
            //------ ���ͼ�Ȩ���ж� ------------------
            //DEV_Login.Operate_enable=2;  //  �����жϼ�Ȩ��
            return true;
        }
    }
    //------------------------ MultiMedia Send--------------------------
    if((MediaObj.Media_transmittingFlag == 2) && (DEV_Login.Operate_enable == 2))
    {
        if(1 == MediaObj.SD_Data_Flag)
        {
            Stuff_MultiMedia_Data_0801H();
            MediaObj.SD_Data_Flag = 0;
            return true;
        }
        return true; // ����808 Э��Ҫ�� �������ý������в������ͱ����Ϣ��
    }
    if(1 == DEV_regist.Enable_sd)
    {
        Stuff_RegisterPacket_0100H(0);   // ע��
        JT808Conf_struct.Msg_Float_ID = 0;
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        DEV_Login.Sd_counter = 0;
        DEV_Login.Operate_enable = 0;
        DEV_regist.Enable_sd = 0;
        //  JT808Conf_struct.Regsiter_Status=1; //��עע�ᣬ�����洢
        return true;
    }
    if(1 == DEV_regist.DeRegst_sd)
    {
        Stuff_DeviceDeregister_0101H();
        DEV_regist.DeRegst_sd = 0;
        return true;
    }
    if((1 == JT808Conf_struct.DURATION.Heart_SDFlag) && (DataLink_Status()) && (SleepState == 0) && (DEV_Login.Operate_enable == 2)) //  ����
    {
        Stuff_DeviceHeartPacket_0002H();
        JT808Conf_struct.DURATION.Heart_SDFlag = 0;
        JT808Conf_struct.DURATION.TCP_SD_state = 1; //��������� 1
        return true;
    }
    if((1 == SleepConfigFlag) && (DataLink_Status()) && (SleepState == 1)) //  ����ʱ����
    {
        // Stuff_DevLogin_0102H();   //  ��Ȩ   ==2 ʱ��Ȩ���
        // rt_kprintf("\r\n	 ����ʱ�ü�Ȩ�������� ! \r\n");
        SleepConfigFlag = 0;
        return true;
    }

    if(1 == SD_ACKflag.f_CurrentPosition_0201H)  // λ����Ϣ��ѯ
    {
        Stuff_Current_Data_0201H();
        SD_ACKflag.f_CurrentPosition_0201H = 0;
        return true;
    }
    if(1 == SD_ACKflag.f_CurrentEventACK_0301H) //  �¼�����
    {
        Stuff_EventACK_0301H();
        SD_ACKflag.f_CurrentEventACK_0301H = 0;
        return true;
    }

    if(2 == ASK_Centre.ASK_SdFlag)    //  ����Ӧ��
    {
        Stuff_ASKACK_0302H();
        ASK_Centre.ASK_SdFlag = 0;
        return true;
    }
    // 9.  ������������Ӧ��
    if( 1 == SD_ACKflag.f_BD_CentreTakeAck_0805H )                                                                           // ������������Ӧ��
    {
        Stuff_CentreTakeACK_BD_0805H( );
        SD_ACKflag.f_BD_CentreTakeAck_0805H = 0;
        return true;
    }
    if(1 == Vech_Control.ACK_SD_Flag) //  ����Ӧ�����
    {
        Stuff_ControlACK_0500H();
        Vech_Control.ACK_SD_Flag = 0;
        return true;
    }

    if(SD_ACKflag.f_MsgBroadCast_0303H == 1)
    {
        Stuff_MSGACK_0303H();
        SD_ACKflag.f_MsgBroadCast_0303H = 0;
        return true;
    }
    if((1 == MediaObj.SD_media_Flag) && (DEV_Login.Operate_enable == 2))
    {
        Stuff_MultiMedia_InfoSD_0800H();
        MediaObj.SD_media_Flag = 2; // original clear  0 ,,HBTDT =2  ,timeout ACK
        return true;
    }
    if(DataTrans.Data_TxLen)
    {
        Stuff_DataTransTx_0900H();  // ����͸�� ��Զ������
        DataTrans_Init();     //clear
        return true;
    }
    if(SD_ACKflag.f_MediaIndexACK_0802H)
    {
        Stuff_MultiMedia_IndexAck_0802H();   // ��ý�������ϱ�
        SD_ACKflag.f_MediaIndexACK_0802H = 0;
        return true;
    }
    if(SD_ACKflag.f_DriverInfoSD_0702H)
    {
        Stuff_DriverInfoSD_0702H();  //  ��ʻԱ��Ϣ�ϱ�
        SD_ACKflag.f_DriverInfoSD_0702H = 0;
        return true;
    }
    //  18.����͸�� ��Զ������
    if(CAN_trans.canid_0705_sdFlag)
    {
        Stuff_CANDataTrans_BD_0705H();
        CAN_trans.canid_0705_sdFlag = 0; // clear
        //   DataTrans_Init();     //clear
        return true;
    }
    if(SD_ACKflag.f_Worklist_SD_0701H)
    {
        Stuff_Worklist_0701H();  //   �����˵�
        SD_ACKflag.f_Worklist_SD_0701H = 0;
        return true;
    }
    //  23.   ��ѯ�ն˲���
    if(SD_ACKflag.f_SettingPram_0104H)
    {
        if(SD_ACKflag.f_SettingPram_0104H == 1)
            Stuff_SettingPram_0104H();
        else if(SD_ACKflag.f_SettingPram_0104H == 2)
            Sub_stuff_AppointedPram_0106();

        SD_ACKflag.f_SettingPram_0104H = 0;
        return true;
    }
    //  24 .  �ն���������ϱ�
    if(SD_ACKflag.f_BD_ISPResualt_0108H)
    {

        Stuff_ISP_Resualt_BD_0108H();

        if(1 == SD_ACKflag.f_BD_ISPResualt_0108H)
        {
            if( File_CRC_Get() == true)
                ISP_file_Check();  // update
        }
        else
            BD_ISP.ISP_running = 0; // recover normal
        SD_ACKflag.f_BD_ISPResualt_0108H = 0;
        return true;
    }
    if(SD_ACKflag.f_CentreCMDack_0001H)
    {
        Stuff_DevCommmonACK_0001H();
        if(SD_ACKflag.f_CentreCMDack_0001H == 2) //  �޸�IP��������Ҫ�ز�
            Close_DataLink();    //  AT_END
        else if(SD_ACKflag.f_CentreCMDack_0001H == 3) //   Զ�̸�λ
        {
            Systerm_Reset_counter = Max_SystemCounter;
            ISP_resetFlag = 2;  //   ����Զ�������������Ƹ�λϵͳ
        }
        else if(SD_ACKflag.f_CentreCMDack_0001H == 5) //   �ر�����ͨ��
        {
            Close_DataLink();
            Stop_Communicate();
        }
        SD_ACKflag.f_CentreCMDack_0001H = 0;
        SD_ACKflag.f_CentreCMDack_resualt = 0;


        return true;
    }

    //  15.  �г���¼�������ϴ�
    //------------ Error	state --------------
    if(Recode_Obj.Error)
    {

        Stuff_RecoderACK_0700H_Error();
        Recode_Obj.Error = 0;
        Recode_Obj.SD_Data_Flag = 0;
        Recode_Obj.CountStep = 0;
        return true;
    }

    //-----------------------------------------
    if((1 == Recode_Obj.SD_Data_Flag) && (1 == Recode_Obj.CountStep))
    {
        //  1. clear	one  packet  flag
        switch( Recode_Obj.CMD )
        {
            /* 			   divide  not	stop	2013-6-20  */
            //----------------------------------------------------
            /*case 0x08:
            case 0x09:
            case 0x10:
            case 0x11:
            case 0x12:
            case 0x15:*/
            //---------------------------------------------------
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
        case 0x13:
        case 0x14:    //	�ϱ��ǲ�ѯ��

        case 0x82: //    �������ó��ƺ�
        case 0x83://	��¼�ǳ��ΰ�װʱ��
        case 0x84:// �����ź���������Ϣ
        case 0xC2: //���ü�¼��ʱ��
        case 0xC3: //�����ٶ�����ϵ��������ϵ����
        case 0xC4: //   ���ó�ʼ���
            //------------------------
            Recode_Obj.SD_Data_Flag = 0;
            Recode_Obj.CountStep = 0;
            break;
        default:
            break;
        }
        //  2.  stuff	 recorder	infomation
        //  judge	packet	type
        if( Recode_Obj.Devide_Flag == 1 )
        {
            packet_type = Packet_Divide;
        }
        else
        {
            packet_type = Packet_Normal;
        }
        if(GB19056.workstate == 0)
            rt_kprintf( "\r\n ��¼�� CMD_ID =0x%2X \r\n", Recode_Obj.CMD );
        if( packet_type == Packet_Divide )
        {
            if(GB19056.workstate == 0)
                rt_kprintf( "\r\n current =%d	Total: %d \r\n", Recode_Obj.Current_pkt_num, Recode_Obj.Total_pkt_num );
        }

        //------------------------------------------------------------------
        Stuff_RecoderACK_0700H( packet_type); //	�г���¼�������ϴ�

        //  3. step  by  step	send   from  00H  ---  07H


        if( Recode_Obj.CountStep == 1 )
        {
            Recode_Obj.CountStep = 2;
            Recode_Obj.timer = 0;
        }

        return true;
    }
    //----------------------------------------------------------------------
    if(Current_UDP_sd == 1) // ����ʱ30  ʵ����10 Current_SD_Duration
    {
        if(PositionSD_Status() && (DEV_Login.Operate_enable == 2) && ((enable == BD_EXT.Trans_GNSS_Flag) || (DispContent == 6)) || (Current_UDP_sd && PositionSD_Status() && (DEV_Login.Operate_enable == 2)) || (PositionSD_Status()))	 //�״ζ�λ�ٷ�
        {
            PositionSD_Disable();
            Current_UDP_sd = 0;
            //1.   ʱ�䳬ǰ�ж�
            //if(Time_FastJudge()==false)
            //return false;
            // 2.
            Stuff_Current_Data_0200H();  // �ϱ���ʱ����
            //---------------  Ӧ����� -----------
            // ACKFromCenterCounter++; // ֻ��עӦ������������Ӧ��ʱ��
            //---------------------------------------------------------------------------------
            if(DispContent)
                rt_kprintf("\r\n ���� GPS -current !\r\n");
            return true;
        }
    }
    else if((RdCycle_RdytoSD == ReadCycle_status) && (0 == BD_ISP.ISP_running) && (DataLink_Status()) && (DEV_Login.Operate_enable == 2) && (CameraState.camera_running == 0)) // ��ȡ����--------- ����GPS
    {
        /* Զ������ʱ�������ϱ�GPS ����Ϊ�п��ܷ��Ͷ�λ����ʱ
             ���ڽ��մ����������������ݰ�,����GSMģ�鴦�������������ǵ�Ƭ����������,���չ����в��ܽ���*/

        //  1. �ж��Ƿ������������ϴ�
        Batch_Value = Stuff_BatchDataTrans_BD_0704H();
        switch(Batch_Value)
        {
        case  0:  // �����ϱ�
            if (false == Stuff_Normal_Data_0200H())
                return false;

        case  1: //  ������������
            //-------- change status  Ready  ACK  ------
            ReadCycle_status = RdCycle_SdOver;
            Send_Rdy4ok = 2; // enable
            //----Ӧ����� ----
            break;
        default:  //nothing except   waiting
            break;

        }
        return true;
    }
    //-------------------------------------------------------------
    return  false;

}
void strtrim(u8 *s, u8 c)
{
    u8		 *p1, *p2;
    u16  i, j;

    if (s == 0) return;

    // delete the trailing characters
    if (*s == 0) return;
    j = strlen((char const *)s);
    p1 = s + j;
    for (i = 0; i < j; i++)
    {
        p1--;
        if (*p1 != c) break;
    }
    if (i < j) p1++;
    *p1 = 0;	// null terminate the undesired trailing characters

    // delete the leading characters
    p1 = s;
    if (*p1 == 0) return;
    for (i = 0; *p1++ == c; i++);
    if (i > 0)
    {
        p2 = s;
        p1--;
        for (; *p1 != 0;) *p2++ = *p1++;
        *p2 = 0;
    }
}

int str2ip(char *buf, u8 *ip)
{
    // convert an ip:port string into a binary values
    int	i;
    u16	_ip[4];


    memset(_ip, 0, sizeof(_ip));

    strtrim((u8 *)buf, ' ');

    i = sscanf(buf, "%u.%u.%u.%u", (u32 *)&_ip[0], (u32 *)&_ip[1], (u32 *)&_ip[2], (u32 *)&_ip[3]);

    *(u8 *)(ip + 0) = (u8)_ip[0];
    *(u8 *)(ip + 1) = (u8)_ip[1];
    *(u8 *)(ip + 2) = (u8)_ip[2];
    *(u8 *)(ip + 3) = (u8)_ip[3];

    return i;
}



int IP_Str(char *buf, u32 IP)
{
    T_IP_Addr	ip;

    if (!buf) return 0;

    ip.ip32 = IP;

    return sprintf(buf, "%u.%u.%u.%u", ip.ip8[0], ip.ip8[1], ip.ip8[2], ip.ip8[3]);
}

u16 AsciiToGb(u8 *dec, u8 InstrLen, u8 *scr)
{
    u16 i = 0, j = 0, m = 0;
    u16 Info_len = 0;


    for(i = 0, j = 0; i < InstrLen; i++, j++)
    {
        m = scr[i];
        if((m >= 0x30) && (m <= 0x39))
        {
            memcpy(&dec[j], &arr_A3B0[(m - '0') * 2], 2);
            j++;
        }
        else if((m >= 0x41) && (m <= 0x4f))
        {
            memcpy(&dec[j], &arr_A3C0[(m - 0x41 + 1) * 2], 2);
            j++;
        }
        else if((m >= 0x50) && (m <= 0x5a))
        {
            memcpy(&dec[j], &arr_A3D0[(m - 0x50) * 2], 2) ;
            j++;
        }
        else if((m >= 0x61) && (m <= 0x6f))
        {
            memcpy(&dec[j], &arr_A3E0[(m - 0x61 + 1) * 2], 2) ;
            j++;
        }
        else if((m >= 0x70) && (m <= 0x7a))
        {
            memcpy(&dec[j], &arr_A3F0[(m - 0x70) * 2], 2)  ;
            j++;
        }
        else
        {
            dec[j] = m;
        }
    }
    Info_len = j;
    return Info_len;
}


// B.   Protocol


//==================================================================================================
// ��һ���� :   ������GPS ����ת����غ���
//==================================================================================================

void Time_pro(u8 *tmpinfo, u8 hour, u8 min , u8 sec)
{
    //---- record  to memory
    GPRMC.utc_hour = hour;
    GPRMC.utc_min = min;
    GPRMC.utc_sec = sec;

    CurrentTime[0] = hour;
    CurrentTime[1] = min;
    CurrentTime[2] = sec;

    //-----------  ���ͨЭ�� -------------
    Temp_Gps_Gprs.Time[0] = hour;
    Temp_Gps_Gprs.Time[1] = ( tmpinfo[2] - 0x30 ) * 10 + tmpinfo[3] - 0x30;
    Temp_Gps_Gprs.Time[2] = ( tmpinfo[4] - 0x30 ) * 10 + tmpinfo[5] - 0x30;

}

void Status_pro(u8 *tmpinfo)
{
    GPRMC.status = tmpinfo[0];
    Posit_ASCII.AV_ASCII = tmpinfo[0];
    //-------------------------���ͨЭ��-----------------------------
    if ( tmpinfo[0] == 'V' || tmpinfo[0] == 'v' )
    {
        UDP_dataPacket_flag = 0X03;
        StatusReg_GPS_V();
    }
    else if ( tmpinfo[0] == 'A' || tmpinfo[0] == 'a' )
    {
        UDP_dataPacket_flag = 0X02;
        StatusReg_GPS_A();
    }
}

void Latitude_pro(u8 *tmpinfo)
{
    u32  latitude;
    u32  vdr_lati = 0;

    GPRMC.latitude_value = atof((char *)tmpinfo);
    /*     Latitude
           ddmm.mmmm
     */
    memset(Posit_ASCII.Lat_ASCII, 0, sizeof(Posit_ASCII.Lat_ASCII));
    memcpy(Posit_ASCII.Lat_ASCII, tmpinfo, strlen((const char *)tmpinfo));
    //--------	808 Э�� --------------------
    if(UDP_dataPacket_flag == 0X02)  //��ȷ�������֮һ��
    {

        //------------  dd part   --------
        latitude = ( u32 ) ( ( tmpinfo[0] - 0x30 ) * 10 + ( u32 ) ( tmpinfo[1] - 0x30 ) ) * 1000000;
        //------------  mm  part  -----------
        /*    ת���ɰ����֮һ��
              mm.mmmm   *  1000000/60=mm.mmmm*50000/3=mm.mmmm*10000*5/3
        */
        latitude = latitude + ( u32 )( (( tmpinfo[2] - 0x30 ) * 100000 + (tmpinfo[3] - 0x30 ) * 10000 + (tmpinfo[5] - 0x30 ) * 1000 + ( tmpinfo[6] - 0x30 ) * 100 + ( tmpinfo[7] - 0x30 ) * 10 + ( tmpinfo[8] - 0x30 )) * 5 / 3);

        if(latitude == 0)
        {
            GPS_getfirst = 0;
            StatusReg_GPS_V();
            return;
        }
        Temp_Gps_Gprs.Latitude[0] = ( u8 ) ( latitude >> 24 );
        Temp_Gps_Gprs.Latitude[1] = ( u8 ) ( latitude >> 16 );
        Temp_Gps_Gprs.Latitude[2] = ( u8 ) ( latitude >> 8 );
        Temp_Gps_Gprs.Latitude[3] = ( u8 ) latitude;

        //  �������г���¼�ǵ�ת��
        //                      ��                                           ��large

        vdr_lati = ( tmpinfo[0] - 0x30 ) * 10 + ( u32 ) ( tmpinfo[1] - 0x30 );
        vdr_lati = vdr_lati * 600000 + ( tmpinfo[2] - 0x30 ) * 100000 + (tmpinfo[3] - 0x30 ) * 10000 + (tmpinfo[5] - 0x30 ) * 1000 + ( tmpinfo[6] - 0x30 ) * 100 + ( tmpinfo[7] - 0x30 ) * 10 + ( tmpinfo[8] - 0x30 );

        VdrData.Lati[0] = ( u8 ) ( vdr_lati >> 24 );
        VdrData.Lati[1] = ( u8 ) ( vdr_lati >> 16 );
        VdrData.Lati[2] = ( u8 ) ( vdr_lati >> 8 );
        VdrData.Lati[3] = ( u8 ) vdr_lati;

        //-------------------------------------------------------------------------------------------
    }
    else
    {
        VdrData.Lati[0] = ( u8 ) ( 0x7F );
        VdrData.Lati[1] = ( u8 ) ( 0xFF );
        VdrData.Lati[2] = ( u8 ) (0xFF );
        VdrData.Lati[3] = ( u8 ) 0xFF;

    }
    //----------------------------------------------
}

void Lat_NS_pro(u8 *tmpinfo)
{
    GPRMC.latitude = tmpinfo[0];
}

void Longitude_pro(u8 *tmpinfo)
{
    u32  longtitude;
    u32  Longi = 0;

    GPRMC.longtitude_value = atof((char *)tmpinfo);
    /*     Latitude
            dddmm.mmmm
      */

    memset(Posit_ASCII.Longi_ASCII, 0, sizeof(Posit_ASCII.Longi_ASCII));
    memcpy(Posit_ASCII.Longi_ASCII, tmpinfo, strlen((const char *)tmpinfo));
    //--------  808Э��  ---------
    if(UDP_dataPacket_flag == 0X02) //��ȷ�������֮һ��
    {
        //------  ddd part -------------------
        longtitude = ( u32 )( ( tmpinfo[0] - 0x30 ) * 100 + ( tmpinfo[1] - 0x30 ) * 10 + ( tmpinfo[2] - 0x30 ) ) * 1000000;
        //------  mm.mmmm --------------------
        /*    ת���ɰ����֮һ��
           mm.mmmm	 *	1000000/60=mm.mmmm*50000/3=mm.mmmm*10000*5/3
        */
        longtitude = longtitude + ( u32 ) (( ( tmpinfo[3] - 0x30 ) * 100000 + ( tmpinfo[4] - 0x30 ) * 10000 + (tmpinfo[6] - 0x30 ) * 1000 + ( tmpinfo[7] - 0x30 ) * 100 + ( tmpinfo[8] - 0x30 ) * 10 + ( tmpinfo[9] - 0x30 )) * 5 / 3);
        if(longtitude == 0)
        {
            GPS_getfirst = 0;
            StatusReg_GPS_V();
            return;
        }

        Temp_Gps_Gprs.Longitude[0] = ( u8 ) ( longtitude >> 24 );
        Temp_Gps_Gprs.Longitude[1] = ( u8 ) ( longtitude >> 16 );
        Temp_Gps_Gprs.Longitude[2] = ( u8 ) ( longtitude >> 8 );
        Temp_Gps_Gprs.Longitude[3] = ( u8 ) longtitude;



        //  �������г���¼�ǵ�ת��
        // 					 �� 										  ��large
        Longi = ( tmpinfo[0] - 0x30 ) * 100 + ( tmpinfo[1] - 0x30 ) * 10 + ( tmpinfo[2] - 0x30 );
        Longi = Longi * 600000 + ( tmpinfo[3] - 0x30 ) * 100000 + ( tmpinfo[4] - 0x30 ) * 10000 + (tmpinfo[6] - 0x30 ) * 1000 + ( tmpinfo[7] - 0x30 ) * 100 + ( tmpinfo[8] - 0x30 ) * 10 + ( tmpinfo[9] - 0x30 );

        VdrData.Longi[0] = ( u8 ) ( Longi >> 24 );
        VdrData.Longi[1] = ( u8 ) ( Longi >> 16 );
        VdrData.Longi[2] = ( u8 ) ( Longi >> 8 );
        VdrData.Longi[3] = ( u8 ) Longi;


    }
    else
    {
        VdrData.Longi[0] = ( u8 ) ( 0x7F);
        VdrData.Longi[1] = ( u8 ) ( 0xFF );
        VdrData.Longi[2] = ( u8 ) ( 0xFF);
        VdrData.Longi[3] = ( u8 ) 0xFF;
    }

    //---------------------------------------------------
}

void Long_WE_pro(u8 *tmpinfo)
{

    GPRMC.longtitude = tmpinfo[0];
}


void Speed_pro(u8 *tmpinfo, u8 Invalue, u8 Point)
{
    u32	  sp = 0, sp_DISP = 0;
    u32     reg = 0;


    //-------------------------------------------------------------------------------------------------------------
    if(Invalue == INIT)
    {
        return;
    }
    else//---------------------------------------------------------------------------------------------------------
    {
        GPRMC.speed = atof((char *)tmpinfo);
        //---------------------------------------------------
        if(UDP_dataPacket_flag == 0x02 )
        {
            //-----808 Э�� --------------
            //�����ֽڵ�λ0.1 km/h
            if ( Point == 1 )	//0.0-9.9=>
            {
                //++++++++  Nathan Modify on 2008-12-1   ++++++++++
                if((tmpinfo[0] >= 0x30) && (tmpinfo[0] <= 0x39) && (tmpinfo[2] >= 0x30) && (tmpinfo[2] <= 0x39))
                {
                    sp = ( tmpinfo[0] - 0x30 ) * 10 + ( tmpinfo[2] - 0x30 );  //����10��
                }
                else
                    return;

            }
            else if ( Point == 2 )  //10.0-99.9
            {
                //++++++++  Nathan Modify on 2008-12-1   ++++++++++
                if((tmpinfo[0] >= 0x30) && (tmpinfo[0] <= 0x39) && (tmpinfo[1] >= 0x30) && (tmpinfo[1] <= 0x39) && (tmpinfo[3] >= 0x30) && (tmpinfo[3] <= 0x39))
                {
                    sp = ( tmpinfo[0] - 0x30 ) * 100 + ( tmpinfo[1] - 0x30 ) * 10 + tmpinfo[3] - 0x30;
                }
                else
                    return;

            }
            else if( Point == 3 ) //100.0-999.9
            {
                //++++++++  Nathan Modify on 2008-12-1	++++++++++
                if((tmpinfo[0] >= 0x30) && (tmpinfo[0] <= 0x39) && (tmpinfo[1] >= 0x30) && (tmpinfo[1] <= 0x39) && (tmpinfo[2] >= 0x30) && (tmpinfo[2] <= 0x39) && (tmpinfo[4] >= 0x30) && (tmpinfo[4] <= 0x39))
                {
                    sp = ( tmpinfo[0] - 0x30 ) * 1000 + ( tmpinfo[1] - 0x30 ) * 100 + ( tmpinfo[2] - 0x30 ) * 10 + tmpinfo[4] - 0x30;
                }
                else
                    return;

            }
            else
            {
                if(JT808Conf_struct.Speed_GetType == 0)
                    Spd_Using = 0;
            }

            // --------  sp ��ǰ��0.1 knot------------------
            sp = (u32)(sp * 185.6) ; //  1 ����=1.856 ǧ��  ������m/h

            if(sp > 220000) //ʱ�ٴ���220km/h���޳�
                return;

            sp_DISP = sp / 100; //  sp_Disp ��λ�� 0.1km/h

            //------------------------------ ͨ��GPSģ�����ݻ�ȡ�����ٶ� --------------------------------
            if(1 == Limit_max_SateFlag)
            {
                if((sp_DISP >= 1200) && (sp_DISP < 1500))
                    sp_DISP = 1200;   //  �ٶȴ���120 km/h   ��С��150 km/h
                if(sp_DISP >= 1500)
                    Illeagle_Data_kickOUT = 1; // �ٶȴ���150  �޳�
                else
                    Speed_gps = (u16)sp_DISP;
            }
            else
                Speed_gps = (u16)sp_DISP;


            //  Speed_gps=Speed_jiade;//800;  //  �ٵ�Ϊ�˲���

            //---------------------------------------------------------------------------
            if(JT808Conf_struct.Speed_GetType)  // ͨ���ٶȴ����� ��ȡ�ٶ�
            {
                K_AdjustUseGPS(Speed_gps);  //  ����Kֵ
                if(JT808Conf_struct.DF_K_adjustState == 0)
                {
                    // ---  ��δУ׼ǰ����õ����ٶ���ͨ��GPS����õ���
                    Spd_Using = Speed_gps;
                }
                else
                {
                    if((Speed_cacu < 50) && (Speed_gps > 150))
                        //  gps ���� 15 km/h   �Ҵ������ٶ�С��5 ������GPS�ٶȴ��洫�����ٶ�
                    {
                        Spd_Using = Speed_gps;
                    }
                    else
                        Spd_Using = Speed_cacu;
                }
            }
            else
            {
                // ��GPS ȡ�ٶ�
                Spd_Using = Speed_gps;  // ��GPS���ݼ���õ��ٶ� ��λ0.1km/h
            }
        }
        else if( UDP_dataPacket_flag == 0x03 )
        {
            Speed_gps = 0;
            if(0 == JT808Conf_struct.Speed_GetType)
            {
                //  δУ׼����£���GPS δ��λ ��ô��� 0
                Spd_Using = Speed_gps;
            }
            if((JT808Conf_struct.Speed_GetType) && (JT808Conf_struct.DF_K_adjustState))
            {
                // �Ӵ�����ȡ�ٶ��ң��Ѿ�У׼
                Spd_Using = Speed_cacu;
            }
        }
    }

    //---------------------------------------------------
    Delta_Speed_judge();//  �жϴ�������GPS�ٶȵĲ�ֵ

    //---------------------------------------------------
}


void Direction_pro(u8 *tmpinfo, u8 Invalue, u8 Point)
{
    u32	  sp = 0;
    //------------------------------------------------------------------------------------------------
    if(Invalue == INIT)
    {
        return;
    }
    else//-------------------------------------------------------------------------------------------
    {
        GPRMC.azimuth_angle = atof((char *)tmpinfo);


        //--------------808 Э��  1 ��-------------------------
        if ( UDP_dataPacket_flag == 0x02 )
        {


            if ( Point == 1 )   //5.8
            {
                if((tmpinfo[0] >= 0x30) && (tmpinfo[0] <= 0x39) && (tmpinfo[2] >= 0x30) && (tmpinfo[2] <= 0x39))
                    sp = ( tmpinfo[0] - 0x30 ) ;
                else
                    return;

            }
            else if ( Point == 2 )  // 14.7
            {
                if((tmpinfo[0] >= 0x30) && (tmpinfo[0] <= 0x39) && (tmpinfo[1] >= 0x30) && (tmpinfo[1] <= 0x39) && (tmpinfo[3] >= 0x30) && (tmpinfo[3] <= 0x39))
                    sp = ( tmpinfo[0] - 0x30 ) * 10 + ( tmpinfo[1] - 0x30 );
                else
                    return;

            }
            else    //357.38
                if ( Point == 3 )
                {
                    if((tmpinfo[0] >= 0x30) && (tmpinfo[0] <= 0x39) && (tmpinfo[1] >= 0x30) && (tmpinfo[1] <= 0x39) && (tmpinfo[2] >= 0x30) && (tmpinfo[2] <= 0x39) && (tmpinfo[4] >= 0x30) && (tmpinfo[4] <= 0x39))
                        sp = ( tmpinfo[0] - 0x30 ) * 100 + ( tmpinfo[1] - 0x30 ) * 10 + ( tmpinfo[2] - 0x30 ) ;
                    else
                        return;

                }
                else
                {
                    sp = 0;
                }
            GPS_direction = sp; //  ��λ 1��

            //----------  �յ㲹�����   ----------
            // Inflexion_Process();



        }
        else if ( UDP_dataPacket_flag == 0x03 )
        {
            GPS_direction = 0;

        }


        return;
    }



}

void Date_pro(u8 *tmpinfo, u8 fDateModify, u8 hour, u8 min , u8 sec)
{
    uint8_t  year = 0, mon = 0, day = 0;
    TDateTime now, now_bak;
    int i = 0;


    day = (( tmpinfo[0] - 0x30 ) * 10 ) + ( tmpinfo[1] - 0x30 );
    mon = (( tmpinfo[2] - 0x30 ) * 10 ) + ( tmpinfo[3] - 0x30 );
    year = (( tmpinfo[4] - 0x30 ) * 10 ) + ( tmpinfo[5] - 0x30 );

    if(fDateModify)
    {
        //sscanf(tmpinfo,"%2d%2d%2d",&day,&mon,&year);
        day++;
        if(mon == 2)
        {
            if ( ( year % 4 ) == 0 )
            {
                if ( day == 30 )
                {
                    day = 1;
                    mon++;
                }
            }
            else if ( day == 29 )
            {
                day = 1;
                mon++;
            }
        }
        else if (( mon == 4 ) || ( mon == 6 ) || ( mon == 9 ) || ( mon == 11 ))
        {
            if ( day == 31 )
            {
                mon++;
                day = 1;
            }
        }
        else
        {
            if ( day == 32 )
            {
                mon++;
                day = 1;
            }
            if( mon == 13 )
            {
                mon = 1;
                year++;
            }
        }
    }

    //--------  ��ݹ��ˣ� �����λ����ǰʱ�䲻�账��ֱ�ӷ���
    if(year < 13)
    {
        //----- ״̬�ĳ� V
        UDP_dataPacket_flag = 0X03;
        StatusReg_GPS_V();

        Year_illigel = 1; // ��ݲ��Ϸ�

        return ;
    }
    else
        Year_illigel = 0;

    GPRMC.utc_year = year;
    GPRMC.utc_mon = mon;
    GPRMC.utc_day = day;
    if((sec == 30) && (GPRMC.status == 'A'))
    {
        now.year = year;
        now.month = mon;
        now.day = day;
        now.hour = hour;
        now.min = min;
        now.sec = sec;
        now.week = 1;
        Device_RTC_set(now);

        //------  ��ȡ����У��---
        now_bak = Get_RTC();
        i = memcmp((u8 *)&now_bak, (u8 *)&now, sizeof(now));
        if(i != 0)
        {
            RT_Total_Config();
            Device_RTC_set(now);
        }

    }
    //------------------------------------------------
    if(GPRMC.status == 'A')   //  ��¼��λʱ��
    {
        Time2BCD(A_time);
        //------- Debug �洢 ÿ��ľ�γ��  || ʵ��Ӧ���� �洢ÿ���ӵ�λ��  -----
        //  ���ݳ���55��ÿ����£���Ĵ����м�¼������ÿ���������һ����λ�ľ�γ�� ,Ԥ��5�����ڴ洢��һСʱ��λ��
        if(sec < 55)
        {
            memcpy(Posit[min].latitude_BgEnd, Gps_Gprs.Latitude, 4); //��γ
            memcpy(Posit[min].longitude_BgEnd, Gps_Gprs.Longitude, 4); //����
            Posit[min].longitude_BgEnd[0] |= 0x80; //  ����
        }
        if((min == 59) && (sec == 55))
        {
            // ÿ��Сʱ��λ����Ϣ
            NandsaveFlg.MintPosit_SaveFlag = 1;
        }
    }
    //---- �洢��ǰ����ʼ���  ����ʱ------------
    if((hour == 0) && (min == 0) && (sec == 3)) // �洢3��ȷ���洢�ɹ�
    {
        DF_Write_RecordAdd(Distance_m_u32, DayStartDistance_32, TYPE_DayDistancAdd);
        JT808Conf_struct.DayStartDistance_32 = DayStartDistance_32;
        JT808Conf_struct.Distance_m_u32 = Distance_m_u32;

        // ������� һ��һ��
        GB19056.speedlog_Day_save_times = 0; //clear
        VdrData.H_15[0] = 0x00;
        GB19056.start_record_speedlog_flag = 0; // clear
    }

    //-------------------------------------------------
    //---------  ���ͨЭ��  -------

    //if(systemTick_TriggerGPS==0)
    {
        Temp_Gps_Gprs.Date[0] = year;
        Temp_Gps_Gprs.Date[1] = mon;
        Temp_Gps_Gprs.Date[2] = day;
    }

}


//---------  GGA --------------------------
void HDop_pro(u8 *tmpinfo)
{
    float dop;

    dop = atof((char *)tmpinfo);
    HDOP_value = dop;		 //  Hdop ��ֵ

}

void  GPS_Delta_DurPro(void)    //��GPS �����ϱ�������
{
    u32  longi_old = 0, longi_new = 0, lait_old = 0 , lati_new = 0;

    //    1.    ��ʱ�ϱ����
    if((1 == JT808Conf_struct.SD_MODE.DUR_TOTALMODE) && (Year_illigel == 0)) // ��ʱ�ϱ�ģʽ
    {
        //----- ��һ�����ݼ�¼��ʱ��
        fomer_time_seconds = ( u32 ) ( BakTime[0] * 60 * 60 ) + ( u32 ) ( BakTime[1] * 60 ) + ( u32 ) BakTime[2];

        //-----  ��ǰ���ݼ�¼��ʱ��
        tmp_time_secnonds = ( u32 ) ( CurrentTime[0] * 60 * 60 ) + ( u32 ) ( CurrentTime[1] * 60 ) + ( u32 )  CurrentTime[2];

        //һ��86400��

        if ( tmp_time_secnonds > fomer_time_seconds )
        {
            delta_time_seconds = tmp_time_secnonds - fomer_time_seconds;
            //systemTickGPS_Clear();
        }
        else if(tmp_time_secnonds < fomer_time_seconds)
        {
            delta_time_seconds = 86400 - fomer_time_seconds + tmp_time_secnonds;
            //systemTickGPS_Clear();
        }
        else
        {
            // systemTickGPS_Set();
            UDP_dataPacket_flag = 0X03;
            StatusReg_GPS_V();
        }

        if((SleepState == 1) && (delta_time_seconds == (Current_SD_Duration - 5) && (Current_SD_Duration > 5))) //  --  ����ʱ �ȷ���Ȩ
        {
            SleepConfigFlag = 1; //����ǰ5 ����һ����Ȩ
        }

        if((delta_time_seconds >= Current_SD_Duration))//limitSend_idle
        {

            Current_State = 0;
            if (BD_ISP.ISP_running == 0)
                PositionSD_Enable();

            memcpy(BakTime, CurrentTime, 3); // update
        }
    }

    //------------------------------ do this every  second-----------------------------------------

    //----- �¹��ɵ�3  :      �ٶȴ��� 0  km   λ�� û�б仯
    longi_old = (Gps_Gprs.Longitude[0] << 24) + (Gps_Gprs.Longitude[1] << 16) + (Gps_Gprs.Longitude[2] << 8) + Gps_Gprs.Longitude[3];
    lait_old = (Gps_Gprs.Latitude[0] << 24) + (Gps_Gprs.Latitude[1] << 16) + (Gps_Gprs.Latitude[2] << 8) + Gps_Gprs.Latitude[3];;
    longi_new = (Temp_Gps_Gprs.Longitude[0] << 24) + (Temp_Gps_Gprs.Longitude[1] << 16) + (Temp_Gps_Gprs.Longitude[2] << 8) + Temp_Gps_Gprs.Longitude[3];
    lati_new = (Temp_Gps_Gprs.Latitude[0] << 24) + (Temp_Gps_Gprs.Latitude[1] << 16) + (Temp_Gps_Gprs.Latitude[2] << 8) + Temp_Gps_Gprs.Latitude[3];
    if(UDP_dataPacket_flag == 0x02)
        GB_doubt_Data3_Trigger(lait_old, longi_old, lati_new, longi_new);


    if(Illeagle_Data_kickOUT == 0)
        memcpy((char *)&Gps_Gprs, (char *)&Temp_Gps_Gprs, sizeof(Temp_Gps_Gprs));
    else
        Illeagle_Data_kickOUT = 0; // clear



    // 3.  ��ʻ��¼������ݲ��� ����,  �Ҷ�λ�������
    // VDR_product_08H_09H_10H();


    //4.   ����Χ�� �ж�  ----------
    /*  if((Temp_Gps_Gprs.Time[2]%20)==0) //   ��֤ʱ�����Բ�ε���Χ��
      {
          CycleRail_Judge(Temp_Gps_Gprs.Latitude,Temp_Gps_Gprs.Longitude);
    	//rt_kprintf("\r\n --- �ж�Բ�ε���Χ��");
      }	*/
    // if((Temp_Gps_Gprs.Time[2]==5)||(Temp_Gps_Gprs.Time[2]==25)||(Temp_Gps_Gprs.Time[2]==45)) //
    if(Temp_Gps_Gprs.Time[2] % 2 == 0) //    ��֤ʱҪ��2 ��
    {
        RectangleRail_Judge(Temp_Gps_Gprs.Latitude, Temp_Gps_Gprs.Longitude);
        //rt_kprintf("\r\n -----�жϾ��ε���Χ��");
    }
    if((Temp_Gps_Gprs.Time[2] % 5) == 0) //
    {
        // printf("\r\n --- �ж�Բ�ε���Χ��");
        // RouteRail_Judge(Temp_Gps_Gprs.Latitude,Temp_Gps_Gprs.Longitude);
        ;
    }
    //rt_kprintf("\r\n Delta_seconds %d \r\n",delta_time_seconds);
    //----------------------------------------------------------------------------------------
}

//---------------------------------------------------------------------------------------------------
void K_AdjustUseGPS(u32  sp_DISP)  // ͨ��GPS У׼  K ֵ  (������ʻ1KM ��������Ŀ)  0.1 km/h ����
{

    u32 Reg_distance = 0;
    u32 Reg_plusNum = 0;
    u16 i = 0;

    if(JT808Conf_struct.DF_K_adjustState)   // ֻ��ûУ׼ʱ����Ч
        return;

    Speed_Rec = (u8)(sp_DISP / 10);	// GPS�ٶ�    ��λ:km/h
    // -------	Ҫ���ٶ���60��65km/h  -------------
    if(((Speed_Rec >= Speed_area) && (Speed_Rec <= (Speed_area + 8))) || ((Speed_Rec >= 40) && (Speed_Rec <= (40 + 8))) || ((Speed_Rec >= 70) && (Speed_Rec <= (70 + 8)))) // Speed_area=60
    {
        Spd_adjust_counter++;
        if(Spd_adjust_counter > K_adjust_Duration) //�������ٶ���60~65����Ϊ�Ѿ���������
        {
            // �û�ȡ��������GPS�ٶ���Ϊ��׼���͸��ݴ���������������ٶȣ���Kֵ��У׼
            Reg_distance = 0;	// clear
            Reg_plusNum = 0;	// clear
            for(i = 2; i < K_adjust_Duration; i++) //�޳�ǰ2������(ǰ�������ݲ��Ǻ�׼ȷ����Ϊʲôʱ�̿�ʼ��ȷ��)
            {
                Reg_distance += Former_gpsSpd[i]; // ����3.6km/h ��ʾ���������˶�����
                Reg_plusNum += Former_DeltaPlus[i];

                //rt_kprintf("\r\n  Former_gpsSpd[%d]=%d  Former_DeltaPlus[%d]=%d \r\n",i,Former_gpsSpd[i],i,Former_DeltaPlus[i]);
            }
            /*
                    ��һ���ж�  �� ����ٶȴ����������ã� ��ô���أ�
                 */
            if(Reg_plusNum < 20)
            {
                Spd_adjust_counter = 0;
                if(GB19056.workstate == 0)
                    rt_kprintf("\r\n    �ٶȴ����� û������!\r\n");
                return;
            }
            //===================================================================
            // ת���ɸ���GPS�ٶȼ�����ʻ�˶����ף�(�ܾ���) ������ڳ���3.6 ��Ϊ�˼��㷽�� ��x10  �ٳ���36
            Reg_distance = (u32)(Reg_distance * 10 / 36); // ת���ɸ���GPS�ٶȼ�����ʻ�˶����ף�(�ܾ���)
            // (Reg_plusNum/Reg_distance) ��ʾ�������������Ծ���(��)= ÿ�ײ������ٸ����� ����ΪKֵ��1000��������������Ӧ�ó���1000
            JT808Conf_struct.Vech_Character_Value = 1000 * Reg_plusNum / Reg_distance;
            //-------  �洢�µ�����ϵ�� --------------------------------
            JT808Conf_struct.DF_K_adjustState = 1;					// clear  Flag
            ModuleStatus |= Status_Pcheck;
            Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));

            Spd_adjust_counter = 0; // clear  counter
        }
        else
        {
            //-------- ��¼�涨ʱ���ڵ���������GPS�ٶ�----------
            Former_gpsSpd[Spd_adjust_counter] = Speed_Rec;
            Former_DeltaPlus[Spd_adjust_counter] = Delta_1s_Plus;
        }

    }
    else
        Spd_adjust_counter = 0; // ֻҪ�ٶȳ���Ԥ�跶Χ ��������0
}


void  Delta_Speed_judge(void)
{
    /*
         Note: �ڴӴ������ٶȣ����ٶȱ�У׼������£���GPS �ٶȴ���40Km/h
                 ��ʱ���ж��ٶȴ�������ʵ���ٶȵĲ�ֵ������ٶȲ�ֵ����������12 km/h
                ���ҳ���һ��ʱ�䣬˵����ǰ����ϵ���Ѿ���׼ȷ�� ��ô��������У׼��
       */
    if((JT808Conf_struct.DF_K_adjustState) && (JT808Conf_struct.Speed_GetType) && (Speed_gps >= 400) && (UDP_dataPacket_flag == 0x02 ))
    {
        // rt_kprintf("\r\n  delta  check \r\n");
        if(abs(Speed_gps - Speed_cacu) > 150) // �ٶ����18Km/h    ����ĵ�λ��0.1km/h
        {
            Spd_Deltacheck_counter++;
            if(Spd_Deltacheck_counter > 30) //  ����30s
            {
                Spd_Deltacheck_counter = 0; // clear
                JT808Conf_struct.DF_K_adjustState = 0; // disable
                // rt_kprintf("\r\n  Re_adust!\r\n");
            }
        }
        else
            Spd_Deltacheck_counter = 0; // clear

    }

}
//==================================================================================================
// �ڶ����� :   �������ⲿ������״̬���
//==================================================================================================
/*
     -----------------------------
     2.1   ��Э����صĹ��ܺ���
     -----------------------------
*/


/*
     -----------------------------
    2.4  ��ͬЭ��״̬�Ĵ����仯
     -----------------------------
*/

void StatusReg_WARN_Enable(void)
{
    //     ��������״̬�� �Ĵ����ı仯
    Warn_Status[3] |= 0x01;//BIT( 0 );
}

void StatusReg_WARN_Clear(void)
{
    //     ��������Ĵ���
    Warn_Status[3] &= ~0x01;//BIT( 0 );
}

void StatusReg_ACC_ON(void)
{
    //    ACC ��
    Car_Status[3] |= 0x01; //  Bit(0)     Set  1  ��ʾ ACC��

}

void StatusReg_ACC_OFF(void)
{
    //    ACC ��

    Car_Status[3] &= ~0x01; //  Bit(0)     Set  01  ��ʾ ACC��
}

void StatusReg_POWER_CUT(void)
{
    //  ����Դ�Ͽ�
    Warn_Status[2]	|= 0x01;//BIT( 0 );
    ModuleStatus |= Status_Battery;
}

void StatusReg_POWER_NORMAL(void)
{
    // ����Դ����
    Warn_Status[2] &= ~0x01;//BIT( 0 );
    ModuleStatus &= ~Status_Battery;
}

void StatusReg_GPS_A(void)
{
    // GPS ��λ
    GPS_getfirst = 1;
    Car_Status[3] |= 0x02; //Bit(1)
    ModuleStatus |= Status_GPS;
    UDP_dataPacket_flag = 0X02;
}

void StatusReg_GPS_V(void)
{
    //  GPS ����λ
    Car_Status[3] &= ~0x02; //Bit(1)
    ModuleStatus &= ~Status_GPS;
    UDP_dataPacket_flag = 0X03;
}

void StatusReg_SPD_WARN(void)
{
    //  ���ٱ���
    Warn_Status[3] |= 0x02;//BIT( 1 );
}

void StatusReg_SPD_NORMAL(void)
{
    //  �ٶ�����
    Warn_Status[3] &= ~ 0x02; //BIT( 1 );
}

void StatusReg_Relay_Cut(void)
{
    // ���Ͷϵ�״̬

}

void StatusReg_Relay_Normal(void)
{
    //  ���͵�״̬����

}


void StatusReg_Default(void)
{
    //   ״̬�Ĵ�����ԭĬ������

    Warn_Status[0] = 0x00; //HH
    Warn_Status[1] = 0x00; //HL
    Warn_Status[2] = 0x00; //LH
    Warn_Status[3] = 0x00; //LL
}

//==================================================================================================
// �������� :   ������GPRS���ߴ������Э��
//==================================================================================================
void  Save_GPS(void)
{
    u16 counter_mainguffer, i;
    u8  lati_reg[4];//,regstatus;
    u32 Dis_01km = 0;

    if (PositionSD_Status())
    {
        //  init    DF
        if(DF_initOver == 0)
            return;
        if(Login_Menu_Flag != 1) // û�б�����ǰ���洢���λ����Ϣ��û������
            return;
        //-------------------------------------------------------
        //1.   ʱ�䳬ǰ�ж�
        //if(Time_FastJudge()==false)
        //return ;
        //----------------------- Save GPS --------------------------------------
        /*
                 1  recrod    128  Bytes,   1st   is  the length of  record , last  is  fcs (which is caulated from  length byte to end)

                 ÿ����¼��128���ֽڵ����򡣵�һ���ֽ��ǳ��ȣ����һ���ֽ���У�顣

        */
        memset(GPSsaveBuf, 0, 128);
        GPSsaveBuf_Wr = 0;
        //------------------------------- Stuff ----------------------------------------
        GPSsaveBuf_Wr++;  // ��һ���ֽ��ǳ���������Ϣ�ӵڶ����ֽڿ�ʼ
        // 1. �澯״̬   4 Bytes
        memcpy( ( char * ) GPSsaveBuf + GPSsaveBuf_Wr, ( char * )Warn_Status, 4 );
        GPSsaveBuf_Wr += 4;
        // 2. ����״̬   4 Bytes
        memcpy( ( char * ) GPSsaveBuf + GPSsaveBuf_Wr, ( char * )Car_Status, 4 );
        GPSsaveBuf_Wr += 4;
        // 3.   γ��     4 Bytes
        memcpy(lati_reg, Gps_Gprs.Latitude, 4);
        memcpy( ( char * ) GPSsaveBuf + GPSsaveBuf_Wr, ( char * )Gps_Gprs.Latitude, 4 );//γ��   modify by nathan
        GPSsaveBuf_Wr += 4;
        // 4.   ����     4 Bytes
        memcpy( ( char * ) GPSsaveBuf + GPSsaveBuf_Wr, ( char * )  Gps_Gprs.Longitude, 4 );	 //����    ����  Bit 7->0	���� Bit 7 -> 1
        GPSsaveBuf_Wr += 4;
        // 5.  �߶�	  2 Bytes    m
        GPSsaveBuf[GPSsaveBuf_Wr++] = (GPS_Hight >> 8);	// High
        GPSsaveBuf[GPSsaveBuf_Wr++] = (u8)GPS_Hight; // Low
        // 6.  �ٶ�	  2 Bytes     0.1Km/h
        GPSsaveBuf[GPSsaveBuf_Wr++] = (Speed_gps >> 8);	// High
        GPSsaveBuf[GPSsaveBuf_Wr++] = (u8)Speed_gps; // Low
        // 7.  ����	  2 Bytes	    1��
        GPSsaveBuf[GPSsaveBuf_Wr++] = (GPS_direction >> 8);	//High
        GPSsaveBuf[GPSsaveBuf_Wr++] = GPS_direction; // Low
        // 8.  ����ʱ��	  6 Bytes
        GPSsaveBuf[GPSsaveBuf_Wr++] = (((Gps_Gprs.Date[0]) / 10) << 4) + ((Gps_Gprs.Date[0]) % 10);
        GPSsaveBuf[GPSsaveBuf_Wr++] = ((Gps_Gprs.Date[1] / 10) << 4) + (Gps_Gprs.Date[1] % 10);
        GPSsaveBuf[GPSsaveBuf_Wr++] = ((Gps_Gprs.Date[2] / 10) << 4) + (Gps_Gprs.Date[2] % 10);
        GPSsaveBuf[GPSsaveBuf_Wr++] = ((Gps_Gprs.Time[0] / 10) << 4) + (Gps_Gprs.Time[0] % 10);
        GPSsaveBuf[GPSsaveBuf_Wr++] = ((Gps_Gprs.Time[1] / 10) << 4) + (Gps_Gprs.Time[1] % 10);
        GPSsaveBuf[GPSsaveBuf_Wr++] = ((Gps_Gprs.Time[2] / 10) << 4) + (Gps_Gprs.Time[2] % 10);
        //------------------------------------------------------------------
        if(strncmp(( char * )GPSsaveBuf + GPSsaveBuf_Wr - 3, (const char *)Sdgps_Time, 3) == 0)
        {
            /*if(strncmp((const char*)GPSsaveBuf + GPSsaveBuf_Wr-3,(const char*)Sdgps_Time,3)==0)
            	{
            	PositionSD_Disable();
            	rt_kprintf("\r\n  -->���洢�ϱ�ʱ����ͬ�ĳ�\r\n");
            	return;
            	}
            else*/
            //{    //-------- ��RTC ʱ�� -----
            time_now = Get_RTC();

            if((time_now.year < 100) && (time_now.month < 12) && (time_now.day < 31) && (time_now.hour < 24) && (time_now.min < 60) && (time_now.sec < 60))
            {
                Time2BCD(GPSsaveBuf + GPSsaveBuf_Wr - 6);
                //rt_kprintf("\r\n    ����RTCʱ����! \r\n");
            }
        }
        memcpy(Sdgps_Time, GPSsaveBuf + GPSsaveBuf_Wr - 3, 3); //��������һ�δ洢ʱ��

        //----------------------- ������Ϣ----------------------------------------
        //	������Ϣ 1	-----------------------------
        //	������Ϣ ID
        // if(JT808Conf_struct.Speed_GetType==1) //ѡ�񴫸����ٶȲ��и��ֶ�
        {
            GPSsaveBuf[GPSsaveBuf_Wr++] = 0x03; // ��ʻ��¼�ǵ��ٶ�
            //	������Ϣ����
            GPSsaveBuf[GPSsaveBuf_Wr++] = 2;
            //	����
            GPSsaveBuf[GPSsaveBuf_Wr++] = (u8)(Spd_Using >> 8);
            GPSsaveBuf[GPSsaveBuf_Wr++] = (u8)(Spd_Using);
        }
        //rt_kprintf("\r\n GPS�ٶ�=%d km/h , �������ٶ�=%d km/h\r\n",Speed_gps,Speed_cacu);

        //  ������Ϣ 2  -----------------------------
        //  ������Ϣ ID
        /* GPSsaveBuf[GPSsaveBuf_Wr++]=0x01; // ���ϵ���ʻ���
         //  ������Ϣ����
         GPSsaveBuf[GPSsaveBuf_Wr++]=4;
         //  ����
         Dis_01km=JT808Conf_struct.Distance_m_u32/100;
         GPSsaveBuf[GPSsaveBuf_Wr++]=(Dis_01km>>24);
         GPSsaveBuf[GPSsaveBuf_Wr++]=(Dis_01km>>16);
         GPSsaveBuf[GPSsaveBuf_Wr++]=(Dis_01km>>8);
         GPSsaveBuf[GPSsaveBuf_Wr++]=Dis_01km;
        */

        //	������Ϣ 3
        if(Warn_Status[1] & 0x10)
        {
            //  ������Ϣ ID
            GPSsaveBuf[GPSsaveBuf_Wr++] = 0x12; //  ��������/·�߱���
            //  ������Ϣ����
            GPSsaveBuf[GPSsaveBuf_Wr++] = 6;
            //  ����
            GPSsaveBuf[GPSsaveBuf_Wr++] = InOut_Object.TYPE;
            GPSsaveBuf[GPSsaveBuf_Wr++] = (InOut_Object.ID >> 24);
            GPSsaveBuf[GPSsaveBuf_Wr++] = (InOut_Object.ID >> 16);
            GPSsaveBuf[GPSsaveBuf_Wr++] = (InOut_Object.ID >> 8);
            GPSsaveBuf[GPSsaveBuf_Wr++] = InOut_Object.ID;
            GPSsaveBuf[GPSsaveBuf_Wr++] = InOut_Object.InOutState;
            // rt_kprintf("\r\n ----- 0x0200 ������Ϣ \r\n");
        }

        //	������Ϣ4
        if(Warn_Status[3] & 0x02)
        {
            //  ������Ϣ ID
            GPSsaveBuf[GPSsaveBuf_Wr++] = 0x11; //  ��������/·�߱���
            //  ������Ϣ����
            GPSsaveBuf[GPSsaveBuf_Wr++] = 1;
            //  ����
            GPSsaveBuf[GPSsaveBuf_Wr++] = 0; //  ���ض�λ��

        }

        //---------- ������Ϣ 5 ----
        GPSsaveBuf[GPSsaveBuf_Wr++] = 0x25; //��չ�����ź�״̬
        //  ������Ϣ����
        GPSsaveBuf[GPSsaveBuf_Wr++] = 4;
        //  ����
        GPSsaveBuf[GPSsaveBuf_Wr++] = 0x00;
        GPSsaveBuf[GPSsaveBuf_Wr++] = 0x00;
        GPSsaveBuf[GPSsaveBuf_Wr++] = 0x00;
        GPSsaveBuf[GPSsaveBuf_Wr++] = BD_EXT.Extent_IO_status;
        //  ������Ϣ 5  -----------------------------
        //  ������Ϣ ID
        GPSsaveBuf[GPSsaveBuf_Wr++] = 0x30; //�ź�ǿ��
        //  ������Ϣ����
        GPSsaveBuf[GPSsaveBuf_Wr++] = 1;
        //  ����
        GPSsaveBuf[GPSsaveBuf_Wr++] = BD_EXT.FJ_SignalValue;


        //if(DispContent)
        //	  printf("\r\n---- Satelitenum: %d , CSQ:%d\r\n",Satelite_num,ModuleSQ);
#if  0
        //  ������Ϣ 6  -----------------------------
        //  ������Ϣ ID
        GPSsaveBuf[GPSsaveBuf_Wr++] = 0x2A; //�Զ���io
        //  ������Ϣ����
        GPSsaveBuf[GPSsaveBuf_Wr++] = 2;
        //  ����
        GPSsaveBuf[GPSsaveBuf_Wr++] = 0x00;
        GPSsaveBuf[GPSsaveBuf_Wr++] = 0x00;

        //  ������Ϣ 7 -----------------------------
        //  ������Ϣ ID
        GPSsaveBuf[GPSsaveBuf_Wr++] = 0x2B; //�Զ���ģ�����ϴ� AD
        //  ������Ϣ����
        GPSsaveBuf[GPSsaveBuf_Wr++] = 4;
        GPSsaveBuf[GPSsaveBuf_Wr++] = (BD_EXT.AD_0 >> 8);	// ģ���� 1
        GPSsaveBuf[GPSsaveBuf_Wr++] = BD_EXT.AD_0;
        GPSsaveBuf[GPSsaveBuf_Wr++] = (BD_EXT.AD_1 >> 8);	// ģ���� 2
        GPSsaveBuf[GPSsaveBuf_Wr++] = BD_EXT.AD_1;
#endif
        //------------------------------------------------
        GPSsaveBuf[0] = GPSsaveBuf_Wr;

        //-------------  Caculate  FCS  -----------------------------------
        FCS_GPS_UDP = 0;
        for ( i = 0; i < GPSsaveBuf_Wr; i++ )
        {
            FCS_GPS_UDP ^= *( GPSsaveBuf + i );
        }			   //���ϱ����ݵ�����
        GPSsaveBuf[GPSsaveBuf_Wr++] = FCS_GPS_UDP;
        //-------------------------------- Save  ------------------------------------------
        //OutPrint_HEX("Write����",GPSsaveBuf,GPSsaveBuf_Wr);
        if(Api_cycle_write(GPSsaveBuf, GPSsaveBuf_Wr))
        {
            if(DispContent)
                rt_kprintf("\r\n    GPS Save succed\r\n");
        }
        else
        {
            if(DispContent)
                rt_kprintf("\r\n GPS save fail\r\n");
        }
        //---------------------------------------------------------------------------------
        if(PositionSD_Status())
            PositionSD_Disable();
        //-----------------------------------------------------
    }


}
//----------------------------------------------------------------------
u8  Protocol_Head(u16 MSG_ID, u8 Packet_Type)
{
    u32    Reg_ID = 0;
    //----  clear --------------
    Original_info_Wr = 0;
    //	1. Head

    //  original info
    Original_info[Original_info_Wr++] = (MSG_ID >> 8); // ��ϢID
    Original_info[Original_info_Wr++] = (u8)MSG_ID;

    Original_info[Original_info_Wr++] = 0x00; // �ְ������ܷ�ʽ��״̬λ
    Original_info[Original_info_Wr++] = 28; // ��Ϣ�峤��   λ����Ϣ����Ϊ28���ֽ�

    memcpy(Original_info + Original_info_Wr, SIM_code, 6); // �ն��ֻ��� ���豸��ʶID	BCD
    Original_info_Wr += 6;



    if(Packet_Type == Packet_Divide)
    {
        switch (MediaObj.Media_Type)
        {
        case 0 : // ͼ��
            MediaObj.Media_totalPacketNum = Photo_sdState.Total_packetNum; // ͼƬ�ܰ���
            MediaObj.Media_currentPacketNum = Photo_sdState.SD_packetNum; // ͼƬ��ǰ����
            MediaObj.Media_ID = 1; //  ��ý��ID
            MediaObj.Media_Channel = CameraState.Camera_Number; // ͼƬ����ͷͨ����

            Reg_ID = 0xF000 + CameraState.Camera_Number * 0x0100 + Photo_sdState.SD_packetNum;
            Original_info[Original_info_Wr++] = ( Reg_ID >> 8); //��Ϣ��ˮ��
            Original_info[Original_info_Wr++] =  Reg_ID;

            break;
        case 1 : // ��Ƶ
            MediaObj.Media_totalPacketNum = Sound_sdState.Total_packetNum;	// ��Ƶ�ܰ���
            MediaObj.Media_currentPacketNum = Sound_sdState.SD_packetNum; // ��Ƶ��ǰ����
            MediaObj.Media_ID = 1;	 //  ��ý��ID
            MediaObj.Media_Channel = 1; // ��Ƶͨ����

            Reg_ID = 0xE000 + Sound_sdState.SD_packetNum;
            Original_info[Original_info_Wr++] = ( Reg_ID >> 8); //��Ϣ��ˮ��
            Original_info[Original_info_Wr++] =  Reg_ID;

            break;
        case 2 : // ��Ƶ
            MediaObj.Media_totalPacketNum = Video_sdState.Total_packetNum;	// ��Ƶ�ܰ���
            MediaObj.Media_currentPacketNum = Video_sdState.SD_packetNum; // ��Ƶ��ǰ����
            MediaObj.Media_ID = 1;	 //  ��ý��ID
            MediaObj.Media_Channel = 1; // ��Ƶͨ����
            break;
        case  3:                                                                                    //�г���¼��
            MediaObj.Media_totalPacketNum	= Recode_Obj.Total_pkt_num;                             // ��¼���ܰ���
            MediaObj.Media_currentPacketNum = Recode_Obj.Current_pkt_num;                           // ��¼�ǵ�ǰ����

            Reg_ID = 0xA000 + Recode_Obj.Current_pkt_num;
            Original_info[Original_info_Wr++] = ( Reg_ID >> 8); //��Ϣ��ˮ��
            Original_info[Original_info_Wr++] =  Reg_ID;

            break;
        default:
            return false;
        }
        //    ��ǰ����
        Original_info[Original_info_Wr++] = (MediaObj.Media_totalPacketNum & 0xff00) >> 8; //��block
        Original_info[Original_info_Wr++] = (u8)MediaObj.Media_totalPacketNum; //��block

        //   �ܰ���
        Original_info[Original_info_Wr++] = ((MediaObj.Media_currentPacketNum) & 0xff00) >> 8; //��ǰblock
        Original_info[Original_info_Wr++] = (u8)((MediaObj.Media_currentPacketNum) & 0x00ff); //��ǰblock

    }
    else
    {
        Original_info[Original_info_Wr++] = ( JT808Conf_struct.Msg_Float_ID >> 8); //��Ϣ��ˮ��
        Original_info[Original_info_Wr++] = JT808Conf_struct.Msg_Float_ID;
    }


    return true;

}

void Protocol_End(u8 Packet_Type, u8 LinkNum)
{
    u16 packet_len = 0;
    u16  i = 0;		//Ҫ���͵�UDP �������ݵĳ���
    u8   Gfcs = 0;
    u16   Msg_bodyLen = 0; //  Э�������Ϣֻ��ʾ��Ϣ��     ��������Ϣͷ ��ϢͷĬ�ϳ�����12 , �ְ���Ϣͷ���� 20

    Gfcs = 0;				 //  �������Ϣͷ��ʼ��У��ǰ���ݵ�����  808Э��У��  1Byte
    //---  ��д��Ϣ���� ---
    if(Packet_Normal == Packet_Type)
    {
        Msg_bodyLen = Original_info_Wr - 12;
        Original_info[2] = (Msg_bodyLen >> 8) ;
        Original_info[3] = Msg_bodyLen;
    }
    else if(Packet_Divide == Packet_Type)
    {
        Msg_bodyLen = Original_info_Wr - 16;
        // rt_kprintf("\r\n Divide Infolen=%d  \r\n",Msg_bodyLen);
        Original_info[2] = (Msg_bodyLen >> 8) | 0x20 ; // Bit 13  0x20 ����Bit 13
        Original_info[3] = Msg_bodyLen;
    }
    //---- ����У��  -----
    for(i = 0; i < Original_info_Wr; i++)
        Gfcs ^= Original_info[i];
    Original_info[Original_info_Wr++] = Gfcs; // ��дGУ��λ


    // 1.stuff start
    GPRS_infoWr_Tx = 0;
    GPRS_info[GPRS_infoWr_Tx++] = 0x7e; // Start ��ʶλ
    if(Packet_Divide == Packet_Type)
    {
        //rt_kprintf("\r\n Tx=%d  Divide Infolen=%d  \r\n",GPRS_infoWr_Tx,Original_info_Wr);
        /* rt_kprintf("\r\n PacketContent: ");
        		   for(i=0;i<Original_info_Wr;i++)
                         rt_kprintf(" %X",Original_info[i]);
        		   rt_kprintf("\r\n");
        */
    }
    // 2.  convert
    packet_len = Protocol_808_Encode(GPRS_info + GPRS_infoWr_Tx, Original_info, Original_info_Wr);
    GPRS_infoWr_Tx += packet_len;
    if(Packet_Divide == Packet_Type)
    {
        //rt_kprintf("\r\n Divide  Send Infolen=%d  \r\n",packet_len);

        /* rt_kprintf("\r\n EncodeContent: ");
         for(i=0;i<packet_len;i++)
                           rt_kprintf(" %X",GPRS_info[i+1]);
        rt_kprintf("\r\n");
        */
        //rt_kprintf("\r\n GPRStx  Send Infolen=%d  \r\n",GPRS_infoWr_Tx+1);
    }
    GPRS_info[GPRS_infoWr_Tx++] = 0x7e; //  End  ��ʶ
    //  4. Send

    // 4.1 ������Ϣ����1
    //  if(DispContent==2)
    // {
    //	OutPrint_HEX("App to GSM info",GPRS_info,GPRS_infoWr_Tx);
    // }
    // 4.2   MsgQueue
    WatchDog_Feed();
    Gsm_rxAppData_SemRelease(GPRS_info, GPRS_infoWr_Tx, LinkNum);
    //------------------------------
}
//--------------------------------------------------------------------------------------
u8  Stuff_DevCommmonACK_0001H(void)
{
    // 1. Head
    if(!Protocol_Head(MSG_0x0001, Packet_Normal))  return false;    //�ն�ͨ��Ӧ��
    // 2. content  is null
    //   float ID
    Original_info[Original_info_Wr++] = (u8)(Centre_FloatID >> 8);
    Original_info[Original_info_Wr++] = (u8)Centre_FloatID;
    //  cmd  ID
    Original_info[Original_info_Wr++] = (u8)(Centre_CmdID >> 8);
    Original_info[Original_info_Wr++] = (u8)Centre_CmdID;
    //   resualt
    Original_info[Original_info_Wr++] = SD_ACKflag.f_CentreCMDack_resualt;
    //  3. Send
    Protocol_End(Packet_Normal , 0);
    if(DispContent)
        rt_kprintf("\r\n	Common CMD ACK! \r\n");
    return true;
}
//-------------------------------------------------------------------------------
u8  Stuff_RegisterPacket_0100H(u8  LinkNum)
{
    u8  i = 0;
    // 1. Head
    if(!Protocol_Head(MSG_0x0100, Packet_Normal))
        return false;

    // 2. content
    //  province ID
    Original_info[Original_info_Wr++] = (u8)(Vechicle_Info.Dev_ProvinceID >> 8);
    Original_info[Original_info_Wr++] = (u8)Vechicle_Info.Dev_ProvinceID; // ���12     64  ����
    //  county  ID
    Original_info[Original_info_Wr++] = (u8)(Vechicle_Info.Dev_CityID >> 8); // 101  ����
    Original_info[Original_info_Wr++] = (u8)Vechicle_Info.Dev_CityID;
    //  product Name
    //  product Name
    memcpy(Original_info + Original_info_Wr, "70420", 5); //�����ж�  70104      70523     70218
    Original_info_Wr += 5;
    //  �ն��ͺ� 20 Bytes      -- ����Э����������
    memcpy(Original_info + Original_info_Wr, Vechicle_Info.ProType, 5); //ZD-V01H  HVT100BD1   CM-10A-BD  YW3000-YM/MGB
    Original_info_Wr += 5;
    for(i = 0; i < 15; i++)
        Original_info[Original_info_Wr++] = 0x00;
    //  �ն�ID   7 Bytes    ,
    memcpy(Original_info + Original_info_Wr, DeviceNumberID + 5, 7); // ��������ն�ID �ĺ�7 λ
    Original_info_Wr += 7;
    //  ������ɫ
    if(License_Not_SetEnable == 1)
        Original_info[Original_info_Wr++] = 0; //Vechicle_Info.Dev_Color;
    else
        Original_info[Original_info_Wr++] = 2; //Vechicle_Info.Dev_Color;

    if(License_Not_SetEnable == 0) //  0  ���ó��ƺ�
    {
        //  ����
        memcpy(Original_info + Original_info_Wr, Vechicle_Info.Vech_Num, strlen((const char *)(Vechicle_Info.Vech_Num))); //13);
        Original_info_Wr += strlen((const char *)(Vechicle_Info.Vech_Num));
    }
    else
    {
        // rt_kprintf("\r\n  ������ɫ:0     Need Vin\r\n");
        // ����VIN
        memcpy(Original_info + Original_info_Wr, Vechicle_Info.Vech_VIN, 17);
        Original_info_Wr += 17;
    }


    if(DispContent)
    {
        rt_kprintf("\r\n	SEND Reigster Packet! \r\n");
        rt_kprintf("\r\n  ע�ᷢ��ʱ�� %d-%d-%d %02d:%02d:%02d\r\n", time_now.year + 2000, time_now.month, time_now.day, \
                   time_now.hour, time_now.min, time_now.sec);


    }


    //  3. Send
    Protocol_End(Packet_Normal, LinkNum);
    if(DispContent)
        rt_kprintf("\r\n	SEND Reigster Packet! \r\n");
    return true;

}

//--------------------------------------------------------------------------------------
u8  Stuff_DeviceHeartPacket_0002H(void)
{

    // 1. Head
    if(!Protocol_Head(MSG_0x0002, Packet_Normal))
        return false;
    // 2. content  is null

    //  3. Send
    Protocol_End(Packet_Normal , 0);
    if(DispContent)
        rt_kprintf("\r\n	Send Dev Heart! \r\n");
    return true;

}
//--------------------------------------------------------------------------------------
u8  Stuff_DeviceDeregister_0101H(void)
{
    // 1. Head
    if(!Protocol_Head(MSG_0x0101, Packet_Normal))
        return false; //�ն�ע��
    // 2. content  is null
    //  3. Send
    Protocol_End(Packet_Normal , 0);
    if(DispContent)
        rt_kprintf("\r\n	Deregister  ע��! \r\n");
    return true;
}
//------------------------------------------------------------------------------------
u8  Stuff_DevLogin_0102H(void)
{
    // 1. Head
    if(!Protocol_Head(MSG_0x0102, Packet_Normal))
        return false; //�ն˼�Ȩ
    // 2. content

    memcpy(Original_info + Original_info_Wr, JT808Conf_struct.ConfirmCode, strlen((const char *)JT808Conf_struct.ConfirmCode)); // ��Ȩ��  string Type
    Original_info_Wr += strlen((const char *)JT808Conf_struct.ConfirmCode);
    //  3. Send
    Protocol_End(Packet_Normal , 0);
    if(DispContent)
        rt_kprintf("\r\n	 ���ͼ�Ȩ! \r\n");
    return true;
}

//--------------------------------------------------------------------------------------
u8  Stuff_Normal_Data_0200H(void)
{
    u8 spd_sensorReg[2];
    u8  rd_infolen = 0;
    //  1. Head
    if(!Protocol_Head(MSG_0x0200, Packet_Normal))
        return false;
    // 2. content
    WatchDog_Feed();
    if(Api_cycle_read(Original_info + Original_info_Wr, 128) == false)
    {
        rt_kprintf("\r\n  ��ȡ false\r\n ");
        return false;
    }
    // ��ȡ��Ϣ����
    rd_infolen = Original_info[Original_info_Wr];
    //OutPrint_HEX("read -1",Original_info+Original_info_Wr,rd_infolen+1);
    memcpy(Original_info + Original_info_Wr, Original_info + Original_info_Wr + 1, rd_infolen);
    //OutPrint_HEX("read -2",Original_info+Original_info_Wr,rd_infolen+1);
    Original_info_Wr += rd_infolen - 1; // ���ݳ��� �޳���һ�������ֽ�

    mangQu_read_reg = cycle_read; // update
    //  3. Send
    Protocol_End(Packet_Normal , 0);

    return true;

}
//------------------------------------------------------------------------------------
u8  Stuff_Current_Data_0200H(void)   //  ���ͼ�ʱ���ݲ��洢���洢����
{

    u32  Dis_01km = 0;

    if( Spd_Using <= ( JT808Conf_struct.Speed_warn_MAX * 10) )
        StatusReg_SPD_NORMAL();

    //  1. Head
    if(!Protocol_Head(MSG_0x0200, Packet_Normal))
        return false;
    // 2. content
    //------------------------------- Stuff ----------------------------------------
    // 1. �澯��־  4
    memcpy( ( char * ) Original_info + Original_info_Wr, ( char * )Warn_Status, 4 );
    Original_info_Wr += 4;
    // 2. ״̬  4
    memcpy( ( char * ) Original_info + Original_info_Wr, ( char * )Car_Status, 4 );
    Original_info_Wr += 4;
    // 3.  γ��
    memcpy( ( char * ) Original_info + Original_info_Wr, ( char * )  Gps_Gprs.Latitude, 4 ); //γ��   modify by nathan
    Original_info_Wr += 4;
    // 4.  ����
    memcpy( ( char * ) Original_info + Original_info_Wr, ( char * )  Gps_Gprs.Longitude, 4 );	 //����    ����  Bit 7->0   ���� Bit 7 -> 1
    Original_info_Wr += 4;
    // 5.  �߳�
    Original_info[Original_info_Wr++] = (u8)(GPS_Hight << 8);
    Original_info[Original_info_Wr++] = (u8)GPS_Hight;
    // 6.  �ٶ�    0.1 Km/h
    Original_info[Original_info_Wr++] = (u8)(Speed_gps >> 8); //(Spd_Using>>8);
    Original_info[Original_info_Wr++] = (u8)(Speed_gps); //Spd_Using;
    // 7. ����   ��λ 1��
    Original_info[Original_info_Wr++] = (GPS_direction >> 8); //High
    Original_info[Original_info_Wr++] = GPS_direction; // Low
    // 8.  ����ʱ��
    Original_info[Original_info_Wr++] = (((Gps_Gprs.Date[0]) / 10) << 4) + ((Gps_Gprs.Date[0]) % 10);
    Original_info[Original_info_Wr++] = ((Gps_Gprs.Date[1] / 10) << 4) + (Gps_Gprs.Date[1] % 10);
    Original_info[Original_info_Wr++] = ((Gps_Gprs.Date[2] / 10) << 4) + (Gps_Gprs.Date[2] % 10);
    Original_info[Original_info_Wr++] = ((Gps_Gprs.Time[0] / 10) << 4) + (Gps_Gprs.Time[0] % 10);
    Original_info[Original_info_Wr++] = ((Gps_Gprs.Time[1] / 10) << 4) + (Gps_Gprs.Time[1] % 10);
    Original_info[Original_info_Wr++] = ((Gps_Gprs.Time[2] / 10) << 4) + (Gps_Gprs.Time[2] % 10);


    //----------------------- ������Ϣ----------------------------------------
    //  ������Ϣ 1  -----------------------------
    //  ������Ϣ ID
    // if(JT808Conf_struct.Speed_GetType==1) //ѡ�񴫸����ٶȲ��и��ֶ�
    {
        Original_info[Original_info_Wr++] = 0x03; // ��ʻ��¼�ǵ��ٶ�
        //  ������Ϣ����
        Original_info[Original_info_Wr++] = 2;
        //  ����
        Original_info[Original_info_Wr++] = (u8)(Spd_Using >> 8);
        Original_info[Original_info_Wr++] = (u8)(Spd_Using);
    }
    //rt_kprintf("\r\n GPS�ٶ�=%d km/h , �������ٶ�=%d km/h\r\n",Speed_gps,Speed_cacu);
    /*
     //  ������Ϣ 2  -----------------------------
     //  ������Ϣ ID
     Original_info[Original_info_Wr++]=0x01; // ���ϵ���ʻ���
     //  ������Ϣ����
     Original_info[Original_info_Wr++]=4;
     //  ����
     Dis_01km=JT808Conf_struct.Distance_m_u32/100;
     Original_info[Original_info_Wr++]=(Dis_01km>>24);
     Original_info[Original_info_Wr++]=(Dis_01km>>16);
     Original_info[Original_info_Wr++]=(Dis_01km>>8);
     Original_info[Original_info_Wr++]=Dis_01km;
    */
    //  ������Ϣ 3
    if(Warn_Status[1] & 0x10)
    {
        //  ������Ϣ ID
        Original_info[Original_info_Wr++] = 0x12; //  ��������/·�߱���
        //  ������Ϣ����
        Original_info[Original_info_Wr++] = 6;
        //  ����
        Original_info[Original_info_Wr++] = InOut_Object.TYPE;
        Original_info[Original_info_Wr++] = (InOut_Object.ID >> 24);
        Original_info[Original_info_Wr++] = (InOut_Object.ID >> 16);
        Original_info[Original_info_Wr++] = (InOut_Object.ID >> 8);
        Original_info[Original_info_Wr++] = InOut_Object.ID;
        Original_info[Original_info_Wr++] = InOut_Object.InOutState;
    }

    //  ������Ϣ4
    if(Warn_Status[3] & 0x02)
    {
        //	������Ϣ ID
        Original_info[Original_info_Wr++] = 0x11; //	��������/·�߱���
        //	������Ϣ����
        Original_info[Original_info_Wr++] = 1;
        //	����
        Original_info[Original_info_Wr++] = 0; //  ���ض�λ��

    }

    //---------- ������Ϣ 5 ----
    Original_info[Original_info_Wr++] = 0x25; //��չ�����ź�״̬
    //  ������Ϣ����
    Original_info[Original_info_Wr++] = 4;
    //  ����
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = BD_EXT.Extent_IO_status;

    //  ������Ϣ 5  -----------------------------
    //  ������Ϣ ID
    Original_info[Original_info_Wr++] = 0x30; //�ź�ǿ��
    //  ������Ϣ����
    Original_info[Original_info_Wr++] = 1;
    //  ����
    Original_info[Original_info_Wr++] = BD_EXT.FJ_SignalValue;

    //if(DispContent)
    // 	printf("\r\n---- Satelitenum: %d , CSQ:%d\r\n",Satelite_num,ModuleSQ);
#if  0
    //	������Ϣ 6	-----------------------------
    //  ������Ϣ ID
    Original_info[Original_info_Wr++] = 0x2A; //�Զ���io
    //  ������Ϣ����
    Original_info[Original_info_Wr++] = 2;
    //  ����
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;

    //	������Ϣ 7 -----------------------------
    //  ������Ϣ ID
    Original_info[Original_info_Wr++] = 0x2B; //�Զ���ģ�����ϴ� AD
    //  ������Ϣ����
    Original_info[Original_info_Wr++] = 4;
    Original_info[Original_info_Wr++] = (BD_EXT.AD_0 >> 8);	 // ģ���� 1
    Original_info[Original_info_Wr++] = BD_EXT.AD_0;
    Original_info[Original_info_Wr++] = (BD_EXT.AD_1 >> 8); // ģ���� 2
    Original_info[Original_info_Wr++] = BD_EXT.AD_1;
#endif
    //  3. Send
    Protocol_End(Packet_Normal , 0);

    return true;

}
//-----------------------------------------------------------------------
u8  Stuff_Current_Data_0201H(void)   //   λ����Ϣ��ѯ��Ӧ
{
    u32  Dis_01km = 0;
    //  1. Head
    if(!Protocol_Head(MSG_0x0201, Packet_Normal))
        return false;
    // 2. content
    //------------------------------- Stuff ----------------------------------------
    //   float ID                                                // ��Ӧ����Ӧ����Ϣ����ˮ��
    Original_info[Original_info_Wr++] = (u8)(Centre_FloatID >> 8);
    Original_info[Original_info_Wr++] = (u8)Centre_FloatID;

    // 1. �澯��־  4
    memcpy( ( char * ) Original_info + Original_info_Wr, ( char * )Warn_Status, 4 );
    Original_info_Wr += 4;
    // 2. ״̬  4
    memcpy( ( char * ) Original_info + Original_info_Wr, ( char * )Car_Status, 4 );
    Original_info_Wr += 4;
    // 3.  γ��
    memcpy( ( char * ) Original_info + Original_info_Wr, ( char * )  Gps_Gprs.Latitude, 4 ); //γ��   modify by nathan
    Original_info_Wr += 4;
    // 4.  ����
    memcpy( ( char * ) Original_info + Original_info_Wr, ( char * )  Gps_Gprs.Longitude, 4 );	 //����    ����  Bit 7->0   ���� Bit 7 -> 1
    Original_info_Wr += 4;
    // 5.  �߳�
    Original_info[Original_info_Wr++] = (u8)(GPS_Hight << 8);
    Original_info[Original_info_Wr++] = (u8)GPS_Hight;
    // 6.  �ٶ�    0.1 Km/h
    Original_info[Original_info_Wr++] = (u8)(Speed_gps >> 8);
    Original_info[Original_info_Wr++] = (u8)Speed_gps;
    // 7. ����   ��λ 1��
    Original_info[Original_info_Wr++] = (GPS_direction >> 8); //High
    Original_info[Original_info_Wr++] = GPS_direction; // Low
    // 8.  ����ʱ��
    Original_info[Original_info_Wr++] = (((Gps_Gprs.Date[0]) / 10) << 4) + ((Gps_Gprs.Date[0]) % 10);
    Original_info[Original_info_Wr++] = ((Gps_Gprs.Date[1] / 10) << 4) + (Gps_Gprs.Date[1] % 10);
    Original_info[Original_info_Wr++] = ((Gps_Gprs.Date[2] / 10) << 4) + (Gps_Gprs.Date[2] % 10);
    Original_info[Original_info_Wr++] = ((Gps_Gprs.Time[0] / 10) << 4) + (Gps_Gprs.Time[0] % 10);
    Original_info[Original_info_Wr++] = ((Gps_Gprs.Time[1] / 10) << 4) + (Gps_Gprs.Time[1] % 10);
    Original_info[Original_info_Wr++] = ((Gps_Gprs.Time[2] / 10) << 4) + (Gps_Gprs.Time[2] % 10);


    //----------- ������Ϣ  ------------
    //  ������Ϣ 1  -----------------------------
    //  ������Ϣ ID
    //if(JT808Conf_struct.Speed_GetType==1) //ѡ�񴫸����ٶȲ��и��ֶ�
    {
        Original_info[Original_info_Wr++] = 0x03; // ��ʻ��¼�ǵ��ٶ�
        //  ������Ϣ����
        Original_info[Original_info_Wr++] = 2;
        //  ����
        Original_info[Original_info_Wr++] = (u8)(Spd_Using >> 8);
        Original_info[Original_info_Wr++] = (u8)(Spd_Using);
    }
    /*
     //  ������Ϣ 2  -----------------------------
     //  ������Ϣ ID
     Original_info[Original_info_Wr++]=0x01; // ���ϵ���ʻ���
     //  ������Ϣ����
     Original_info[Original_info_Wr++]=4;
     //  ����
     Dis_01km=JT808Conf_struct.Distance_m_u32/100;
     Original_info[Original_info_Wr++]=(Dis_01km>>24);
     Original_info[Original_info_Wr++]=(Dis_01km>>16);
     Original_info[Original_info_Wr++]=(Dis_01km>>8);
     Original_info[Original_info_Wr++]=Dis_01km;
    */

    //  ������Ϣ 3
    if(Warn_Status[1] & 0x10)
    {
        //  ������Ϣ ID
        Original_info[Original_info_Wr++] = 0x12; //  ��������/·�߱���
        //  ������Ϣ����
        Original_info[Original_info_Wr++] = 6;
        //  ����
        Original_info[Original_info_Wr++] = InOut_Object.TYPE;
        Original_info[Original_info_Wr++] = (InOut_Object.ID >> 24);
        Original_info[Original_info_Wr++] = (InOut_Object.ID >> 16);
        Original_info[Original_info_Wr++] = (InOut_Object.ID >> 8);
        Original_info[Original_info_Wr++] = InOut_Object.ID;
        Original_info[Original_info_Wr++] = InOut_Object.InOutState;
    }

    //  ������Ϣ4
    if(Warn_Status[3] & 0x02)
    {
        //  ������Ϣ ID
        Original_info[Original_info_Wr++] = 0x11; //  ��������/·�߱���
        //  ������Ϣ����
        Original_info[Original_info_Wr++] = 1;
        //  ����
        Original_info[Original_info_Wr++] = 0; //  ���ض�λ��

    }

    //---------- ������Ϣ 5 ----
    Original_info[Original_info_Wr++] = 0x25; //��չ�����ź�״̬
    //  ������Ϣ����
    Original_info[Original_info_Wr++] = 4;
    //  ����
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = BD_EXT.Extent_IO_status;
    //  ������Ϣ 5  -----------------------------
    //  ������Ϣ ID
    Original_info[Original_info_Wr++] = 0x30; //�ź�ǿ��
    //  ������Ϣ����
    Original_info[Original_info_Wr++] = 1;
    //  ����
    Original_info[Original_info_Wr++] = BD_EXT.FJ_SignalValue;

    //if(DispContent)
    // 	printf("\r\n---- Satelitenum: %d , CSQ:%d\r\n",Satelite_num,ModuleSQ);

    //	������Ϣ 6	-----------------------------
    //  ������Ϣ ID
    Original_info[Original_info_Wr++] = 0x2A; //�Զ���io
    //  ������Ϣ����
    Original_info[Original_info_Wr++] = 2;
    //  ����
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;

    //	������Ϣ 7 -----------------------------
    //  ������Ϣ ID
    Original_info[Original_info_Wr++] = 0x2B; //�Զ���ģ�����ϴ� AD
    //  ������Ϣ����
    Original_info[Original_info_Wr++] = 4;
    Original_info[Original_info_Wr++] = (BD_EXT.AD_0 >> 8);	 // ģ���� 1
    Original_info[Original_info_Wr++] = BD_EXT.AD_0;
    Original_info[Original_info_Wr++] = (BD_EXT.AD_1 >> 8);	 // ģ���� 2
    Original_info[Original_info_Wr++] = BD_EXT.AD_1;

    //  3. Send
    Protocol_End(Packet_Normal , 0);

    return true;

}

u8 Paramater_0106_stuff(u32 cmdid, u8 *deststr)
{
    u8  reg_str[30];
    u32  reg_u32 = 0;

    switch (cmdid)
    {
        //	�����б�
    case  MSG_0x0001:
        //  A.1 ���������
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x01;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.Heart_Dur >> 24); // ����ֵ
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.Heart_Dur >> 16);
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.Heart_Dur >> 8);
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.Heart_Dur);
        break;
    case  MSG_0x0002:
        //  A.2 TCP  ��ϢӦ����
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x02;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.TCP_ACK_Dur >> 24);	 // ����ֵ
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.TCP_ACK_Dur >> 16);
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.TCP_ACK_Dur >> 8);
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.TCP_ACK_Dur);
        break;
    case  MSG_0x0003:
        //  A.3 TCP	��Ϣ�ش�����
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x03;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.TCP_ReSD_Num >> 24); // ����ֵ
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.TCP_ReSD_Num >> 16);
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.TCP_ReSD_Num >> 8);
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.TCP_ReSD_Num);
        break;
    case  0x0004:
        //  A.4	UDP Ӧ��ʱ
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x04;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = (5 >> 24);	 // ����ֵ
        Original_info[Original_info_Wr++] = (5 >> 16);
        Original_info[Original_info_Wr++] = (5 >> 8);
        Original_info[Original_info_Wr++] = (5);
        break;
    case  0x0005:
        //  A.5	UDP �ش�����
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x05;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.UDP_ReSD_Num >> 24); // ����ֵ
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.UDP_ReSD_Num >> 16);
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.UDP_ReSD_Num >> 8);
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.UDP_ReSD_Num);
        break;
    case 0x0006:
        //  A.6	SMS��ϢӦ��ʱʱ��
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x06;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = 0; // ����ֵ
        Original_info[Original_info_Wr++] = 0;
        Original_info[Original_info_Wr++] = 0;
        Original_info[Original_info_Wr++] = 5;
        break;
    case  0x0007:

        //  A.7	SMS ��Ϣ�ش�����
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x07;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = 0; // ����ֵ
        Original_info[Original_info_Wr++] = 0;
        Original_info[Original_info_Wr++] = 0;
        Original_info[Original_info_Wr++] = 3;
        break;
    case  0x0010:
        //   A.8  APN �ַ���
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x10;
        Original_info[Original_info_Wr++] = strlen((const char *)APN_String); // ��������
        memcpy( ( char * ) Original_info + Original_info_Wr, ( char * )APN_String, strlen((const char *)APN_String)); // ����ֵ
        Original_info_Wr += strlen((const char *)APN_String);
        break;
    case 0x0011:
        //	A.9  APN �û���
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x11;
        Original_info[Original_info_Wr++] = 4; // ��������
        memcpy( ( char * ) Original_info + Original_info_Wr, "user", 4); // ����ֵ
        Original_info_Wr += 4;
        break;
    case 0x0012:
        //	A.10  APN ����
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x12;
        Original_info[Original_info_Wr++] = 4; // ��������
        memcpy( ( char * ) Original_info + Original_info_Wr, "user", 4); // ����ֵ
        Original_info_Wr += 4;
        break;
    case 0x0013:
        //   A.11	 ��������IP
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x13;
        // ��������

        memset(reg_str, 0, sizeof(reg_str));	 //  ��д����
        memcpy(reg_str, "jt1.gghypt.net", strlen((char const *)"jt1.gghypt.net"));
        // memcpy(reg_str,"fde.0132456.net",strlen((char const*)"jt1.gghypt.net"));
        Original_info[Original_info_Wr++] = strlen((const char *)reg_str);
        memcpy( ( char * ) Original_info + Original_info_Wr, ( char * )reg_str, strlen((const char *)reg_str));	// ����ֵ
        Original_info_Wr += strlen((const char *)reg_str);
        break;
    case 0x0014:
        //   A.12	 BAK  APN �ַ���
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x14;
        Original_info[Original_info_Wr++] = strlen((const char *)"UNINET"); // ��������
        memcpy( ( char * ) Original_info + Original_info_Wr, "UNINET", 6); // ����ֵ
        Original_info_Wr += 6;
        break;

    case 0x0015:
        //	A.13 BAK APN �û���
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x15;
        Original_info[Original_info_Wr++] = 4; // ��������
        memcpy( ( char * ) Original_info + Original_info_Wr, "user", 4); // ����ֵ
        Original_info_Wr += 4;
        break;
    case 0x0016:
        //	A.14  BAK  APN ����
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x16;
        Original_info[Original_info_Wr++] = 4; // ��������
        memcpy( ( char * ) Original_info + Original_info_Wr, "user", 4); // ����ֵ
        Original_info_Wr += 4;
        break;
    case 0x0017:
        //	A.15   ����IP
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x17;
        // ��������
        memset(reg_str, 0, sizeof(reg_str));	 //  ��д����
        memcpy(reg_str, DomainNameStr_aux, strlen((char const *)DomainNameStr_aux));
        // memcpy(reg_str,"fde.0132456.net",strlen((char const*)"jt1.gghypt.net"));
        Original_info[Original_info_Wr++] = strlen((const char *)reg_str);
        memcpy( ( char * ) Original_info + Original_info_Wr, ( char * )reg_str, strlen((const char *)reg_str));	// ����ֵ
        Original_info_Wr += strlen((const char *)reg_str);
        break;

    case 0x0018:

        //  A.16  ������TCP�˿�
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x18;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = 0x00; // ����ֵ
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = (7008 >> 8);
        Original_info[Original_info_Wr++] = (u8)(7008);
        break;
    case 0x0019:

        //	A.17  ���÷���TCP�˿�
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x19;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = 0x00; // ����ֵ
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = (7008 >> 8);
        Original_info[Original_info_Wr++] = (u8)(7008);
        break;

    case 0x001a:

        //  A.18  IC����֤ ����������ַ
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x1A;
        Original_info[Original_info_Wr++] = 0  ; // ��������

        //memcpy(( char * ) Original_info+ Original_info_Wr,"202.96.42.113",13);
        //Original_info_Wr+=13;
        break;
    case 0x001b:
        //	A.19  IC����֤ ��������TCP
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x1B;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = 0x00; // ����ֵ
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        break;
    case 0x001c:
        //	A.20  IC����֤ ��������UDP
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x1C;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = 0x00; // ����ֵ
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        break;
    case 0x001d:

        //  A.21  IC����֤  ���÷�������ַ
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x1D;
        Original_info[Original_info_Wr++] = 0  ; // ��������

        //memcpy(( char * ) Original_info+ Original_info_Wr,"202.96.42.114",13);
        //Original_info_Wr+=13;
        break;
    case 0x0020:

        //  A.22  λ�û㱨����
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x20;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = 0x00; // ����ֵ
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = JT808Conf_struct.SD_MODE.Send_strategy;
        break;
    case 0x0021:
        //  A.23  λ�û㱨����
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x21;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = 0x00; // ����ֵ
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = JT808Conf_struct.PositionSd_Stratage;
        break;
    case 0x0022:
        //  A.24 ��ʻԱδ��¼ �㱨���
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x22;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = 0x00; // ����ֵ
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 30;
        break;
    case 0x0027:

        //  A.25  ����ʱ�㱨���
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x27;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        JT808Conf_struct.DURATION.Sleep_Dur = 300;
        Original_info[Original_info_Wr++] = (300 >> 24); // ����ֵ
        Original_info[Original_info_Wr++] = (300 >> 16);
        Original_info[Original_info_Wr++] = (300 >> 8);
        Original_info[Original_info_Wr++] = (u8)(300);
        break;
    case 0x0028:
        //	A.26 ��������ʱ
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x28;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = 0x00; // ����ֵ
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 10;
        break;
    case 0x0029:
        //   A.27	  ȱʡʱ���ϱ����
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x29;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.Default_Dur >> 24);	 // ����ֵ
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.Default_Dur >> 16);
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.Default_Dur >> 8);
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.Default_Dur);
        break;
    case 0x002c:
        //   A.28	  ȱʡ�����ϱ����
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x2c;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        //JT808Conf_struct.DISTANCE.Defalut_DistDelta=500;
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DISTANCE.Defalut_DistDelta >> 24); // ����ֵ
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DISTANCE.Defalut_DistDelta >> 16);
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DISTANCE.Defalut_DistDelta >> 8);
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DISTANCE.Defalut_DistDelta);
        break;
    case 0x002d:
        //   A.29	  ��ʻԱδ��¼���
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x2d;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DISTANCE.Defalut_DistDelta >> 24); // ����ֵ
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DISTANCE.Defalut_DistDelta >> 16);
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DISTANCE.Defalut_DistDelta >> 8);
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DISTANCE.Defalut_DistDelta);
        break;
    case 0x002e:

        //   A.30	����ʱ�㱨������
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x2e;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DISTANCE.Defalut_DistDelta >> 24); // ����ֵ
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DISTANCE.Defalut_DistDelta >> 16);
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DISTANCE.Defalut_DistDelta >> 8);
        Original_info[Original_info_Wr++] = (JT808Conf_struct.DISTANCE.Defalut_DistDelta);
        break;
    case 0x002f:
        //   A.31	  ��������ʱ ������
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x2f;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = (200 >> 24); // ����ֵ
        Original_info[Original_info_Wr++] = (200 >> 16);
        Original_info[Original_info_Wr++] = (200 >> 8);
        Original_info[Original_info_Wr++] = (200);
        break;
    case 0x0030:
        //   A.32 	�յ㲹���Ƕ�
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x30;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = (45 >> 24); // ����ֵ
        Original_info[Original_info_Wr++] = (45 >> 16);
        Original_info[Original_info_Wr++] = (45 >> 8);
        Original_info[Original_info_Wr++] = (45);
        break;
    case 0x0031:
        //	 A.33	  ����Χ���뾶
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x31;
        Original_info[Original_info_Wr++] = 2  ; // ��������
        // ����ֵ
        Original_info[Original_info_Wr++] = (500 >> 8);
        Original_info[Original_info_Wr++] = (u8)(500);
        break;
    case 0x0040:
        //   A.34	  ������������
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x40;
        Original_info[Original_info_Wr++] = 0  ; // ��������
        break;
    case 0x0041:
        //	 A.35	 ��λ�绰����
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x41;
        Original_info[Original_info_Wr++] = 0  ; // ��������
        break;

    case 0x0042:
        //   A.36	  �ָ��������õ绰����
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x42;
        Original_info[Original_info_Wr++] = 0  ; // ��������
        break;
    case 0x0043:
        //  A37	���ƽ̨SMS ��Ϣ����
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x43;
        Original_info[Original_info_Wr++] = 5  ; // ��������


        memcpy(( char * ) Original_info + Original_info_Wr, JT808Conf_struct.SMS_RXNum, strlen((const char *)JT808Conf_struct.SMS_RXNum));
        Original_info_Wr += strlen(JT808Conf_struct.SMS_RXNum);
        break;
    case 0x0044:

        //   A.38	  sms ��Ϣ��������
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x44;
        Original_info[Original_info_Wr++] = 0  ; // ��������
        break;
    case 0x0045:
        //	 A.39	��������
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x45;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 1;	// ACC on �Զ�����
        break;
    case 0x0046:

        //	A.40	 ÿ��ͨ��ʱ��
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x46;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = (120 >> 24); // ����ֵ
        Original_info[Original_info_Wr++] = (120 >> 16);
        Original_info[Original_info_Wr++] = (120 >> 8);
        Original_info[Original_info_Wr++] = (120);
        break;
    case 0x0047:

        //   A.41    ÿ��ͨ��ʱ��
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x47;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = (36000 >> 24);	 // ����ֵ
        Original_info[Original_info_Wr++] = (36000 >> 16);
        Original_info[Original_info_Wr++] = (36000 >> 8);
        Original_info[Original_info_Wr++] = (36000);
        break;
    case 0x0048:

        //   A42	 ���ü�������
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x48;
        Original_info[Original_info_Wr++] = 0  ; // ��������
        break;
    case 0x0049:
        //   A.43    ƽ̨�����Ȩ����
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x49;
        Original_info[Original_info_Wr++] = 0  ; // ��������
        break;
    case 0x0050:

        //	 A.44	 ����������
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x50;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = Warn_MaskWord[0];
        Original_info[Original_info_Wr++] = Warn_MaskWord[1];
        Original_info[Original_info_Wr++] = Warn_MaskWord[2];
        Original_info[Original_info_Wr++] = Warn_MaskWord[3];
        break;
    case 0x0051:

        //   A.45  ����	����Sms  ����
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x51;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = Text_MaskWord[0];
        Original_info[Original_info_Wr++] = Text_MaskWord[1];
        Original_info[Original_info_Wr++] = Text_MaskWord[2];
        Original_info[Original_info_Wr++] = Text_MaskWord[3];
        break;
    case 0x0052:

        //   A.46	  �������տ���
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x52;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x01;
        break;
    case 0x0053:

        //   A.47	�������մ洢��־
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x53;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        break;
    case 0x0054:

        //   A.48	  �ؼ�������־
        Original_info[Original_info_Wr++] = 0x00;	 // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x54;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = Key_MaskWord[0];
        Original_info[Original_info_Wr++] = Key_MaskWord[1];
        Original_info[Original_info_Wr++] = Key_MaskWord[2];
        Original_info[Original_info_Wr++] = Key_MaskWord[3];
        break;
    case 0x0055:

        //	A.49   ����ٶ�����
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x55;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        JT808Conf_struct.Speed_warn_MAX = 100;
        Original_info[Original_info_Wr++] = ( JT808Conf_struct.Speed_warn_MAX >> 24);	// ����ֵ
        Original_info[Original_info_Wr++] = ( JT808Conf_struct.Speed_warn_MAX >> 16);
        Original_info[Original_info_Wr++] = ( JT808Conf_struct.Speed_warn_MAX >> 8);
        Original_info[Original_info_Wr++] = ( JT808Conf_struct.Speed_warn_MAX);
        break;
    case 0x0056:

        //	A.50	 ���ٳ���ʱ��
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x56;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = JT808Conf_struct.Spd_Exd_LimitSeconds;
        break;
    case 0x0057:

        //  A.51	������ʻ����
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x57;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_DrvKeepingSec >> 24);	 // ����ֵ
        Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_DrvKeepingSec >> 16);
        Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_DrvKeepingSec >> 8);
        Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_DrvKeepingSec);
        break;
    case 0x0058:

        //  A.52  �����ۼƼ�ʻ����
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x58;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_DayAccumlateDrvSec >> 24);	 // ����ֵ
        Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_DayAccumlateDrvSec >> 16);
        Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_DayAccumlateDrvSec >> 8);
        Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_DayAccumlateDrvSec);
        break;
    case 0x0059:

        //   A.53	 ��С��Ϣʱ��
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x59;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_MinSleepSec >> 24);	 // ����ֵ
        Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_MinSleepSec >> 16);
        Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_MinSleepSec >> 8);
        Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_MinSleepSec);
        break;
    case 0x005A:

        //   A.54	�ͣ��ʱ��
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x5A;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        TiredConf_struct.TiredDoor.Door_MaxParkingSec = 3600;
        Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_MaxParkingSec >> 24); // ����ֵ
        Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_MaxParkingSec >> 16);
        Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_MaxParkingSec >> 8);
        Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_MaxParkingSec);
        break;
    case 0x005B:

        //   A.55  ���ٱ���Ԥ����ֵ
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x5B;
        Original_info[Original_info_Wr++] = 2  ; // ��������
        // ����ֵ
        Original_info[Original_info_Wr++] = (JT808Conf_struct.BD_MaxSpd_preWarnValue >> 8); // 100
        Original_info[Original_info_Wr++] = (JT808Conf_struct.BD_MaxSpd_preWarnValue);
        break;
    case 0x005C:


        //	A.56  ƣ�ͼ�ʻԤ����ֵ
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x5C;
        Original_info[Original_info_Wr++] = 2  ; // ��������
        // ����ֵ
        Original_info[Original_info_Wr++] = (JT808Conf_struct.BD_TiredDrv_preWarnValue >> 8);
        Original_info[Original_info_Wr++] = (JT808Conf_struct.BD_TiredDrv_preWarnValue);
        break;
    case 0x005D:

        //	 A.57  ��ײ������������
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x5D;
        Original_info[Original_info_Wr++] = 2  ; // ��������
        // ����ֵ
        Original_info[Original_info_Wr++] = (17924 >> 8);
        Original_info[Original_info_Wr++] = (17924);
        break;
    case 0x005E:
        //	 A.58  �෭������������
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x5E;
        Original_info[Original_info_Wr++] = 2  ; // ��������
        // ����ֵ
        Original_info[Original_info_Wr++] = (30 >> 8);
        Original_info[Original_info_Wr++] = (30);
        break;
    case 0x0064:
        //   A.59	 ��ʱ���տ���
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x64;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        break;
    case 0x0065:

        //   A.60	�������տ���
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x65;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        break;
    case 0x0070:

        //   A.61 ͼ����Ƶ����
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x70;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x05;
        break;
    case 0x0071:

        //   A.62	����
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x71;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 127;
        break;
    case 0x0072:

        //   A.63	 �Աȶ�
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x72;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 64;
        break;
    case 0x0073:


        //   A.64	���Ͷ�
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x73;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 64;
        break;
    case 0x0074:


        //   A.65	ɫ��
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x74;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 127;
        break;
    case 0x0080:

        //   A.66	 ������̱����
        Original_info[Original_info_Wr++] = 0x00;	 // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x64;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        reg_u32 = JT808Conf_struct.Distance_m_u32 / 100;
        Original_info[Original_info_Wr++] = (reg_u32 >> 24);	 // ����ֵ
        Original_info[Original_info_Wr++] = (reg_u32 >> 16);
        Original_info[Original_info_Wr++] = (reg_u32 >> 8);
        Original_info[Original_info_Wr++] = (reg_u32);
        break;
    case 0x0081:



        //   A.67	 ��������ʡ��
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x81;
        Original_info[Original_info_Wr++] = 2  ; // ��������
        Original_info[Original_info_Wr++] = (u8)(10 >> 8);
        Original_info[Original_info_Wr++] = (u8)10;
        break;
    case 0x0082:

        //	 A.68	������������
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x82;
        Original_info[Original_info_Wr++] = 2  ; // ������?
        //  county  ID
        Original_info[Original_info_Wr++] = (u8)(1010 >> 8);
        Original_info[Original_info_Wr++] = (u8)1010;
        break;
    case 0x0083:


        //	A.69   ���ƺ�
        Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x83;
        Original_info[Original_info_Wr++] = strlen((const char *)Vechicle_Info.Vech_Num); // ��������
        memcpy( ( char * ) Original_info + Original_info_Wr, ( char * )Vechicle_Info.Vech_Num, strlen((const char *)Vechicle_Info.Vech_Num) ); // ����ֵ
        Original_info_Wr += strlen((const char *)Vechicle_Info.Vech_Num);
        break;
    case 0x0084:

        //	 A.70	������ɫ
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x84;
        Original_info[Original_info_Wr++] = 1  ; // ������?
        Original_info[Original_info_Wr++] = Vechicle_Info.Dev_Color;
        break;
    case 0x0090:

        //	 A.71	GNSS  ģʽ����
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x90;
        Original_info[Original_info_Wr++] = 1  ; // ������?

        if( GpsStatus.Position_Moule_Status == 1)
            Original_info[Original_info_Wr++] = 2;

        if( GpsStatus.Position_Moule_Status == 2)
            Original_info[Original_info_Wr++] = 1;

        if( GpsStatus.Position_Moule_Status == 3)
            Original_info[Original_info_Wr++] = 3;
        break;
    case 0x0091:

        //	 A.72	GNSS ����������
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x91;
        Original_info[Original_info_Wr++] = 1  ; // ������?
        Original_info[Original_info_Wr++] = 2;
        break;
    case 0x0092:

        //	 A.73	GNSS nmea  �������Ƶ��
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x92;
        Original_info[Original_info_Wr++] = 1  ; // ������?
        Original_info[Original_info_Wr++] = 1;
        break;
    case 0x0093:

        //   A74	������̱����
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x93;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 1;
        break;
    case 0x0094:

        //	 A.75  GNSS��λ�ϴ���ʽ
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x94;
        Original_info[Original_info_Wr++] = 1  ; // ������?
        Original_info[Original_info_Wr++] = 0;
        break;
    case 0x0095:

        //   A76	ģ����ϸ�����ϴ�����
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x95;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0;
        break;
    case 0x0100:

        //   A77	CAN1   �ɼ����
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x01;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        break;
    case 0x0101:

        //   A78	CAN1   �ϴ����
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x01;
        Original_info[Original_info_Wr++] = 0x01;
        Original_info[Original_info_Wr++] = 2  ; // ��������
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        break;
    case 0x0102:

        //   A79	 CAN2	�ɼ����
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x01;
        Original_info[Original_info_Wr++] = 0x02;
        Original_info[Original_info_Wr++] = 4  ; // ��������
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0;
        break;
    case 0x0103:

        //   A80  CAN2   �ϴ����
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x01;
        Original_info[Original_info_Wr++] = 0x03;
        Original_info[Original_info_Wr++] = 2  ; // ��������
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        break;
    case 0x0110:


        //   A81  CAN	����ID �����ɼ�����
        Original_info[Original_info_Wr++] = 0x00;	// ����ID 4Bytes
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x01;
        Original_info[Original_info_Wr++] = 0x10;
        Original_info[Original_info_Wr++] = 8 ; // ��������
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0x00;
        Original_info[Original_info_Wr++] = 0;
        break;
    default:
        break;
    }
    return true;
}

//-----------------------------------------------------------------------
u8  Sub_stuff_AppointedPram_0106(void)
{
    // 7E81060005013901234505004F0100000021B77E
    u16  i = 0, len = 0;


    //  1. Head
    if(!Protocol_Head(MSG_0x0104, Packet_Normal))
        return false; // �ն˲����ϴ�

    //   float ID
    Original_info[Original_info_Wr++] = (u8)(Centre_FloatID >> 8);
    Original_info[Original_info_Wr++] = (u8)Centre_FloatID;
    //   ��������
    Original_info[Original_info_Wr++] = Setting_Qry.Num_pram;

    for(i = 0; i < Setting_Qry.Num_pram; i++)
    {
        len = Paramater_0106_stuff(Setting_Qry.List_pram[i], Original_info); //    ��д��Ϣ
        rt_kprintf("\r\n SendID:  %X  i=%d ", Setting_Qry.List_pram[i], i);
        //  Original_info_Wr+=len;
    }

    // Paramater_0106_stuff(CMD_U32ID,Original_info);	//	  ��д��Ϣ
    // rt_kprintf("\r\n SendID:  %4X ",CMD_U32ID);

    //  3. Send
    Protocol_End(Packet_Normal , 0);

    return true;

}
u8  Stuff_SettingPram_0104H(void)
{
    u8  reg_str[30];
    u32  reg_u32 = 0;

    //  1. Head
    if(!Protocol_Head(MSG_0x0104, Packet_Normal))
        return false; // �ն˲����ϴ�

    //  2. content
    //   float ID
    Original_info[Original_info_Wr++] = (u8)(Centre_FloatID >> 8);
    Original_info[Original_info_Wr++] = (u8)Centre_FloatID;
    //   ��������
    Original_info[Original_info_Wr++] = 81;
    //   �����б�
    //  A.1 ���������
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x01;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.Heart_Dur >> 24); // ����ֵ
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.Heart_Dur >> 16);
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.Heart_Dur >> 8);
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.Heart_Dur);
    //  A.2 TCP  ��ϢӦ����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x02;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.TCP_ACK_Dur >> 24); // ����ֵ
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.TCP_ACK_Dur >> 16);
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.TCP_ACK_Dur >> 8);
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.TCP_ACK_Dur);

    //  A.3 TCP  ��Ϣ�ش�����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x03;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.TCP_ReSD_Num >> 24); // ����ֵ
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.TCP_ReSD_Num >> 16);
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.TCP_ReSD_Num >> 8);
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.TCP_ReSD_Num);

    //  A.4   UDP Ӧ��ʱ
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x04;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = (5 >> 24); // ����ֵ
    Original_info[Original_info_Wr++] = (5 >> 16);
    Original_info[Original_info_Wr++] = (5 >> 8);
    Original_info[Original_info_Wr++] = (5);


    //  A.5   UDP �ش�����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x05;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.UDP_ReSD_Num >> 24); // ����ֵ
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.UDP_ReSD_Num >> 16);
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.UDP_ReSD_Num >> 8);
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.UDP_ReSD_Num);


    //  A.6   SMS��ϢӦ��ʱʱ��
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x06;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = 0; // ����ֵ
    Original_info[Original_info_Wr++] = 0;
    Original_info[Original_info_Wr++] = 0;
    Original_info[Original_info_Wr++] = 5;



    //  A.7   SMS ��Ϣ�ش�����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x07;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = 0; // ����ֵ
    Original_info[Original_info_Wr++] = 0;
    Original_info[Original_info_Wr++] = 0;
    Original_info[Original_info_Wr++] = 3;

    //   A.8  APN �ַ���
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x10;
    Original_info[Original_info_Wr++] = strlen((const char *)APN_String); // ��������
    memcpy( ( char * ) Original_info + Original_info_Wr, ( char * )APN_String, strlen((const char *)APN_String)); // ����ֵ
    Original_info_Wr += strlen((const char *)APN_String);

    //   A.9  APN �û���
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x11;
    Original_info[Original_info_Wr++] = 4; // ��������
    memcpy( ( char * ) Original_info + Original_info_Wr, "user", 4); // ����ֵ
    Original_info_Wr += 4;

    //   A.10  APN ����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x12;
    Original_info[Original_info_Wr++] = 4; // ��������
    memcpy( ( char * ) Original_info + Original_info_Wr, "user", 4); // ����ֵ
    Original_info_Wr += 4;

    //   A.11   ��������IP
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x13;
    // ��������

    memset(reg_str, 0, sizeof(reg_str));   //  ��д����
    memcpy(reg_str, "jt1.gghypt.net", strlen((char const *)"jt1.gghypt.net"));
    // memcpy(reg_str,"fde.0132456.net",strlen((char const*)"jt1.gghypt.net"));
    Original_info[Original_info_Wr++] = strlen((const char *)reg_str);
    memcpy( ( char * ) Original_info + Original_info_Wr, ( char * )reg_str, strlen((const char *)reg_str)); // ����ֵ
    Original_info_Wr += strlen((const char *)reg_str);

    //   A.12   BAK  APN �ַ���
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x14;
    Original_info[Original_info_Wr++] = strlen((const char *)"UNINET"); // ��������
    memcpy( ( char * ) Original_info + Original_info_Wr, "UNINET", 6); // ����ֵ
    Original_info_Wr += 6;

    //   A.13 BAK APN �û���
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x15;
    Original_info[Original_info_Wr++] = 4; // ��������
    memcpy( ( char * ) Original_info + Original_info_Wr, "user", 4); // ����ֵ
    Original_info_Wr += 4;

    //   A.14  BAK  APN ����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x16;
    Original_info[Original_info_Wr++] = 4; // ��������
    memcpy( ( char * ) Original_info + Original_info_Wr, "user", 4); // ����ֵ
    Original_info_Wr += 4;

    //  A.15   ����IP
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x17;
    // ��������
    memset(reg_str, 0, sizeof(reg_str));   //  ��д����
    memcpy(reg_str, DomainNameStr_aux, strlen((char const *)DomainNameStr_aux));
    // memcpy(reg_str,"fde.0132456.net",strlen((char const*)"jt1.gghypt.net"));
    Original_info[Original_info_Wr++] = strlen((const char *)reg_str);
    memcpy( ( char * ) Original_info + Original_info_Wr, ( char * )reg_str, strlen((const char *)reg_str)); // ����ֵ
    Original_info_Wr += strlen((const char *)reg_str);




    //  A.16  ������TCP�˿�
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x18;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = 0x00; // ����ֵ
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = (7008 >> 8);
    Original_info[Original_info_Wr++] = (u8)7008;

    //  A.17  ���÷���TCP�˿�
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x19;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = 0x00; // ����ֵ
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = (7008 >> 8);
    Original_info[Original_info_Wr++] = (u8)(7008);

    //  A.18  IC����֤ ����������ַ
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x1A;
    Original_info[Original_info_Wr++] = 0  ; // ��������

    //memcpy(( char * ) Original_info+ Original_info_Wr,"202.96.42.113",13);
    //Original_info_Wr+=13;

    //  A.19  IC����֤ ��������TCP
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x1B;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = 0x00; // ����ֵ
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;

    //  A.20  IC����֤ ��������UDP
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x1C;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = 0x00; // ����ֵ
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;

    //  A.21  IC����֤  ���÷�������ַ
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x1D;
    Original_info[Original_info_Wr++] = 0  ; // ��������

    //memcpy(( char * ) Original_info+ Original_info_Wr,"202.96.42.114",13);
    //Original_info_Wr+=13;


    //  A.22  λ�û㱨����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x20;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = 0x00; // ����ֵ
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = JT808Conf_struct.SD_MODE.Send_strategy;

    //  A.23  λ�û㱨����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x21;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = 0x00; // ����ֵ
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = JT808Conf_struct.PositionSd_Stratage;

    //  A.24 ��ʻԱδ��¼ �㱨���
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x22;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = 0x00; // ����ֵ
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 30;

    //  A.25  ����ʱ�㱨���
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x27;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    JT808Conf_struct.DURATION.Sleep_Dur = 300;
    Original_info[Original_info_Wr++] = (300 >> 24); // ����ֵ
    Original_info[Original_info_Wr++] = (300 >> 16);
    Original_info[Original_info_Wr++] = (300 >> 8);
    Original_info[Original_info_Wr++] = (u8)(300);

    //  A.26 ��������ʱ
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x28;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = 0x00; // ����ֵ
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 10;


    //   A.27    ȱʡʱ���ϱ����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x29;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.Default_Dur >> 24); // ����ֵ
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.Default_Dur >> 16);
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.Default_Dur >> 8);
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DURATION.Default_Dur);

    //   A.28    ȱʡ�����ϱ����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x2c;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    //JT808Conf_struct.DISTANCE.Defalut_DistDelta=500;
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DISTANCE.Defalut_DistDelta >> 24); // ����ֵ
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DISTANCE.Defalut_DistDelta >> 16);
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DISTANCE.Defalut_DistDelta >> 8);
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DISTANCE.Defalut_DistDelta);


    //   A.29    ��ʻԱδ��¼���
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x2d;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DISTANCE.Defalut_DistDelta >> 24); // ����ֵ
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DISTANCE.Defalut_DistDelta >> 16);
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DISTANCE.Defalut_DistDelta >> 8);
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DISTANCE.Defalut_DistDelta);


    //   A.30  ����ʱ�㱨������
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x2e;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DISTANCE.Defalut_DistDelta >> 24); // ����ֵ
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DISTANCE.Defalut_DistDelta >> 16);
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DISTANCE.Defalut_DistDelta >> 8);
    Original_info[Original_info_Wr++] = (JT808Conf_struct.DISTANCE.Defalut_DistDelta);

    //   A.31    ��������ʱ ������
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x2f;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = (200 >> 24); // ����ֵ
    Original_info[Original_info_Wr++] = (200 >> 16);
    Original_info[Original_info_Wr++] = (200 >> 8);
    Original_info[Original_info_Wr++] = (200);

    //   A.32     �յ㲹���Ƕ�
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x30;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = (45 >> 24); // ����ֵ
    Original_info[Original_info_Wr++] = (45 >> 16);
    Original_info[Original_info_Wr++] = (45 >> 8);
    Original_info[Original_info_Wr++] = (45);


    //   A.33     ����Χ���뾶
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x31;
    Original_info[Original_info_Wr++] = 2  ; // ��������
    // ����ֵ
    Original_info[Original_info_Wr++] = (500 >> 8);
    Original_info[Original_info_Wr++] = (u8)(500);


    //   A.34    ������������
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x40;
    Original_info[Original_info_Wr++] = 0  ; // ��������

    //   A.35    ��λ�绰����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x41;
    Original_info[Original_info_Wr++] = 0  ; // ��������


    //   A.36    �ָ��������õ绰����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x42;
    Original_info[Original_info_Wr++] = 0  ; // ��������


    //  A37  ���ƽ̨SMS ��Ϣ����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x43;
    Original_info[Original_info_Wr++] = 5  ; // ��������

    memcpy(( char * ) Original_info + Original_info_Wr, JT808Conf_struct.SMS_RXNum, strlen((const char *)JT808Conf_struct.SMS_RXNum));
    Original_info_Wr += strlen((const char *)JT808Conf_struct.SMS_RXNum);



    //   A.38    sms ��Ϣ��������
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x44;
    Original_info[Original_info_Wr++] = 0  ; // ��������


    //   A.39   ��������
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x45;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 1; // ACC on �Զ�����



    //   A.40     ÿ��ͨ��ʱ��
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x46;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = (120 >> 24); // ����ֵ
    Original_info[Original_info_Wr++] = (120 >> 16);
    Original_info[Original_info_Wr++] = (120 >> 8);
    Original_info[Original_info_Wr++] = (120);


    //   A.41    ÿ��ͨ��ʱ��
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x47;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = (36000 >> 24); // ����ֵ
    Original_info[Original_info_Wr++] = (36000 >> 16);
    Original_info[Original_info_Wr++] = (36000 >> 8);
    Original_info[Original_info_Wr++] = (36000);


    //   A42    ���ü�������
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x48;
    Original_info[Original_info_Wr++] = 0  ; // ��������


    //   A.43    ƽ̨�����Ȩ����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x49;
    Original_info[Original_info_Wr++] = 0  ; // ��������


    //   A.44    ����������
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x50;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = Warn_MaskWord[0];
    Original_info[Original_info_Wr++] = Warn_MaskWord[1];
    Original_info[Original_info_Wr++] = Warn_MaskWord[2];
    Original_info[Original_info_Wr++] = Warn_MaskWord[3];


    //   A.45  ����   ����Sms  ����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x51;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = Text_MaskWord[0];
    Original_info[Original_info_Wr++] = Text_MaskWord[1];
    Original_info[Original_info_Wr++] = Text_MaskWord[2];
    Original_info[Original_info_Wr++] = Text_MaskWord[3];

    //   A.46    �������տ���
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x52;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x01;

    //   A.47  �������մ洢��־
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x53;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;

    //   A.48    �ؼ�������־
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x54;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = Key_MaskWord[0];
    Original_info[Original_info_Wr++] = Key_MaskWord[1];
    Original_info[Original_info_Wr++] = Key_MaskWord[2];
    Original_info[Original_info_Wr++] = Key_MaskWord[3];

    //   A.49   ����ٶ�����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x55;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    JT808Conf_struct.Speed_warn_MAX = 100;
    Original_info[Original_info_Wr++] = ( JT808Conf_struct.Speed_warn_MAX >> 24); // ����ֵ
    Original_info[Original_info_Wr++] = ( JT808Conf_struct.Speed_warn_MAX >> 16);
    Original_info[Original_info_Wr++] = ( JT808Conf_struct.Speed_warn_MAX >> 8);
    Original_info[Original_info_Wr++] = ( JT808Conf_struct.Speed_warn_MAX);

    //   A.50     ���ٳ���ʱ��
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x56;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = JT808Conf_struct.Spd_Exd_LimitSeconds;

    //  A.51   ������ʻ����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x57;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_DrvKeepingSec >> 24); // ����ֵ
    Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_DrvKeepingSec >> 16);
    Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_DrvKeepingSec >> 8);
    Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_DrvKeepingSec);

    //  A.52  �����ۼƼ�ʻ����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x58;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_DayAccumlateDrvSec >> 24); // ����ֵ
    Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_DayAccumlateDrvSec >> 16);
    Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_DayAccumlateDrvSec >> 8);
    Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_DayAccumlateDrvSec);



    //   A.53   ��С��Ϣʱ��
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x59;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_MinSleepSec >> 24);	 // ����ֵ
    Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_MinSleepSec >> 16);
    Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_MinSleepSec >> 8);
    Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_MinSleepSec);

    //   A.54  �ͣ��ʱ��
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x5A;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    TiredConf_struct.TiredDoor.Door_MaxParkingSec = 3600;
    Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_MaxParkingSec >> 24);	 // ����ֵ
    Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_MaxParkingSec >> 16);
    Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_MaxParkingSec >> 8);
    Original_info[Original_info_Wr++] = (TiredConf_struct.TiredDoor.Door_MaxParkingSec);

    //   A.55  ���ٱ���Ԥ����ֵ
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x5B;
    Original_info[Original_info_Wr++] = 2  ; // ��������
    // ����ֵ
    Original_info[Original_info_Wr++] = (JT808Conf_struct.BD_MaxSpd_preWarnValue >> 8); // 100
    Original_info[Original_info_Wr++] = (JT808Conf_struct.BD_MaxSpd_preWarnValue);


    //   A.56  ƣ�ͼ�ʻԤ����ֵ
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x5C;
    Original_info[Original_info_Wr++] = 2  ; // ��������
    // ����ֵ
    Original_info[Original_info_Wr++] = (JT808Conf_struct.BD_TiredDrv_preWarnValue >> 8);
    Original_info[Original_info_Wr++] = (JT808Conf_struct.BD_TiredDrv_preWarnValue);



    //   A.57  ��ײ������������
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x5D;
    Original_info[Original_info_Wr++] = 2  ; // ��������
    // ����ֵ
    Original_info[Original_info_Wr++] = (17924 >> 8);
    Original_info[Original_info_Wr++] = (17924);



    //   A.58  �෭������������
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x5E;
    Original_info[Original_info_Wr++] = 2  ; // ��������
    // ����ֵ
    Original_info[Original_info_Wr++] = (30 >> 8);
    Original_info[Original_info_Wr++] = (30);


    //   A.59   ��ʱ���տ���
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x80;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    reg_u32 = JT808Conf_struct.Distance_m_u32 / 100;
    Original_info[Original_info_Wr++] = (reg_u32 >> 24);	// ����ֵ
    Original_info[Original_info_Wr++] = (reg_u32 >> 16);
    Original_info[Original_info_Wr++] = (reg_u32 >> 8);
    Original_info[Original_info_Wr++] = (reg_u32);

    //   A.60  �������տ���
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x65;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;

    //   A.61 ͼ����Ƶ����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x70;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x05;

    //   A.62  ����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x71;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 127;

    //   A.63   �Աȶ�
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x72;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 64;


    //   A.64  ���Ͷ�
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x73;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 64;


    //   A.65  ɫ��
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x74;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 127;

    //   A.66   ������̱����
    Original_info[Original_info_Wr++] = 0x00;	 // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x64;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    reg_u32 = JT808Conf_struct.Distance_m_u32 / 100;
    Original_info[Original_info_Wr++] = (reg_u32 >> 24);	 // ����ֵ
    Original_info[Original_info_Wr++] = (reg_u32 >> 16);
    Original_info[Original_info_Wr++] = (reg_u32 >> 8);
    Original_info[Original_info_Wr++] = (reg_u32);



    //   A.67   ��������ʡ��
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x81;
    Original_info[Original_info_Wr++] = 2  ; // ��������
    Original_info[Original_info_Wr++] = (u8)(10 >> 8);
    Original_info[Original_info_Wr++] = (u8)10;

    //   A.68   ������������
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x82;
    Original_info[Original_info_Wr++] = 2  ; // ������?
    //  county  ID
    Original_info[Original_info_Wr++] = (u8)(1010 >> 8);
    Original_info[Original_info_Wr++] = (u8)1010;


    //   A.69   ���ƺ�
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x83;
    Original_info[Original_info_Wr++] = strlen((const char *)Vechicle_Info.Vech_Num); // ��������
    memcpy( ( char * ) Original_info + Original_info_Wr, ( char * )Vechicle_Info.Vech_Num, strlen((const char *)Vechicle_Info.Vech_Num) ); // ����ֵ
    Original_info_Wr += strlen((const char *)Vechicle_Info.Vech_Num);

    //   A.70   ������ɫ
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x84;
    Original_info[Original_info_Wr++] = 1  ; // ������?
    Original_info[Original_info_Wr++] = Vechicle_Info.Dev_Color;

    //   A.71   GNSS  ģʽ����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x90;
    Original_info[Original_info_Wr++] = 1  ; // ������?

    if( GpsStatus.Position_Moule_Status == 1)
        Original_info[Original_info_Wr++] = 2;

    if( GpsStatus.Position_Moule_Status == 2)
        Original_info[Original_info_Wr++] = 1;

    if( GpsStatus.Position_Moule_Status == 3)
        Original_info[Original_info_Wr++] = 3;

    //   A.72   GNSS ����������
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x91;
    Original_info[Original_info_Wr++] = 1  ; // ������?
    Original_info[Original_info_Wr++] = 2;

    //   A.73   GNSS nmea  �������Ƶ��
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x92;
    Original_info[Original_info_Wr++] = 1  ; // ������?
    Original_info[Original_info_Wr++] = 1;

    //   A74   ������̱����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x93;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 1;

    //   A.75  GNSS��λ�ϴ���ʽ
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x94;
    Original_info[Original_info_Wr++] = 1  ; // ������?
    Original_info[Original_info_Wr++] = 0;

    //   A76   ģ����ϸ�����ϴ�����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x95;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0;

    //   A77   CAN1   �ɼ����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x01;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;

    //   A78   CAN1   �ϴ����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x01;
    Original_info[Original_info_Wr++] = 0x01;
    Original_info[Original_info_Wr++] = 2  ; // ��������
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;

    //   A79    CAN2   �ɼ����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x01;
    Original_info[Original_info_Wr++] = 0x02;
    Original_info[Original_info_Wr++] = 4  ; // ��������
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0;

    //   A80  CAN2   �ϴ����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x01;
    Original_info[Original_info_Wr++] = 0x03;
    Original_info[Original_info_Wr++] = 2  ; // ��������
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;



    //   A81  CAN  ����ID �����ɼ�����
    Original_info[Original_info_Wr++] = 0x00; // ����ID 4Bytes
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x01;
    Original_info[Original_info_Wr++] = 0x10;
    Original_info[Original_info_Wr++] = 8 ; // ��������
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0x00;
    Original_info[Original_info_Wr++] = 0;
    //  3. Send
    Protocol_End(Packet_Normal , 0);

    return true;
}


u8  Stuff_DeviceAttribute_BD_0107H(void)
{
    //   u16   infoLen=0;
    // 1. Head
    if(!Protocol_Head(MSG_0x0107, Packet_Normal))
        return false;
    // 2. content
    Original_info[Original_info_Wr++] = (ProductAttribute._1_DevType >> 8);
    Original_info[Original_info_Wr++] = ProductAttribute._1_DevType;
    memcpy( ( char * ) Original_info + Original_info_Wr, (u8 *)"70420", 5);
    Original_info_Wr += 5;
    memcpy( ( char * ) Original_info + Original_info_Wr, (u8 *)ProductAttribute._3_Dev_TYPENUM, 20);
    Original_info_Wr += 20;
    memcpy( ( char * ) Original_info + Original_info_Wr, (u8 *)DeviceNumberID + 5, 7);
    Original_info_Wr += 7;
    memcpy( ( char * ) Original_info + Original_info_Wr, (u8 *)ProductAttribute._5_Sim_ICCID, 10);
    Original_info_Wr += 10;

    Original_info[Original_info_Wr++] = ProductAttribute._6_HardwareVer_Len;
    memcpy( ( char * ) Original_info + Original_info_Wr, (u8 *)ProductAttribute._7_HardwareVer, ProductAttribute._6_HardwareVer_Len);
    Original_info_Wr += ProductAttribute._6_HardwareVer_Len;

    //Original_info[Original_info_Wr++]=ProductAttribute._8_SoftwareVer_len;
    //memcpy( ( char * ) Original_info+ Original_info_Wr,(u8*)ProductAttribute._9_SoftwareVer,ProductAttribute._8_SoftwareVer_len);
    //Original_info_Wr+=ProductAttribute._8_SoftwareVer_len;

    Original_info[Original_info_Wr++] = ProductAttribute._10_FirmWareVer_len;
    memcpy( ( char * ) Original_info + Original_info_Wr, (u8 *)ProductAttribute._11_FirmWare, ProductAttribute._10_FirmWareVer_len);
    Original_info_Wr += ProductAttribute._10_FirmWareVer_len;

    Original_info[Original_info_Wr++] = ProductAttribute._12_GNSSAttribute;
    Original_info[Original_info_Wr++] = ProductAttribute._13_ComModuleAttribute;

    /*
     infoLen=sizeof(ProductAttribute);
     memcpy( ( char * ) Original_info+ Original_info_Wr,(u8*)&ProductAttribute,infoLen);
     Original_info_Wr+=infoLen;
       */

    //  3. Send
    Protocol_End(Packet_Normal , 0);
    return true;
}
//--------------------------------------------------------------------------
u8  Stuff_EventACK_0301H(void)
{
    // 1. Head
    if(!Protocol_Head(MSG_0x0301, Packet_Normal))
        return false;
    // 2. content
    Original_info[Original_info_Wr++] = EventObj.Event_ID; // �����¼�ID

    //  3. Send
    Protocol_End(Packet_Normal , 0);
    return true;

}

u8  Stuff_ASKACK_0302H(void)
{

    // 1. Head
    if(!Protocol_Head(MSG_0x0302, Packet_Normal))
        return false;
    // 2. content
    //  Ӧ����ˮ��
    Original_info[Original_info_Wr++] = (ASK_Centre.ASK_floatID >> 8); // �����¼�ID
    Original_info[Original_info_Wr++] = ASK_Centre.ASK_floatID;
    Original_info[Original_info_Wr++] = ASK_Centre.ASK_answerID;
    //  3. Send
    Protocol_End(Packet_Normal , 0);
    return true;

}

u8  Stuff_MSGACK_0303H(void)
{

    // 1. Head
    if(!Protocol_Head(MSG_0x0303, Packet_Normal))
        return false;
    // 2. content
    //  Ӧ����ˮ��
    Original_info[Original_info_Wr++] = MSG_BroadCast_Obj.INFO_TYPE;
    Original_info[Original_info_Wr++] = MSG_BroadCast_Obj.INFO_PlyCancel; //  0  ȡ��  1 �㲥
    //  3. Send
    Protocol_End(Packet_Normal , 0);
    return true;

}


void  Recorder_init(void)
{
    Recode_Obj.Float_ID = 0;   //  ������ˮ��
    Recode_Obj.CMD = 0;   //  ���ݲɼ�
    Recode_Obj.SD_Data_Flag = 0; //  ���ͷ��������ر�־
    Recode_Obj.CountStep = 0; //  ����������Ҫһ��һ������
    Recode_Obj.timer = 0;
    //--------- add on  5-4
    Recode_Obj.Devide_Flag = 0; //  ��Ҫ�ְ��ϴ���־λ
    Recode_Obj.Total_pkt_num = 0; // �ְ��ܰ���
    Recode_Obj.Current_pkt_num = 0; // ��ǰ���Ͱ��� �� 1  ��ʼ
    Recode_Obj.Read_indexNum = 0;
    Recode_Obj.fcs = 0;
    Recode_Obj.Error = 0;
}




u8  Stuff_ControlACK_0500H(void)   //   ��������Ӧ��
{

    //  1. Head
    if(!Protocol_Head(MSG_0x0500, Packet_Normal))
        return false;
    // 2. content
    //------------------------------- Stuff ----------------------------------------
    //   float ID                                                // ��Ӧ����Ӧ����Ϣ����ˮ��
    Original_info[Original_info_Wr++] = (u8)(Vech_Control.CMD_FloatID >> 8);
    Original_info[Original_info_Wr++] = (u8)Vech_Control.CMD_FloatID;

    // 1. �澯��־  4
    memcpy( ( char * ) Original_info + Original_info_Wr, ( char * )Warn_Status, 4 );
    Original_info_Wr += 4;
    // 2. ״̬  4
    memcpy( ( char * ) Original_info + Original_info_Wr, ( char * )Car_Status, 4 );
    Original_info_Wr += 4;
    // 3.  γ��
    memcpy( ( char * ) Original_info + Original_info_Wr, ( char * )  Gps_Gprs.Latitude, 4 ); //γ��   modify by nathan
    Original_info_Wr += 4;
    // 4.  ����
    memcpy( ( char * ) Original_info + Original_info_Wr, ( char * )  Gps_Gprs.Longitude, 4 );	 //����    ����  Bit 7->0   ���� Bit 7 -> 1
    Original_info_Wr += 4;
    // 5.  �߳�
    Original_info[Original_info_Wr++] = (u8)(GPS_Hight << 8);
    Original_info[Original_info_Wr++] = (u8)GPS_Hight;
    // 6.  �ٶ�    0.1 Km/h
    Original_info[Original_info_Wr++] = (u8)(Speed_gps >> 8);
    Original_info[Original_info_Wr++] = (u8)Speed_gps;
    // 7. ����   ��λ 1��
    Original_info[Original_info_Wr++] = (GPS_direction >> 8); //High
    Original_info[Original_info_Wr++] = GPS_direction; // Low
    // 8.  ����ʱ��
    Original_info[Original_info_Wr++] = (((Gps_Gprs.Date[0]) / 10) << 4) + ((Gps_Gprs.Date[0]) % 10);
    Original_info[Original_info_Wr++] = ((Gps_Gprs.Date[1] / 10) << 4) + (Gps_Gprs.Date[1] % 10);
    Original_info[Original_info_Wr++] = ((Gps_Gprs.Date[2] / 10) << 4) + (Gps_Gprs.Date[2] % 10);
    Original_info[Original_info_Wr++] = ((Gps_Gprs.Time[0] / 10) << 4) + (Gps_Gprs.Time[0] % 10);
    Original_info[Original_info_Wr++] = ((Gps_Gprs.Time[1] / 10) << 4) + (Gps_Gprs.Time[1] % 10);
    Original_info[Original_info_Wr++] = ((Gps_Gprs.Time[2] / 10) << 4) + (Gps_Gprs.Time[2] % 10);

    //  3. Send
    Protocol_End(Packet_Normal , 0);
    return true;

}

u8  Stuff_RecoderACK_0700H_Error(void)
{

    u16  SregLen = 0, Swr = 0; //,Gwr=0; // S:serial	G: GPRS
    u8			Sfcs = 0;
    u16  i = 0;

    //  1. Head
    if( !Protocol_Head( MSG_0x0700, Packet_Normal ) )
    {
        return false;
    }


    Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
    Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
    Original_info[Original_info_Wr++]	= Recode_Obj.CMD;	// ������
    Swr 								= Original_info_Wr;

    Original_info[Original_info_Wr++]	= 0x55; 			// ��ʼͷ
    Original_info[Original_info_Wr++]	= 0x7A;
    if(Recode_Obj.Error == 1)
        Original_info[Original_info_Wr++]	= 0xFA; 			//������
    else if(Recode_Obj.Error == 2)
        Original_info[Original_info_Wr++]	= 0xFB; 			//������
    Original_info[Original_info_Wr++]	= 0x00; 			// ������

    //---------------  ��д���� A Э��	Serial Data   У��λ  -------------------------------------

    Sfcs = 0;                            //  ����SУ�� ��Ox55 ��ʼ
    for( i = Swr; i < Original_info_Wr; i++ )
    {
        Sfcs ^= Original_info[i];
    }
    //Original_info[Original_info_Wr++] = Sfcs;               // ��дFCS

    /*bitter:���һ������fcs*/


    Original_info[Original_info_Wr++] = Sfcs;               // ��дFCS


    //  3. Send
    Protocol_End( Packet_Normal, 0 );

    //  4.     ����Ƿְ� �жϽ���
    return true;
}


u8  Stuff_RecoderACK_0700H( u8 PaketType )  //   �г���¼�������ϴ�
{

    u16	SregLen = 0, Swr = 0; //,Gwr=0; // S:serial  G: GPRS
    //   u16      Reg_len_position=0;
    u8	       Sfcs = 0;
    u16	i = 0;
    u32	regdis = 0, reg2 = 0;
    //   u8   	Reg[70];
    //   u8       QueryRecNum=0;  // ��ѯ��¼��Ŀ
    u16    stuff_len = 0;

    //  1. Head
    if( !Protocol_Head( MSG_0x0700, PaketType ) )
    {
        return false;
    }

    switch( Recode_Obj.CMD )
    {
    case   0x00:                                                //  ִ�б�׼�汾���
    case   0x01:
    case   0x02:
    case   0x03:
    case   0x04:
    case   0x05:
    case   0x06:
    case   0x07:
        Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
        Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
        Original_info[Original_info_Wr++]	= Recode_Obj.CMD;   // ������
        Swr									= Original_info_Wr;
        Original_info_Wr += GB_557A_protocol_00_07_stuff(Original_info + Original_info_Wr, Recode_Obj.CMD);
        break;

    case 0x08:
        // 5 index per packet                                                            //  08   �ɼ�ָ������ʻ�ٶȼ�¼
        if( ( PaketType == Packet_Divide ) && ( Recode_Obj.Current_pkt_num == 1 ) )
        {
            Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
            Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
            Original_info[Original_info_Wr++]	= Recode_Obj.CMD;           // ������
        }
        Swr									= Original_info_Wr;
        Original_info[Original_info_Wr++]	= 0x55;                     // ��ʼͷ
        Original_info[Original_info_Wr++]	= 0x7A;
        Original_info[Original_info_Wr++]	= 0x08;                     //������

        if(Vdr_Wr_Rd_Offset.V_08H_Write > 5)
            SregLen = 630;                     // ��Ϣ����       630
        else
            SregLen = Vdr_Wr_Rd_Offset.V_08H_Write * 126;

        Original_info[Original_info_Wr++]	= SregLen >> 8;             // Hi
        Original_info[Original_info_Wr++]	= SregLen;                  // Lo

        Original_info[Original_info_Wr++] = 0x00;                       // ������

        //	��Ϣ����
        //WatchDog_Feed( );
        //get_08h( Original_info + Original_info_Wr );                        //126*5=630        num=576  packet
        if(Vdr_Wr_Rd_Offset.V_08H_Write == 0)
        {
            Original_info_Wr += stuff_len;
            break;
        }

        if(Vdr_Wr_Rd_Offset.V_08H_Write > 5)
        {
            stuff_len = stuff_drvData(0x08, Vdr_Wr_Rd_Offset.V_08H_Write - Recode_Obj.Read_indexNum, 5, Original_info + Original_info_Wr);
            Recode_Obj.Read_indexNum += 5;
        }
        else
        {
            stuff_len = stuff_drvData(0x08, Vdr_Wr_Rd_Offset.V_08H_Write, Vdr_Wr_Rd_Offset.V_08H_Write, Original_info + Original_info_Wr);
            Recode_Obj.Read_indexNum += Vdr_Wr_Rd_Offset.V_08H_Write;
        }

        Original_info_Wr += stuff_len;
        //  ������Ҫ�ְ�����  -----nate
        break;
    case   0x09:      // 1  index per packet                                                     // 09   ָ����λ����Ϣ��¼
        if( ( PaketType == Packet_Divide ) && ( Recode_Obj.Current_pkt_num == 1 ) )
        {
            Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
            Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
            Original_info[Original_info_Wr++]	= Recode_Obj.CMD;           // ������
        }
        Swr									= Original_info_Wr;
        Original_info[Original_info_Wr++]	= 0x55;                     // ��ʼͷ
        Original_info[Original_info_Wr++]	= 0x7A;

        Original_info[Original_info_Wr++] = 0x09;                       //������

        if(Vdr_Wr_Rd_Offset.V_09H_Write > 0)
            SregLen								= 666;  //666 * 2;                  // ��Ϣ����
        else
            SregLen = 0;
        Original_info[Original_info_Wr++]	= SregLen >> 8;             // Hi      666=0x29A
        Original_info[Original_info_Wr++]	= SregLen;                  // Lo

        //Original_info[Original_info_Wr++]=0;    // Hi      666=0x29A
        //Original_info[Original_info_Wr++]=0;	   // Lo

        Original_info[Original_info_Wr++] = 0x00;                       // ������

        //	��Ϣ����
        WatchDog_Feed( );

        if(Vdr_Wr_Rd_Offset.V_09H_Write > 0)
        {
            stuff_len = stuff_drvData(0x09, Vdr_Wr_Rd_Offset.V_09H_Write - Recode_Obj.Read_indexNum, 1, Original_info + Original_info_Wr);
            Original_info_Wr += stuff_len;
            Recode_Obj.Read_indexNum++;
        }

        break;
    case   0x10:     // 2  index per packet                                                        // 10-13     10   �¹��ɵ�ɼ���¼
        //�¹��ɵ�����
        if( ( PaketType == Packet_Divide ) && ( Recode_Obj.Current_pkt_num == 1 ) )
        {
            Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
            Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
            Original_info[Original_info_Wr++]	= Recode_Obj.CMD;           // ������
        }
        Swr									= Original_info_Wr;

        Original_info[Original_info_Wr++]	= 0x55;                     // ��ʼͷ
        Original_info[Original_info_Wr++]	= 0x7A;

        Original_info[Original_info_Wr++] = 0x10;                       //������

        if(Vdr_Wr_Rd_Offset.V_10H_Write > 0)
            SregLen					= 234 ;//*100;                //0		 // ��Ϣ����
        else
            SregLen = 0;
        Original_info[Original_info_Wr++]	= (u8)( SregLen >> 8 );     // Hi
        Original_info[Original_info_Wr++]	= (u8)SregLen;              // Lo

        Original_info[Original_info_Wr++] = 0x00;                       // ������

        //------- ��Ϣ���� ------
        //WatchDog_Feed( );
        delay_ms( 3 );

        if(Vdr_Wr_Rd_Offset.V_10H_Write > 0)
        {
            stuff_len = stuff_drvData(0x10, Vdr_Wr_Rd_Offset.V_10H_Write - Recode_Obj.Read_indexNum, 1, Original_info + Original_info_Wr);
            Original_info_Wr += stuff_len;
            Recode_Obj.Read_indexNum++;
        }

        break;

    case  0x11:   // 10  index per packet                                                           // 11 �ɼ�ָ���ĵĳ�ʱ��ʻ��¼
        if( ( PaketType == Packet_Divide ) && ( Recode_Obj.Current_pkt_num == 1 ) )
        {
            Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
            Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
            Original_info[Original_info_Wr++]	= Recode_Obj.CMD;           // ������
        }
        Swr									= Original_info_Wr;

        Original_info[Original_info_Wr++]	= 0x55;                     // ��ʼͷ
        Original_info[Original_info_Wr++]	= 0x7A;

        Original_info[Original_info_Wr++] = 0x11;                       //������

        //	SregLen								= 50 * 10;                 // ��Ϣ����
        if(Vdr_Wr_Rd_Offset.V_11H_Write == 0)
            break;

        if(Vdr_Wr_Rd_Offset.V_11H_Write > 10)
            SregLen = 500;                     // ��Ϣ����      50*10
        else
            SregLen = Vdr_Wr_Rd_Offset.V_11H_Write * 50;
        Original_info[Original_info_Wr++]	= (u8)( SregLen >> 8 );     // Hi
        Original_info[Original_info_Wr++]	= (u8)SregLen;              // Lo    65x7

        Original_info[Original_info_Wr++] = 0x00;                       // ������




        /*
               ÿ�� 50 bytes  ��100 ��    ��ȡ����ÿ10 ����һ��  500 packet    Totalnum=10
         */
        WatchDog_Feed( );
        //get_11h( Original_info + Original_info_Wr );                        //50  packetsize      num=100
        //Original_info_Wr += 500;
        if(Vdr_Wr_Rd_Offset.V_11H_Write == 0)
        {
            Original_info_Wr += stuff_len;
            break;
        }

        if(Vdr_Wr_Rd_Offset.V_11H_Write > 10)
        {
            stuff_len = stuff_drvData(0x11, Vdr_Wr_Rd_Offset.V_11H_Write - Recode_Obj.Read_indexNum, 10, Original_info + Original_info_Wr);
            Recode_Obj.Read_indexNum += 10;
        }
        else
        {
            stuff_len = stuff_drvData(0x11, Vdr_Wr_Rd_Offset.V_11H_Write, Vdr_Wr_Rd_Offset.V_11H_Write, Original_info + Original_info_Wr);
            Recode_Obj.Read_indexNum += Vdr_Wr_Rd_Offset.V_11H_Write;
        }

        Original_info_Wr += stuff_len;
        break;

    case  0x12:   // 10 index per packet                                                           // 12 �ɼ�ָ����ʻ����ݼ�¼  ---Devide
        if( ( PaketType == Packet_Divide ) && ( Recode_Obj.Current_pkt_num == 1 ) )
        {
            Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
            Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
            Original_info[Original_info_Wr++]	= Recode_Obj.CMD;           // ������
        }
        Swr									= Original_info_Wr;

        Original_info[Original_info_Wr++]	= 0x55;                     // ��ʼͷ
        Original_info[Original_info_Wr++]	= 0x7A;

        Original_info[Original_info_Wr++] = 0x12;                       //������

        if(Vdr_Wr_Rd_Offset.V_12H_Write > 10)
            SregLen = 250;                     // ��Ϣ����      50*10
        else
            SregLen = Vdr_Wr_Rd_Offset.V_12H_Write * 25;
        Original_info[Original_info_Wr++]	= (u8)( SregLen >> 8 );     // Hi
        Original_info[Original_info_Wr++]	= (u8)SregLen;              // Lo    65x7

        Original_info[Original_info_Wr++]  = 0x00;                      // ������

        //------- ��Ϣ���� ------


        /*
                ��ʻԱ��ݵ�¼��¼  ÿ��25 bytes      200��      20��һ�� 500 packetsize  totalnum=10
         */
        WatchDog_Feed( );
        if(Vdr_Wr_Rd_Offset.V_12H_Write == 0)
        {
            Original_info_Wr += stuff_len;
            break;
        }

        if(Vdr_Wr_Rd_Offset.V_12H_Write > 10)
        {
            stuff_len = stuff_drvData(0x12, Vdr_Wr_Rd_Offset.V_12H_Write - Recode_Obj.Read_indexNum, 10, Original_info + Original_info_Wr);
            Recode_Obj.Read_indexNum += 10;
        }
        else
        {
            stuff_len = stuff_drvData(0x12, Vdr_Wr_Rd_Offset.V_12H_Write, Vdr_Wr_Rd_Offset.V_12H_Write, Original_info + Original_info_Wr);
            Recode_Obj.Read_indexNum += Vdr_Wr_Rd_Offset.V_12H_Write;
        }

        Original_info_Wr += stuff_len;
        break;
    case  0x13:   // 100   index per packet                                                   // 13 �ɼ���¼���ⲿ�����¼
        Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
        Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
        Original_info[Original_info_Wr++]	= Recode_Obj.CMD;       // ������
        Swr									= Original_info_Wr;
        Original_info[Original_info_Wr++]	= 0x55;                 // ��ʼͷ
        Original_info[Original_info_Wr++]	= 0x7A;

        Original_info[Original_info_Wr++] = 0x13;                   //������


        //SregLen								= 700;                  // ��Ϣ����
        if(Vdr_Wr_Rd_Offset.V_13H_Write > 100)
            SregLen = 700;                     // ��Ϣ����      50*10
        else
            SregLen = Vdr_Wr_Rd_Offset.V_13H_Write * 7;

        Original_info[Original_info_Wr++]	= (u8)( SregLen >> 8 ); // Hi
        Original_info[Original_info_Wr++]	= (u8)SregLen;          // Lo    65x7

        Original_info[Original_info_Wr++] = 0x00;                   // ������
        //------- ��Ϣ���� ------


        /*
           �ⲿ�����¼   7 ���ֽ�  100 ��       һ��������
         */
        WatchDog_Feed( );
        //get_13h( Original_info + Original_info_Wr );
        //Original_info_Wr += 700;

        if(Vdr_Wr_Rd_Offset.V_13H_Write > 100)
        {
            stuff_len = stuff_drvData(0x13, Vdr_Wr_Rd_Offset.V_13H_Write - Recode_Obj.Read_indexNum, 100, Original_info + Original_info_Wr);
            Recode_Obj.Read_indexNum += 100;
        }
        else
        {
            stuff_len = stuff_drvData(0x13, Vdr_Wr_Rd_Offset.V_13H_Write, Vdr_Wr_Rd_Offset.V_13H_Write, Original_info + Original_info_Wr);
            Recode_Obj.Read_indexNum += Vdr_Wr_Rd_Offset.V_13H_Write;
        }

        Original_info_Wr += stuff_len;

        break;
    case   0x14: // 100 index per packet
        Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
        Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
        Original_info[Original_info_Wr++]	= Recode_Obj.CMD;       // ������
        Swr									= Original_info_Wr;
        // 14 ��¼�ǲ����޸ļ�¼
        Original_info[Original_info_Wr++]	= 0x55;                 // ��ʼͷ
        Original_info[Original_info_Wr++]	= 0x7A;

        Original_info[Original_info_Wr++] = 0x14;                   //������

        //SregLen								= 700;                  // ��Ϣ����
        if(Vdr_Wr_Rd_Offset.V_14H_Write > 100)
            SregLen = 700;                     // ��Ϣ����      50*10
        else
            SregLen = Vdr_Wr_Rd_Offset.V_14H_Write * 7;


        Original_info[Original_info_Wr++]	= (u8)( SregLen >> 8 ); // Hi
        Original_info[Original_info_Wr++]	= (u8)SregLen;          // Lo    65x7

        Original_info[Original_info_Wr++] = 0x00;                   // ������
        //------- ��Ϣ���� ------


        /*
               ÿ�� 7 ���ֽ�   100 ��    1��������
         */
        WatchDog_Feed( );
        // get_14h( Original_info + Original_info_Wr );
        // Original_info_Wr += 700;


        if(Vdr_Wr_Rd_Offset.V_14H_Write > 100)
        {
            stuff_len = stuff_drvData(0x14, Vdr_Wr_Rd_Offset.V_14H_Write - Recode_Obj.Read_indexNum, 100, Original_info + Original_info_Wr);
            Recode_Obj.Read_indexNum += 100;
        }
        else
        {
            stuff_len = stuff_drvData(0x14, Vdr_Wr_Rd_Offset.V_14H_Write, Vdr_Wr_Rd_Offset.V_14H_Write, Original_info + Original_info_Wr);
            Recode_Obj.Read_indexNum += Vdr_Wr_Rd_Offset.V_14H_Write;
        }

        Original_info_Wr += stuff_len;

        break;

    case    0x15:       // 4 index per packet                                                // 15 �ɼ�ָ�����ٶ�״̬��־     --------Divde
        if( ( PaketType == Packet_Divide ) && ( Recode_Obj.Current_pkt_num == 1 ) )
        {
            Original_info[Original_info_Wr++]	= (u8)( Recode_Obj.Float_ID >> 8 );
            Original_info[Original_info_Wr++]	= (u8)Recode_Obj.Float_ID;
            Original_info[Original_info_Wr++]	= Recode_Obj.CMD;       // ������
        }
        Swr									= Original_info_Wr;
        Original_info[Original_info_Wr++]	= 0x55;                 // ��ʼͷ
        Original_info[Original_info_Wr++]	= 0x7A;

        Original_info[Original_info_Wr++] = 0x15;                   //������

        //SregLen								= 133*2;             // ��Ϣ����
        if(Vdr_Wr_Rd_Offset.V_15H_Write > 2)
            SregLen = 266;                    // ��Ϣ����      50*10
        else
            SregLen = Vdr_Wr_Rd_Offset.V_15H_Write * 133;
        Original_info[Original_info_Wr++]	= (u8)( SregLen >> 8 ); // Hi
        Original_info[Original_info_Wr++]	= (u8)SregLen;          // Lo    65x7

        Original_info[Original_info_Wr++] = 0x00;                   // ������



        /*
               ÿ�� 133 ���ֽ�   10 ��    1��������     5*133=665        totalnum=2
         */
        WatchDog_Feed( );
        //get_15h( Original_info + Original_info_Wr );
        //Original_info_Wr += 133;
        if(Vdr_Wr_Rd_Offset.V_15H_Write == 0)
        {
            Original_info_Wr += stuff_len;
            break;
        }

        if(Vdr_Wr_Rd_Offset.V_15H_Write > 2)
        {
            stuff_len = stuff_drvData(0x15, Vdr_Wr_Rd_Offset.V_15H_Write - Recode_Obj.Read_indexNum, 2, Original_info + Original_info_Wr);
            Recode_Obj.Read_indexNum += 2;
        }
        else
        {
            stuff_len = stuff_drvData(0x15, Vdr_Wr_Rd_Offset.V_15H_Write, Vdr_Wr_Rd_Offset.V_15H_Write, Original_info + Original_info_Wr);
            Recode_Obj.Read_indexNum += Vdr_Wr_Rd_Offset.V_15H_Write;

        }
        Original_info_Wr += stuff_len;
        break;
    default:

        break;
    }
    //---------------  ��д���� A Э��	Serial Data   У��λ  -------------------------------------

    Sfcs = 0;                            //  ����SУ�� ��Ox55 ��ʼ
    for( i = Swr; i < Original_info_Wr; i++ )
    {
        Sfcs ^= Original_info[i];
    }
    //Original_info[Original_info_Wr++] = Sfcs;               // ��дFCS

    /*bitter:���һ������fcs*/
#if 1
    if( PaketType == Packet_Divide )
    {
        Recode_Obj.fcs ^= Sfcs;
        if( Recode_Obj.Current_pkt_num == Recode_Obj.Total_pkt_num )
        {
            Original_info[Original_info_Wr++] = Recode_Obj.fcs; // ��дFCS
        }
    }
    else
    {
        Original_info[Original_info_Wr++] = Sfcs;               // ��дFCS
    }
#endif

    //  3. Send
    Protocol_End( PaketType, 0 );

    //  4.     ����Ƿְ� �жϽ���
    if( Recode_Obj.Devide_Flag == 1 )  // ����Ӧ��
    {
        if(Recode_Obj.Current_pkt_num >= Recode_Obj.Total_pkt_num )
        {
            Recorder_init( );           //  clear
        }
        else
        {
            Recode_Obj.Current_pkt_num++;
        }
    }

    if( DispContent )
    {
        rt_kprintf( "\r\n	SEND Recorder Data ! \r\n");
    }

    return true;
}


//------------------------------------------------------------------
u8  Stuff_DataTransTx_0900H(void)
{

    // 1. Head
    if(!Protocol_Head(MSG_0x0900, Packet_Normal))
        return false;
    // 2. content
    //  Ӧ����ˮ��
    Original_info[Original_info_Wr++] = DataTrans.TYPE; // ����͸�����ݵ�����
    memcpy(Original_info + Original_info_Wr, DataTrans.Data_Tx, DataTrans.Data_TxLen);
    Original_info_Wr += DataTrans.Data_TxLen;
    //  3. Send
    Protocol_End(Packet_Normal , 0);
    return true;

}

//----------------------------------------------------------------------
u8  Stuff_CentreTakeACK_BD_0805H( void )
{
    // 1. Head
    if( !Protocol_Head( MSG_0x0805, Packet_Normal ) )
    {
        return false;
    }
    // 2. content
    //   float ID
    Original_info[Original_info_Wr++]	= (u8)( Centre_FloatID >> 8 );      //  Ӧ����ˮ��
    Original_info[Original_info_Wr++]	= (u8)Centre_FloatID;
    Original_info[Original_info_Wr++]	= (u8)CameraState.SingleCamra_TakeResualt_BD;   // ����Ӧ����
    Original_info[Original_info_Wr++]	= 0x00;                             //  �ɹ����ն�ý�����    1
    Original_info[Original_info_Wr++]	= 1;
    Original_info[Original_info_Wr++]	= 0;                                // ��ý��ID �б�
    Original_info[Original_info_Wr++]	= 0;
    Original_info[Original_info_Wr++]	= 0;
    Original_info[Original_info_Wr++]	= 1;
    //  3. Send
    Protocol_End( Packet_Normal, 0 );
    if( DispContent )
    {
        rt_kprintf( "\r\n	����ͷ��������Ӧ�� \r\n");
    }

    return true;
}

//----------------------------------------------------------------------

u8  Stuff_MultiMedia_InfoSD_0800H(void)
{

    // 1. Head
    if(!Protocol_Head(MSG_0x0800, Packet_Normal))
        return false;
    // 2. content
    switch (MediaObj.Media_Type)
    {
    case 0 : // ͼ��
        MediaObj.Media_totalPacketNum = Photo_sdState.Total_packetNum; // ͼƬ�ܰ���
        MediaObj.Media_currentPacketNum = Photo_sdState.SD_packetNum;	// ͼƬ��ǰ����
        MediaObj.Media_ID = 1; //  ��ý��ID
        MediaObj.Media_Channel = CameraState.Camera_Number; // ͼƬ����ͷͨ����
        break;
    case 1 : // ��Ƶ
#ifdef REC_VOICE_ENABLE
        MediaObj.Media_totalPacketNum = Sound_sdState.Total_packetNum;	// ͼƬ�ܰ���
        MediaObj.Media_currentPacketNum = Sound_sdState.SD_packetNum; // ͼƬ��ǰ����
        MediaObj.Media_ID = 1;	 //  ��ý��ID
        MediaObj.Media_Channel = 1; // ͼƬ����ͷͨ����
#endif
        break;
    default:
        return false;
    }

    //  MediaID
    Original_info[Original_info_Wr++] = (MediaObj.Media_ID >> 24); // �����¼�ID
    Original_info[Original_info_Wr++] = (MediaObj.Media_ID >> 16);
    Original_info[Original_info_Wr++] = (MediaObj.Media_ID >> 8);
    Original_info[Original_info_Wr++] = MediaObj.Media_ID;
    //  Type
    Original_info[Original_info_Wr++] = MediaObj.Media_Type;
    //  MediaCode Type
    Original_info[Original_info_Wr++] = MediaObj.Media_CodeType;
    Original_info[Original_info_Wr++] = MediaObj.Event_Code;
    Original_info[Original_info_Wr++] = MediaObj.Media_Channel;


    //  3. Send
    Protocol_End(Packet_Normal , 0);
    if(DispContent)
        rt_kprintf("\r\n	���Ͷ�ý���¼���Ϣ�ϴ�  \r\n");
    return true;

}

//--------------------------------------------------------------------------
u8  Stuff_MultiMedia_Data_0801H(void)
{
    u16 inadd = 0, readsize = 0; //,soundpage=0,sounddelta=0;
    //	u8  instr[SpxGet_Size];


    //  rt_kprintf("\r\n  1--- pic_total_num:  %d	current_num:  %d  MediaObj.Media_Type: %d \r\n ",MediaObj.Media_totalPacketNum,MediaObj.Media_currentPacketNum,MediaObj.Media_Type);
    // 1. Head
    if(!Protocol_Head(MSG_0x0801, Packet_Divide))
        return false;
    // 2. content1  ==>  MediaHead
    if(MediaObj.Media_currentPacketNum == 1)
    {
        //  MediaID
        Original_info[Original_info_Wr++] = (MediaObj.Media_ID >> 24); //  ��ý��ID
        Original_info[Original_info_Wr++] = (MediaObj.Media_ID >> 16);
        Original_info[Original_info_Wr++] = (MediaObj.Media_ID >> 8);
        Original_info[Original_info_Wr++] = MediaObj.Media_ID;
        //  Type
        Original_info[Original_info_Wr++] = MediaObj.Media_Type;  // ��ý������
        //  MediaCode Type
        Original_info[Original_info_Wr++] = MediaObj.Media_CodeType; // ��ý������ʽ
        Original_info[Original_info_Wr++] = MediaObj.Event_Code;   // ��ý���¼�����
        Original_info[Original_info_Wr++] = MediaObj.Media_Channel; // ͨ��ID

        //  Position Inifo
        //  �澯��־  4
        memcpy( ( char * ) Original_info + Original_info_Wr, ( char * )Warn_Status, 4 );
        Original_info_Wr += 4;
        // . ״̬  4
        memcpy( ( char * ) Original_info + Original_info_Wr, ( char * )Car_Status, 4 );
        Original_info_Wr += 4;
        //   γ��
        memcpy( ( char * ) Original_info + Original_info_Wr, ( char * )  Gps_Gprs.Latitude, 4 ); //γ��   modify by nathan
        Original_info_Wr += 4;
        //   ����
        memcpy( ( char * ) Original_info + Original_info_Wr, ( char * )  Gps_Gprs.Longitude, 4 );	 //����    ����  Bit 7->0   ���� Bit 7 -> 1
        Original_info_Wr += 4;
        //   �߳�
        Original_info[Original_info_Wr++] = (u8)(GPS_Hight << 8);
        Original_info[Original_info_Wr++] = (u8)GPS_Hight;
        //   �ٶ�    0.1 Km/h
        Original_info[Original_info_Wr++] = (u8)(Speed_gps >> 8); //(Spd_Using>>8);
        Original_info[Original_info_Wr++] = (u8)(Speed_gps); //Spd_Using;
        //   ����   ��λ 1��
        Original_info[Original_info_Wr++] = (GPS_direction >> 8); //High
        Original_info[Original_info_Wr++] = GPS_direction; // Low
        //   ����ʱ��
        Original_info[Original_info_Wr++] = (((Gps_Gprs.Date[0]) / 10) << 4) + ((Gps_Gprs.Date[0]) % 10);
        Original_info[Original_info_Wr++] = ((Gps_Gprs.Date[1] / 10) << 4) + (Gps_Gprs.Date[1] % 10);
        Original_info[Original_info_Wr++] = ((Gps_Gprs.Date[2] / 10) << 4) + (Gps_Gprs.Date[2] % 10);
        Original_info[Original_info_Wr++] = ((Gps_Gprs.Time[0] / 10) << 4) + (Gps_Gprs.Time[0] % 10);
        Original_info[Original_info_Wr++] = ((Gps_Gprs.Time[1] / 10) << 4) + (Gps_Gprs.Time[1] % 10);
        Original_info[Original_info_Wr++] = ((Gps_Gprs.Time[2] / 10) << 4) + (Gps_Gprs.Time[2] % 10);
        //------------
    }
    // 4. content3  ==> Media Info
    switch (MediaObj.Media_Type)
    {
    case 0 : // ͼ��

        if(((Photo_sdState.photo_sending) == enable) && ((Photo_sdState.SD_flag) == enable))
        {
            Photo_sdState.SD_flag = disable; // clear
        }
        else if((1 == MediaObj.RSD_State) && (Photo_sdState.SD_flag == enable)) //  �������
        {
            Photo_sdState.SD_flag = disable; // clear
        }
        else
            return false;
        //  ---------------  ��д����  ---------------
        Photo_sdState.photo_sendTimeout = 0; // clear timeout
        //			read		Photo_sdState.SD_packetNum��1��ʼ����
        //			content_startoffset 	picpage_offset				 contentpage_offset
        if(TF_Card_Status() == 0)
        {
            if(CameraState.Camera_Number == 1)
                Api_DFdirectory_Read(camera_1, Original_info + Original_info_Wr, 512, 1, MediaObj.Media_currentPacketNum);
            else if(CameraState.Camera_Number == 2)
                Api_DFdirectory_Read(camera_2, Original_info + Original_info_Wr, 512, 1, MediaObj.Media_currentPacketNum);
            else if(CameraState.Camera_Number == 3)
                Api_DFdirectory_Read(camera_3, Original_info + Original_info_Wr, 512, 1, MediaObj.Media_currentPacketNum);
            else if(CameraState.Camera_Number == 4)
                Api_DFdirectory_Read(camera_4, Original_info + Original_info_Wr, 512, 1, MediaObj.Media_currentPacketNum);

            DF_delay_ms(10);
            inadd = (Photo_sdState.SD_packetNum - 1) << 9; //����512
            if(PicFileSize > inadd)
            {
                if((PicFileSize - inadd) > 512)
                    readsize = 512;
                else
                {
                    readsize = PicFileSize - inadd; // ���һ��
                    rt_kprintf("\r\n   ���һ�� readsize =%d \r\n", readsize);
                }
            }
            else
                return false;
        }
        else if(TF_Card_Status() == 1)
        {
            ;
        }
        Original_info_Wr += readsize;		 //
        break;
    case 1 : // ��Ƶ
#ifdef REC_VOICE_ENABLE
        if(((Sound_sdState.photo_sending) == enable) && ((Sound_sdState.SD_flag) == enable))
        {
            Sound_sdState.SD_flag = disable; // clear
        }
        else
            return false;
        //------------------------------------------------------------------------
        //  ---------------  ��д����  ---------------
        //			read		Photo_sdState.SD_packetNum��1��ʼ����
        //			content_startoffset 	picpage_offset				 contentpage_offset
        if(TF_Card_Status() == 0)
        {
            Api_DFdirectory_Read(voice, Original_info + Original_info_Wr, 512, 1, MediaObj.Media_currentPacketNum);
            inadd = (Sound_sdState.SD_packetNum - 1) << 9; //����512
            if(SrcFileSize > inadd)
            {
                if((SrcFileSize - inadd) > 512)
                    readsize = 512;
                else
                {
                    readsize = SrcFileSize - inadd; // ���һ��
                    rt_kprintf("\r\n   ���һ�� readsize =%d \r\n", readsize);
                }
            }
            else
                return false;
        }
        if(GB19056.workstate == 0)
            rt_kprintf("\r\n Sound_sdState.SD_packetNum= %d   filesize=%d  readsize=%d  \r\n", Sound_sdState.SD_packetNum, SrcFileSize, SrcFileSize - inadd);
        Original_info_Wr += readsize;


#endif
        break;
    default:
        return false;
    }


    if(MediaObj.Media_currentPacketNum > MediaObj.Media_totalPacketNum)
    {
        return false;
    }
    //  5. Send

    Protocol_End(Packet_Divide, 0);

    if(DispContent)
        rt_kprintf("\r\n	Send Media Data \r\n");
    //  else
    {
        rt_kprintf("\r\n pic_total_num:  %d   current_num:  %d   \r\n ", MediaObj.Media_totalPacketNum, MediaObj.Media_currentPacketNum);
        if(MediaObj.Media_currentPacketNum >= MediaObj.Media_totalPacketNum)
        {
            rt_kprintf("\r\n Media ���һ��block\r\n");

            if(0 == MediaObj.RSD_State)	// �����˳����ģʽ�£����Ϊֹͣ״̬,�ȴ��������ش�
            {
                MediaObj.RSD_State = 2;
                MediaObj.RSD_Timer = 0;
            }

        }

    }
    //----------  �ۼӷ��ͱ��� --------------------
    if(0 == MediaObj.RSD_State)
    {
        if(MediaObj.Media_currentPacketNum < MediaObj.Media_totalPacketNum)
        {
            //  ͼƬ
            if(Photo_sdState.photo_sending == enable)
                Photo_sdState.SD_packetNum++;
            //  ��Ƶ
            if(Sound_sdState.photo_sending == enable)
                Sound_sdState.SD_packetNum++;
            //��Ƶ
            if(Video_sdState.photo_sending == enable)
                Video_sdState.SD_packetNum++;
        }
    }
    else if(1 == MediaObj.RSD_State)
    {
        if(GB19056.workstate == 0)
            rt_kprintf("\r\n  MediaObj.RSD_Reader =%d\r\n", MediaObj.RSD_Reader);
        MediaObj.RSD_Reader++;
        if(MediaObj.RSD_Reader == MediaObj.RSD_total)
            MediaObj.RSD_State = 2; //  ��λ�ȴ�״̬���ȴ��������ٷ��ش�ָ��
    }
    //----------  ����  -------------------
    return true;

}
//----------------------------------------------------------------------
u8  Stuff_MultiMedia_IndexAck_0802H(void)
{
    u16  totalNum = 0, lenregwr = 0;
    u16  i = 0;


    // 1. Head
    if(!Protocol_Head(MSG_0x0802, Packet_Normal))
        return false;
    // 2. content
    //   float ID  Ӧ����ˮ��
    Original_info[Original_info_Wr++] = (u8)(Centre_FloatID >> 8);
    Original_info[Original_info_Wr++] = (u8)Centre_FloatID;

    //------- ��ý�������� ----
    lenregwr = Original_info_Wr;
    Original_info[Original_info_Wr++] = (u8)(totalNum >> 8); // ��ʱռ��λ��
    Original_info[Original_info_Wr++] = (u8)totalNum;

    //----- ������ЧЧλ�� ----
    totalNum = 0;
    for(i = 0; i < 8; i++)
    {
        if(SD_ACKflag.f_MediaIndexACK_0802H == 1) // ͼ��
        {
            Api_RecordNum_Read(pic_index, i, (u8 *)&MediaIndex, sizeof(MediaIndex));
        }
        else if(SD_ACKflag.f_MediaIndexACK_0802H == 2) // ��Ƶ
        {
            Api_RecordNum_Read(voice_index, i, (u8 *)&MediaIndex, sizeof(MediaIndex));
        }
        // rt_kprintf("\r\n Effective_Flag %d  f_QueryEventCode %d  EventCode %d  \r\n",MediaIndex.Effective_Flag,SD_ACKflag.f_QueryEventCode,MediaIndex.EventCode);
        if((MediaIndex.Effective_Flag == 1) && (SD_ACKflag.f_QueryEventCode == MediaIndex.EventCode))
        {
            //  ������Ч�����������Ӧ���͵�����
            Original_info[Original_info_Wr++] = (u8)(MediaIndex.MediaID >> 24); //  ��ý��ID dworrd
            Original_info[Original_info_Wr++] = (u8)(MediaIndex.MediaID >> 16);
            Original_info[Original_info_Wr++] = (u8)(MediaIndex.MediaID >> 8);
            Original_info[Original_info_Wr++] = (u8)(MediaIndex.MediaID);
            Original_info[Original_info_Wr++] = MediaIndex.Type; //  ��ý������
            Original_info[Original_info_Wr++] = MediaIndex.ID; //  ͨ��
            Original_info[Original_info_Wr++] = MediaIndex.EventCode;
            memcpy(Original_info + Original_info_Wr, MediaIndex.PosInfo, 28);
            Original_info_Wr += 28;
            totalNum++;
        }

    }

    //---------   ����������  --------
    Original_info[lenregwr] = (u8)(totalNum >> 8);
    Original_info[lenregwr + 1] = totalNum;

    //  3. Send
    Protocol_End(Packet_Normal , 0);
    if(DispContent)
        rt_kprintf("\r\n	Send Media Index \r\n");
    return true;

}
//--------------------------------------------------------------------------------------
u8  Stuff_DriverInfoSD_0702H(void)
{
    u8 i = 0;
    // 1. Head
    if(!Protocol_Head(MSG_0x0702, Packet_Normal))
        return false;

    // 2. content
    //   ��ʻԱ��������
    i = strlen((const char *) JT808Conf_struct.Driver_Info.DriveName);
    Original_info[Original_info_Wr++] = i;
    memcpy(Original_info + Original_info_Wr, JT808Conf_struct.Driver_Info.DriveName, i); // name
    Original_info_Wr += i;
    memcpy(Original_info + Original_info_Wr, JT808Conf_struct.Driver_Info.Drv_CareerID, 20); //��ҵ�ʸ�֤
    Original_info_Wr += 20;
    i = strlen((const char *)JT808Conf_struct.Driver_Info.Comfirm_agentID); // ��������
    Original_info[Original_info_Wr++] = i;
    memcpy(Original_info + Original_info_Wr, JT808Conf_struct.Driver_Info.Comfirm_agentID, i);
    Original_info_Wr += i;
    //----- ��Ч��
    Original_info[Original_info_Wr++] = 0x20;
    Time2BCD(Original_info + Original_info_Wr);
    Original_info_Wr += 3; // ֻҪ������

    // 3. Send
    Protocol_End(Packet_Normal , 0);
    return true;

}
u8  Stuff_ISP_Resualt_BD_0108H(void)
{
    // 1. Head
    if(!Protocol_Head(MSG_0x0108, Packet_Normal))
        return false;
    // 2. content
    BD_ISP.ISP_running = 0; // clear
    //----------------------------------------------------
    Original_info[Original_info_Wr++] = BD_ISP.Update_Type; // ��������
    Original_info[Original_info_Wr++] = SD_ACKflag.f_BD_ISPResualt_0108H - 1; //BD_ISP.Update_Type;  // �������

    //  3. Send
    Protocol_End(Packet_Normal , 0);
    if(DispContent)
        rt_kprintf("\r\n	Զ����������ϱ�   %d \r\n", SD_ACKflag.f_BD_ISPResualt_0108H - 1);

    return true;
}
u8  Stuff_BatchDataTrans_BD_0704H(void)
{
    /*  Note:
                      ��ȡ�洢ʱ�п��ܴ��ڶ�ȡУ�鲻��ȷ�Ŀ��ܣ�
                      ���������rd_error Ҫ��¼�������
                      ͬʱ���ļ�¼��ҲҪ�ݼ�
      */
    u8   Rd_error_Counter = 0; // ��ȡ���������
    u8   StuffNum_last = 0; // ������д�����
    u8   i = 0;
    u16  len_wr_reg = 0; //   ���ȵ�λ�±� ��¼
    u8   rd_infolen = 0;
    u8   BubaoFlag = 1; //    1  �ǲ�������
    u8   Res_0704 = 0; // ��Ҫ�ְ�1  ����Ҫ0

    //0 .  congifrm   batch  num
    // 0.1  ��ȡ�洢��С
    if(cycle_read < cycle_write)
    {
        delta_0704_rd = cycle_write - cycle_read;
    }
    else   // write С�� read
    {
        delta_0704_rd = cycle_write + Max_CycleNum - cycle_read;
    }

    // 0.2  ���ݷ��ͼ���ж�ÿ����С
    //  0.2.1     ���ж��Ƿ��д洢����Ϣ
    if(delta_0704_rd)
    {
        //  �ж��Ƿ��кܶ�洢������
        if(delta_0704_rd >= Max_PKGRecNum_0704) //Max_PKGRecNum_0704
        {
            delta_0704_rd = Max_PKGRecNum_0704;
            Res_0704 = 1;
        }
        else if((delta_0704_rd > Limit_packek_Num) && (Current_SD_Duration >= 10)) // ��15 С����3  ���ְ�
        {
            delta_0704_rd = Limit_packek_Num;
            Res_0704 = 1;
        }
        else
            Res_0704 = 0;

    }
    //------------------------
    //  ä�����ϱ��� �Ҵ洢��ֵС�ڵ���   3
    if(Res_0704 == 0)
    {
        if (Current_SD_Duration >= 30) //  ����30  ����ÿ���ϱ�
        {
            if(delta_0704_rd == 1)
            {
                delta_0704_rd = 0;
                return  false;  //  Ҫ�ϱ���������
            }
            else
                return  nothing; // ��ִ�в���ֱ�ӷ���
        }
        else if(Current_SD_Duration >= 10)	// 10 ������ÿ3����һ��
        {
            // ��ʱ�ν���ѹ��      �����ϱ�ʱ���: 5:00  ---20:00
            if((Gps_Gprs.Time[0] >= 5) && (Gps_Gprs.Time[0] <= 19))
            {
                if(delta_0704_rd == 1)
                {
                    delta_0704_rd = 0;
                    return  false;  //  Ҫ�ϱ���������
                }
                else
                    return  nothing; // ��ִ�в���ֱ�ӷ���

            }
            else
            {
                //  �������ϱ�ʱ��� 3 ��һ���ϱ�
                if(delta_0704_rd >= Limit_packek_Num)
                {
                    delta_0704_rd = Limit_packek_Num;
                    BubaoFlag = 0; // �����ϱ�
                }
                else
                    return  nothing;   // С��5  ��ִ���κβ���ֱ�ӷ���
            }

        }
        else  // С�ڵ���10
        {
            //  �������ϱ�ʱ���8 ��һ���ϱ�
            BubaoFlag = 0; // �����ϱ�
            if(delta_0704_rd >= 8)
                delta_0704_rd = 8;
            else
                return  nothing;	// С��5  ��ִ���κβ���ֱ�ӷ���
        }

    }
    if(GB19056.workstate == 0)
        rt_kprintf("\r\n	 delat_0704=%d    read=%d   write=%d\r\n", delta_0704_rd, cycle_read, cycle_write);
    // 1. Head
    if(!Protocol_Head(MSG_0x0704, Packet_Normal))
        return false;
    // 2. content
    //  2.1	���������
    Original_info[Original_info_Wr++]	 = 0x00; //  10000=0x2710
    len_wr_reg = Original_info_Wr; //��¼�����±�
    Original_info[Original_info_Wr++]	 = delta_0704_rd;

    //  2.2	��������	 1	ä������	0:	 ����λ�������㱨
    Original_info[Original_info_Wr++] = BubaoFlag;  // ����ĳ������ϴ�

    //  2.3  ������Ŀ

    mangQu_read_reg = cycle_read; //	�洢��ǰ�ļ�¼
    for(i = 0; i < delta_0704_rd; i++)
    {
        //   ��ȡ��Ϣ
        memset(reg_128, 0, sizeof(reg_128));
        if( ReadCycleGPS(cycle_read, reg_128, 128) == false)	 // ʵ������ֻ��28���ֽ�
        {
            Rd_error_Counter++;
            continue;
        }
        cycle_read++;
        //----------  ������Ϣ���� --------------------------
        rd_infolen = reg_128[0];
        Original_info[Original_info_Wr++]   = 0;
        Original_info[Original_info_Wr++]   = rd_infolen - 1; // 28+ ������Ϣ����

        memcpy(Original_info + Original_info_Wr, reg_128 + 1, rd_infolen);
        Original_info_Wr += rd_infolen - 1;	 // ���ݳ��� �޳���һ�������ֽ�


        //OutPrint_HEX("read -1"reg_128,rd_infolen+1);

        //OutPrint_HEX("read -2",reg_128+1,rd_infolen);
        //==================================================
    }
    // ����0704 ������
    StuffNum_last = delta_0704_rd - Rd_error_Counter;
    Original_info[len_wr_reg] = StuffNum_last;

    //----------------------------------------------
    if(StuffNum_last == 0)
    {
        //rt_kprintf("\r\n	��λ���������ϴ� 0 ������ \r\n");
        // PositionSD_Enable();
        // Current_UDP_sd=1;
        return false;
    }
    //---------------------------
    //  3. Send
    Protocol_End(Packet_Normal , 0);
    if(DispContent)
        rt_kprintf("\r\n	��λ���������ϴ�  delta=%d  true=%d ��¼\r\n", delta_0704_rd, StuffNum_last);
    //----------------------------------------------
    if(StuffNum_last == 0)
    {
        PositionSD_Enable();
        Current_UDP_sd = 1;
        return false;
    }
    //---------------------------
    return true;
}


u8  Stuff_CANDataTrans_BD_0705H(void)
{
    u16   DataNum = 0, i = 0;
    u32   read_rd = 0;
    // 1. Head
    if(!Protocol_Head(MSG_0x0705, Packet_Normal))
        return false;
    // 2. content
    // ���������
    DataNum = (CAN_trans.canid_1_SdWr >> 3); // ����8
    Original_info[Original_info_Wr++] = (DataNum >> 8);
    Original_info[Original_info_Wr++] = DataNum;
    Can_sdnum += DataNum;
    //����ʱ��
    Original_info[Original_info_Wr++] = ((time_now.hour / 10) << 4) + (time_now.hour % 10);
    Original_info[Original_info_Wr++] = ((time_now.min / 10) << 4) + (time_now.min % 10);
    Original_info[Original_info_Wr++] = ((time_now.sec / 10) << 4) + (time_now.sec % 10);
    Original_info[Original_info_Wr++] = (Can_same % 10); //0x00;  // ms ����
    Original_info[Original_info_Wr++] = 0x00;
    //  CAN ����������
    read_rd = 0;
    for(i = 0; i < DataNum; i++)
    {
        /*
        Original_info[Original_info_Wr++]= (CAN_trans.canid_1_Filter_ID>>24)|0x40;// ����͸�����ݵ�����
        	Original_info[Original_info_Wr++]=(CAN_trans.canid_1_Filter_ID>>16);
        	Original_info[Original_info_Wr++]=(CAN_trans.canid_1_Filter_ID>>8);
        	Original_info[Original_info_Wr++]=CAN_trans.canid_1_Filter_ID;
          */

        Original_info[Original_info_Wr++] = (CAN_trans.canid_1_ID_SdBUF[i] >> 24) | 0x40; // ����͸�����ݵ�����
        Original_info[Original_info_Wr++] = (CAN_trans.canid_1_ID_SdBUF[i] >> 16);
        Original_info[Original_info_Wr++] = (CAN_trans.canid_1_ID_SdBUF[i] >> 8);
        Original_info[Original_info_Wr++] = CAN_trans.canid_1_ID_SdBUF[i];

        //--------------------------------------------------------------------------
        memcpy(Original_info + Original_info_Wr, CAN_trans.canid_1_Sdbuf + read_rd, 8);
        Original_info_Wr += 8;
        read_rd += 8;
    }
    CAN_trans.canid_1_SdWr = 0;
    //  3. Send
    Protocol_End(Packet_Normal , 0);
    return true;
}

//---------------------------------------------------------------------------------
u8  Stuff_Worklist_0701H(void)
{
    u32  listlen = 215;
    // 1. Head
    if(!Protocol_Head(MSG_0x0701, Packet_Normal))
        return false;

    // 2. content
    //   ��Ϣ����
    listlen = 207;
    Original_info[Original_info_Wr++] = (listlen >> 24); // �����¼�ID
    Original_info[Original_info_Wr++] = (listlen >> 16);
    Original_info[Original_info_Wr++] = (listlen >> 8);
    Original_info[Original_info_Wr++] = listlen;

    memcpy(Original_info + Original_info_Wr, "���˵�λ:�����һ��ͨ�Ź㲥���޹�˾ �绰:022-26237216  ", 55);
    Original_info_Wr += 55;
    memcpy(Original_info + Original_info_Wr, "���˵�λ:����������乫˾ �绰:022-86692666  ", 45);
    Original_info_Wr += 45;
    memcpy(Original_info + Original_info_Wr, "��Ʒ����:GPS�����ն�  ��װ��ʽ:  ��ʽ   ÿ������: 20   ����: 30��  ", 67);
    Original_info_Wr += 67;
    memcpy(Original_info + Original_info_Wr, "����:��ʽС���� �˴����� :  2012-1-11   ", 40);
    Original_info_Wr += 40;

    // 3. Send
    Protocol_End(Packet_Normal , 0);
    return true;

}
//-------------------- ISP Check  ---------------------------------------------
void  ISP_file_Check(void)
{
    u8  FileHead[100];
    u8  ISP_judge_resualt = 0;
    u32  HardVersion = 0;

    memset(ISP_buffer, 0, sizeof(ISP_buffer));
    SST25V_BufferRead(ISP_buffer, ISP_StartArea, 256);
    //---�ж��ļ����±�־---------------------
    if(ISP_buffer[32] != ISP_BYTE_CrcPass) //  ����У��ͨ����  ���±�־�ĳ�0xE1     ��ǰ��0xF1
    {
        rt_kprintf("\r\n �����ͺ���ȷ\r\n");
        return;
    }

    /*
       ���   �ֽ���	����			  ��ע
      1 		  1    ���±�־ 	 1 ��ʾ��Ҫ����   0 ��ʾ����Ҫ����
      2-5			  4   �豸����				 0x0000 0001  ST712   TWA1
    									0x0000 0002   STM32  103  ��A1
    									0x0000 0003   STM32  101  ������
    									0x0000 0004   STM32  A3  sst25
    									0x0000 0005   STM32  �г���¼��
      6-9		 4	   ����汾 	 ÿ���豸���ʹ�  0x0000 00001 ��ʼ���ݰ汾���ε���
      10-29 	  20	����		' mm-dd-yyyy HH:MM:SS'
      30-31 	  2    ��ҳ��		   ��������Ϣҳ
      32-35 	  4    ������ڵ�ַ
      36-200	   165	  Ԥ��
      201-		  n    �ļ���

    */
    //------------   Type check  ---------------------
    memset(FileHead, 0, sizeof(FileHead));
    memcpy(FileHead, ISP_buffer, 32);
    rt_kprintf( "\r\n FileHead:%s\r\n", FileHead );


    //------    �ļ���ʽ�ж�
    if(strncmp(ISP_buffer + 32 + 13, "70420TW705", 10) == 0) //�жϳ��̺��ͺ�
    {
        ISP_judge_resualt++;// step 1
        rt_kprintf("\r\n �����ͺ���ȷ\r\n");

        // hardware
        HardVersion = (ISP_buffer[32 + 38] << 24) + (ISP_buffer[32 + 39] << 16) + (ISP_buffer[32 + 40] << 8) + ISP_buffer[32 + 41];
        HardWareVerion = HardWareGet();
        if(HardWareVerion == HardVersion)	// Ҫ������ǰ���ϰ��� ȫ1
        {
            ISP_judge_resualt++;// step 2
            rt_kprintf("\r\n Ӳ���汾:%d\r\n", HardVersion);
        }
        else
            rt_kprintf("\r\n Ӳ���汾��ƥ��!\r\n");
        //firmware
        if(strncmp((const char *)ISP_buffer + 32 + 42, "HBGGHYPT", 8) == 0)
        {
            ISP_judge_resualt++;// step 3
            rt_kprintf("\r\n  �̼��汾:HBGGHYPT\r\n");
        }
        // operater
        if(strncmp((const char *)ISP_buffer + 32 + 62, "HBTDT", 8) == 0)
        {
            ISP_judge_resualt++;// step 4
            rt_kprintf("\r\n  �̼��汾:HBTDT\r\n");
        }

    }

    //ISP_judge_resualt=4;
    if(ISP_judge_resualt == 4)
    {
        //------- enable  flag -------------
        SST25V_BufferRead( FileHead, 0x001000, 100 );
        FileHead[32] = ISP_BYTE_Rdy2Update;    //-----  �ļ����±�־  ʹ������ʱ����
        SST25V_BufferWrite( FileHead, 0x001000, 100);

        {
            Systerm_Reset_counter = (Max_SystemCounter - 5);	 // ׼�������������³���
            ISP_resetFlag = 1; //׼������
            rt_kprintf( "\r\n ׼���������³���!\r\n" );
        }

        // rt_kprintf( "\r\n ��������ˣ��������µȴ��ж� У����� \r\n" );
    }
    else
    {
        //------- enable  flag -------------
        SST25V_BufferRead( FileHead, 0x001000, 100 );
        FileHead[32] = ISP_BYTE_TypeNotmatch;    //-----   �ļ�У��ͨ������ ���Ͳ�ƥ��
        SST25V_BufferWrite( FileHead, 0x001000, 100);
        BD_ISP.ISP_running = 0; // recover normal
        rt_kprintf( "\r\n ��ز�����ƥ�䲻�����!\r\n" );
    }

}
// FINSH_FUNCTION_EXPORT(ISP_file_Check, ISP_file_Check);



//----------------------------------------------------------------------------------
void Stuff_O200_Info_Only( u8 *Instr)
{
    u8  Infowr = 0;

    // 1. �澯��־  4
    memcpy( ( char * ) Instr + Infowr, ( char * )Warn_Status, 4 );
    Infowr += 4;
    // 2. ״̬  4
    memcpy( ( char * ) Instr + Infowr, ( char * )Car_Status, 4 );
    Infowr += 4;
    // 3.  γ��
    memcpy( ( char * ) Instr + Infowr, ( char * )  Gps_Gprs.Latitude, 4 ); //γ��   modify by nathan
    Infowr += 4;
    // 4.  ����
    memcpy( ( char * ) Instr + Infowr, ( char * )  Gps_Gprs.Longitude, 4 );	 //����    ����  Bit 7->0   ���� Bit 7 -> 1
    Infowr += 4;
    // 5.  �߳�
    Instr[Infowr++] = (u8)(GPS_Hight << 8);
    Instr[Infowr++] = (u8)GPS_Hight;
    // 6.  �ٶ�    0.1 Km/h
    Instr[Infowr++] = (u8)(Speed_gps >> 8);
    Instr[Infowr++] = (u8)Speed_gps;
    // 7. ����   ��λ 1��
    Instr[Infowr++] = (GPS_direction >> 8); //High
    Instr[Infowr++] = GPS_direction; // Low
    // 8.  ����ʱ��
    Instr[Infowr++] = (((Gps_Gprs.Date[0]) / 10) << 4) + ((Gps_Gprs.Date[0]) % 10);
    Instr[Infowr++] = ((Gps_Gprs.Date[1] / 10) << 4) + (Gps_Gprs.Date[1] % 10);
    Instr[Infowr++] = ((Gps_Gprs.Date[2] / 10) << 4) + (Gps_Gprs.Date[2] % 10);
    Instr[Infowr++] = ((Gps_Gprs.Time[0] / 10) << 4) + (Gps_Gprs.Time[0] % 10);
    Instr[Infowr++] = ((Gps_Gprs.Time[1] / 10) << 4) + (Gps_Gprs.Time[1] % 10);
    Instr[Infowr++] = ((Gps_Gprs.Time[2] / 10) << 4) + (Gps_Gprs.Time[2] % 10);

}
//-----------------------------------------------------
u8  Save_MediaIndex( u8 type, u8 *name, u8 ID, u8 Evencode)
{
    u8   i = 0;

    if((type != 1) && (type != 0))
        return false;

    //----- ������Чλ�� ----
    for(i = 0; i < 8; i++)
    {
        if(type == 0) // ͼ��
        {
            Api_RecordNum_Read(pic_index, i, (u8 *)&MediaIndex, sizeof(MediaIndex));
        }
        else if(type == 1) // ��Ƶ
        {
            Api_RecordNum_Read(voice_index, 1, (u8 *)&MediaIndex, sizeof(MediaIndex));
        }
        if(MediaIndex.Effective_Flag == 0)
            break;
    }
    if(i == 8) // �����������ӵ�һ����ʼ
        i = 0;
    //----  ��д��Ϣ -------------
    memset((u8 *)&MediaIndex, 0, sizeof(MediaIndex));
    MediaIndex.MediaID = JT808Conf_struct.Msg_Float_ID;
    MediaIndex.Type = type;
    MediaIndex.ID = ID;
    MediaIndex.Effective_Flag = 1;
    MediaIndex.EventCode = Evencode;
    memcpy(MediaIndex.FileName, name, strlen((const char *)name));
    Stuff_O200_Info_Only(MediaIndex.PosInfo);

    if(type == 0) // ͼ��
    {
        Api_RecordNum_Write(pic_index, i, (u8 *)&MediaIndex, sizeof(MediaIndex));
    }
    else if(type == 1) // ��Ƶ
    {
        Api_RecordNum_Write(voice_index, i, (u8 *)&MediaIndex, sizeof(MediaIndex));
    }
    return true;

}
//------------------------------------------------------------------
u8  CentreSet_subService_8103H(u32 SubID, u8 infolen, u8 *Content )
{

    u8    i = 0;
    u8    reg_str[80];
    u8    reg_in[20];
    u32   resualtu32 = 0;

    if(GB19056.workstate == 0)
        rt_kprintf("\r\n    �յ������������� SubID=%X \r\n", SubID);

    switch(SubID)
    {
    case 0x0001:  // �ն����������ͼ��  ��λ:s
        if(infolen != 4)
            break;
        JT808Conf_struct.DURATION.Heart_Dur = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        //    rt_kprintf("\r\n ���������: %d s\r\n",JT808Conf_struct.DURATION.Heart_Dur);
        break;
    case 0x0002:  // TCP ��ϢӦ��ʱʱ��  ��λ:s
        if(infolen != 4)
            break;
        JT808Conf_struct.DURATION.TCP_ACK_Dur = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        //rt_kprintf("\r\n TCP��ϢӦ����: %d s\r\n",JT808Conf_struct.DURATION.TCP_ACK_Dur);
        break;
    case 0x0003:  //  TCP ��Ϣ�ش�����
        if(infolen != 4)
            break;
        JT808Conf_struct.DURATION.TCP_ReSD_Num = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        //rt_kprintf("\r\n TCP�ش�����: %d\r\n",JT808Conf_struct.DURATION.TCP_ReSD_Num);
        break;
    case 0x0004:  // UDP ��ϢӦ��ʱʱ��  ��λ:s
        if(infolen != 4)
            break;
        JT808Conf_struct.DURATION.UDP_ACK_Dur = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        // rt_kprintf("\r\n UDPӦ��ʱ: %d\r\n",JT808Conf_struct.DURATION.UDP_ACK_Dur);
        break;
    case 0x0005:  //  UDP ��Ϣ�ش�����
        if(infolen != 4)
            break;
        JT808Conf_struct.DURATION.UDP_ReSD_Num = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        //rt_kprintf("\r\n UDP�ش�����: %d\r\n",JT808Conf_struct.DURATION.UDP_ReSD_Num);
        break;
    case 0x0010:  //  ��������APN
        if(infolen == 0)
            break;
        memset(APN_String, 0, sizeof(APN_String));
        memcpy(APN_String, (char *)Content, infolen);
        memset((u8 *)SysConf_struct.APN_str, 0, sizeof(APN_String));
        memcpy((u8 *)SysConf_struct.APN_str, (char *)Content, infolen);
        Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));


        DataLink_APN_Set(APN_String, 0);

        break;
    case 0x0013:  //  ����������ַ  IP ������
        memset(reg_in, 0, sizeof(reg_in));
        memcpy(reg_in, Content, infolen);
        // rt_kprintf("\r\n  �������޸� ��IP ������: %s \r\n",reg_in);
        Fail_Flag = 1;
        break;// ƽ̨�����޸�������IP

        //----------------------------

        i = str2ip((char *)reg_in, RemoteIP_main);
        if (i <= 3)
        {
            //rt_kprintf("\r\n  ����: %s \r\n",reg_in);

            memset(DomainNameStr, 0, sizeof(DomainNameStr));
            memset(SysConf_struct.DNSR, 0, sizeof(DomainNameStr));
            memcpy(DomainNameStr, (char *)Content, infolen);
            memcpy(SysConf_struct.DNSR, (char *)Content, infolen);
            Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));

            //----- ���� GSM ģ��------
            DataLink_DNSR_Set(SysConf_struct.DNSR, 1);


            SD_ACKflag.f_CentreCMDack_0001H = 1 ; // 2 DataLink_EndFlag=1; //AT_End(); �ȷ��ؽ���ٹҶ�
            break;
        }
        memset(reg_str, 0, sizeof(reg_str));
        IP_Str((char *)reg_str, *( u32 * ) RemoteIP_main);
        strcat((char *)reg_str, " :");
        sprintf((char *)reg_str + strlen((const char *)reg_str), "%u\r\n", RemotePort_main);
        memcpy(SysConf_struct.IP_Main, RemoteIP_main, 4);
        SysConf_struct.Port_main = RemotePort_main;

        Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
        // rt_kprintf("\r\n ���������������� IP \r\n");
        // rt_kprintf("\r\n SOCKET :");
        // rt_kprintf((char*)reg_str);
        //-----------  Below add by Nathan  ----------------------------
        //  rt_kprintf("\r\n		   ����IP: %d.%d.%d.%d : %d \r\n",RemoteIP_aux[0],RemoteIP_aux[1],RemoteIP_aux[2],RemoteIP_aux[3],RemotePort_main);

        //-----------  Below add by Nathan  ----------------------------
        DataLink_MainSocket_set(RemoteIP_main, RemotePort_main, 0);
        //-------------------------------------------------------------

        SD_ACKflag.f_CentreCMDack_0001H = 1 ; //DataLink_EndFlag=1; //AT_End(); �ȷ��ؽ���ٹҶ�

        break;
    case 0x0014:  // ���ݷ����� APN

        break;
    case 0x0017:  // ���ݷ�����  IP



        memset(reg_in, 0, sizeof(reg_in));
        memcpy(reg_in, Content, infolen);

        //rt_kprintf("\r\n  �������޸� ����IP ������: %s \r\n",reg_in);
        Fail_Flag = 1;
        break;// ƽ̨�����޸�������IP

        //----------------------------
        i = str2ip((char *)reg_in, RemoteIP_aux);
        if (i <= 3)
        {
            // rt_kprintf("\r\n  ����aux: %s \r\n",reg_in);
            memset(DomainNameStr_aux, 0, sizeof(DomainNameStr_aux));
            memset(SysConf_struct.DNSR_Aux, 0, sizeof(DomainNameStr_aux));
            memcpy(DomainNameStr_aux, (char *)Content, infolen);
            memcpy(SysConf_struct.DNSR_Aux, (char *)Content, infolen);
            Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
            //----- ���� GSM ģ��------
            DataLink_DNSR2_Set(SysConf_struct.DNSR_Aux, 1);

            SD_ACKflag.f_CentreCMDack_0001H = 1 ; //DataLink_EndFlag=1; //AT_End(); �ȷ��ؽ���ٹҶ�
            break;
        }
        memset(reg_str, 0, sizeof(reg_str));
        IP_Str((char *)reg_str, *( u32 * ) RemoteIP_aux);
        strcat((char *)reg_str, " :");
        sprintf((char *)reg_str + strlen((const char *)reg_str), "%u\r\n", RemotePort_aux);

        Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
        //rt_kprintf("\r\n �������ñ��÷����� IP \r\n");
        //rt_kprintf("\r\nUDP SOCKET :");
        //rt_kprintf((char*)reg_str);
        DataLink_AuxSocket_set(RemoteIP_aux, RemotePort_aux, 1);
        //-----------  Below add by Nathan  ----------------------------
        // rt_kprintf("\r\n 		����IP: %d.%d.%d.%d : %d \r\n",RemoteIP_aux[0],RemoteIP_aux[1],RemoteIP_aux[2],RemoteIP_aux[3],RemotePort_aux);
        break;
    case 0x0018:  //  ������ TCP �˿�
        //----------------------------
        if(infolen != 4)
            break;
        RemotePort_main = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];

        Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
        // rt_kprintf("\r\n ���������������� PORT :%d\r\n",RemotePort_main);
        //rt_kprintf("\r\nUDP SOCKET :");
        // rt_kprintf((char*)reg_str);
        //-----------  Below add by Nathan  ----------------------------
        DataLink_MainSocket_set(RemoteIP_main, RemotePort_main, 0);					 //-------------------------------------------------------------
        SD_ACKflag.f_CentreCMDack_0001H = 1 ; //DataLink_EndFlag=1; //AT_End(); �ȷ��ؽ���ٹҶ�
        break;
    case 0x0019:  //  ������ UDP �˿�

        if(infolen != 4)
            break;
        RemotePort_aux = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];

        Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
        //rt_kprintf("\r\n ��������UDP������ PORT \r\n");
        //rt_kprintf("\r\nUDP SOCKET :");
        //rt_kprintf((char*)reg_str);
        //rt_kprintf("\r\n		 ����IP: %d.%d.%d.%d : %d \r\n",RemoteIP_aux[0],RemoteIP_aux[1],RemoteIP_aux[2],RemoteIP_aux[3],RemotePort_aux);
        break;
    case 0x0020:  //  �㱨����  0 ��ʱ�㱨  1 ����㱨 2 ��ʱ�Ͷ���㱨
        if(infolen != 4)
            break;
        resualtu32 = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];

        JT808Conf_struct.SD_MODE.Send_strategy = resualtu32;
        switch(resualtu32)
        {
        case 0://rt_kprintf("\r\n ��ʱ�㱨 \r\n");
            JT808Conf_struct.SD_MODE.DUR_TOTALMODE = 1; // ʹ�ܶ�ʱ����
            JT808Conf_struct.SD_MODE.Dur_DefaultMode = 1; //  ȱʡ��ʽ�ϱ�

            JT808Conf_struct.SD_MODE.DIST_TOTALMODE = 0;
            JT808Conf_struct.SD_MODE.Dist_DefaultMode = 0;
            break;
        case 1://rt_kprintf("\r\n ����㱨 \r\n");
            JT808Conf_struct.SD_MODE.DIST_TOTALMODE = 1;
            JT808Conf_struct.SD_MODE.Dist_DefaultMode = 1;

            JT808Conf_struct.SD_MODE.DUR_TOTALMODE = 0; // ʹ�ܶ�ʱ����
            JT808Conf_struct.SD_MODE.Dur_DefaultMode = 0; //  ȱʡ��ʽ�ϱ�
            break;
        case 2://rt_kprintf("\r\n ��ʱ�Ͷ���㱨\r\n");
            JT808Conf_struct.SD_MODE.DIST_TOTALMODE = 1;
            JT808Conf_struct.SD_MODE.Dist_DefaultMode = 1;

            JT808Conf_struct.SD_MODE.DUR_TOTALMODE = 1; // ʹ�ܶ�ʱ����
            JT808Conf_struct.SD_MODE.Dur_DefaultMode = 1; //  ȱʡ��ʽ�ϱ�

            break;
        default:
            break;

        }

        Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
        break;
    case 0x0021:  //  λ�û㱨����  0 ����ACC�ϱ�  1 ����ACC�͵�¼״̬�ϱ�
        JT808Conf_struct.PositionSd_Stratage = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];
        break;
        //--------

    case 0x0022:  //  ��ʻԱδ��¼ �㱨ʱ���� ��λ:s    >0
        if(infolen != 4)
            break;
        JT808Conf_struct.DURATION.NoDrvLogin_Dur = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        //      rt_kprintf("\r\n ��ʻԱδ��¼�㱨���: %d\r\n",JT808Conf_struct.DURATION.NoDrvLogin_Dur);
        break;
    case 0x0027:   //  ����ʱ�㱨ʱ��������λ s  >0
        if(infolen != 4)
            break;
        JT808Conf_struct.DURATION.Sleep_Dur = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        //rt_kprintf("\r\n ���߻㱨ʱ����: %d \r\n",JT808Conf_struct.DURATION.Sleep_Dur);
        break;
    case 0x0028:   //  ��������ʱ�㱨ʱ����  ��λ s
        if(infolen != 4)
            break;
        JT808Conf_struct.DURATION.Emegence_Dur = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        //rt_kprintf("\r\n ��������ʱ����: %d \r\n",JT808Conf_struct.DURATION.Emegence_Dur);
        break;
    case 0x0029:   //  ȱʡʱ��㱨���  ��λ s
        if(infolen != 4)
            break;
        JT808Conf_struct.DURATION.Default_Dur = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        //rt_kprintf("\r\n ȱʡ�㱨ʱ����: %d \r\n",JT808Conf_struct.DURATION.Default_Dur);
        break;
        //---------

    case 0x002C:   //  ȱʡ����㱨���  ��λ ��
        if(infolen != 4)
            break;
        JT808Conf_struct.DISTANCE.Defalut_DistDelta = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        //rt_kprintf("\r\n ȱʡ����㱨���: %d m\r\n",JT808Conf_struct.DISTANCE.Defalut_DistDelta);
        break;
    case 0x002D:   //  ��ʻԱδ��¼�㱨������ ��λ ��
        if(infolen != 4)
            break;
        JT808Conf_struct.DISTANCE.NoDrvLogin_Dist = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        //rt_kprintf("\r\n ��ʻԱδ��¼�㱨����: %d m\r\n",JT808Conf_struct.DISTANCE.NoDrvLogin_Dist);
        break;
    case 0x002E:   //  ����ʱ�㱨������  ��λ ��
        if(infolen != 4)
            break;
        JT808Conf_struct.DISTANCE.Sleep_Dist = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        //rt_kprintf("\r\n ����ʱ�����ϱ����: %d m\r\n",JT808Conf_struct.DISTANCE.Sleep_Dist);
        break;
    case 0x002F:   //  ��������ʱ�㱨������  ��λ ��
        if(infolen != 4)
            break;
        JT808Conf_struct.DISTANCE.Emergen_Dist = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        //rt_kprintf("\r\n ��������ʱ�����ϱ����: %d m\r\n",JT808Conf_struct.DISTANCE.Emergen_Dist);
        break;
    case 0x0030:   //  �յ㲹���Ƕ� , <180
        if(infolen != 4)
            break;
        JT808Conf_struct.DURATION.SD_Delta_maxAngle = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        //rt_kprintf("\r\n �յ㲹���Ƕ�: %d ��\r\n",JT808Conf_struct.DISTANCE.Emergen_Dist);
        break;

    case 0x0040:   //   ���ƽ̨�绰����
        if(infolen == 0)
            break;
        i = strlen((const char *)JT808Conf_struct.LISTEN_Num);
        //rt_kprintf("\r\n old: %s \r\n",JT808Conf_struct.LISTEN_Num);

        memset(JT808Conf_struct.LISTEN_Num, 0, sizeof(JT808Conf_struct.LISTEN_Num));
        memcpy(JT808Conf_struct.LISTEN_Num, Content, infolen);
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        //rt_kprintf("\r\n new: %s \r\n",JT808Conf_struct.LISTEN_Num);

        //CallState=CallState_rdytoDialLis;  // ׼����ʼ�����������
        //rt_kprintf("\r\n ���ü��ƽ̨����: %s \r\n",JT808Conf_struct.LISTEN_Num);

        break;
    case 0x0041:   //   ��λ�绰���룬�ɲ��ô˵绰���벦���ն˵绰���ն˸�λ
        if(infolen == 0)
            break;
        memset(reg_str, 0, sizeof(reg_str));
        memcpy(reg_str, Content, infolen);
        //rt_kprintf("\r\n ��λ�绰���� %s \r\n",reg_str);
        break;
    case 0x0042:   //   �ָ��������õ绰���ɲ��øõ绰�������ն˻ָ���������

        break;
    case 0x0045:   //  �ն˵绰�������� 0 �Զ�����  1 ACC ON�Զ����� OFFʱ�ֶ�����

        break;
    case 0x0046:   //  ÿ��ͨ���ʱ�� ����λ  ��

        break;
    case 0x0047:   //  �����ͨ��ʱ�䣬��λ  ��

        break;
    case 0x0048:   //  �����绰����
        if(infolen == 0)
            break;
        memset(JT808Conf_struct.LISTEN_Num, 0, sizeof(JT808Conf_struct.LISTEN_Num));
        memcpy(JT808Conf_struct.LISTEN_Num, Content, infolen);
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        CallState = CallState_rdytoDialLis; // ׼����ʼ�����������
        rt_kprintf("\r\n ���������������: %s \r\n", JT808Conf_struct.LISTEN_Num);
        break;

        //----------
    case 0x0050:  //  ���������֣� ��λ����Ϣ�б�����־���Ӧ����ӦλΪ1ʱ����������---

        if(infolen != 4)
            break;
        Warn_MaskWord[0] = Content[0];
        Warn_MaskWord[1] = Content[1];
        Warn_MaskWord[2] = Content[2];
        Warn_MaskWord[3] = Content[3];
        resualtu32 = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];
        // rt_kprintf("\r\n ����������: %x \r\n",resualtu32);    //
        break;
    case 0x0052:  //  �������տ��أ� �뱨����־��Ӧ��λ1ʱ������

        break;
    case 0x0053:  //  �������մ洢  	�뱨����־��Ӧ��λ1ʱ�����մ洢 ����ʵʱ�ϴ�

        break;
    case 0x0054:  //  �ؼ���־  		�뱨����־��Ӧ��λ1  Ϊ�ؼ�����
        if(infolen != 4)
            break;
        Key_MaskWord[0] = Content[0];
        Key_MaskWord[1] = Content[1];
        Key_MaskWord[2] = Content[2];
        Key_MaskWord[3] = Content[3];
        resualtu32 = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];
        //  rt_kprintf("\r\n �ؼ�������־: %x \r\n",resualtu32);    //


        break;
        //---------

    case 0x0055:  //  ����ٶ�   ��λ   ǧ��ÿСʱ
        if(infolen != 4)
            break;
        JT808Conf_struct.Speed_warn_MAX = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8)	+ Content[3];
        memset(reg_str, 0, sizeof(reg_str));
        memcpy(reg_str, & JT808Conf_struct.Speed_warn_MAX, 4);
        memcpy(reg_str + 4, &JT808Conf_struct.Spd_Exd_LimitSeconds, 4);
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        rt_kprintf("\r\n ����ٶ�: %d km/h \r\n", JT808Conf_struct.Speed_warn_MAX);
        Spd_ExpInit();
        break;
    case 0x0056:  //  ���ٳ���ʱ��    ��λ s
        if(infolen != 4)
            break;
        JT808Conf_struct.Spd_Exd_LimitSeconds = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8)	+ Content[3];
        memset(reg_str, 0, sizeof(reg_str));
        memcpy(reg_str, & JT808Conf_struct.Speed_warn_MAX, 4);
        memcpy(reg_str + 4, &JT808Conf_struct.Spd_Exd_LimitSeconds, 4);
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        rt_kprintf("\r\n ��ʱ����ʱ��: %d s \r\n", JT808Conf_struct.Spd_Exd_LimitSeconds);
        Spd_ExpInit();
        break;
    case 0x0057:  //  ������ʻʱ������ ��λ  s
        if(infolen != 4)
            break;
        TiredConf_struct.TiredDoor.Door_DrvKeepingSec = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8)   + Content[3];
        Api_Config_write(tired_config, 0, (u8 *)&TiredConf_struct, sizeof(TiredConf_struct));
        Warn_Status[3] &= ~0x04; //BIT(2)	�Ӵ�ƣ�ͼ�ʻ����
        // rt_kprintf("\r\n ������ʻʱ������: %d s \r\n",TiredConf_struct.TiredDoor.Door_DrvKeepingSec);
        break;
    case 0x0058:  //  �����ۼƼ�ʻʱ������  ��λ  s
        if(infolen != 4)
            break;
        TiredConf_struct.TiredDoor.Door_DayAccumlateDrvSec = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8)   + Content[3];
        Api_Config_write(tired_config, 0, (u8 *)&TiredConf_struct, sizeof(TiredConf_struct));
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        //    rt_kprintf("\r\n �����ۼƼ�ʻʱ��: %d s \r\n",TiredConf_struct.TiredDoor.Door_DayAccumlateDrvSec);
        break;
    case 0x0059:  //  ��С��Ϣʱ��  ��λ s
        if(infolen != 4)
            break;
        TiredConf_struct.TiredDoor.Door_MinSleepSec = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8)   + Content[3];
        Api_Config_write(tired_config, 0, (u8 *)&TiredConf_struct, sizeof(TiredConf_struct));
        //rt_kprintf("\r\n ��С��Ϣʱ��: %d s \r\n",TiredConf_struct.TiredDoor.Door_MinSleepSec);
        break;
    case 0x005A:  //  �ͣ��ʱ��   ��λ s
        if(infolen != 4)
            break;
        TiredConf_struct.TiredDoor.Door_MaxParkingSec = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8)	 + Content[3];
        Api_Config_write(tired_config, 0, (u8 *)&TiredConf_struct, sizeof(TiredConf_struct));
        TiredConf_struct.TiredDoor.Parking_currentcnt = 0;
        Warn_Status[1] &= ~0x08; // �����ʱ����
        //rt_kprintf("\r\n �ͣ��ʱ��: %d s \r\n",TiredConf_struct.TiredDoor.Door_MaxParkingSec);
        break;
        //---------
    case  0x0070: //  ͼ��/��Ƶ����  1-10  1 ���

        break;
    case  0x0071: //  ����  0-255

        break;
    case  0x0072: //  �Աȶ�  0-127

        break;
    case  0x0073: // ���Ͷ�  0-127

        break;
    case  0x0074: // ɫ��   0-255

        break;
        //---------
    case  0x0080: // ������̱����   1/10 km
        // resualtu32=JT808Conf_struct.Distance_m_u32/100;

        resualtu32 = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8)  + Content[3];
        //  Api_Config_Recwrite_Large(jt808,0,(u8*)&JT808Conf_struct,sizeof(JT808Conf_struct));
        DayStartDistance_32 = resualtu32;
        Distance_m_u32 = 0;
        DF_Write_RecordAdd(Distance_m_u32, DayStartDistance_32, TYPE_DayDistancAdd);
        JT808Conf_struct.DayStartDistance_32 = DayStartDistance_32;
        JT808Conf_struct.Distance_m_u32 = Distance_m_u32;
        // rt_kprintf("\r\n �����������:  %d  1/10km/h\r\n",resualtu32);


        break;
    case  0x0081: // �������ڵ�ʡ��ID
        if(infolen != 2)
            break;
        Vechicle_Info.Dev_ProvinceID = (Content[0] << 8) + Content[1];
        DF_WriteFlashSector(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
        WatchDog_Feed();
        DF_WriteFlashSector(DF_VehicleBAK_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
        WatchDog_Feed();
        DF_WriteFlashSector(DF_VehicleBAK2_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
        //rt_kprintf("\r\n ��������ʡ��ID: 0x%X \r\n",Vechicle_Info.Dev_ProvinceID);
        break;
    case  0x0082: // ������������ID
        if(infolen != 2)
            break;
        Vechicle_Info.Dev_ProvinceID = (Content[0] << 8) + Content[1];
        DF_WriteFlashSector(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
        WatchDog_Feed();
        DF_WriteFlashSector(DF_VehicleBAK_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
        WatchDog_Feed();
        DF_WriteFlashSector(DF_VehicleBAK2_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
        //rt_kprintf("\r\n ������������ID: 0x%X \r\n",Vechicle_Info.Dev_ProvinceID);
        break;
    case  0x0083: // ������ͨ�����Ű䷢�Ļ���������
        if(infolen < 4)
            break;
        memset(Vechicle_Info.Vech_Num, 0, sizeof(Vechicle_Info.Vech_Num));
        memcpy(Vechicle_Info.Vech_Num, Content, infolen);
        DF_WriteFlashSector(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
        WatchDog_Feed();
        DF_WriteFlashSector(DF_VehicleBAK_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
        WatchDog_Feed();
        DF_WriteFlashSector(DF_VehicleBAK2_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
        //rt_kprintf("\r\n ��������ʻ֤��: %s  \r\n",Vechicle_Info.Vech_Num);
        break;
    case  0x0084: // ������ɫ  ���չ��ҹ涨
        if(infolen != 1)
            break;
        Vechicle_Info.Dev_Color = Content[0];
        DF_WriteFlashSector(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
        WatchDog_Feed();
        DF_WriteFlashSector(DF_VehicleBAK_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
        WatchDog_Feed();
        DF_WriteFlashSector(DF_VehicleBAK2_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
        //rt_kprintf("\r\n ������ɫ: %d  \r\n",Vechicle_Info.Dev_Color);

        //  redial();
        // idip("clear");
        break;
        //--------------- BD  ����----------------------------------
    case  0x001A: //  IC�� ��������������
        memset(reg_in, 0, sizeof(reg_in));
        memcpy(reg_in, Content, infolen);
        //----------------------------

        i = str2ip((char *)reg_in, SysConf_struct.BD_IC_main_IP);
        if (i <= 3)
        {
            rt_kprintf("\r\n IC  �� ����: %s \r\n", reg_in);

            memset(SysConf_struct.BD_IC_DNSR, 0, sizeof(SysConf_struct.BD_IC_DNSR));
            memcpy(SysConf_struct.BD_IC_DNSR, (char *)Content, infolen);
            Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
            break;
        }
        memset(reg_str, 0, sizeof(reg_str));
        IP_Str((char *)reg_str, *( u32 * ) SysConf_struct.BD_IC_main_IP);
        Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
        rt_kprintf("\r\n IC  ��IP: %d.%d.%d.%d \r\n", SysConf_struct.BD_IC_main_IP[0], SysConf_struct.BD_IC_main_IP[1], SysConf_struct.BD_IC_main_IP[2], SysConf_struct.BD_IC_main_IP[3]);
        break;
    case  0x001B:// IC ��  ��TCP�˿�
        SysConf_struct.BD_IC_TCP_port = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];
        Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
        // rt_kprintf("\r\n IC  TCP Port: %d \r\n",SysConf_struct.BD_IC_TCP_port);
        //  IC ������
        DataLink_IC_Socket_set(SysConf_struct.BD_IC_main_IP, SysConf_struct.BD_IC_TCP_port, 0);
        break;
    case  0x001C:// IC��   ��UDP �˿�
        SysConf_struct.BD_IC_UDP_port = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];
        Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
        // rt_kprintf("\r\n IC  UDP Port : %d \r\n",SysConf_struct.BD_IC_UDP_port);
        break;
    case  0x001D:// IC �����÷������Ͷ˿�
        memset(reg_in, 0, sizeof(reg_in));
        memcpy(reg_in, Content, infolen);
        //----------------------------
        i = str2ip((char *)reg_in, SysConf_struct.BD_IC_Aux_IP);
        if (i <= 3)
        {
            rt_kprintf("\r\n IC  �� ����: %s \r\n", reg_in);

            memset(SysConf_struct.BD_IC_DNSR_Aux, 0, sizeof(SysConf_struct.BD_IC_DNSR_Aux));
            memcpy(SysConf_struct.BD_IC_DNSR_Aux, (char *)Content, infolen);
            Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
            break;
        }
        memset(reg_str, 0, sizeof(reg_str));
        IP_Str((char *)reg_str, *( u32 * ) SysConf_struct.BD_IC_Aux_IP);
        Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
        // rt_kprintf("\r\n IC  ����IP: %d.%d.%d.%d \r\n",SysConf_struct.BD_IC_Aux_IP[0],SysConf_struct.BD_IC_Aux_IP[1],SysConf_struct.BD_IC_Aux_IP[2],SysConf_struct.BD_IC_Aux_IP[3]);

        break;
    case   0x0031://  ����Χ���뾶(�Ƿ�λ����ֵ)
        if(infolen != 2)
            break;
        JT808Conf_struct.BD_CycleRadius_DoorValue = (Content[0] << 8) + Content[1];
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        //rt_kprintf("\r\n ����Χ���뾶(�Ƿ��ƶ���ֵ): %d m\r\n",JT808Conf_struct.BD_CycleRadius_DoorValue);
        break;
    case   0x005B: // ���ٱ���Ԥ����ֵ   1/10 KM/h
        if(infolen != 2)
            break;
        JT808Conf_struct.BD_MaxSpd_preWarnValue = (Content[0] << 8) + Content[1];
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        //rt_kprintf("\r\n ���ٱ���Ԥ����ֵ: %d x 0.1km/h\r\n",JT808Conf_struct.BD_MaxSpd_preWarnValue);

        break;
    case   0x005C: // ƣ�ͼ�ʻ��ֵ  ��λ:s
        if(infolen != 2)
            break;
        JT808Conf_struct.BD_TiredDrv_preWarnValue = (Content[0] << 8) + Content[1];
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        //rt_kprintf("\r\n ƣ�ͼ�ʻ��ֵ: %d s\r\n",JT808Conf_struct.BD_TiredDrv_preWarnValue);
        break;
    case   0x005D:// ��ײ������������
        if(infolen != 2)
            break;
        JT808Conf_struct.BD_Collision_Setting = (Content[0] << 8) + Content[1];
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        //rt_kprintf("\r\n ��ײ������������ : %x \r\n",JT808Conf_struct.BD_MaxSpd_preWarnValue);
        break;
    case   0x005E: // �෭������������   Ĭ��30��
        if(infolen != 2)
            break;
        JT808Conf_struct.BD_Laydown_Setting = (Content[0] << 8) + Content[1];
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        //rt_kprintf("\r\n�෭������������: %x r\n",JT808Conf_struct.BD_Laydown_Setting);
        break;
        //---  CAMERA
    case   0x0064://  ��ʱ���տ���
        if(infolen != 4)
            break;
        JT808Conf_struct.BD_CameraTakeByTime_Settings = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        //  rt_kprintf("\r\n ��ʱ���տ���: %X\r\n",JT808Conf_struct.BD_CameraTakeByTime_Settings);
        break;
    case   0x0065://  ���������տ���
        if(infolen != 4)
            break;
        JT808Conf_struct.BD_CameraTakeByDistance_Settings = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        // rt_kprintf("\r\n  ���������տ���: %X\r\n",JT808Conf_struct.BD_CameraTakeByDistance_Settings);
        break;
        //--- GNSS
    case    0x0090: // GNSS ��λģʽ

        JT808Conf_struct.BD_EXT.GNSS_Mode = Content[0];
        //  rt_kprintf("\r\n  GNSS Value= 0x%2X	\r\n",Content[0]);
        switch(JT808Conf_struct.BD_EXT.GNSS_Mode)
        {
        case 0x01:  // �� GPS ��λģʽ
            gps_mode("2");
            Car_Status[1] &= ~0x0C; // clear bit3 bit2      1100
            Car_Status[1] |= 0x04; // Gps mode   0100
            break;
        case  0x02:  // 	��BD2 ��λģʽ
            gps_mode("1");
            Car_Status[1] &= ~0x0C; // clear bit3 bit2
            Car_Status[1] |= 0x08; // BD mode	1000
            break;
        case  0x03: //  BD2+GPS ��λģʽ
            gps_mode("3");
            Car_Status[1] &= ~0x0C; // clear bit3 bit2
            Car_Status[1] |= 0x0C; // BD+GPS  mode	1100
            break;
        }
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        break;
    case    0x0091: // GNSS ������
        JT808Conf_struct.BD_EXT.GNSS_Baud = Content[0];
        switch(JT808Conf_struct.BD_EXT.GNSS_Baud)
        {
        case 0x00:  //  4800
            JT808Conf_struct.BD_EXT.GNSS_Baud_Value = 4800;
            //rt_thread_delay(5);
            //gps_write("$PCAS01,0*1C\r\n",14);
            //rt_thread_delay(5);
            // gps_baud( 4800 );
            break;
        case 0x01: //9600   --default
            JT808Conf_struct.BD_EXT.GNSS_Baud_Value = 9600;
            //rt_thread_delay(5);
            //gps_write("$PCAS01,1*1D\r\n",14);
            //rt_thread_delay(5);
            //gps_baud( 9600 );
            break;
        case  0x02: // 19200
            JT808Conf_struct.BD_EXT.GNSS_Baud_Value = 19200;
            //rt_thread_delay(5);
            //gps_write("$PCAS01,2*1E\r\n",14);
            //rt_thread_delay(5);
            //gps_baud( 19200 );
            break;
        case  0x03://  38400
            JT808Conf_struct.BD_EXT.GNSS_Baud_Value = 38400;
            //rt_thread_delay(5);
            //gps_write("$PCAS01,3*1F\r\n",14);
            //rt_thread_delay(5);
            //gps_baud( 38400 );
            break;
        case  0x04:// 57600
            JT808Conf_struct.BD_EXT.GNSS_Baud_Value = 57600;
            //rt_thread_delay(5);
            //gps_write("$PCAS01,4*18\r\n",14);
            //rt_thread_delay(5);
            //gps_baud( 57600 );
            break;
        case   0x05:// 115200
            JT808Conf_struct.BD_EXT.GNSS_Baud_Value = 115200;
            //rt_thread_delay(5);
            // gps_write("$PCAS01,5*19\r\n",14);
            //rt_thread_delay(5);
            // gps_baud( 115200 );
            break;

        }
        //---UART_GPS_Init(baud);  //   �޸Ĵ��ڲ�����
        rt_thread_delay(20);
        //rt_kprintf("\r\n ��������GNSS ������:  %d s\r\n",JT808Conf_struct.BD_EXT.GNSS_Baud_Value);

        break;
    case    0x0092: // GNSS ģ����ϸ��λ�������Ƶ��

        break;
    case    0x0093://  GNSS ģ����ϸ��λ���ݲɼ�Ƶ��  1

        break;
    case    0x0094:// GNSS ģ����ϸ��λ�����ϴ���ʽ
        if(Content[0] == 0x01)
            rt_kprintf("\r\n ��ʱ���ϴ�");
        if(Content[0] == 0x00)
            rt_kprintf("\r\n ���ش洢���ϴ�");

        break;
    case    0x0095://  GNSS ģ����ϸ�ϴ�����
        // GNSS_rawdata.SendDuration=(Content[0]<<24)+(Content[1]<<16)+(Content[2]<<8) +Content[3];;  //
        break;
        //----CAN--
    case      0x0100://  CAN  ����ͨ�� 1  �ɼ����              0   : ��ʾ���ɼ�
        if(infolen != 4)
            break;
        CAN_trans.can1_sample_dur = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];
        Can_RXnum = 0;
        Can_sdnum = 0;
        Can_same = 0;
        break;
    case    0x0101://  CAN  ����ͨ�� 1 �ϴ�ʱ����    0 :  ��ʾ���ϴ�
        if(infolen != 2)
            break;
        CAN_trans.can1_trans_dur = (Content[0] << 8) + Content[1];

        break;
    case    0x0102://  CAN  ����ͨ�� 2  �ɼ����              0   : ��ʾ���ɼ�
        if(infolen != 4)
            break;
        CAN_trans.can2_sample_dur = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];
        break;
    case    0x0103://  CAN  ����ͨ�� 2 �ϴ�ʱ����    0 :  ��ʾ���ϴ�
        if(infolen != 2)
            break;
        CAN_trans.can2_trans_dur = (Content[0] << 8) + Content[1];
        break;
    case    0x0110://  CAN ����ID �����ɼ�����
        CAN_trans.canid_2_NotGetID = ((Content[4] & 0x1F) << 24) + (Content[5] << 16) + (Content[6] << 8) + Content[7];
        //rt_kprintf("\r\n���ɼ�ID  0x0110= %08X\r\n",CAN_trans.canid_2_NotGetID);

        break;
    case    0x0111://  CAN ����ID �����ɼ����� ����
        if(infolen != 8)
            break;
        OutPrint_HEX("0x0111", Content, 8);
        memcpy(	CAN_trans.canid_1, Content, 8);
        memset(CAN_trans.canid_1_Rxbuf, 0, sizeof(CAN_trans.canid_1_Rxbuf));
        CAN_trans.canid_1_RxWr = 0;  // clear  write
        CAN_trans.canid_timer = 0;
        CAN_trans.canid_0705_sdFlag = 0;

        //------ ������ֵ --------
        CAN_trans.canid_1_sample_dur = (Content[0] << 24) + (Content[1] << 16) + (Content[2] << 8) + Content[3];
        if(Content[4] & 0x40) // bit 30
            CAN_trans.canid_1_ext_state = 1;
        else
            CAN_trans.canid_1_ext_state = 0;
        CAN_trans.canid_1_Filter_ID = ((Content[4] & 0x1F) << 24) + (Content[5] << 16) + (Content[6] << 8) + Content[7];

        //  rt_kprintf("\r\n FilterID=%08X, EXTstate: %d   can1_samle=%d ms   canid_1_sample_dur=%dms    Trans_dur=%d s\r\n", CAN_trans.canid_1_Filter_ID, CAN_trans.canid_1_ext_state,CAN_trans.can1_sample_dur,CAN_trans.canid_1_sample_dur,CAN_trans.can1_trans_dur);

        break;
    default:
        return false;
    }

    return true;
}

//--------------------------------------------------------------------
u8  CentreSet_subService_8105H(u32 Control_ID, u8 infolen, u8 *Content )
{

    switch(Control_ID)
    {
    case 1:  //  ������������  ����֮����÷ֺŷָ�   ָ���ʽ����:
        /*
        URL ��ַ���������ƣ������û������������룻��ַ��TCP�˿ڣ�UDP�˿ڣ�������ID; Ӳ���汾���̼��汾�����ӵ�ָ��������ָ���Ƿ�����ʱ�ޣ�
        ��ĳ����������ֵ����ſ�
          */
        rt_kprintf("\r\n �������� \r\n");
        rt_kprintf("\r\n ����: %s\r\n", Content);
        break;
    case 2:  // �����ն�����ָ��������
        /*
        ���ӿ��ƣ����ƽ̨��Ȩ�룻���ŵ����ƣ� �����û������������룻��ַ��TCP�˿ڣ�UDP�˿ڣ����ӵ�ָ��������ʱ��
        ��ÿ����������ֵ����ſ�
         */
        // rt_kprintf("\r\n �ն˿�������ָ��������\r\n");
        // rt_kprintf("\r\n ����: %s\r\n",Content);
        break;
    case 3:  //  �ն˹ػ�
        SD_ACKflag.f_CentreCMDack_0001H = 5;
        // rt_kprintf("\r\n �ն˹ػ� \r\n");
        break;
    case 4:  //  �ն˸�λ
        SD_ACKflag.f_CentreCMDack_0001H = 3;
        // rt_kprintf("\r\n �ն˸�λ \r\n");
        break;
    case 5: //   �ն˻ָ���������
        /* if(SysConf_struct.Version_ID==SYSID)   //  check  wether need  update  or not
        {
        	SysConf_struct.Version_ID=SYSID+1;
        	Api_Config_write(config,ID_CONF_SYS,(u8*)&SysConf_struct,sizeof(SysConf_struct));
        	Systerm_Reset_counter=Max_SystemCounter;
        	ISP_resetFlag=2;    //   ����Զ�������������Ƹ�λϵͳ
        }
        rt_kprintf("\r\n �ָ��������� \r\n"); */
        break;
    case 6: //   �ر�����ͨ��
        SD_ACKflag.f_CentreCMDack_0001H = 5;
        // rt_kprintf("\r\n �ر�����ͨ�� \r\n");
        break;
    case 7: //   �ر���������ͨ��
        SD_ACKflag.f_CentreCMDack_0001H = 5;
        // rt_kprintf("\r\n �ر�����ͨ�� \r\n");
        break;
    default:
        return false;

    }
    return  true;
}

//-------------------------------------------------------------------
void CenterSet_subService_8701H(u8 cmd,  u8 *Instr)
{
    TDateTime now;
    u32  reg_dis = 0, regdis = 0, reg2 = 0;

    switch(cmd)
    {
    case 0x82: //	  �������ó��ƺ�
        memset(Vechicle_Info.Vech_VIN, 0, sizeof(Vechicle_Info.Vech_VIN));
        memset(Vechicle_Info.Vech_Num, 0, sizeof(Vechicle_Info.Vech_Num));
        memset(Vechicle_Info.Vech_Type, 0, sizeof(Vechicle_Info.Vech_Type));

        //-----------------------------------------------------------------------
        memcpy(Vechicle_Info.Vech_VIN, Instr, 17);
        memcpy(Vechicle_Info.Vech_Num, Instr + 17, 12);
        memcpy(Vechicle_Info.Vech_Type, Instr + 29, 12);

        DF_WriteFlashSector(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
        WatchDog_Feed();
        DF_WriteFlashSector(DF_VehicleBAK_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
        WatchDog_Feed();
        DF_WriteFlashSector(DF_VehicleBAK2_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
        break;
    case 0xC2: //���ü�¼��ʱ��
        // ûɶ�ã������ظ����У�����GPSУ׼�͹���
        //  ������ ʱ���� BCD
        now.year = (Instr[0] >> 4) * 10 + (Instr[0] & 0x0F);
        now.month = (Instr[1] >> 4) * 10 + (Instr[1] & 0x0F);
        now.day = (Instr[2] >> 4) * 10 + (Instr[2] & 0x0F);
        now.hour = (Instr[3] >> 4) * 10 + (Instr[3] & 0x0F);
        now.min = (Instr[4] >> 4) * 10 + (Instr[4] & 0x0F);
        now.sec = (Instr[5] >> 4) * 10 + (Instr[5] & 0x0F);
        now.week = 1;
        Device_RTC_set(now);
        break;

    case 0xC3: //�����ٶ�����ϵ��������ϵ����
        // ǰ6 ���ǵ�ǰʱ��
        now.year = (Instr[0] >> 4) * 10 + (Instr[0] & 0x0F);
        now.month = (Instr[1] >> 4) * 10 + (Instr[1] & 0x0F);
        now.day = (Instr[2] >> 4) * 10 + (Instr[2] & 0x0F);
        now.hour = (Instr[3] >> 4) * 10 + (Instr[3] & 0x0F);
        now.min = (Instr[4] >> 4) * 10 + (Instr[4] & 0x0F);
        now.sec = (Instr[5] >> 4) * 10 + (Instr[5] & 0x0F);
        now.week = 1;
        Device_RTC_set(now);
        JT808Conf_struct.Vech_Character_Value = (Instr[6] << 8) + (u32)Instr[7]; // ����ϵ��  �ٶ�����ϵ��
        JT808Conf_struct.DF_K_adjustState = 0;
        ModuleStatus &= ~Status_Pcheck;
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        break;
    case 0x83: //  ��¼�ǳ��ΰ�װʱ��

        memcpy(JT808Conf_struct.FirstSetupDate, Instr, 6);
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        break;
    case 0x84: // �����ź���������Ϣ
        memcpy(Setting08, Instr, 80);
        break;

    case 0xC4: //   ���ó�ʼ���

        /*   Time2BCD( Original_info + Original_info_Wr );                       // ��¼��ʵʱʱ��
        	Original_info_Wr					+= 6;
        	Original_info[Original_info_Wr++]	= 0x13;                         // ���ΰ�װʱ��
        	Original_info[Original_info_Wr++]	= 0x03;
        	Original_info[Original_info_Wr++]	= 0x01;
        	Original_info[Original_info_Wr++]	= 0x08;
        	Original_info[Original_info_Wr++]	= 0x30;
        	Original_info[Original_info_Wr++]	= 0x26;
        */
        now.year = (Instr[0] >> 4) * 10 + (Instr[0] & 0x0F);
        now.month = (Instr[1] >> 4) * 10 + (Instr[1] & 0x0F);
        now.day = (Instr[2] >> 4) * 10 + (Instr[2] & 0x0F);
        now.hour = (Instr[3] >> 4) * 10 + (Instr[3] & 0x0F);
        now.min = (Instr[4] >> 4) * 10 + (Instr[4] & 0x0F);
        now.sec = (Instr[5] >> 4) * 10 + (Instr[5] & 0x0F);
        now.week = 1;
        Device_RTC_set(now);



        memcpy(JT808Conf_struct.FirstSetupDate, Instr + 6, 6);
        //Api_Config_Recwrite_Large(jt808,0,(u8*)&JT808Conf_struct,sizeof(JT808Conf_struct));
        // instr     + 12      ���ó�ʼ��̵�λ��   BCD

        reg_dis = (Instr[12] >> 4) * 10000000 + (Instr[12] & 0x0F) * 1000000 + (Instr[13] >> 4) * 100000 + (Instr[13] & 0x0F) * 10000 \
                  +(Instr[14] >> 4) * 1000 + (Instr[14] & 0x0F) * 100 + (Instr[15] >> 4) * 10 + (Instr[15] & 0x0F);

        Distance_m_u32 = reg_dis * 100;
        DF_Write_RecordAdd(Distance_m_u32, DayStartDistance_32, TYPE_DayDistancAdd);
        JT808Conf_struct.DayStartDistance_32 = DayStartDistance_32;
        JT808Conf_struct.Distance_m_u32 = Distance_m_u32;
        //Api_Config_Recwrite_Large(jt808,0,(u8*)&JT808Conf_struct,sizeof(JT808Conf_struct));
        //--- ��ʼ���
        Original_info[Original_info_Wr++]	= 0x00;
        Original_info[Original_info_Wr++]	= 0x00;
        Original_info[Original_info_Wr++]	= 0x00;
        Original_info[Original_info_Wr++]	= 0x00;
        // -- �ۻ���� 3���ֽ� ��λ0.1km    6λ
        regdis								= JT808Conf_struct.Distance_m_u32 / 100; //��λ0.1km
        reg2								= regdis / 100000;
        Original_info[Original_info_Wr++]	= 0x00;
        Original_info[Original_info_Wr++]	= ( reg2 << 4 ) + ( regdis % 100000 / 10000 );
        Original_info[Original_info_Wr++]	= ( ( regdis % 10000 / 1000 ) << 4 ) + ( regdis % 1000 / 100 );
        Original_info[Original_info_Wr++]	= ( ( regdis % 100 / 10 ) << 4 ) + ( regdis % 10 );




        break;
    default:
        Recode_Obj.Error = 2; // ���ô���
        break;
    }

    if(Recode_Obj.Error != 2)
        VDR_product_14H(cmd);

}
//-------------------------------------------
void  Media_Start_Init( u8  MdType , u8  MdCodeType)
{
    MediaObj.Media_Type = MdType; //	ָ����ǰ�����ý�������   0  ��ʾͼƬ
    MediaObj.Media_CodeType = MdCodeType; //  ��ý������ʽ   0  ��ʾJPEG	��ʽ
    MediaObj.SD_media_Flag = 1; //  �ö�ý���¼���Ϣ���ͱ�־λ  ����ʼͼƬ����
    MediaObj.SD_Eventstate = 1;		 //  ��ʼ���ڷ���״̬
    MediaObj.RSD_State = 0;
    MediaObj.RSD_Timer = 0;
    MediaObj.RSD_total = 0;
    MediaObj.RSD_Reader = 0;
    MediaObj.SD_Data_Flag = 0;
    MediaObj.Media_transmittingFlag = 0;

    //----------------------------------------------------
}

void Media_Clear_State(void)
{
    // �����Meia Type
    MediaObj.MaxSd_counter = 0;
    MediaObj.SD_Eventstate = 0;
    MediaObj.SD_timer = 0;
    MediaObj.SD_media_Flag = 0;
    MediaObj.SD_Data_Flag = 0;
    MediaObj.RSD_State = 0;
    MediaObj.RSD_Timer = 0;
    MediaObj.RSD_total = 0;
    MediaObj.RSD_Reader = 0;
    MediaObj.Media_transmittingFlag = 0;
}
void  Media_Timer(void)
{
    if(1 == MediaObj.SD_Eventstate)
    {
        MediaObj.SD_timer++;
        if(MediaObj.SD_timer > 6)
        {
            MediaObj.SD_timer = 0;
            MediaObj.SD_media_Flag = 1;
            MediaObj.MaxSd_counter++;
            if(MediaObj.MaxSd_counter > 5)
                Media_Clear_State();
        }

    }
}
void Media_RSdMode_Timer(void)
{
    if((1 == MediaObj.RSD_State))
    {
        MediaObj.RSD_Timer++;
        if(MediaObj.RSD_Timer > 12)
        {
            MediaObj.RSD_Timer = 0;
            MediaObj.SD_Data_Flag = 1; // ���ش����Ͷ�ý����Ϣ��־λ
            switch(MediaObj.Media_Type)    //   ͼƬ�ش�����
            {
            case 0://  ͼ��
                Photo_sdState.SD_packetNum = MediaObj.Media_ReSdList[MediaObj.RSD_Reader];
                Photo_sdState.SD_flag = 1;
                if(GB19056.workstate == 0)
                    rt_kprintf("\r\n ���� %d ��\r\n", Photo_sdState.SD_packetNum);
                break;
            case 1:// ��Ƶ
                Sound_sdState.SD_packetNum = MediaObj.Media_ReSdList[MediaObj.RSD_Reader];
                Sound_sdState.SD_flag = 1;
                break;
#if 0
            case 2:// ��Ƶ
                Video_sdState.SD_packetNum = MediaObj.Media_ReSdList[MediaObj.RSD_Reader];
                Video_sdState.SD_flag = 1;
                break;
#endif
            default:
                break;

            }
        }
    }
    else if(2 == MediaObj.RSD_State)
    {
        MediaObj.RSD_Timer++;
        if(MediaObj.RSD_Timer > 180) //   ���״̬һֱ�ڵȴ��ҳ���30s�����״̬
        {
            switch (MediaObj.Media_Type)
            {
            case 0 : // ͼ��
                Photo_send_end();  // �����ϴ�����

                break;
            case 1 : // ��Ƶ
#ifdef REC_VOICE_ENABLE
                Sound_send_end();
#endif
                break;
            case 2 : // ��Ƶ
                //   Video_send_end();
                break;
            default:
                break;
            }
            Media_Clear_State();
            if(GB19056.workstate == 0)
                rt_kprintf("\r\n ��Ϣ�ش���ʱ����! \r\n");

            Check_MultiTakeResult_b4Trans();  // ��·����ͷ����״̬���

        }

    }




}
#ifdef   MEDIA
//-------- photo send -------------------------------------
void Photo_send_start(u16 Numpic)
{

    //  UINT ByteRead;
    // FIL FileCurrent;
    if(GB19056.workstate == 0)
        rt_kprintf("   \r\n  Photo_send_start =%d \r\n", Numpic);
    Photo_sdState.photo_sending = other;	//disable
    Photo_sdState.SD_flag = 0;
    Photo_sdState.SD_packetNum = 1; // ��1 ��ʼ
    Photo_sdState.Exeption_timer = 0;

    if(CameraState.Camera_Number == 1)
        PicFileSize = Api_DFdirectory_Query(camera_1, 1);
    else if(CameraState.Camera_Number == 2)
        PicFileSize = Api_DFdirectory_Query(camera_2, 1);
    else if(CameraState.Camera_Number == 3)
        PicFileSize = Api_DFdirectory_Query(camera_3, 1);
    else if(CameraState.Camera_Number == 4)
        PicFileSize = Api_DFdirectory_Query(camera_4, 1);


    // rt_kprintf("\r\n    open Pic =%s",PictureName);

    if(PicFileSize % 512)
        Photo_sdState.Total_packetNum = PicFileSize / 512 + 1;
    else
        Photo_sdState.Total_packetNum = PicFileSize / 512;

    if(GB19056.workstate == 0)
        rt_kprintf("\r\n    Camera %d  ReadpicStart total :%d ��Pagesize: %d Bytes\r\n\r\n", CameraState.Camera_Number, Photo_sdState.Total_packetNum, PicFileSize);
    if((CameraState.Camera_Number == 0) || (Photo_sdState.Total_packetNum == 0))
    {
        Photo_send_end(); // clear  state
        rt_kprintf("\r\n  ͼƬ�ܰ���Ϊ�� ������ͷ���Ϊ0 ����������ʧ�ܵ����� \r\n");
    }


    // -------  MultiMedia Related --------
    Media_Start_Init(0, 0); // 0: ͼƬ��ʽ 0: JPEG �ļ���ʽ

}

#ifdef REC_VOICE_ENABLE
u8  Sound_send_start(void)
{
    u8 sound_name[20];
    //  u16  i;
    //  u8  oldstate=0;
    //  u16 i2,j;
    // u8  WrieEnd=0,LeftLen=0;


    Sound_sdState.photo_sending = disable;
    Sound_sdState.SD_flag = 0;
    Sound_sdState.SD_packetNum = 1; // ��1 ��ʼ
    Sound_sdState.Exeption_timer = 0;

    //---  Speex_Init();	 // speachX ��ʼ��
    // 1. �������µ��ļ�
    /*  memset((u8*)&MediaIndex,0,sizeof(MediaIndex));
      for(i=0;i<8;i++)
      {
    	 Api_RecordNum_Read(voice_index, i+1, (u8*)&MediaIndex, sizeof(MediaIndex));
            if(MediaIndex.Effective_Flag==1)
            {
                break;
    	  }
      }
      if(MediaIndex.Effective_Flag)
      {
    	rt_kprintf("\r\n ����filename:%s\r\n",MediaIndex.FileName);
      }
      else
      	{
      	   rt_kprintf("\r\n û���Ѵ洢����Ƶ�ļ� \r\n");
      	   return false	;
      	}

    */

    //	2.	����wav �ļ�

    //	3. �ļ���С
    // file name
    memset(sound_name, 0, sizeof(sound_name));
    DF_ReadFlash(SoundStart_offdet, 4, sound_name, 20);
    SrcFileSize = Api_DFdirectory_Query(voice, 1);
    //  Sound_sdState.Total_packetNum=(SrcFileSize/512); // ÿ��100���ֽ�
    if(SrcFileSize % 512)
        Sound_sdState.Total_packetNum = SrcFileSize / 512 + 1;
    else
        Sound_sdState.Total_packetNum = SrcFileSize / 512;

    if(GB19056.workstate == 0)
        rt_kprintf("\r\n	�ļ���: %s��С: %d Bytes  totalpacketnum=%d \r\n", sound_name, SrcFileSize, Sound_sdState.Total_packetNum);

    // -------  MultiMedia Related --------
    Media_Start_Init(1, 3); // ��Ƶ  wav ��ʽ   0:JPEG ;   1: TIF ;   2:MP3;  3:WAV  4: WMV  ��������
    //    5   amr
    return true;
}
#endif

void Photo_send_TimeOut(void)
{
    if(Photo_sdState.photo_sending)
    {
        Photo_sdState.photo_sendTimeout++;
        if(Photo_sdState.photo_sendTimeout > 150)
        {
            Photo_sdState.photo_sendTimeout = 0;
            Photo_send_end();// clear
        }
    }
}


void Photo_send_end(void)
{
    Photo_sdState.photo_sending = 0;
    Photo_sdState.SD_flag = 0;
    Photo_sdState.SD_packetNum = 0;
    Photo_sdState.Total_packetNum = 0;
    Photo_sdState.Exeption_timer = 0;
    MediaObj.Media_transmittingFlag = 0; // clear
    Media_Clear_State();
}

#ifdef REC_VOICE_ENABLE
void Sound_send_end(void)
{
    Sound_sdState.photo_sending = 0;
    Sound_sdState.SD_flag = 0;
    Sound_sdState.SD_packetNum = 0;
    Sound_sdState.Total_packetNum = 0;
    Sound_sdState.Exeption_timer = 0;
    MediaObj.Media_transmittingFlag = 0; // clear
    mp3_sendstate = 0;
#ifdef REC_VOICE_ENABLE
    VocREC.running = 0; // clear
#endif
    Media_Clear_State();
}
#endif


#ifdef REC_VOICE_ENABLE
void Sound_Timer(void)
{

    if((Sound_sdState.photo_sending == enable) && (1 == MediaObj.Media_Type)) // ��Ƶ
    {
        if((Sound_sdState.SD_packetNum <= Sound_sdState.Total_packetNum + 1) && (0 == MediaObj.RSD_State))
        {
            //  һ�¶�ʱ����	��˳���͹�������	 ��   �յ��ش���ʼ����Ч
            Sound_sdState.Data_SD_counter++;
            if( Sound_sdState.Data_SD_counter > 14)
            {
                Sound_sdState.Data_SD_counter = 0;
                Sound_sdState.Exeption_timer = 0;
                Sound_sdState.SD_flag = 1;
                MediaObj.SD_Data_Flag = 1;

                //rt_kprintf("\r\n Sound  Transmit set Flag \r\n");
            }
        }
    }
}
#endif

void Photo_Timer(void)
{
    if((Photo_sdState.photo_sending == enable) && (0 == MediaObj.Media_Type))
    {
        if((Photo_sdState.SD_packetNum <= Photo_sdState.Total_packetNum + 1) && (0 == MediaObj.RSD_State))
        {
            //  һ�¶�ʱ����   ��˳���͹�������   ��   �յ��ش���ʼ����Ч
            Photo_sdState.Data_SD_counter++;
            if( Photo_sdState.Data_SD_counter > 14) //40   12
            {
                Photo_sdState.Data_SD_counter = 0;
                Photo_sdState.Exeption_timer = 0;
                Photo_sdState.SD_flag = 1;
                MediaObj.SD_Data_Flag = 1;
            }
        }
    }
}

void Meida_Trans_Exception(void)
{
    u8  resualt = 0;

    if(Photo_sdState.photo_sending == enable)
    {
        if( Photo_sdState.Exeption_timer++ > 50)
        {
            Photo_send_end();
            resualt = 1;
        }
    }
#ifdef REC_VOICE_ENABLE
    else if(Sound_sdState.photo_sending == enable)
    {
        if( Sound_sdState.Exeption_timer++ > 50)
        {
            Sound_send_end();
            resualt = 2;
        }
    }
#endif

    if(resualt)
        rt_kprintf("\r\n   Media  Trans  Timeout  resualt: %d\r\n", resualt);

}

void Media_Timer_Service(void)
{
    //----------------------------------
    if(DataLink_Status() && (DEV_Login.Operate_enable == 2))
    {
        if(Photo_sdState.photo_sending == enable)
            Photo_Timer();

#ifdef REC_VOICE_ENABLE
        else if(Sound_sdState.photo_sending == enable)
            Sound_Timer();
#endif
        // else
        // Video_Timer();
        Media_RSdMode_Timer();
    }
}

#endif

//------------------------------------------------------------
void DataTrans_Init(void)
{
    DataTrans.Data_RxLen = 0;
    DataTrans.Data_TxLen = 0;
    DataTrans.Tx_Wr = 0;
    memset(DataTrans.DataRx, 0, sizeof((const char *)DataTrans.DataRx));
    memset(DataTrans.Data_Tx, 0, sizeof((const char *)DataTrans.Data_Tx));
}
//------------------------------------------------------------
void DoorCameraInit(void)
{
    DoorOpen.currentState = 0;
    DoorOpen.BakState = 0;
}
//-----------------------------------------------------------
void Spd_ExpInit(void)
{
    speed_Exd.current_maxSpd = 0;
    speed_Exd.dur_seconds = 0;
    speed_Exd.excd_status = 0;
    memset((char *)(speed_Exd.ex_startTime), 0, 5);
    speed_Exd.speed_flag = 0;
}

//  �ӱ����ͨ��ý���¼���Ϣ�ϴ�����Ӧ�𲻺ã����Ե�������
void Multimedia_0800H_ACK_process(void)
{
    Media_Clear_State();  //  clear

    if(0 == MediaObj.Media_Type)
    {
        MediaObj.Media_transmittingFlag = 1;
        PositionSD_Enable();
        Current_UDP_sd = 1;

        Photo_sdState.photo_sending = enable;
        Photo_sdState.SD_packetNum = 1; // ��һ����ʼ
        PositionSD_Enable();  //   ʹ���ϱ�
        if(GB19056.workstate == 0)
            rt_kprintf("\r\n ��ʼ�ϴ���Ƭ! ....\r\n");
    }
    else if(1 == MediaObj.Media_Type)
    {
        MediaObj.Media_transmittingFlag = 1;

        Sound_sdState.photo_sending = enable;
        Sound_sdState.SD_packetNum = 1; // ��һ����ʼ
        PositionSD_Enable();  //   ʹ���ϱ�
        Current_UDP_sd = 1;
        if(GB19056.workstate == 0)
            rt_kprintf("\r\n ��ʼ�ϴ���Ƶ! ....\r\n");
    }
    /*	else
    	if(2==MediaObj.Media_Type)
    	{
    	   MediaObj.Media_transmittingFlag=1;
    	   PositionSD_Enable();  //   ʹ���ϱ�
    	   Current_UDP_sd=1;
    	   Video_sdState.photo_sending=enable;
    	   Video_sdState.SD_packetNum=1; // ��һ����ʼ
    	   rt_kprintf("\r\n ��ʼ�ϴ���Ƶ! ....\r\n");

        }
    */

}

u16  Instr_2_GBK(u8 *SrcINstr, u16 Inlen, u8 *DstOutstr )
{
    u16 i = 0, j = 0;


    //�Է�GBK���봦��------------------------------------
    for(i = 0, j = 0; i < Inlen; i++)
    {
        if((SrcINstr[i] >= 0xA1) && (SrcINstr[i + 1] >= 0xA0))
        {
            DstOutstr[j] = SrcINstr[i];
            DstOutstr[j + 1] = SrcINstr[i + 1];
            j += 2;
            i++;
        }
        else
        {
            DstOutstr[j] = ' ';
            DstOutstr[j + 1] = SrcINstr[i];
            j += 2;
        }
    }
    return   j;
}


//-----------------------------------------------------------
void TCP_RX_Process( u8  LinkNum)  //  ---- 808  ��׼Э��
{
    u16	i = 0, j = 0; //,DF_PageAddr;
    u16  infolen = 0, contentlen = 0;
    u8  ISP_judge_resualt = 0;
    u32 HardVersion = 0;
    // u8   ireg[5];
    u8   Ack_Resualt = 1;
    u16  Ack_CMDid_8001 = 0;
    u8   Total_ParaNum = 0;      // �������ò�������
    u8   Process_Resualt = 0; //  bit ��ʾ   bit0 ��ʾ 1  bit 1 ��ʾ2
    u8   ContentRdAdd = 0; // ��ǰ��ȡ���ĵ�ַ
    u8   SubInfolen = 0;   // ����Ϣ����
    u8   Reg_buf[22];
    u8   CheckResualt = 0;
    u32  reg_u32 = 0;
    u16  GB19056infolen = 0;
    //----------------      �г���¼��808 Э�� ���մ���   --------------------------

    //  0.  Decode
    Protocol_808_Decode();
    //  1.  fliter head
    if(UDP_HEX_Rx[0] != 0x7e)         //   ����ͷ
        return;
    //  2.  check Centre Ack
    Centre_CmdID = (UDP_HEX_Rx[1] << 8) + UDP_HEX_Rx[2]; // ���յ�������ϢID
    Centre_FloatID = (UDP_HEX_Rx[11] << 8) + UDP_HEX_Rx[12]; // ���յ�������Ϣ��ˮ��


    //  3.   get infolen    ( ����Ϊ��Ϣ��ĳ���)    ���ְ��Ļ�  ��Ϣͷ����Ϊ12 ��������У��ĳ��� =infolen+12
    //infolen =( u16 )((UDP_HEX_Rx[3]&0x3f) << 8 ) + ( u16 ) UDP_HEX_Rx[4];
    infolen = ( u16 )((UDP_HEX_Rx[3] & 0x03) << 8 ) + ( u16 ) UDP_HEX_Rx[4];
    contentlen = infolen + 12; //  ����У���ֽڵĳ���

    //  4.   Check  Fcs

    FCS_RX_UDP = 0;
    //nop;nop;
    for ( i = 0; i < (UDP_DecodeHex_Len - 3); i++ ) //������յ����ݵ�����
    {
        FCS_RX_UDP ^= UDP_HEX_Rx[1 + i];
    }
    //nop;
    // ------- FCS filter -----------------
    if( UDP_HEX_Rx[UDP_DecodeHex_Len - 2] != FCS_RX_UDP ) //�ж�У����
    {
        if(GB19056.workstate == 0)
        {
            rt_kprintf("\r\n  Protocolinfolen=%d   UDP_hexRx_len=%d", infolen, UDP_hexRx_len);
            rt_kprintf("\r\n808Э��У�����	  Caucate %x  ,RX  %x\r\n", FCS_RX_UDP, UDP_HEX_Rx[UDP_DecodeHex_Len - 2]);
        }
        // OutPrint_HEX("UDP_HEX_RX",UDP_HEX_Rx,UDP_DecodeHex_Len);
        //-----------------  memset  -------------------------------------
        memset(UDP_HEX_Rx, 0, sizeof(UDP_HEX_Rx));
        UDP_hexRx_len = 0;

        FCS_error_counter++;
        if(FCS_error_counter > 3)
        {
            redial();
            FCS_error_counter = 0;
        }
        return;
    }
    FCS_error_counter = 0; // clear
    //  else
    // rt_kprintf("\r\n 808Э��У����ȷ	  Caucate %x  ,RX  %x\r\n",FCS_RX_UDP,UDP_HEX_Rx[UDP_DecodeHex_Len-2]);

    //   5 .  Classify  Process
    if(GB19056.workstate == 0)
        rt_kprintf("\r\n           CentreCMD = 0x%X  \r\n", Centre_CmdID); // add for  debug

    switch(Centre_CmdID)
    {
    case  0x8001:  //ƽ̨ͨ��Ӧ��
        // ��û�зְ�����Ļ�  ��Ϣͷ��12  ��0��ʼ�����12���ֽ�����Ϣ�������

        //  13 14  ��Ӧ���ն���Ϣ��ˮ��
        //  15 16  ��Ӧ�ն˵���Ϣ
        Ack_CMDid_8001 = (UDP_HEX_Rx[15] << 8) + UDP_HEX_Rx[16];

        switch(Ack_CMDid_8001)   // �ж϶�Ӧ�ն���Ϣ��ID�����ִ���
        {
        case 0x0200:    //  ��Ӧλ����Ϣ��Ӧ��
            //----- �ж�ȷ���Ƿ�ɹ�
            if(0x00 != UDP_HEX_Rx[17])
                break;
            //--------------------------------
            if (Warn_Status[3] & 0x01)
            {
                StatusReg_WARN_Clear();
                f_Exigent_warning = 0;
                warn_flag = 0;
                Send_warn_times = 0;
                StatusReg_WARN_Clear();
                if(GB19056.workstate == 0)
                    rt_kprintf( "\r\n���������յ�Ӧ�𣬵����!\r\n");
            }

            if((Warn_Status[0] & 0x20) == 0x20)
            {
                Warn_Status[0] &= ~0x20;
                if(GB19056.workstate == 0)
                    rt_kprintf( "\r\n��ײ�෭�յ�Ӧ�𣬵����!\r\n");
            }


            //------------------------------------
            if(GB19056.workstate == 0)
                rt_kprintf( "\r\nCentre ACK!\r\n");

            //-------------------------------------------------------------------
            Api_cycle_Update();
            //--------------  ��ý���ϴ����  --------------
            if(MediaObj.Media_transmittingFlag == 1) // clear
            {
                MediaObj.Media_transmittingFlag = 2;
                if(Duomeiti_sdFlag == 1)
                {
                    Duomeiti_sdFlag = 0;
                    Media_Clear_State();
                    Photo_send_end();
#ifdef REC_VOICE_ENABLE
                    Sound_send_end();
#endif
                    //rt_kprintf("\r\n  �ֶ��ϱ���ý���ϴ�����\r\n");
                }
                //rt_kprintf("\r\n  ��ý����Ϣǰ�Ķ�ý�巢����� \r\n");
            }

            break;
        case 0x0002:  //  ��������Ӧ��
            //  �����н����  ---
            JT808Conf_struct.DURATION.TCP_ACK_DurCnter = 0; //clear
            JT808Conf_struct.DURATION.TCP_SD_state = 0; //clear
            if(GB19056.workstate == 0)
                rt_kprintf( "\r\n  Centre  Heart ACK!\r\n");
            break;
        case 0x0101:  //  �ն�ע��Ӧ��
        case 0x0003:
            if(0 == UDP_HEX_Rx[17])
            {
                // ע���ɹ�

                memset(Reg_buf, 0, sizeof(Reg_buf));
                memcpy(Reg_buf, JT808Conf_struct.ConfirmCode, 20);
                JT808Conf_struct.Regsiter_Status = 0;
                Reg_buf[20] = JT808Conf_struct.Regsiter_Status;
                Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
                rt_kprintf("\r\n  �ն�ע���ɹ�!  \r\n");
                redial();
                idip("clear");
            }

            break;
        case 0x0102:  //  �ն˼�Ȩ
            if(GB19056.workstate == 0)
                rt_kprintf("\r\n �յ���Ȩ���: %x \r\n", UDP_HEX_Rx[17]);
            if(0 == UDP_HEX_Rx[17])
            {
                // ��Ȩ�ɹ�
                DEV_Login.Operate_enable = 2; // ��Ȩ���
                if(DataLink_Status())
                    DataLinkOK_Process();
                rt_kprintf("\r\n  �ն˼�Ȩ�ɹ�!  \r\n");
                //  ��½����ʹ�ܷ���һ������
                JT808Conf_struct.DURATION.Heart_SDFlag = 1;
            }
            break;
        case 0x0800:  // ��ý���¼���Ϣ�ϴ�
            if(GB19056.workstate == 0)
                rt_kprintf("\r\n ��ý���¼���Ϣ�ϴ���Ӧ! \r\n");
            Media_Clear_State();  //  clear

            if(0 == MediaObj.Media_Type)
            {
                MediaObj.Media_transmittingFlag = 1;
                PositionSD_Enable();
                Current_UDP_sd = 1;

                Photo_sdState.photo_sending = enable;
                Photo_sdState.SD_packetNum = 1; // ��һ����ʼ
                PositionSD_Enable();  //   ʹ���ϱ�
                if(GB19056.workstate == 0)
                    rt_kprintf("\r\n ��ʼ�ϴ���Ƭ! ....\r\n");
            }
            else if(1 == MediaObj.Media_Type)
            {
                MediaObj.Media_transmittingFlag = 1;

                Sound_sdState.photo_sending = enable;
                Sound_sdState.SD_packetNum = 1; // ��һ����ʼ
                PositionSD_Enable();  //   ʹ���ϱ�
                Current_UDP_sd = 1;
                if(GB19056.workstate == 0)
                    rt_kprintf("\r\n ��ʼ�ϴ���Ƶ! ....\r\n");
            }
            /*else
            if(2==MediaObj.Media_Type)
            {
               MediaObj.Media_transmittingFlag=1;
               PositionSD_Enable();  //   ʹ���ϱ�
               Current_UDP_sd=1;
               Video_sdState.photo_sending=enable;
               Video_sdState.SD_packetNum=1; // ��һ����ʼ
               rt_kprintf("\r\n ��ʼ�ϴ���Ƶ! ....\r\n");
                                          }	*/
            break;
        case 0x0702:
            // rt_kprintf("\r\n  ��ʻԱ��Ϣ�ϱ�---����Ӧ��!  \r\n");

            break;
        case 0x0701:
            //  rt_kprintf("\r\n  �����˵��ϱ�---����Ӧ��!  \r\n");

            break;
        case 0x0704:
            if(GB19056.workstate == 0)
                rt_kprintf( "\r\n  0704H-ack  \r\n");
            //-----------------
            if(Send_Rdy4ok == 2)
            {
                Api_cycle_Update();
                Send_Rdy4ok = 0;
                ACK_timer = 0;
            }
            break;
        case 0x0705: 	//
            //			  rt_kprintf("\r\n can-ack");
            break;


        default	 :
            break;
        }


        //-------------------------------
        break;
    case  0x8100:    //  ������Ķ��ն�ע����Ϣ��Ӧ��
        //-----------------------------------------------------------
        switch(UDP_HEX_Rx[15])
        {
        case 0:
            rt_kprintf("\r\n   ----ע��ɹ�\r\n");
            memset(JT808Conf_struct.ConfirmCode, 0, sizeof(JT808Conf_struct.ConfirmCode));
            memcpy(JT808Conf_struct.ConfirmCode, UDP_HEX_Rx + 16, infolen - 3);

            memset(Reg_buf, 0, sizeof(Reg_buf));
            memcpy(Reg_buf, JT808Conf_struct.ConfirmCode, 20);
            JT808Conf_struct.Regsiter_Status = 1;
            Reg_buf[20] = JT808Conf_struct.Regsiter_Status;
            Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
            rt_kprintf("��Ȩ��: %s\r\n		   ��Ȩ�볤��: %d\r\n", JT808Conf_struct.ConfirmCode, strlen((const char *)JT808Conf_struct.ConfirmCode));
            //-------- ע��ɹ���������ʼ��Ȩ ------
            DEV_Login.Operate_enable = 1;
            DEV_Login.Sd_counter = 0;
            DEV_Login.Enable_sd = 1;
            break;
        case 1:
            rt_kprintf("\r\n   ----�����ѱ�ע��\r\n");
            break;
        case 2:
            rt_kprintf("\r\n   ----���ݿ����޸ó���\r\n");
            break;
        case 3:
            rt_kprintf("\r\n   ----�ն��ѱ�ע��\r\n");
            if(0 == JT808Conf_struct.Regsiter_Status)
            {
                ;//JT808Conf_struct.Regsiter_Status=2;  // not  1
                //DEV_regist.DeRegst_sd=1;
            }
            else if(1 == JT808Conf_struct.Regsiter_Status)
                DEV_Login.Operate_enable = 1; //��ʼ��Ȩ

            break;
        case 4:
            rt_kprintf("\r\n   ----���ݿ����޸��ն�\r\n");
            break;
        }
        break;
    case  0x8103:    //  �����ն˲���
        //  Ack_Resualt=0;
        Fail_Flag = 0;
        if(contentlen)
        {
            // �������������ÿ��ֻ�·�����һ������
            Total_ParaNum = UDP_HEX_Rx[13]; // �������ò�������
            if(GB19056.workstate == 0)
                rt_kprintf("\r\n Set ParametersNum =%d  \r\n", Total_ParaNum);
            //-------------------------------------------------------------------
            ContentRdAdd = 14;
            Process_Resualt = 0; // clear resualt
            for(i = 0; i < Total_ParaNum; i++)
            {
                //  ��������DWORD 4 ���ֽ�
                SubCMD_8103H = (UDP_HEX_Rx[ContentRdAdd] << 24) + (UDP_HEX_Rx[ContentRdAdd + 1] << 16) + (UDP_HEX_Rx[ContentRdAdd + 2] << 8) + UDP_HEX_Rx[ContentRdAdd + 3];
                //  ����Ϣ����
                SubInfolen = UDP_HEX_Rx[ContentRdAdd + 4];
                //  ��������Ϣ ������óɹ�����ӦBit λ��Ϊ 1 ���򱣳� 0
                if(CentreSet_subService_8103H(SubCMD_8103H, SubInfolen, UDP_HEX_Rx + ContentRdAdd + 5))
                    Process_Resualt |= (0x01 << i);
                //  �ƶ�ƫ�Ƶ�ַ
                ContentRdAdd += 5 + UDP_HEX_Rx[ContentRdAdd + 4]; // ƫ���±�
            }

            //--------------�ж����е����ý��  ---------------
            /* for(i=0;i<Total_ParaNum;i++)
             {
                 if(!((Process_Resualt>>0)&0x01))
                 	{
                      Ack_Resualt=1; //  1  ��ʾʧ��
                      break;
                 	}
            	 if(i==(Total_ParaNum-1))  //  ���õ����һ��ȷ�ϳɹ�
            	 	Ack_Resualt=0;  //  �ɹ�/ȷ��
             }*/

            if(Fail_Flag == 0)
                Ack_Resualt = 0;
            else
                Ack_Resualt = 1; // ����ʧ��

        }

        //-------------------------------------------------------------------
        if(SD_ACKflag.f_CentreCMDack_0001H == 2)
        {
            Ack_Resualt = 0;
            SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
        }
        else if(SD_ACKflag.f_CentreCMDack_0001H == 0)
        {
            SD_ACKflag.f_CentreCMDack_0001H = 1;
            SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
        }
        rt_kprintf("\r\n  Set Device !\r\n");

        break;
    case  0x8104:    //  ��ѯ�ն˲���
        SD_ACKflag.f_SettingPram_0104H = 1; // ����ʲô���ݻظ�ͳһ���
        rt_kprintf("\r\n  ���Ĳ�ѯ�ն˲��� !\r\n");
        break;
    case  0x8105:     // �ն˿���
        // Ack_Resualt=0;
        rt_kprintf("\r\ny  �ն˿��� -1!\r\n");
        if(contentlen)
        {
            // �������������ÿ��ֻ�·�����һ������
            Total_ParaNum = UDP_HEX_Rx[13]; //  �ն˿���������
            rt_kprintf("\r\n Set ParametersNum =%d  \r\n", Total_ParaNum);
            //-------------------------------------------------------------------
            if(CentreSet_subService_8105H(Total_ParaNum, contentlen - 1, UDP_HEX_Rx + 14))
                Ack_Resualt = 0; // ���سɹ�
        }

        //-------------------------------------------------------------------
        Ack_Resualt = 0;
        if(SD_ACKflag.f_CentreCMDack_0001H == 0)
        {
            SD_ACKflag.f_CentreCMDack_0001H = 1;
            SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
        }
        rt_kprintf("\r\ny  �ն˿��� !\r\n");

        break;
    case  0x8201:     // λ����Ϣ��ѯ    λ����Ϣ��ѯ��Ϣ��Ϊ��
        SD_ACKflag.f_CurrentPosition_0201H = 1;
        rt_kprintf("\r\n  λ����Ϣ��ѯ !\r\n");
        break;
    case  0x8202:     // ��ʱλ�ø��ٿ���
        Ack_Resualt = 0;

        //  13 14  ʱ����
        JT808Conf_struct.RT_LOCK.Lock_Dur = (UDP_HEX_Rx[13] << 8) + UDP_HEX_Rx[14];
        //  15 16  17 18 ��Ӧ�ն˵���Ϣ
        JT808Conf_struct.RT_LOCK.Lock_KeepDur = (UDP_HEX_Rx[15] << 24) + (UDP_HEX_Rx[16] << 16) + (UDP_HEX_Rx[17] << 8) + UDP_HEX_Rx[18];

        JT808Conf_struct.RT_LOCK.Lock_state = 1;  // Enable Flag
        JT808Conf_struct.RT_LOCK.Lock_KeepCnter = 0; //  ���ּ�����
        Current_SD_Duration = JT808Conf_struct.RT_LOCK.Lock_Dur; //���ķ��ͼ��

        JT808Conf_struct.SD_MODE.DUR_TOTALMODE = 1; // ���¶�ʱ���״̬λ
        JT808Conf_struct.SD_MODE.Dur_DefaultMode = 1;
        //  ��������
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));

        // if(SD_ACKflag.f_CentreCMDack_0001H==0)
        {
            Ack_Resualt = 0;
            SD_ACKflag.f_CentreCMDack_0001H = 1;
            SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
        }
        rt_kprintf("\r\n  ��ʱλ�ø��ٿ���!\r\n");
        break;
    case  0x8300:    //  �ı���Ϣ�·�
        Ack_Resualt = 0;
        TextInfo.TEXT_FLAG = UDP_HEX_Rx[13];
        if(TextInfo.TEXT_FLAG & 0x09) // ����Ƿ��TTS�ն�  ������Ҳ��TTS����
        {


            //  TTS
            TTS_Get_Data(UDP_HEX_Rx + 14, infolen - 1);

        }
        if((TextInfo.TEXT_FLAG & 0x04) || (TextInfo.TEXT_FLAG & 0x01)) // ����Ƿ���ն���ʾ��
        {
            //    1. ������������
            if(( strncmp( (char *)UDP_HEX_Rx + 14, "TW703#", 6 ) == 0 ) || ( strncmp( (char *)UDP_HEX_Rx + 14, "TW705#", 6 ) == 0 ))                                        //�����޸�UDP��IP�Ͷ˿�
            {
                //-----------  �Զ��� ��Ϣ�����޸� Э�� ----------------------------------
                SMS_protocol( (UDP_HEX_Rx + 14) + 5, (infolen - 1) - 5 , SMS_ACK_none);
            }
            else
            {
                //     2. ��������
                memset( TextInfo.TEXT_Content, 0, sizeof(TextInfo.TEXT_Content));
                memcpy(TextInfo.TEXT_Content, UDP_HEX_Rx + 14, infolen - 1);
                //---------------------
                //�Է�GBK���봦��------------------------------------
                contentlen = Instr_2_GBK(UDP_HEX_Rx + 14, infolen - 1, TextInfo.TEXT_Content);
                //-------------
                TextInfo.TEXT_SD_FLAG = 1;	// �÷��͸���ʾ����־λ  // ||||||||||||||||||||||||||||||||||

                //========================================
                TextInforCounter++;
                rt_kprintf("\r\nд���յ��ĵ� %d ����Ϣ,��Ϣ����=%d,��Ϣ:%s", TextInforCounter, contentlen, TextInfo.TEXT_Content);
                TEXTMSG_Write(TextInforCounter, 1, contentlen, TextInfo.TEXT_Content);
            }
            //========================================
        }
        //------- ���� ----
        //  if(SD_ACKflag.f_CentreCMDack_0001H==0)
        {
            Ack_Resualt = 0;
            SD_ACKflag.f_CentreCMDack_0001H = 1;
            SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
        }
        rt_kprintf("\r\n �ı���Ϣ: %s\r\n", TextInfo.TEXT_Content);
        break;
    case  0x8301:    //  �¼�����
        if(contentlen)
        {
            //--- ��������--
            switch(UDP_HEX_Rx[13])
            {
            case 0 :  //  ɾ���ն����������¼���������󲻴�����ַ�
                Event_Init(1);
                // rt_kprintf("\r\n ɾ�������¼�\r\n");
                break;
            case 1:  // �����¼�
                //if(UDP_HEX_Rx[13]==1)
                //	rt_kprintf("\r\n �����¼�\r\n");
                //break;
            case 2:  // ׷���¼�
                // if(UDP_HEX_Rx[13]==2)
                // 	rt_kprintf("\r\n ׷���¼�\r\n");
                //break;
            case 3:  // �޸��¼�
                //  if(UDP_HEX_Rx[13]==3)
                // rt_kprintf("\r\n �޸��¼�\r\n");
                //break;
            case 4:  // ɾ���ض��¼�
                // if(UDP_HEX_Rx[13]==4)
                //	 rt_kprintf("\r\n ɾ���ص��¼�\r\n");
                Total_ParaNum = UDP_HEX_Rx[14]; // �������ò�������
                rt_kprintf("\r\n Set ParametersNum =%d  \r\n", Total_ParaNum);
                if(Total_ParaNum != 1)
                    break;
                //-------------------------------
                if((UDP_HEX_Rx[15] > 8) && (UDP_HEX_Rx[15] == 0))
                    EventObj.Event_ID = 0;
                else
                    EventObj.Event_ID = UDP_HEX_Rx[15];

                // EventObj.Event_Len=UDP_HEX_Rx[16];
                memset(EventObj.Event_Str, 0, sizeof(EventObj.Event_Str));
                //----  Instr  ת GBK ----------------------
                EventObj.Event_Len = Instr_2_GBK(UDP_HEX_Rx + 17, UDP_HEX_Rx[16], EventObj.Event_Str);
                EventObj.Event_Effective = 1;
                Api_RecordNum_Write(event_808, EventObj.Event_ID, (u8 *)&EventObj, sizeof(EventObj));
                rt_kprintf("\r\n �¼�����:%s\r\n", EventObj.Event_Str);
                rt_kprintf("\r\n �¼�����:%s\r\n", EventObj.Event_Str);
                break;
            default:
                break;

            }

            //---------���� -------
            // if(SD_ACKflag.f_CentreCMDack_0001H==0) // һ��ظ�
            {
                SD_ACKflag.f_CentreCMDack_0001H = 1;
                Ack_Resualt = 0;
                SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
            }
            /*
            if(SD_ACKflag.f_CurrentEventACK_0301H==0)
            {
              SD_ACKflag.f_CurrentEventACK_0301H=1;
            }
                */
        }

        break;
    case  0x8302:    // �����·�
        if(UDP_HEX_Rx[13] & 0x04) // ����־�Ƿ����ʾ�ն�
        {
            if(TextInfo.TEXT_FLAG & 0x09) // ����Ƿ��TTS�ն�  ������Ҳ��TTS����
            {

                //  TTS
                TTS_Get_Data(UDP_HEX_Rx + 15, infolen - 1);

            }
        }
        rt_kprintf("\r\n  �����·����� \r\n");
        {
            //ASK_Centre.ASK_infolen=UDP_HEX_Rx[14];
            memset(ASK_Centre.ASK_info, 0, sizeof(ASK_Centre.ASK_info));
            //----  Instr  ת GBK ----------------------
            ASK_Centre.ASK_infolen = Instr_2_GBK(UDP_HEX_Rx + 15, UDP_HEX_Rx[14], ASK_Centre.ASK_info);
            // memcpy(ASK_Centre.ASK_info,UDP_HEX_Rx+15,ASK_Centre.ASK_infolen);
            rt_kprintf("\r\n  ����: %s \r\n", ASK_Centre.ASK_info);
            memset(ASK_Centre.ASK_answer, 0, sizeof(ASK_Centre.ASK_answer));
            //----  Instr  ת GBK ----------------------
            contentlen = Instr_2_GBK(UDP_HEX_Rx + 15 + UDP_HEX_Rx[14], infolen - 2 - UDP_HEX_Rx[14], ASK_Centre.ASK_answer);
            // memcpy(ASK_Centre.ASK_answer,UDP_HEX_Rx+15+ASK_Centre.ASK_infolen,infolen-2-ASK_Centre.ASK_infolen);

            ASK_Centre.ASK_SdFlag = 1; // ||||||||||||||||||||||||||||||||||
            ASK_Centre.ASK_floatID = Centre_FloatID; // ���� FloatID

            rt_kprintf("\r\n ����Answer:%s\r\n", ASK_Centre.ASK_answer + 3);
            Api_RecordNum_Write(ask_quesstion, 1, (u8 *)&ASK_Centre, sizeof(ASK_Centre));

            ASK_Centre.ASK_disp_Enable = 1;

        }

        // if(SD_ACKflag.f_CentreCMDack_0001H==0)
        {
            Ack_Resualt = 0;
            SD_ACKflag.f_CentreCMDack_0001H = 1;
            SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
        }


        break;
    case  0x8303:    //  ��Ϣ�㲥�˵�����
        //--- ��������--
        switch(UDP_HEX_Rx[13])
        {
        case 0 :  //  ɾ���ն�����������Ϣ
            MSG_BroadCast_Init(1);
            rt_kprintf("\r\n ɾ����Ϣ\r\n");
            break;
        case 1:  // ���²˵�
            if(UDP_HEX_Rx[13] == 1)
                rt_kprintf("\r\n ���²˵�\r\n");
            //break;
        case 2:  // ׷�Ӳ˵�
            if(UDP_HEX_Rx[13] == 2)
                rt_kprintf("\r\n ׷�Ӳ˵�\r\n");
            //break;
        case 3:  // �޸Ĳ˵�
            if(UDP_HEX_Rx[13] == 3)
                rt_kprintf("\r\n �޸Ĳ˵�\r\n");
            Total_ParaNum = UDP_HEX_Rx[14];         // ��Ϣ������
            rt_kprintf("\r\n Set ParametersNum =%d  \r\n", Total_ParaNum);
            if(Total_ParaNum != 1)
                break;
            //-------------------------------
            if((UDP_HEX_Rx[15] > 8) && (UDP_HEX_Rx[15] == 0))
                MSG_BroadCast_Obj.INFO_TYPE = 0;
            else
                MSG_BroadCast_Obj.INFO_TYPE = UDP_HEX_Rx[15];

            contentlen = (UDP_HEX_Rx[16] << 8) + UDP_HEX_Rx[17];
            memset(MSG_BroadCast_Obj.INFO_STR, 0, sizeof(MSG_BroadCast_Obj.INFO_STR));
            //----  Instr  ת GBK ----------------------
            MSG_BroadCast_Obj.INFO_LEN = Instr_2_GBK(UDP_HEX_Rx + 18, contentlen, MSG_BroadCast_Obj.INFO_STR);
            //memcpy(MSG_BroadCast_Obj.INFO_STR,UDP_HEX_Rx+18,MSG_BroadCast_Obj.INFO_LEN);
            MSG_BroadCast_Obj.INFO_Effective = 1;
            MSG_BroadCast_Obj.INFO_PlyCancel = 1;
            Api_RecordNum_Write(msg_broadcast, MSG_BroadCast_Obj.INFO_TYPE, (u8 *)&MSG_BroadCast_Obj, sizeof(MSG_BroadCast_Obj));
            rt_kprintf("\r\n ��Ϣ�㲥����:%s\r\n", MSG_BroadCast_Obj.INFO_STR);
            break;
        default:
            break;

        }

        //---------���� -------
        // if(SD_ACKflag.f_CentreCMDack_0001H==0) // һ��ظ�
        {
            SD_ACKflag.f_CentreCMDack_0001H = 1;
            Ack_Resualt = 0;
            SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
        }

        /*
                            if(SD_ACKflag.f_MsgBroadCast_0303H==0)
         {
             SD_ACKflag.f_MsgBroadCast_0303H=1;
                     }
                     */
        break;
    case  0x8304:    //  ��Ϣ����
        Ack_Resualt = 0;
        MSG_BroadCast_Obj.INFO_TYPE = UDP_HEX_Rx[13]; //  ��Ϣ����
        MSG_BroadCast_Obj.INFO_LEN = (UDP_HEX_Rx[14] << 8) + UDP_HEX_Rx[15];
        memset(MSG_BroadCast_Obj.INFO_STR, 0, sizeof(MSG_BroadCast_Obj.INFO_STR));

        //----  Instr  ת GBK ----------------------
        // contentlen=Instr_2_GBK(UDP_HEX_Rx+16,infolen-3,TextInfo.TEXT_Content);
        memcpy(MSG_BroadCast_Obj.INFO_STR, UDP_HEX_Rx + 16, infolen - 3);


        MSG_BroadCast_Obj.INFO_SDFlag = 1;  // ||||||||||||||||||||||||||||||||||
        Api_RecordNum_Write(msg_broadcast, MSG_BroadCast_Obj.INFO_TYPE, (u8 *)&MSG_BroadCast_Obj, sizeof(MSG_BroadCast_Obj));



        // --------  ���͸��ı���Ϣ  --------------
        memset( TextInfo.TEXT_Content, 0, sizeof(TextInfo.TEXT_Content));

        //----  Instr  ת GBK ----------------------
        contentlen = Instr_2_GBK(UDP_HEX_Rx + 16, infolen - 3, TextInfo.TEXT_Content);
        // memcpy(TextInfo.TEXT_Content,UDP_HEX_Rx+16,infolen-3);
        TextInfo.TEXT_SD_FLAG = 1;  // �÷��͸���ʾ����־λ	// ||||||||||||||||||||||||||||||||||


        rt_kprintf("\r\n ��Ϣ��������:%s\r\n", TextInfo.TEXT_Content);
        //------- ���� ----
        //  if(SD_ACKflag.f_CentreCMDack_0001H==0)
        {
            Ack_Resualt = 0;
            SD_ACKflag.f_CentreCMDack_0001H = 1;
            SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
        }
        break;
    case  0x8400:    //  �绰�ز�

        if(infolen == 0)
            break;
        if(0 == UDP_HEX_Rx[13]) // ��ͨͨ��
        {
            Speak_ON;
            rt_kprintf("\r\n   �绰�ز�-->��ͨͨ��\r\n");
        }
        else if(1 == UDP_HEX_Rx[13]) //  ����
        {
            Speak_OFF;
            rt_kprintf("\r\n   �绰�ز�-->����");
        }
        else
            break;
        memset(JT808Conf_struct.LISTEN_Num, 0, sizeof(JT808Conf_struct.LISTEN_Num));
        memcpy(JT808Conf_struct.LISTEN_Num, UDP_HEX_Rx + 14, infolen - 1);
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        CallState = CallState_rdytoDialLis; // ׼����ʼ�����������

        // if(SD_ACKflag.f_CentreCMDack_0001H==0)
        {
            SD_ACKflag.f_CentreCMDack_0001H = 1;
            SD_ACKflag.f_CentreCMDack_resualt = 0;
        }
        break;
    case  0x8401:  //   ���õ绰��

        switch(UDP_HEX_Rx[13])
        {
        case 0 :  //  ɾ���ն�����������Ϣ
            // PhoneBook_Init(1);
            rt_kprintf("\r\n ɾ���绰��\r\n");
            break;
        case 1:  // ���²˵�
            if(UDP_HEX_Rx[13] == 1)
                rt_kprintf("\r\n ���µ绰��\r\n");
        case 3:  // �޸Ĳ˵�
            if(UDP_HEX_Rx[13] == 3)
                rt_kprintf("\r\n �޸ĵ绰��\r\n");
            Rx_PhoneBOOK.CALL_TYPE = UDP_HEX_Rx[15]; // ��־ ���������
            Rx_PhoneBOOK.NumLen = UDP_HEX_Rx[16];
            memset(Rx_PhoneBOOK.NumberStr, 0, sizeof(Rx_PhoneBOOK.NumberStr));
            memcpy(Rx_PhoneBOOK.NumberStr, UDP_HEX_Rx + 17, Rx_PhoneBOOK.NumLen);

            contentlen = UDP_HEX_Rx[17 + Rx_PhoneBOOK.NumLen];
            memset(Rx_PhoneBOOK.UserStr, 0, sizeof(Rx_PhoneBOOK.UserStr));
            //----  Instr  ת GBK ----------------------
            Rx_PhoneBOOK.UserLen = Instr_2_GBK(UDP_HEX_Rx + 18 + Rx_PhoneBOOK.NumLen, contentlen, Rx_PhoneBOOK.UserStr);
            //memcpy(Rx_PhoneBOOK.UserStr,UDP_HEX_Rx+18+Rx_PhoneBOOK.NumLen,Rx_PhoneBOOK.UserLen);

            for(i = 0; i < 8; i++)
            {
                PhoneBook.CALL_TYPE = 2; //���Ͷ���Ϊ���
                PhoneBook.NumLen = 0;  // ���볤��
                memset(PhoneBook.NumberStr, 0, sizeof(PhoneBook.NumberStr));
                PhoneBook.UserLen = 0;
                memset(PhoneBook.UserStr, 0, sizeof(PhoneBook.UserStr));
                Api_RecordNum_Write(phonebook, i + 1, (u8 *)&PhoneBook, sizeof(PhoneBook));
                if(strncmp((char *)PhoneBook.UserStr, (const char *)Rx_PhoneBOOK.UserStr, Rx_PhoneBOOK.UserLen) == 0)
                {
                    // �ҵ���ͬ���ֵİ���ǰ��ɾ�����µĴ���
                    Api_RecordNum_Write(phonebook, i + 1, (u8 *)&Rx_PhoneBOOK, sizeof(Rx_PhoneBOOK));
                    break;  // ����for
                }

            }
            break;
        case 2:  // ׷�Ӳ˵�
            if(UDP_HEX_Rx[13] == 2)
                rt_kprintf("\r\n ׷�ӵ绰��\r\n");
            Rx_PhoneBOOK.CALL_TYPE = UDP_HEX_Rx[15]; // ��־ ���������
            Rx_PhoneBOOK.NumLen = UDP_HEX_Rx[16];
            memset(Rx_PhoneBOOK.NumberStr, 0, sizeof(Rx_PhoneBOOK.NumberStr));
            memcpy(Rx_PhoneBOOK.NumberStr, UDP_HEX_Rx + 17, Rx_PhoneBOOK.NumLen);
            //  Rx_PhoneBOOK.UserLen=UDP_HEX_Rx[17+Rx_PhoneBOOK.NumLen];
            //  memset(Rx_PhoneBOOK.UserStr,0,sizeof(Rx_PhoneBOOK.UserStr));
            Rx_PhoneBOOK.Effective_Flag = 1; // ��Ч��־λ
            // memcpy(Rx_PhoneBOOK.UserStr,UDP_HEX_Rx+18+Rx_PhoneBOOK.NumLen,Rx_PhoneBOOK.UserLen);
            //---------------------------------------------------
            contentlen = UDP_HEX_Rx[17 + Rx_PhoneBOOK.NumLen];
            memset(Rx_PhoneBOOK.UserStr, 0, sizeof(Rx_PhoneBOOK.UserStr));
            //----  Instr  ת GBK ----------------------
            Rx_PhoneBOOK.UserLen = Instr_2_GBK(UDP_HEX_Rx + 18 + Rx_PhoneBOOK.NumLen, contentlen, Rx_PhoneBOOK.UserStr);
            //memcpy(Rx_PhoneBOOK.UserStr,UDP_HEX_Rx+18+Rx_PhoneBOOK.NumLen,Rx_PhoneBOOK.UserLen);
            //-------------------------------------------

            Api_RecordNum_Write(phonebook, UDP_HEX_Rx[14], (u8 *)&Rx_PhoneBOOK, sizeof(Rx_PhoneBOOK));
            rt_kprintf("\r\n Name:%s\r\n", Rx_PhoneBOOK.UserStr);
            rt_kprintf("\r\n Number:%s\r\n", Rx_PhoneBOOK.NumberStr);
            break;
        default:
            break;

        }


        // if(SD_ACKflag.f_CentreCMDack_0001H==0)
        {
            SD_ACKflag.f_CentreCMDack_0001H = 1;
            Ack_Resualt = 0;
            SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
        }

        break;
    case  0x8500:    //  ��������
        Vech_Control.Control_Flag = UDP_HEX_Rx[13];
        if(UDP_HEX_Rx[13] & 0x01)
        {
            // ���ż���       bit 12
            Car_Status[2] |= 0x10;   // ��Ҫ���Ƽ̵���
            rt_kprintf("\r\n  �������� \r\n");
        }
        else
        {
            // ���Ž���
            Car_Status[2] &= ~0x10;  // ��Ҫ���Ƽ̵���
            rt_kprintf("\r\n  �������� \r\n");
        }
        Vech_Control.ACK_SD_Flag = 1;
        break;
    case  0x8600:    //  ����Բ������
        rt_kprintf("\r\n  ����Բ������ \r\n");
        if(UDP_HEX_Rx[14] == 1) //  ����֧������һ������
        {
            switch(UDP_HEX_Rx[13])
            {
            case 1:  // ׷������
                for(i = 0; i < 8; i++)
                {
                    memset((u8 *)&Rail_Cycle, 0, sizeof(Rail_Cycle));
                    Api_RecordNum_Write(Rail_cycle, Rail_Cycle.Area_ID, (u8 *)&Rail_Cycle, sizeof(Rail_Cycle));
                    Rails_Routline_Read();
                    if(Rail_Cycle.Area_attribute) // �ҳ���δʹ�õ�
                        break;

                }
                if(8 == i) //  ��������ˣ���ô�� 0
                {
                    i = 0;
                }

            case 0:  // ��������
            case 2:  // �޸�����
                memset((u8 *)&Rail_Cycle, 0, sizeof(Rail_Cycle));
                Rail_Cycle.Area_ID = (UDP_HEX_Rx[15] << 24) + (UDP_HEX_Rx[16] << 16) + (UDP_HEX_Rx[17] << 8) + UDP_HEX_Rx[18];
                Rail_Cycle.Area_attribute = (UDP_HEX_Rx[19] << 8) + UDP_HEX_Rx[20];
                Rail_Cycle.Center_Latitude = (UDP_HEX_Rx[21] << 24) + (UDP_HEX_Rx[22] << 16) + (UDP_HEX_Rx[23] << 8) + UDP_HEX_Rx[24];
                Rail_Cycle.Center_Longitude = (UDP_HEX_Rx[25] << 24) + (UDP_HEX_Rx[26] << 16) + (UDP_HEX_Rx[27] << 8) + UDP_HEX_Rx[28];
                Rail_Cycle.Radius = (UDP_HEX_Rx[29] << 24) + (UDP_HEX_Rx[30] << 16) + (UDP_HEX_Rx[31] << 8) + UDP_HEX_Rx[32];
                memcpy(Rail_Cycle.StartTimeBCD, UDP_HEX_Rx + 33, 6);
                memcpy(Rail_Cycle.EndTimeBCD, UDP_HEX_Rx + 39, 6);
                Rail_Cycle.MaxSpd = (UDP_HEX_Rx[45] << 8) + UDP_HEX_Rx[46];
                Rail_Cycle.KeepDur = UDP_HEX_Rx[47];
                Rail_Cycle.Effective_flag = 1;

                if((Rail_Cycle.Area_ID > 8) || (Rail_Cycle.Area_ID == 0))
                    Rail_Cycle.Area_ID = 1;
                Api_RecordNum_Write(Rail_cycle, Rail_Cycle.Area_ID, (u8 *)&Rail_Cycle, sizeof(Rail_Cycle));

                Rails_Routline_Read();
                break;
            default:
                break;

            }

        }
        //------- ���� ----
        //   if(SD_ACKflag.f_CentreCMDack_0001H==0)
        {
            SD_ACKflag.f_CentreCMDack_0001H = 1;
            Ack_Resualt = 0;
            SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
        }
        break;
    case  0x8601:    //  ɾ��Բ������
        rt_kprintf("\r\n  ɾ��Բ������ \r\n");
        if(0 == UDP_HEX_Rx[13]) // ������
            RailCycle_Init();  // ɾ����������
        else
        {
            memset((u8 *)&Rail_Cycle, 0, sizeof(Rail_Cycle)); //  clear all  first
            for(i = 0; i < UDP_HEX_Rx[13]; i++)
            {
                Rail_Cycle.Area_ID = (UDP_HEX_Rx[14 + i] << 24) + (UDP_HEX_Rx[15 + i] << 16) + (UDP_HEX_Rx[16 + i] << 8) + UDP_HEX_Rx[17 + i];
                if((Rail_Cycle.Area_ID > 8) || (Rail_Cycle.Area_ID == 0))
                    Rail_Cycle.Area_ID = 1;
                Rail_Cycle.Effective_flag = 0; // clear
                Api_RecordNum_Write(Rail_cycle, Rail_Cycle.Area_ID, (u8 *)&Rail_Cycle, sizeof(Rail_Cycle)); // ɾ����Ӧ��Χ��
                Rails_Routline_Read();
            }

        }

        //----------------
        //   if(SD_ACKflag.f_CentreCMDack_0001H==0)
        {
            SD_ACKflag.f_CentreCMDack_0001H = 1;
            Ack_Resualt = 0;
            SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
        }
        break;
    case  0x8602:    //  ���þ�������
        rt_kprintf("\r\n  ���þ������� \r\n");
        if(UDP_HEX_Rx[14] == 1) //  ����֧������һ������
        {
            switch(UDP_HEX_Rx[13])
            {
            case 1:  // ׷������
            case 0:  // ��������
            case 2:  // �޸�����
                memset((u8 *)&Rail_Rectangle, 0, sizeof(Rail_Rectangle));
                Rail_Rectangle.Area_ID = (UDP_HEX_Rx[15] << 24) + (UDP_HEX_Rx[16] << 16) + (UDP_HEX_Rx[17] << 8) + UDP_HEX_Rx[18];
                Rail_Rectangle.Area_attribute = (UDP_HEX_Rx[19] << 8) + UDP_HEX_Rx[20];
                Rail_Rectangle.LeftUp_Latitude = (UDP_HEX_Rx[21] << 24) + (UDP_HEX_Rx[22] << 16) + (UDP_HEX_Rx[23] << 8) + UDP_HEX_Rx[24];
                Rail_Rectangle.LeftUp_Longitude = (UDP_HEX_Rx[25] << 24) + (UDP_HEX_Rx[26] << 16) + (UDP_HEX_Rx[27] << 8) + UDP_HEX_Rx[28];
                Rail_Rectangle.RightDown_Latitude = (UDP_HEX_Rx[29] << 24) + (UDP_HEX_Rx[30] << 16) + (UDP_HEX_Rx[31] << 8) + UDP_HEX_Rx[32];
                Rail_Rectangle.RightDown_Longitude = (UDP_HEX_Rx[33] << 24) + (UDP_HEX_Rx[34] << 16) + (UDP_HEX_Rx[35] << 8) + UDP_HEX_Rx[36];
                memcpy(Rail_Rectangle.StartTimeBCD, UDP_HEX_Rx + 37, 6);
                memcpy(Rail_Rectangle.EndTimeBCD, UDP_HEX_Rx + 43, 6);
                Rail_Rectangle.MaxSpd = (UDP_HEX_Rx[49] << 8) + UDP_HEX_Rx[50];
                Rail_Rectangle.KeepDur = UDP_HEX_Rx[51];
                Rail_Rectangle.Effective_flag = 1;

                if((Rail_Rectangle.Area_ID > 8) || (Rail_Rectangle.Area_ID == 0))
                    Rail_Rectangle.Area_ID = 1;
                Api_RecordNum_Write(Rail_rect, Rail_Rectangle.Area_ID, (u8 *)&Rail_Rectangle, sizeof(Rail_Rectangle));

                rt_kprintf("\r\n   ��������  ����Χ�� leftLati=%d leftlongi=%d\r\n", Rail_Rectangle.LeftUp_Latitude, Rail_Rectangle.LeftUp_Longitude);

                rt_kprintf("\r\n    attribute:%X         ����Χ�� rightLati=%d rightlongi=%d\r\n", Rail_Rectangle.Area_attribute, Rail_Rectangle.RightDown_Latitude, Rail_Rectangle.RightDown_Longitude);
                Rails_Routline_Read();
                break;
            default:
                break;

            }

        }
        //----------------
        //if(SD_ACKflag.f_CentreCMDack_0001H==0)
        {
            SD_ACKflag.f_CentreCMDack_0001H = 1;
            Ack_Resualt = 0;
            SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
        }
        break;
    case  0x8603:    //  ɾ����������
        rt_kprintf("\r\n  ɾ���������� \r\n");
        if(0 == UDP_HEX_Rx[13]) // ������
        {
            RailRect_Init();  // ɾ����������
            Rails_Routline_Read();
        }
        else
        {
            memset((u8 *)&Rail_Rectangle, 0, sizeof(Rail_Rectangle)); //  clear all  first
            for(i = 0; i < UDP_HEX_Rx[13]; i++)
            {
                Rail_Rectangle.Area_ID = (UDP_HEX_Rx[14 + i] << 24) + (UDP_HEX_Rx[15 + i] << 16) + (UDP_HEX_Rx[16 + i] << 8) + UDP_HEX_Rx[17 + i];
                if((Rail_Rectangle.Area_ID > 8) || (Rail_Rectangle.Area_ID == 0))
                    Rail_Rectangle.Area_ID = 1;
                Rail_Rectangle.Effective_flag = 0;
                Api_RecordNum_Write(Rail_rect, Rail_Rectangle.Area_ID, (u8 *)&Rail_Rectangle, sizeof(Rail_Rectangle)); // ɾ����Ӧ��Χ��
                Rails_Routline_Read();
            }
        }


        //----------------
        //  if(SD_ACKflag.f_CentreCMDack_0001H==0)
        {
            SD_ACKflag.f_CentreCMDack_0001H = 1;
            Ack_Resualt = 0;
            SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
        }
        break;
    case  0x8604:	//  ���������
        rt_kprintf("\r\n  ���ö�������� \r\n");
        if(UDP_HEX_Rx[14] == 1) //  ����֧������һ������
        {
            switch(UDP_HEX_Rx[13])
            {
            case 1:  // ׷������
            case 0:  // ��������
            case 2:  // �޸�����
                memset((u8 *)&Rail_Polygen, 0, sizeof(Rail_Polygen));
                Rail_Polygen.Area_ID = (UDP_HEX_Rx[15] << 24) + (UDP_HEX_Rx[16] << 16) + (UDP_HEX_Rx[17] << 8) + UDP_HEX_Rx[18];
                Rail_Polygen.Area_attribute = (UDP_HEX_Rx[19] << 8) + UDP_HEX_Rx[20];
                memcpy(Rail_Polygen.StartTimeBCD, UDP_HEX_Rx + 20, 6);
                memcpy(Rail_Polygen.EndTimeBCD, UDP_HEX_Rx + 26, 6);
                Rail_Polygen.MaxSpd = (UDP_HEX_Rx[32] << 8) + UDP_HEX_Rx[33];
                Rail_Polygen.KeepDur = UDP_HEX_Rx[34];
                Rail_Polygen.Acme_Num = UDP_HEX_Rx[35];
                Rail_Polygen.Acme1_Latitude = (UDP_HEX_Rx[36] << 24) + (UDP_HEX_Rx[37] << 16) + (UDP_HEX_Rx[38] << 8) + UDP_HEX_Rx[39];
                Rail_Polygen.Acme1_Longitude = (UDP_HEX_Rx[40] << 24) + (UDP_HEX_Rx[41] << 16) + (UDP_HEX_Rx[42] << 8) + UDP_HEX_Rx[43];
                Rail_Polygen.Acme2_Latitude = (UDP_HEX_Rx[44] << 24) + (UDP_HEX_Rx[45] << 16) + (UDP_HEX_Rx[46] << 8) + UDP_HEX_Rx[47];
                Rail_Polygen.Acme2_Longitude = (UDP_HEX_Rx[48] << 24) + (UDP_HEX_Rx[49] << 16) + (UDP_HEX_Rx[50] << 8) + UDP_HEX_Rx[51];
                Rail_Polygen.Acme3_Latitude = (UDP_HEX_Rx[52] << 24) + (UDP_HEX_Rx[53] << 16) + (UDP_HEX_Rx[54] << 8) + UDP_HEX_Rx[55];
                Rail_Polygen.Acme3_Longitude = (UDP_HEX_Rx[56] << 24) + (UDP_HEX_Rx[57] << 16) + (UDP_HEX_Rx[58] << 8) + UDP_HEX_Rx[59];

                if((Rail_Polygen.Area_ID > 8) || (Rail_Polygen.Area_ID == 0))
                    Rail_Polygen.Area_ID = 1;
                Rail_Polygen.Effective_flag = 1;
                Api_RecordNum_Write(Rail_polygen, Rail_Polygen.Area_ID, (u8 *)&Rail_Polygen, sizeof(Rail_Polygen));
                break;
            default:
                break;

            }

        }

        //----------------
        // if(SD_ACKflag.f_CentreCMDack_0001H==0)
        {
            SD_ACKflag.f_CentreCMDack_0001H = 1;
            Ack_Resualt = 0;
            SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
        }
        break;
    case  0x8605:    //  ɾ���������
        rt_kprintf("\r\n  ɾ����������� \r\n");
        if(0 == UDP_HEX_Rx[13]) // ������
            RailPolygen_Init();  // ɾ����������
        else
        {
            memset((u8 *)&Rail_Polygen, 0, sizeof(Rail_Polygen)); //  clear all  first
            for(i = 0; i < UDP_HEX_Rx[13]; i++)
            {
                Rail_Polygen.Area_ID = (UDP_HEX_Rx[14 + i] << 24) + (UDP_HEX_Rx[15 + i] << 16) + (UDP_HEX_Rx[16 + i] << 8) + UDP_HEX_Rx[17 + i];
                if((Rail_Polygen.Area_ID > 8) || (Rail_Polygen.Area_ID == 0))
                    Rail_Polygen.Area_ID = 1;
                Rail_Polygen.Effective_flag = 0;
                Api_RecordNum_Write(Rail_polygen, Rail_Polygen.Area_ID, (u8 *)&Rail_Polygen, sizeof(Rail_Polygen));
            }
        }

        //----------------
        // if(SD_ACKflag.f_CentreCMDack_0001H==0)
        {
            SD_ACKflag.f_CentreCMDack_0001H = 1;
            Ack_Resualt = 0;
            SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
        }
        break;
    case  0x8606:    //  ����·��
        rt_kprintf("\r\n  ����·�� \r\n");
        memset((u8 *)&ROUTE_Obj, 0, sizeof(ROUTE_Obj)); //  clear all  first
        ROUTE_Obj.Route_ID = (UDP_HEX_Rx[13] << 24) + (UDP_HEX_Rx[14] << 16) + (UDP_HEX_Rx[15] << 8) + UDP_HEX_Rx[16];
        ROUTE_Obj.Route_attribute = (UDP_HEX_Rx[17] << 8) + UDP_HEX_Rx[18];
        memcpy(ROUTE_Obj.StartTimeBCD, UDP_HEX_Rx + 19, 6);
        memcpy(ROUTE_Obj.EndTimeBCD, UDP_HEX_Rx + 25, 6);
        ROUTE_Obj.Points_Num = (UDP_HEX_Rx[31] << 8) + UDP_HEX_Rx[32];
        rt_kprintf("\r\n ROUTE_Obj.ID:  %d  \r\n ", ROUTE_Obj.Route_ID);
        rt_kprintf("\r\n ROUTE_Obj.ID:  %04X  \r\n ", ROUTE_Obj.Route_attribute);
        rt_kprintf("\r\n ROUTE_Obj.Points_Num:  %d  \r\n ", ROUTE_Obj.Points_Num);
        if(ROUTE_Obj.Points_Num < 3)
        {
            SD_ACKflag.f_CentreCMDack_0001H = 1;
            Ack_Resualt = 0;
            SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
            break;
        }
        reg_u32 = 33;
        for(i = 0; i < 6; i++) // �յ���Ŀ
        {
            // if((infolen+32)<reg_u32)
            //	 break;

            ROUTE_Obj.RoutePoints[i].POINT_ID = (UDP_HEX_Rx[reg_u32] << 24) + (UDP_HEX_Rx[reg_u32 + 1] << 16) + (UDP_HEX_Rx[reg_u32 + 2] << 8) + UDP_HEX_Rx[reg_u32 + 3];
            reg_u32 += 4;
            rt_kprintf("\r\n PointID=%08x\r\n", ROUTE_Obj.RoutePoints[i].POINT_ID);
            ROUTE_Obj.RoutePoints[i].Line_ID = (UDP_HEX_Rx[reg_u32] << 24) + (UDP_HEX_Rx[reg_u32 + 1] << 16) + (UDP_HEX_Rx[reg_u32 + 2] << 8) + UDP_HEX_Rx[reg_u32 + 3];
            reg_u32 += 4;
            rt_kprintf("\r\n LineID=%08x\r\n", ROUTE_Obj.RoutePoints[i].Line_ID);
            ROUTE_Obj.RoutePoints[i].POINT_Latitude = (UDP_HEX_Rx[reg_u32] << 24) + (UDP_HEX_Rx[reg_u32 + 1] << 16) + (UDP_HEX_Rx[reg_u32 + 2] << 8) + UDP_HEX_Rx[reg_u32 + 3];
            reg_u32 += 4;
            rt_kprintf("\r\n LatiID=%08x\r\n", ROUTE_Obj.RoutePoints[i].POINT_Latitude);
            ROUTE_Obj.RoutePoints[i].POINT_Longitude = (UDP_HEX_Rx[reg_u32] << 24) + (UDP_HEX_Rx[reg_u32 + 1] << 16) + (UDP_HEX_Rx[reg_u32 + 2] << 8) + UDP_HEX_Rx[reg_u32 + 3];
            reg_u32 += 4;
            rt_kprintf("\r\n LongID=%08x\r\n", ROUTE_Obj.RoutePoints[i].POINT_Longitude);
            ROUTE_Obj.RoutePoints[i].Width = UDP_HEX_Rx[reg_u32++];
            rt_kprintf("\r\n Width=%02x\r\n", ROUTE_Obj.RoutePoints[i].Width);
            ROUTE_Obj.RoutePoints[i].Atribute = UDP_HEX_Rx[reg_u32++];
            rt_kprintf("\r\n atrit=%02x\r\n\r\n", ROUTE_Obj.RoutePoints[i].Atribute);
            if(ROUTE_Obj.RoutePoints[i].Atribute == 0)
                ;
            else if(ROUTE_Obj.RoutePoints[i].Atribute == 1)
                ROUTE_Obj.RoutePoints[i].MaxSpd = (UDP_HEX_Rx[reg_u32++] << 8) + UDP_HEX_Rx[reg_u32++];
            else
            {
                ROUTE_Obj.RoutePoints[i].TooLongValue = (UDP_HEX_Rx[reg_u32++] << 8) + UDP_HEX_Rx[reg_u32++];
                ROUTE_Obj.RoutePoints[i].TooLessValue = (UDP_HEX_Rx[reg_u32++] << 8) + UDP_HEX_Rx[reg_u32++];
                ROUTE_Obj.RoutePoints[i].MaxSpd = (UDP_HEX_Rx[reg_u32++] << 8) + UDP_HEX_Rx[reg_u32++];
                ROUTE_Obj.RoutePoints[i].KeepDur = (UDP_HEX_Rx[reg_u32++] << 8) + UDP_HEX_Rx[reg_u32++];
            }

        }

        if((ROUTE_Obj.Route_ID > Route_Mum) || (ROUTE_Obj.Route_ID == 0))
            ROUTE_Obj.Route_ID = 1;
        ROUTE_Obj.Effective_flag = 1;
        Api_RecordNum_Write(route_line, ROUTE_Obj.Route_ID, (u8 *)&ROUTE_Obj, sizeof(ROUTE_Obj)); // ɾ����Ӧ��Χ��



        //----------------
        // if(SD_ACKflag.f_CentreCMDack_0001H==0)
        {
            SD_ACKflag.f_CentreCMDack_0001H = 1;
            Ack_Resualt = 0;
            SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
        }
        break;
    case  0x8607:    //  ɾ��·��
        rt_kprintf("\r\n  ɾ��·�� \r\n");
        if(0 == UDP_HEX_Rx[13]) // ������
            RouteLine_Init();  // ɾ����������
        else
        {
            memset((u8 *)&ROUTE_Obj, 0, sizeof(ROUTE_Obj)); //  clear all  first
            for(i = 0; i < UDP_HEX_Rx[13]; i++)
            {
                ROUTE_Obj.Route_ID = (UDP_HEX_Rx[14 + i] << 24) + (UDP_HEX_Rx[15 + i] << 16) + (UDP_HEX_Rx[16 + i] << 8) + UDP_HEX_Rx[17 + i];
                if((ROUTE_Obj.Route_ID > Route_Mum) || (ROUTE_Obj.Route_ID == 0))
                    ROUTE_Obj.Route_ID = 1;
                ROUTE_Obj.Effective_flag = 0;
                Api_RecordNum_Write(route_line, ROUTE_Obj.Route_ID, (u8 *)&ROUTE_Obj, sizeof(ROUTE_Obj)); 	 // ɾ����Ӧ��Χ��
            }
        }

        //----------------
        // if(SD_ACKflag.f_CentreCMDack_0001H==0)
        {
            SD_ACKflag.f_CentreCMDack_0001H = 1;
            Ack_Resualt = 0;
            SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
        }
        break;
    case  0x8700:    //  �г���¼�����ݲɼ�����
        /*
        						 �г���¼�Ƿ��ͳ�����ʼ
          */
        rt_kprintf( "\r\n  ��¼�ǲɼ����� \r\n" );
        Recode_Obj.Float_ID = Centre_FloatID;

        /*
                                   exam1  gghypt: 7E 87 00 00 15 01 39 01 23 45 03 01 A1 09 AA 75 09 00 0E 00 13 01 01 01 01 01 14 01 01 01 01 01 0A  D5 67 7E
                                   exam2   cctic:   7E 87 00 00 01 01 36 01 30 00 01 C9 47 15 1A 7E
                                  */

        //------ �ж���û����ʼʱ�� -----------
        Recode_Obj.Get_withDateFlag = 0; // ��ʼ��û�� GB19056 ����
        if((UDP_HEX_Rx[14] == 0xAA) && (UDP_HEX_Rx[15] == 0x75) && (UDP_HEX_Rx[13] == UDP_HEX_Rx[16]))
        {
            // �жϸ��ֶ��Ƿ��� GB19056 2012 ���ֶ�����  ��Э����д��Ϊ�գ�
            // Ȼ���ж������·��������ֺ�GB19056 ����������Ƿ�һ��
            GB19056infolen = (UDP_HEX_Rx[17] << 8) + UDP_HEX_Rx[18];
            if(GB19056infolen == 0x0E) // ������14
                switch(UDP_HEX_Rx[16])
                {
                    // �ж�ID �Ƿ���  08 -15
                case 0x08:
                case 0x09:
                case 0x10:
                case 0x11:
                case 0x12:
                case 0x13:
                case 0x14:
                case 0x15:
                    for(i = 0; i < 6; i++)
                    {
                        Recode_Obj.Get_startDate[i] = UDP_HEX_Rx[20 + i];
                        Recode_Obj.Get_endDate[i] = UDP_HEX_Rx[26 + i];

                    }
                    Recode_Obj.Get_recNum = (UDP_HEX_Rx[31] << 8) + UDP_HEX_Rx[32]; // ��ȡ������
                    Recode_Obj.Get_withDateFlag = 1;

                    // ---  ��¼�ǻ�ȡ�ɼ�ָ����־ʱ��--
                    if(UDP_HEX_Rx[16] == 0x15)
                        VDR_get_15H_StartEnd_Time(UDP_HEX_Rx + 20, UDP_HEX_Rx + 26);


                    rt_kprintf( "\r\n  ��¼��8700 ��AA 75 start   %2X-%2X-%2X  %2X:%2X:%2X\r\n", UDP_HEX_Rx[20], UDP_HEX_Rx[21], UDP_HEX_Rx[22], UDP_HEX_Rx[23], UDP_HEX_Rx[24], UDP_HEX_Rx[25]);
                    rt_kprintf( "\r\n                     end     %2X-%2X-%2X  %2X:%2X:%2X  GetNUM: %d \r\n", UDP_HEX_Rx[26], UDP_HEX_Rx[27], UDP_HEX_Rx[28], UDP_HEX_Rx[29], UDP_HEX_Rx[30], UDP_HEX_Rx[31], Recode_Obj.Get_recNum);
                    break;
                default:
                    Recode_Obj.Get_withDateFlag = 0;
                    break;
                }




        }

        //Recode_Obj.CMD=UDP_HEX_Rx[13];
        //	Stuff  Hardly
        switch( UDP_HEX_Rx[13] )
        {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
            /*case 0x08:
              case 0x09:
             case 0x10:
             case 0x11:
             case 0x12:
             case 0x13:
             case 0x14:
             case 0x15: */
            Recode_Obj.CMD   = UDP_HEX_Rx[13];
            Recode_Obj.SD_Data_Flag = 1;
            Recode_Obj.CountStep	  = 1;
            Recode_Obj.Devide_Flag = 0; //clear
            break;
            //--------- Lagre  block -------


            /*
                  cmd 		maxnum
                  15H	       2
                  12H 	10
             11H 	10
             10H  	100
             09H 	360
             08H 	576
             */

        case	0x08:  // 126  bytes per index       5 index per packet
            //  0.  get 808 total num
            if(Vdr_Wr_Rd_Offset.V_08H_Write <= 5)
                Recode_Obj.Total_pkt_num = 1;
            else if(Vdr_Wr_Rd_Offset.V_08H_Write > 5)
            {
                if(Vdr_Wr_Rd_Offset.V_08H_Write % 5)
                    Recode_Obj.Total_pkt_num = Vdr_Wr_Rd_Offset.V_08H_Write / 5 + 1;
                else
                    Recode_Obj.Total_pkt_num = Vdr_Wr_Rd_Offset.V_08H_Write / 5;
            }
            //  1. iniit    variables
            Recode_Obj.Current_pkt_num		  = 1;
            Recode_Obj.Devide_Flag			  = 1;
            Recode_Obj.Read_indexNum = 0; // ��ȡ��¼���
            //--------------------------------
            Recode_Obj.CMD		  = UDP_HEX_Rx[13];
            Recode_Obj.SD_Data_Flag = 1;
            Recode_Obj.CountStep	  = 1;
            MediaObj.Media_Type	  = 3; //��ʻ��¼��
            break;

        case	0x09:  // 666 bytes  per  index     1  index  per packet
            Recode_Obj.Total_pkt_num = Vdr_Wr_Rd_Offset.V_09H_Write;
            Recode_Obj.Current_pkt_num		  = 1; //0
            Recode_Obj.Devide_Flag			  = 1;
            Recode_Obj.Read_indexNum = 0; // ��ȡ��¼���
            //--------------------------------
            Recode_Obj.CMD		  = UDP_HEX_Rx[13];
            Recode_Obj.SD_Data_Flag = 1;
            Recode_Obj.CountStep	  = 1;
            MediaObj.Media_Type	  = 3; //��ʻ��¼��
            break;
        case	0x10: //234 byte per index      1 index per packet
            //  0.  get 808 total num
            Recode_Obj.Total_pkt_num = Vdr_Wr_Rd_Offset.V_10H_Write;
            //  1. iniit    variables
            Recode_Obj.Current_pkt_num  = 1;
            Recode_Obj.Devide_Flag	  = 1;
            Recode_Obj.Read_indexNum = 0; // ��ȡ��¼���
            //--------------------------------
            Recode_Obj.CMD		  = UDP_HEX_Rx[13];
            Recode_Obj.SD_Data_Flag = 1;
            Recode_Obj.CountStep	  = 1;
            Recode_Obj.fcs			  = 0;
            MediaObj.Media_Type	  = 3; //��ʻ��¼��
            break;
        case	0x11: // 50 bytes per index     10 index per packet
            if(Vdr_Wr_Rd_Offset.V_11H_Write <= 10)
                Recode_Obj.Total_pkt_num = 1;
            else if(Vdr_Wr_Rd_Offset.V_11H_Write > 10)
            {
                if(Vdr_Wr_Rd_Offset.V_11H_Write % 10)
                    Recode_Obj.Total_pkt_num = Vdr_Wr_Rd_Offset.V_11H_Write / 10 + 1;
                else
                    Recode_Obj.Total_pkt_num = Vdr_Wr_Rd_Offset.V_11H_Write / 10;
            }
            Recode_Obj.Current_pkt_num  = 1;
            Recode_Obj.Devide_Flag	  = 1;
            Recode_Obj.Read_indexNum = 0; // ��ȡ��¼���
            Recode_Obj.CMD			  = UDP_HEX_Rx[13];
            Recode_Obj.SD_Data_Flag	  = 1;
            Recode_Obj.CountStep		  = 1;
            Recode_Obj.fcs			  = 0;
            MediaObj.Media_Type		  = 3; //��ʻ��¼��
            break;
        case	0x12: // 25 bytes per index
            //--------------------------------------------------
            if(Vdr_Wr_Rd_Offset.V_12H_Write <= 10)
                Recode_Obj.Total_pkt_num = 1;
            else if(Vdr_Wr_Rd_Offset.V_12H_Write > 10)
            {
                if(Vdr_Wr_Rd_Offset.V_12H_Write % 10)
                    Recode_Obj.Total_pkt_num = Vdr_Wr_Rd_Offset.V_12H_Write / 10 + 1;
                else
                    Recode_Obj.Total_pkt_num = Vdr_Wr_Rd_Offset.V_12H_Write / 10;
            }
            //--------------------------------------------------
            Recode_Obj.Current_pkt_num  = 1;
            Recode_Obj.Devide_Flag	  = 1;
            Recode_Obj.Read_indexNum = 0; // ��ȡ��¼���
            Recode_Obj.CMD			  = UDP_HEX_Rx[13];
            Recode_Obj.SD_Data_Flag	  = 1;
            Recode_Obj.CountStep		  = 1;
            Recode_Obj.fcs			  = 0;
            MediaObj.Media_Type		  = 3; //��ʻ��¼��
            break;
        case	0x13:// 7 bytes per index
            //--------------------------------
            Recode_Obj.CMD		  = UDP_HEX_Rx[13];
            Recode_Obj.SD_Data_Flag = 1;
            Recode_Obj.CountStep	  = 1;
            break;
        case	0x14: // 7 bytes per index
            //--------------------------------
            Recode_Obj.CMD		  = UDP_HEX_Rx[13];
            Recode_Obj.SD_Data_Flag = 1;
            Recode_Obj.CountStep	  = 1;
            break;
        case	0x15: //// 133  bytes per index    2  index  per  packet
            //--------------------------------------------------
            if(Vdr_Wr_Rd_Offset.V_15H_Write <= 2)
                Recode_Obj.Total_pkt_num = 1;
            else if(Vdr_Wr_Rd_Offset.V_15H_Write > 2)
            {
                if(Vdr_Wr_Rd_Offset.V_15H_Write % 2)
                    Recode_Obj.Total_pkt_num = Vdr_Wr_Rd_Offset.V_15H_Write / 2 + 1;
                else
                    Recode_Obj.Total_pkt_num = Vdr_Wr_Rd_Offset.V_15H_Write / 2;
            }
            //--------------------------------------------------
            Recode_Obj.Current_pkt_num  = 1;
            Recode_Obj.Devide_Flag	  = 1;
            Recode_Obj.Read_indexNum = 0; // ��ȡ��¼���
            Recode_Obj.CMD			  = UDP_HEX_Rx[13];
            Recode_Obj.SD_Data_Flag	  = 1;
            Recode_Obj.CountStep		  = 1;
            Recode_Obj.fcs			  = 0;
            MediaObj.Media_Type		  = 3;						  //��ʻ��¼��
            break;
        default:
            Recode_Obj.Error = 1; //  �ɼ�����
            break;
        }
        break;
    case  0x8701:    //  ��ʻ��¼�ǲ����´�����
        rt_kprintf("\r\n  ��¼�ǲ����´� \r\n");
        Recode_Obj.Float_ID = Centre_FloatID;
        Recode_Obj.CMD = UDP_HEX_Rx[13];
        Recode_Obj.SD_Data_Flag = 1;
        CenterSet_subService_8701H(Recode_Obj.CMD, UDP_HEX_Rx + 14); //����2B���Ⱥ�1 ������
        break;
    case  0x8800:    //   ��ý�������ϴ�Ӧ��
        if(infolen == 5)
        {
            //  �ж��Ƿ����ش�ID�б����û�����ʾ���Ľ������!
            switch (MediaObj.Media_Type)
            {
            case 0 : // ͼ��
                Photo_send_end();  // �����ϴ�����
                rt_kprintf("\r\n ͼ�������! \r\n");
                //------------��·���մ���  -------
                CheckResualt = Check_MultiTakeResult_b4Trans();

                break;
            case 1 : // ��Ƶ
#ifdef REC_VOICE_ENABLE
                Sound_send_end();
                rt_kprintf("\r\n ��Ƶ�������! \r\n");
#endif
                break;

            default:
                break;
            }
            if(CheckResualt == 0)
                Media_Clear_State();
        }
        else
        {
            //  �ش���ID �б�
            //if(UDP_HEX_Rx[17]!=0)
            if((UDP_HEX_Rx[17] != 0) && (MultiTake.Taking == 0))
            {
                MediaObj.RSD_State = 1; //   �����ش�״̬
                MediaObj.RSD_Timer = 0; //   ����ش���ʱ��
                MediaObj.RSD_Reader = 0;
                MediaObj.RSD_total = UDP_HEX_Rx[17];  // �ش�������


                memset(MediaObj.Media_ReSdList, 0, sizeof(MediaObj.Media_ReSdList));
                //   ��ȡ�ش��б�
                j = 0;
                // �����ܰ�����ߵ�һ���ֽ��������ϰ汾�����°汾
                if(UDP_HEX_Rx[18] != 0) // �ϰ汾
                {
                    for(i = 0; i < MediaObj.RSD_total; i++)
                    {
                        //----- ���ͨ-----
                        MediaObj.Media_ReSdList[i] = UDP_HEX_Rx[18 + i];
                    }
                }
                else
                {
                    //  �°汾Э��
                    for(i = 0; i < MediaObj.RSD_total; i++)
                    {
                        // formal
                        MediaObj.Media_ReSdList[i] = (UDP_HEX_Rx[18 + j] << 8) + UDP_HEX_Rx[19 + j];
                        j += 2;
                    }
                }

                rt_kprintf("\r\n  8800 �ش��б�Total=%d: ", MediaObj.RSD_total);
                for(i = 0; i < MediaObj.RSD_total; i++)
                    rt_kprintf("%d,", MediaObj.Media_ReSdList[i]);
                rt_kprintf("\r\n");

            }
        }

        break;
        //---------  BD  add -------------------------------
    case  0x8003: // BD--8.4 �ְ������ϴ�
        // nouse	Detach_PKG.Original_floatID=(UDP_HEX_Rx[13]<<8)+UDP_HEX_Rx[14];//ԭʼ��Ϣ��ˮ��
        MediaObj.RSD_State = 1; //   �����ش�״̬
        MediaObj.RSD_Timer = 0; //   ����ش���ʱ��
        MediaObj.RSD_Reader = 0;
        MediaObj.RSD_total = UDP_HEX_Rx[15];
        //  �ش��б�
        reg_u32 = 0; // �������±������
        for(i = 3; i < infolen; i += 2)
        {
            MediaObj.Media_ReSdList[reg_u32] = (UDP_HEX_Rx[16 + 2 * reg_u32] << 24) + UDP_HEX_Rx[17 + 2 * reg_u32 + 1];
            reg_u32++;
        }

        rt_kprintf("\r\n  8003 �ش��б�Total=%d: ", MediaObj.RSD_total);
        for(i = 0; i < MediaObj.RSD_total; i++)
            rt_kprintf("%d,", MediaObj.Media_ReSdList[i]);
        rt_kprintf("\r\n");

        {
            SD_ACKflag.f_CentreCMDack_0001H = 1;
            Ack_Resualt = 0;
            SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
        }

        break;

    case  0x8801:   //    ����ͷ��������

        Camera_Obj.Channel_ID = UDP_HEX_Rx[13];   //   ͨ��
        Camera_Obj.Operate_state = UDP_HEX_Rx[18]; //   �Ƿ񱣴��־λ
        //----------------------------------

        if((Camera_Take_Enable()) && (Photo_sdState.photo_sending == 0)) //ͼƬ�����в�����
        {
            CameraState.Camera_Number = UDP_HEX_Rx[13];
            if((CameraState.Camera_Number > Max_CameraNum) && (CameraState.Camera_Number < 1))
                break;

            Camera_Start(CameraState.Camera_Number);   //��ʼ����
            CameraState.SingleCamera_TakeRetry = 0; // clear
        }
        rt_kprintf("\r\n   ���ļ�ʱ����  Camera: %d    \r\n", CameraState.Camera_Number);

        if(UDP_HEX_Rx[18] == 0x01)  // ���ղ��ϴ�
        {
            CameraState.Camera_Take_not_trans = 1;
            rt_kprintf("\r\n   ���ղ��ϴ�\r\n");
        }
        else
            CameraState.Camera_Take_not_trans = 0;
        {
            SD_ACKflag.f_CentreCMDack_0001H = 1;
            Ack_Resualt = 0;
            SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
        }
        break;
    case  0x8802:   //    �洢��ý�����ݼ���
        SD_ACKflag.f_QueryEventCode = UDP_HEX_Rx[15];
        switch(UDP_HEX_Rx[13])
        {
        case 0:  // ͼ��
            SD_ACKflag.f_MediaIndexACK_0802H = 1;
            rt_kprintf("\r\n  ���Ĳ�ѯͼ������ \r\n");
            break;
        case 1:  //  ��Ƶ
            SD_ACKflag.f_MediaIndexACK_0802H = 2;
            rt_kprintf("\r\n  ���Ĳ�ѯ��Ƶ���� \r\n");
        case 2:  //  ��Ƶ
            SD_ACKflag.f_MediaIndexACK_0802H = 3;
        default:
            break;
        }

        break;
    case  0x8803:   //    �洢��ý�������ϴ�����
        rt_kprintf("\r\n ��ý�������ϴ�\r\n");
        switch(UDP_HEX_Rx[13])
        {
        case 0:  // ͼ��
            rt_kprintf("\r\n   �ϴ�����ͼƬ\r\n");
            break;
        case 1:  //  ��Ƶ
            // MP3_send_start();
            rt_kprintf("\r\n  �ϴ�������Ƶ \r\n");
            break;
        case 2:  //  ��Ƶ
            // Video_send_start();
            // MP3_send_start();
            rt_kprintf("\r\n  �ϴ�������Ƶ  �������� ����Ƶ\r\n");
            break;
        default:
            break;
        }

        //----------------------------------------------------------
        // if(SD_ACKflag.f_CentreCMDack_0001H==0)
        {
            SD_ACKflag.f_CentreCMDack_0001H = 1;
            Ack_Resualt = 0;
            SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
        }
        break;
    case  0x8804:   //    ¼����ʼ����

        //#if  1
        switch(UDP_HEX_Rx[13])
        {
        case 0x00:  // ͣ¼��

            break;
        case 0x01:  // ��ʼ¼��
#ifdef REC_VOICE_ENABLE
            VOC_REC_Start();
#endif
            break;

        }


        //------------------------------
        //if(SD_ACKflag.f_CentreCMDack_0001H==0)
        {
            SD_ACKflag.f_CentreCMDack_0001H = 1;
            Ack_Resualt = 0;
            SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
        }
        break;

    case  0x8900:   //    ��������͸��
        switch(UDP_HEX_Rx[13]) // ͸����Ϣ����  // BD
        {
        case  0x41 : // ����1  ͸����Ϣ
            rt_kprintf("\r\n ����1 ͸��:");
            for(i = 0; i < (infolen - 1); i++)
                rt_kprintf("%c", UDP_HEX_Rx[14 + i]);
            rt_kprintf("\r\n");

            break;
        case  0x42:  //  ����2 ͸����Ϣ
            rt_kprintf("\r\n ����2 ͸��:");
            for(i = 0; i < (infolen - 1); i++)
                rt_kprintf("%c", UDP_HEX_Rx[14 + i]);
            rt_kprintf("\r\n");

            break;
        case  0x0B:  //  IC  �� ��Ϣ
            /*                         memcpy(IC_MOD.IC_Rx,UDP_HEX_Rx+14,infolen-1);
             rt_kprintf("\r\n IC ��͸��len=%dBytes  RX:",infolen-1);
             for(i=0;i<infolen-1;i++)
             	rt_kprintf("%2X ",UDP_HEX_Rx[14+i]);
             rt_kprintf("\r\n");
                                     //------ ֱ�ӷ��͸�IC ��ģ��-----
                                     Reg_buf[0]=0x00;
             memcpy(Reg_buf,IC_MOD.IC_Rx,infolen-1);
                                     DeviceData_Encode_Send(0x0B,0x40,Reg_buf,infolen);
                   return;  */
            break;

        }
        //---------------------------------------------------
        if(LinkNum == 0)
        {
            DataTrans.TYPE = UDP_HEX_Rx[13];
            memset(DataTrans.DataRx, 0, sizeof(DataTrans.DataRx));
            memcpy(DataTrans.DataRx, UDP_HEX_Rx + 14, infolen - 1);
            DataTrans.Data_RxLen = infolen - 1;

            //--------- �͸�С��Ļ----------
            memset( TextInfo.TEXT_Content, 0, sizeof(TextInfo.TEXT_Content));
            AsciiToGb(TextInfo.TEXT_Content, infolen - 1, UDP_HEX_Rx + 14);
            TextInfo.TEXT_SD_FLAG = 1;	// �÷��͸���ʾ����־λ  // ||||||||||||||||||||||||||||||||||

            //========================================

            // if(SD_ACKflag.f_CentreCMDack_0001H==0)
            {
                SD_ACKflag.f_CentreCMDack_0001H = 1;
                Ack_Resualt = 0;
                SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
            }
        }
        else
        {
            rt_kprintf("\r\n͸���������ʹ���UDP_HEX_Rx[13]=%d", UDP_HEX_Rx[13]);
        }
        break;
    case  0x8A00:   //    ƽ̨RSA��Կ


        // if(SD_ACKflag.f_CentreCMDack_0001H==0)
    {
        SD_ACKflag.f_CentreCMDack_0001H = 1;
        Ack_Resualt = 0;
        SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
    }
    break;
    case  0x8106://BD--8.11 ��ѯָ���ն˲���
        //7E 81 06 00 05 01 39 01 23 45 05 00 2A 01 00 00 00 21 D2 7E
        //7E81060005013901234505002A0100000021D27E
        //7E81060005013901234505002A03000000210000009400000055D27E
        Setting_Qry.Num_pram = UDP_HEX_Rx[13]; // ����
        if(Setting_Qry.Num_pram > 90)
            Setting_Qry.Num_pram = 90;
        reg_u32 = 0; // �������±������

        //  CMD_U32ID=(UDP_HEX_Rx[14+4*reg_u32]<<24)+(UDP_HEX_Rx[14+4*reg_u32+1]<<16)+(UDP_HEX_Rx[14+4*reg_u32+2]<<8)+(UDP_HEX_Rx[14+4*reg_u32+3]);

        for(i = 0; i < Setting_Qry.Num_pram; i++)
        {
            Setting_Qry.List_pram[reg_u32] = (UDP_HEX_Rx[14 + 4 * reg_u32] << 24) + (UDP_HEX_Rx[14 + 4 * reg_u32 + 1] << 16) + (UDP_HEX_Rx[14 + 4 * reg_u32 + 2] << 8) + (UDP_HEX_Rx[14 + 4 * reg_u32 + 3]);
            rt_kprintf("ID:  %4X ", Setting_Qry.List_pram[reg_u32]);
            reg_u32++;
        }




        SD_ACKflag.f_SettingPram_0104H = 2;	// ���Ĳ�ѯָ������
        break;
    case  0x8107:// BD--8.14 ��ѯ�ն�����
        SD_ACKflag.f_BD_DeviceAttribute_8107 = 1; // ��Ϣ��Ϊ��
        break;
    case   0x8108://BD--8.16 �·��ն�������   Զ������  (��Ҫ)

        //   1.  �ְ��ж�bit λ
        if(UDP_HEX_Rx[3] & 0x20)
        {
            //  �ְ��ж�
            BD_ISP.Total_PacketNum = (UDP_HEX_Rx[13] << 8) + UDP_HEX_Rx[14]; // �ܰ���
            BD_ISP.CurrentPacket_Num = (UDP_HEX_Rx[15] << 8) + UDP_HEX_Rx[16]; // ��ǰ����Ŵ�1 ��ʼ
        }
        else
        {
            //------------   ACK   Flag -----------------
            SD_ACKflag.f_CentreCMDack_0001H = 1;
            Ack_Resualt = 0;
            SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
        }
        //  exception
        if(BD_ISP.Total_PacketNum == 0)
        {
            //rt_kprintf("\r\n �ܰ���Ϊ0  �쳣\r\n");
            break;
        }

        //   2.  ���
        if(BD_ISP.CurrentPacket_Num == 1)
        {
            ISP_judge_resualt = 0;
            BD_ISP.Update_Type = UDP_HEX_Rx[17]; //����������
            //----  Debug ------
            switch(BD_ISP.Update_Type)
            {
            case  0:
                rt_kprintf("\r\n ��������: �ն�\r\n");
                break;
            case  12:
                // rt_kprintf("\r\n ��������: IC ��������\r\n");
                break;
            case  52:
                // rt_kprintf("\r\n ��������: ����ģ��\r\n");
                break;
            }
            //----------------------
            memcpy(BD_ISP.ProductID, UDP_HEX_Rx + 18, 5); // ������ID
            BD_ISP.Version_len = UDP_HEX_Rx[23]; // �汾�ų���
            memcpy(BD_ISP.VersionStr, UDP_HEX_Rx + 20, BD_ISP.Version_len);	// ����汾��
            i = 24 + BD_ISP.Version_len;
            BD_ISP.Content_len = (UDP_HEX_Rx[i] << 24) + (UDP_HEX_Rx[i + 1] << 16) + (UDP_HEX_Rx[i + 2] << 8) + UDP_HEX_Rx[i + 3]; // ����������
            i += 4;
            rt_kprintf("\r\n ������ID:%s\r\n", BD_ISP.ProductID);
            rt_kprintf("\r\n ����������:%d Bytes\r\n", BD_ISP.Content_len);

            infolen = infolen - 11 - BD_ISP.Version_len; // infolen

            BD_ISP.PacketRX_wr = 0; // clear
            BD_ISP.ISP_running = 1;

            i = 17 + 24;

            //   �ж��Լ��ļ�
            memcpy(BD_ISP.ContentData, UDP_HEX_Rx + i, infolen); // ����������
            //SST25V_BufferRead(ISP_buffer,ISP_StartArea,256); //---------------
            //OutPrint_HEX("��һ��",BD_ISP.ContentData,infolen);
            if(strncmp(BD_ISP.ContentData + 32 + 13, "70420TW705", 10) == 0) //�жϳ��̺��ͺ�
                //if(strncmp(BD_ISP.ContentData+32+13,ISP_buffer+14,10)==0) //�жϳ��̺��ͺ�
            {
                ISP_judge_resualt++;// step 1
                rt_kprintf("\r\n �����ͺ���ȷ\r\n");

                // hardware
                HardVersion = (BD_ISP.ContentData[32 + 38] << 24) + (BD_ISP.ContentData[32 + 39] << 16) + (BD_ISP.ContentData[32 + 40] << 8) + BD_ISP.ContentData[32 + 41];
                rt_kprintf("\r\n Ӳ���汾:%d\r\n", HardVersion);
                HardWareVerion = HardWareGet();
                if(HardWareVerion == HardVersion)	// Ҫ������ǰ���ϰ��� ȫ1
                    ISP_judge_resualt++;// step 2
                else
                    rt_kprintf("\r\n Ӳ���汾��ƥ��!\r\n");

                //firmware
                if(strncmp((const char *)BD_ISP.ContentData + 32 + 42, "HBGGHYPT", 8) == 0)
                {
                    ISP_judge_resualt++;// step 3
                    rt_kprintf("\r\n  �̼��汾:HBGGHYPT\r\n");
                }
                // operater
                if(strncmp((const char *)BD_ISP.ContentData + 32 + 62, "HBTDT", 5) == 0)
                {
                    ISP_judge_resualt++;// step 4
                    rt_kprintf("\r\n  �̼��汾:HBTDT\r\n");
                }

            }


            //  �����ж��Ƿ��Ǳ�����Ȼ����в������粻�Ƿ���ȡ��
            // ���� ��һ���������������
            if(ISP_judge_resualt == 4)
                DF_Erase();
            else
            {
                SD_ACKflag.f_BD_ISPResualt_0108H = 3; // ȡ��
                rt_kprintf("\r\n ���Ͳ�ƥ��ʧ��!\r\n");
                break;
            }


        }
        else
            i = 17;
        //3. --------  ����������  -------------
        if(BD_ISP.CurrentPacket_Num != BD_ISP.Total_PacketNum)
            BD_ISP.PacketSizeGet = infolen;

        if(infolen) //  ���������Ժ������������ͬ�Ĵ洢����д
        {
            memcpy(BD_ISP.ContentData, UDP_HEX_Rx + i, infolen); // ����������
            BD_ISP.PacketRX_wr += infolen;
            //---- �滻���±�־-----
            if(BD_ISP.CurrentPacket_Num == 1)
                BD_ISP.ContentData[32] = ISP_BYTE_StartValue;

            // OutPrint_HEX("Write",BD_ISP.ContentData,infolen);
            //  �ж�д���ַ
            if(((BD_ISP.CurrentPacket_Num - 1)*BD_ISP.PacketSizeGet) > 257024) //257024=251*1024
            {
                SD_ACKflag.f_BD_ISPResualt_0108H = 3; // ȡ��
                rt_kprintf("\r\n ��ַƫ������ --�쳣 ��ʧ��!\r\n");
                break;
            }
            DF_TAKE;
            //  write area
            WatchDog_Feed();
            SST25V_BufferWrite(BD_ISP.ContentData, ISP_StartArea + (BD_ISP.CurrentPacket_Num - 1)*BD_ISP.PacketSizeGet, infolen);
            delay_ms(360);
            WatchDog_Feed();

            // read
            memset(ISP_buffer, 0, sizeof(ISP_buffer));
            SST25V_BufferRead(ISP_buffer, ISP_StartArea + (BD_ISP.CurrentPacket_Num - 1)*BD_ISP.PacketSizeGet, infolen);
            delay_ms(220);
            DF_RELEASE;
            for(i = 0; i < infolen; i++)
            {
                if(BD_ISP.ContentData[i] != ISP_buffer[i])
                {
                    rt_kprintf("\r\n ISP error at :%d   wr=0x%2X rd=0x%2X\r\n", i, BD_ISP.ContentData[i], ISP_buffer[i]);
                    break;
                }

            }
            if(i != infolen)
            {
                rt_kprintf("\r\n DF ��д �г���!  \r\n");
                return;
            }
            //  OutPrint_HEX("Read",ISP_buffer,infolen);

        }

        rt_kprintf("\r\n  ISP  ��ǰ����:%d	--- �ܰ���:%d    contentlen=%d Bytes \r\n", BD_ISP.CurrentPacket_Num, BD_ISP.Total_PacketNum, infolen);

        //4.----------- ISP state update ----------------------
        BD_ISP.ISP_running = 1;
        BD_ISP.ISP_runTimer = 0;

        //5. --------  ISP ACK process  -----------
        if((BD_ISP.CurrentPacket_Num == BD_ISP.Total_PacketNum) && (i == infolen))
        {
            // File_CRC_Get();
            SD_ACKflag.f_BD_ISPResualt_0108H = 1; //����
            rt_kprintf("\r\n ���һ��\r\n");
        }
        else if(BD_ISP.CurrentPacket_Num > BD_ISP.Total_PacketNum)
        {

            SD_ACKflag.f_BD_ISPResualt_0108H = 3; // ȡ��
            rt_kprintf("\r\n ��ǰ�������ܰ��� --�쳣 ��ʧ��!\r\n");
        }
        else
        {

            //------------	 ACK   Flag -----------------
            SD_ACKflag.f_CentreCMDack_0001H = 1;
            Ack_Resualt = 0;
            SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
            //------------------------------------------------------------------------

        }
        break;
    case   0x8203://BD--8.22  �˹�ȷ�ϱ�����Ϣ
        rt_kprintf( "\r\n�յ�8203!\r\n");
        if (Warn_Status[3] & 0x01) //��������
        {
            StatusReg_WARN_Clear();
            f_Exigent_warning = 0;
            warn_flag = 0;
            Send_warn_times = 0;
            StatusReg_WARN_Clear();
            rt_kprintf( "\r\n���������յ�Ӧ�𣬵����!\r\n");
        }

        HumanConfirmWarn.Warn_FloatID = (UDP_HEX_Rx[13] << 8) + UDP_HEX_Rx[14];
        HumanConfirmWarn.ConfirmWarnType = (UDP_HEX_Rx[15] << 24) + (UDP_HEX_Rx[16] << 16) + (UDP_HEX_Rx[17] << 8) + UDP_HEX_Rx[18];
        if(HumanConfirmWarn.Warn_FloatID == 0x00)
        {
            //  ���Ϊ0 ��ʾ������Ϣ
            Warn_Status[3] &= ~0x01;  // bit 0
            Warn_Status[3] &= ~0x08;	// bit3
            Warn_Status[1] &= ~0x10;	// bit20 �������򱨾�
            Warn_Status[1] &= ~0x20;  // bit 21 ������·����
            Warn_Status[1] &= ~0x40; 	// bit 22  ·����ʻʱ�䲻�㱨��
            Warn_Status[0] &= ~0x08;  // bit 27  ȷ�ϳ����Ƿ���𱨾�
            Warn_Status[0] &= ~0x10;  // bit 28  ȷ�ϳ����Ƿ�����
            Warn_Status[0] &= ~0x80;  //bit 31  �Ƿ����� (�ն˲���������ʱ�����ſ�����Ч)
        }
        else
        {
            //--------------------------------
            if ((Warn_Status[3] & 0x01) && (0x00000001 & HumanConfirmWarn.ConfirmWarnType)) //��������
            {
                StatusReg_WARN_Clear();
                f_Exigent_warning = 0;
                warn_flag = 0;
                Send_warn_times = 0;
                StatusReg_WARN_Clear();
                rt_kprintf( "\r\n���������յ�Ӧ�𣬵����!\r\n");
            }
            if( (Warn_Status[3] & 0x08) && (0x00000008 & HumanConfirmWarn.ConfirmWarnType)) //Σ�ձ���-BD
            {
                Warn_Status[3] &= ~0x08;
            }

            //------------------------------------
            if((Warn_Status[1] & 0x10) && (0x00001000 & HumanConfirmWarn.ConfirmWarnType)) // �������򱨾�
            {
                InOut_Object.TYPE = 0; //Բ������
                InOut_Object.ID = 0; //  ID
                InOut_Object.InOutState = 0; //  ������
                Warn_Status[1] &= ~0x10;
            }
            if((Warn_Status[1] & 0x20) && (0x00002000 & HumanConfirmWarn.ConfirmWarnType)) // ����·�߱���
            {
                InOut_Object.TYPE = 0; //Բ������
                InOut_Object.ID = 0; //  ID
                InOut_Object.InOutState = 0; //  ������
                Warn_Status[1] &= ~0x20;
            }
            if((Warn_Status[1] & 0x40) && (0x00004000 & HumanConfirmWarn.ConfirmWarnType)) // ·����ʻʱ�䲻�㡢����
            {
                Warn_Status[1] &= ~0x40;
            }
            //-----------------------------------------
            if((Warn_Status[0] & 0x08) && (0x08000000 & HumanConfirmWarn.ConfirmWarnType)) //�����Ƿ����
            {
                Warn_Status[0] &= ~0x08;
            }
            if((Warn_Status[0] & 0x10) && (0x10000000 & HumanConfirmWarn.ConfirmWarnType)) //�����Ƿ�λ��
            {
                Warn_Status[0] &= ~0x10;
            }
            if((Warn_Status[0] & 0x80) && (0x80000000 & HumanConfirmWarn.ConfirmWarnType)) //�Ƿ����ű���(�ն�δ��������ʱ�����жϷǷ�����)
            {
                Warn_Status[0] &= ~0x80;
            }
            //------------------------------------
        }

        //------------   ACK   Flag -----------------
        SD_ACKflag.f_CentreCMDack_0001H = 1;
        Ack_Resualt = 0;
        SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
        //---------  ����ȷ�Ϻ��ͣ��������Ͷ�λ��
        PositionSD_Enable();
        Current_UDP_sd = 1;
        break;
    case   0x8702:// BD-8.47-  �ϱ���ʻԱ�����Ϣ����
        SD_ACKflag.f_DriverInfoSD_0702H = 1; // ��Ϣ��Ϊ��
        break;
    case  0x8805:   //BD--8.55    �����洢��ý�����ݼ����ϴ�����	---- ����Э��Ҫ��

        reg_u32 = (UDP_HEX_Rx[13] << 24) + (UDP_HEX_Rx[14] << 16) + (UDP_HEX_Rx[15] << 8) + UDP_HEX_Rx[16];
        //rt_kprintf("\r\n�����洢��ý�����ݼ����ϴ� MeidiaID=%d ɾ����־: %d ",reg_u32,UDP_HEX_Rx[17]);

        CameraState.Camera_Number = 1;
        Photo_send_start(CameraState.Camera_Number);  //�ڲ��Ƕ�·���յ����������Ϳ����ϴ���
        //------------------------------
        // if(SD_ACKflag.f_CentreCMDack_0001H==0)
        {
            SD_ACKflag.f_CentreCMDack_0001H = 1;
            Ack_Resualt = 0;
            SD_ACKflag.f_CentreCMDack_resualt = Ack_Resualt;
        }
        break;
    default:
        break;
    }

    //-----------------  memset  -------------------------------------
    //memset(UDP_HEX_Rx, 0, sizeof(UDP_HEX_Rx));
    //UDP_hexRx_len= 0;
    return;

}


void Time2BCD(u8 *dest)
{
#if 0
    if(UDP_dataPacket_flag == 0x02)
    {

        dest[0] = ((Temp_Gps_Gprs.Date[0] / 10) << 4) + (Temp_Gps_Gprs.Date[0] % 10);
        dest[1] = ((Temp_Gps_Gprs.Date[1] / 10) << 4) + (Temp_Gps_Gprs.Date[1] % 10); //Temp_Gps_Gprs.Date[1];
        dest[2] = ((Temp_Gps_Gprs.Date[2] / 10) << 4) + (Temp_Gps_Gprs.Date[2] % 10); //Temp_Gps_Gprs.Date[2];

        dest[3] = ((Temp_Gps_Gprs.Time[0] / 10) << 4) + (Temp_Gps_Gprs.Time[0] % 10); //Temp_Gps_Gprs.Time[0];
        dest[4] = ((Temp_Gps_Gprs.Time[1] / 10) << 4) + (Temp_Gps_Gprs.Time[1] % 10); //Temp_Gps_Gprs.Time[1];
        dest[5] = ((Temp_Gps_Gprs.Time[2] / 10) << 4) + (Temp_Gps_Gprs.Time[2] % 10); //Temp_Gps_Gprs.Time[2];

    }
    else
#endif
    {
        dest[0] = ((time_now.year / 10) << 4) + (time_now.year % 10);
        dest[1] = ((time_now.month / 10) << 4) + (time_now.month % 10);
        dest[2] = ((time_now.day / 10) << 4) + (time_now.day % 10);
        dest[3] = ((time_now.hour / 10) << 4) + (time_now.hour % 10);
        dest[4] = ((time_now.min / 10) << 4) + (time_now.min % 10);
        dest[5] = ((time_now.sec / 10) << 4) + (time_now.sec % 10);
    }

}
//==================================================================================================
// ���Ĳ��� :   �������г���¼�����Э�� �� ��¼A
//==================================================================================================

//  1.  �����������������

//------------------------------------------------------------------
void  Process_GPRSIN_DeviceData(u8 *instr, u16  infolen)
{
    u8  fcs = 0;
    u16 i = 0;



    //   caculate  and   check fcs
    for(i = 0; i < infolen - 1; i++)
        fcs ^= instr[i];

    if(fcs != instr[infolen - 1])
        return;
    //  classify  cmd
    switch(instr[2])   // AAH 75H CMD
    {
        //  ����
    case 0x00:  // �ɼ��г���¼��ִ�б�׼�汾��
        Adata_ACKflag.A_Flag__Up_Ver_00H = 0xff;
        break;
    case 0x01:  // �ɼ���ǰ��ʻ����Ϣ
        Adata_ACKflag.A_Flag_Up_DrvInfo_01H = instr[2];
        break;
    case 0x02:  // �ɼ���¼�ǵ�ʵʱʱ��
        Adata_ACKflag.A_Flag_Up_RTC_02H = instr[2];
        break;
    case 0x03:  // �ɼ���ʻ���
        Adata_ACKflag.A_Flag_Up_Dist_03H = instr[2];
        break;
    case 0x04:  // �ɼ���¼���ٶ�����ϵ��
        Adata_ACKflag.A_Flag_Up_PLUS_04H = instr[2];
        break;
    case 0x06:  // �ɼ�������Ϣ
        Adata_ACKflag.A_Flag_Up_VechInfo_06H = instr[2];
        break;
    case 0x08:  // �ɼ���¼��״̬�ź�������Ϣ
        Adata_ACKflag.A_Flag_Up_SetInfo_08H = instr[2];
        break;
    case 0x16:  // �ɼ���¼��Ψһ���
        Adata_ACKflag.A_Flag_Up_DevID_16H = instr[2];
        break;
    case 0x09:  // �ɼ�ָ����ÿ����ƽ���ٶȼ�¼
        Adata_ACKflag.A_Flag_Up_AvrgSec_09H = instr[2]; // ����ʼ����ʱ��
        break;
    case 0x05: // �ɼ�ָ����ÿ����ƽ���ٶȼ�¼
        Adata_ACKflag.A_Flag_Up_AvrgMin_05H = instr[2]; // ����ʼ����ʱ��
        break;
    case 0x13: // �ɼ�ָ����λ����Ϣ��¼
        Adata_ACKflag.A_Flag_Up_Posit_13H = instr[2];
        break;
    case 0x07: // �ɼ��¹��ɵ��¼
        Adata_ACKflag.A_Flag_Up_Doubt_07H = instr[2]; // ����ʼ����ʱ��
        break;
    case 0x11: // �ɼ�ָ����ƣ�ͼ�ʻ��¼
        Adata_ACKflag.A_Flag_Up_Tired_11H = instr[2]; // ����ʼ����ʱ��
        break;
    case 0x10: // �ɼ�ָ���ĵ�¼�˳���¼
        Adata_ACKflag.A_Flag_Up_LogIn_10H = instr[2]; // ����ʼ����ʱ��
        break;
    case 0x14: // �ɼ�ָ���ļ�¼���ⲿ�����¼
        Adata_ACKflag.A_Flag_Up_Powercut_14H = instr[2]; // ����ʼ����ʱ��
        break;
    case 0x15: // �ɼ�ָ���ļ�¼�ǲ����޸ļ�¼
        Adata_ACKflag.A_Flag_Up_SetMdfy_15H = instr[2];
        break;
        //  ����
    case 0x82: // ���ó�����Ϣ
        memset(Vechicle_Info.Vech_VIN, 0, sizeof(Vechicle_Info.Vech_VIN));
        memset(Vechicle_Info.Vech_Num, 0, sizeof(Vechicle_Info.Vech_Num));
        memset(Vechicle_Info.Vech_Type, 0, sizeof(Vechicle_Info.Vech_Type));

        //-----------------------------------------------------------------------
        memcpy(Vechicle_Info.Vech_VIN, instr, 17);
        memcpy(Vechicle_Info.Vech_Num, instr + 17, 12);
        memcpy(Vechicle_Info.Vech_Type, instr + 29, 12);

        DF_TAKE;
        DF_WriteFlashSector(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
        WatchDog_Feed();
        DF_WriteFlashSector(DF_VehicleBAK_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
        WatchDog_Feed();
        DF_WriteFlashSector(DF_VehicleBAK2_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
        DF_RELEASE;
        Adata_ACKflag.A_Flag_Dn_DrvInfo_82H = instr[2];

        Settingchg_Status = 0x82; //���ó�����Ϣ
        NandsaveFlg.Setting_SaveFlag = 1; //�洢�����޸ļ�¼
    case 0x83:  // ���ó��ΰ�װ����
        memcpy(JT808Conf_struct.FirstSetupDate, instr, 6);
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        Adata_ACKflag.A_Flag_Dn_SetupDate_83H = instr[2];
        Settingchg_Status = 0x83; //���ó�����Ϣ
        NandsaveFlg.Setting_SaveFlag = 1; //�洢�����޸ļ�¼
        break;
    case 0x84:  // ����״̬����Ϣ
        Settingchg_Status = 0x84; //���ó�����Ϣ
        NandsaveFlg.Setting_SaveFlag = 1; //�洢�����޸ļ�¼
        break;
    case 0xc2: // ���ü�¼��ʱ��
        Adata_ACKflag.A_Flag_Dn_RTC_C2H = instr[2];
        Settingchg_Status = 0xc2; //���ó�����Ϣ
        NandsaveFlg.Setting_SaveFlag = 1; //�洢�����޸ļ�¼
        break;
    case 0xc3: // ���ü�¼���ٶ�����ϵ��
        JT808Conf_struct.Vech_Character_Value = (u32)(instr[0] << 24) + (u32)(instr[1] << 16) + (u32)(instr[2] << 8) + (u32)(instr[3]); // ����ϵ��	�ٶ�����ϵ��
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
        Adata_ACKflag.A_Flag_Dn_Plus_C3H = instr[2];
        Settingchg_Status = 0xc3; //���ó�����Ϣ
        NandsaveFlg.Setting_SaveFlag = 1; //�洢�����޸ļ�¼
        break;
    default:

        break;




    }
}

#if 0

u8 RecordSerial_output_Str(const char *fmt, ...)
{
    u8 regstr[100], fcs = 0;
    u16 reglen = 0, i = 0;

    va_list args;
    va_start(args, fmt);
    memset(regstr, 0, sizeof(regstr));
    regstr[0] = 0x55; // Э��ͷ
    regstr[1] = 0x7A;
    regstr[2] = 0xFE; // ������ ����Ԥ�������ֱ�ʾ�������
    //  3,4 Ϊ�����ֽ������д
    regstr[5] = 0x00; // ������  0x00

    reglen = vsprintf((char *)regstr + 6, fmt, args);
    va_end(args);
    regstr[3] = (u8)(reglen >> 8); // ��д����  ������Ϊ��Ϣ���ݵĳ���
    regstr[4] = (u8)reglen;

    reglen += 6;
    fcs = 0;
    for(i = 0; i < reglen; i++)
        fcs ^= regstr[i];
    regstr[reglen] = fcs;
    reglen++;
    for(i = 0; i < reglen; i++)
        rt_kprintf("%c", regstr[i]);

    return 1;
}

#endif

void SpeedWarnJudge(void)  //  �ٶȱ����ж�
{
    //  0    GB   ���ٱ����� �жϴ�������    (������֤�����⣬����ʵ�ʲ�����ôŪ��)
#if 0
    if(Spd_Using > ( JT808Conf_struct.Speed_warn_MAX * 10) )
    {
        if(GB19056.SPK_Speed_Warn.Warn_state_Enable == 0)
            GB19056.SPK_Speed_Warn.Warn_state_Enable = 1;
    }
    else
        GB19056.SPK_Speed_Warn.Warn_state_Enable = 0; // clear
#endif
    //======================================================================
    //--------  �ٶȱ���  -------
    if(  JT808Conf_struct.Speed_warn_MAX > 0 )  //> 0
    {

        //----- GPS  ��ʱ�ٶ�	0.1 km/h  ---------
        if  ( Spd_Using > ( JT808Conf_struct.Speed_warn_MAX * 10) )
            // if( DebugSpd > ( JT808Conf_struct.Speed_warn_MAX*10) )
        {
            speed_Exd.dur_seconds++;
            if ( speed_Exd.dur_seconds > JT808Conf_struct.Spd_Exd_LimitSeconds)
            {
                speed_Exd.dur_seconds = 0;
                if ( speed_Exd.speed_flag != 1 )
                {
                    speed_Exd.speed_flag = 1;
                    //PositionsSD_Enable();	  //  �ر�GPS ��Ϣ

                    StatusReg_SPD_WARN(); //  ���ٱ���״̬
                    rt_kprintf("\r\n  ���ٱ���\r\n");

                    // modify  ����Ҫ��
                    if(GB19056.SPK_Speed_Warn.Warn_state_Enable == 0)
                        GB19056.SPK_Speed_Warn.Warn_state_Enable = 1;
                }
                //---------------------------------------------
                Time2BCD(speed_Exd.ex_startTime); //��¼���ٱ�����ʼʱ��
                if(speed_Exd.current_maxSpd < Spd_Using) //������ٶ�
                    speed_Exd.current_maxSpd = Spd_Using;
                speed_Exd.excd_status = 1;
                speed_Exd.dur_seconds++;

                //----------------------------------------------
            }

            if(speed_Exd.excd_status == 1) // ʹ��flag ��ʼ��ʱ
            {
                speed_Exd.dur_seconds++;
                if(speed_Exd.current_maxSpd < Spd_Using) //������ٶ�
                    speed_Exd.current_maxSpd = Spd_Using;
            }

        }
        else
        {
            StatusReg_SPD_NORMAL(); //  ����ٶȱ���״̬�Ĵ���

            if(speed_Exd.excd_status != 2)
            {
                StatusReg_SPD_NORMAL(); //  ����ٶȱ���״̬�Ĵ���
                speed_Exd.dur_seconds = 0;
                speed_Exd.speed_flag = 0;
            }
            //----------------------------------------------
            if(speed_Exd.excd_status == 1)
            {
                Time2BCD(speed_Exd.ex_endTime); //��¼���ٱ�������ʱ��
                speed_Exd.excd_status = 2;
            }
            else if(speed_Exd.excd_status == 0)
                Spd_ExpInit();
            //----------------------------------------------
            //    modify  ����Ҫ��
            GB19056.SPK_Speed_Warn.Warn_state_Enable = 0; // clear
        }
    }//------- �ٶȱ��� over	 ---


}


u16  Protocol_808_Encode(u8 *Dest, u8 *Src, u16 srclen)
{
    u16  lencnt = 0, destcnt = 0;

    for(lencnt = 0; lencnt < srclen; lencnt++)
    {
        if(Src[lencnt] == 0x7e) // 7e ת��
        {
            Dest[destcnt++] = 0x7d;
            Dest[destcnt++] = 0x02;
        }
        else if(Src[lencnt] == 0x7d) //  7d  ת��
        {
            Dest[destcnt++] = 0x7d;
            Dest[destcnt++] = 0x01;
        }
        else
            Dest[destcnt++] = Src[lencnt]; // ԭʼ��Ϣ
    }

    return destcnt; //����ת���ĳ���

}
//-------------------------------------------------------------------------------
void Protocol_808_Decode(void)  // ����ָ��buffer :  UDP_HEX_Rx
{
    //-----------------------------------
    u16 i = 0;

    // 1.  clear  write_counter
    UDP_DecodeHex_Len = 0; //clear DecodeLen

    // 2   decode process
    for(i = 0; i < UDP_hexRx_len; i++)
    {
        if((UDP_HEX_Rx[i] == 0x7d) && (UDP_HEX_Rx[i + 1] == 0x02))
        {
            UDP_HEX_Rx[UDP_DecodeHex_Len] = 0x7e;
            i++;
        }
        else if((UDP_HEX_Rx[i] == 0x7d) && (UDP_HEX_Rx[i + 1] == 0x01))
        {
            UDP_HEX_Rx[UDP_DecodeHex_Len] = 0x7d;
            i++;
        }
        else
        {
            UDP_HEX_Rx[UDP_DecodeHex_Len] = UDP_HEX_Rx[i];
        }
        UDP_DecodeHex_Len++;
    }
    //  3.  The  End
}
//---------  �յ㲹�����Գ���  ---------------------------
//#if 0
/*
void Inflexion_Process(void)
{            //
  u16  once_delta=0;


	Inflexion_Current=GPS_direction;  //  update new
   //-----------------------------------------------------------------------
    if(Inflexion_Current>Inflexion_Bak)   // �����жϴ�С
    	{  // ����
            if((Inflexion_Current-Inflexion_Bak)>300)  // �ж��Ƿ��ü�С
			{   //  �����ֵ����300 ��˵����С��
			     once_delta=Inflexion_Bak+360-Inflexion_Current;  //�жϲ�ֵ����ֵ
			     InflexDelta_Accumulate+=once_delta;
				 if((once_delta>=15)&&(once_delta<=60)) // �Ƕ���С�仯�ʲ���С��15��  �յ㲹���ǶȲ�����180 Ҫ������3s  ����ÿ�벻����60
				 {
					    if((Inflexion_chgcnter>0)&&(InflexLarge_or_Small==2))  //�ж�֮ǰ�Ƿ�һֱ��С��
					    	{
		                        Inflexion_chgcnter++;
								if((InflexDelta_Accumulate>=3)&&(InflexDelta_Accumulate>JT808Conf_struct.DURATION.SD_Delta_maxAngle)&&(InflexDelta_Accumulate<=180))
									{     //Ҫ�����ٳ���3s	�ۼƹսǲ����ǶȲ��ô���180
                                           InflexLarge_or_Small=0;
										   Inflexion_chgcnter=0;
										   InflexDelta_Accumulate=0;
										   PositionSD_Enable=1; // ���͹յ��־λ
										   rt_kprintf("\r\n �յ��ϱ� --1\r\n");
									}
								else
								  InflexLarge_or_Small=2; // �����С��
					    	}
						else
							{
							   InflexLarge_or_Small=2;  // ���ǵ�һ��С��
							   Inflexion_chgcnter=1;
							   InflexDelta_Accumulate=once_delta;
							}
				 }
				 else
				 {    //  С�� 15 �������
				   InflexLarge_or_Small=0;
				   Inflexion_chgcnter=0;
				   InflexDelta_Accumulate=0;
				 }

            }
			else		// current���������ı�Bak ��
			{
			   once_delta=Inflexion_Current-Inflexion_Bak;  //�жϲ�ֵ����ֵ
			   InflexDelta_Accumulate+=once_delta;
			   if((once_delta>=15)&&(once_delta<=60)) // �Ƕ���С�仯�ʲ���С��15��  �յ㲹���ǶȲ�����180 Ҫ������3s  ����ÿ�벻����60
			   {
	               if((Inflexion_chgcnter>0)&&(InflexLarge_or_Small==1)) //�ж�֮ǰ�Ƿ�һֱ����
				   {
				       Inflexion_chgcnter++;
					   if((InflexDelta_Accumulate>=3)&&(InflexDelta_Accumulate>JT808Conf_struct.DURATION.SD_Delta_maxAngle)&&(InflexDelta_Accumulate<=180))
						   {	 //Ҫ�����ٳ���3s	�ۼƹսǲ����ǶȲ��ô���180
								  InflexLarge_or_Small=0;
								  Inflexion_chgcnter=0;
								  InflexDelta_Accumulate=0;
								  PositionSD_Enable(); // ���͹յ��־λ
								  rt_kprintf("\r\n �յ��ϱ� --2\r\n");
						   }
					   else
					     InflexLarge_or_Small=1; // ����Ǵ���
	               }
				   else
				   	{
                       InflexLarge_or_Small=1;  // ���ǵ�һ�δ���
					   Inflexion_chgcnter=1;
					   InflexDelta_Accumulate=once_delta;
				   	}
			   }
			    else
				 {     // С��15�Ⱦ������
				   InflexLarge_or_Small=0;
				   Inflexion_chgcnter=0;
				   InflexDelta_Accumulate=0;
				 }

			}
    	}
	else
	 if(Inflexion_Current<Inflexion_Bak)
	 	{  // ��С
               if((Inflexion_Bak-Inflexion_Current)>300)  // �ж��Ƿ�������
               { //  �����ֵ����300 ��˵���Ǵ���
                  once_delta=Inflexion_Current+360-Inflexion_Bak;  //�жϲ�ֵ����ֵ
			      InflexDelta_Accumulate+=once_delta;
				  if((once_delta>=15)&&(once_delta<=60)) // �Ƕ���С�仯�ʲ���С��15��	�յ㲹���ǶȲ�����180 Ҫ������3s  ����ÿ�벻����60
                  {   // ��С�仯�� ��С�� 15
                     if((Inflexion_chgcnter>0)&&(InflexLarge_or_Small==1))  //�ж�֮ǰ�Ƿ�һֱ�Ǵ���
					    	{
		                        Inflexion_chgcnter++;
								if((InflexDelta_Accumulate>=3)&&(InflexDelta_Accumulate>JT808Conf_struct.DURATION.SD_Delta_maxAngle)&&(InflexDelta_Accumulate<=180))
									{     //Ҫ�����ٳ���3s	�ۼƹսǲ����ǶȲ��ô���180
                                           InflexLarge_or_Small=0;
										   Inflexion_chgcnter=0;
										   InflexDelta_Accumulate=0;
										   PositionSD_Enable(); // ���͹յ��־λ
										   rt_kprintf("\r\n �յ��ϱ� --3\r\n");
									}
								else
								  InflexLarge_or_Small=1; // ����Ǵ���
					    	}
						else
							{
							   InflexLarge_or_Small=1;  // ���ǵ�һ�δ���
							   Inflexion_chgcnter=1;
							   InflexDelta_Accumulate=once_delta;
							}

                  }
				  else
				  {    //  С�� 15 �������
				   InflexLarge_or_Small=0;
				   Inflexion_chgcnter=0;
				   InflexDelta_Accumulate=0;
				  }
			  }//---------------------------
			   else 	   // current ���������ı�Bak С
			   {
				  once_delta=Inflexion_Bak-Inflexion_Current;  //�жϲ�ֵ����ֵ
				  InflexDelta_Accumulate+=once_delta;
				  if((once_delta>=15)&&(once_delta<=60)) // �Ƕ���С�仯�ʲ���С��15��	�յ㲹���ǶȲ�����180 Ҫ������3s  ����ÿ�벻����60
				  {
					  if((Inflexion_chgcnter>0)&&(InflexLarge_or_Small==2)) //�ж�֮ǰ�Ƿ�һֱС��
					  {
						  Inflexion_chgcnter++;
						  if((InflexDelta_Accumulate>=3)&&(InflexDelta_Accumulate>JT808Conf_struct.DURATION.SD_Delta_maxAngle)&&(InflexDelta_Accumulate<=180))
							  { 	//Ҫ�����ٳ���3s	�ۼƹսǲ����ǶȲ��ô���180
									 InflexLarge_or_Small=0;
									 Inflexion_chgcnter=0;
									 InflexDelta_Accumulate=0;
									 PositionSD_Enable(); // ���͹յ��־λ
									 rt_kprintf("\r\n �յ��ϱ� --4\r\n");
							  }
						  else
						   InflexLarge_or_Small=2; // �����С��
					  }
					  else
					   {
						  InflexLarge_or_Small=2;  // ���ǵ�һ��С��
						  Inflexion_chgcnter=1;
						  InflexDelta_Accumulate=once_delta;
					   }
				  }
				   else
					{	  // С��15�Ⱦ������
					  InflexLarge_or_Small=0;
					  Inflexion_chgcnter=0;
					  InflexDelta_Accumulate=0;
					}

			   }



	 	}
	else
		{
		  InflexLarge_or_Small=0;
		  Inflexion_chgcnter=0;
		  InflexDelta_Accumulate=0;
		}

    //--------------------------------------------------------
    Inflexion_Bak=Inflexion_Current; //  throw  old	 to  Bak

}*/
//#endif

void  Sleep_Mode_ConfigEnter(void)
{
    if(SleepState == 0)
    {
        SleepCounter++;
        if(SleepCounter > 15) // ��������
        {
            SleepCounter = 0;
            SleepState = 1;
            if(JT808Conf_struct.RT_LOCK.Lock_state != 1)
                Current_SD_Duration = JT808Conf_struct.DURATION.Sleep_Dur; // 5����
            if(DataLink_Status())
                PositionSD_Enable();  //  ���߾ͷ���    �ӱ����ͨҪ����������
        }
    }
}

void  Sleep_Mode_ConfigExit(void)
{

    if((JT808Conf_struct.RT_LOCK.Lock_state != 1) && (DF_initOver == 1))
        Current_SD_Duration = JT808Conf_struct.DURATION.Default_Dur;

    JT808Conf_struct.DURATION.Heart_Dur = 300;

    if(SleepState == 1)
    {
        SleepState = 0;
    }
    SleepState = 0;
    SleepCounter = 0;
}

#if 0
u16 WaveFile_EncodeHeader(u32 inFilesize , u8 *DestStr)
{
    u32 Filesize = 0, i = 0; // Header Len  =44Bytes


    //  1. RIFF
    memcpy(DestStr, "RIFF", 4);
    i += 4;
    //  2. Wave �ļ� ��С  С��ģʽ
    Filesize = 0x24 + (inFilesize << 3); // ����16 �� 36 wave �ļ���С
    rt_kprintf("\r\n .wav �ļ���С: %d Rawdata: %d \r\n ", Filesize, (inFilesize << 3));
    DestStr[i++] = Filesize; // LL
    DestStr[i++] = (Filesize >> 8); //LH
    DestStr[i++] = (Filesize >> 16); //HL
    DestStr[i++] = (Filesize >> 24); //HH
    //  3. WAVE string
    memcpy(DestStr + i, "WAVE", 4);
    i += 4;
    //  4. fmt string
    memcpy(DestStr + i, "fmt ", 4);
    i += 4;
    //  5. PCM Code
    DestStr[i++] = 0x10; // LL
    DestStr[i++] = 0x00; //LH
    DestStr[i++] = 0x00; //HL
    DestStr[i++] = 0x00; //HH
    //  6. Audio Format  PCM=1
    DestStr[i++] = 0x01; // L
    DestStr[i++] = 0x00; //H
    //  7. NumChannels  ͨ����
    DestStr[i++] = 0x01; // L
    DestStr[i++] = 0x00; //H
    //  8. SampleRate     8000<=>0x00001F40    16000<=>0x00003E80
    DestStr[i++] = 0x40; //0x40; // LL
    DestStr[i++] = 0x1F; //0x1F;//LH
    DestStr[i++] = 0x00; //HL
    DestStr[i++] = 0x00; //HH
    //  9.ByteRate       == SampleRate * NumChannels * BitsPerSample/8  ==8000x1x8/8
    DestStr[i++] = 0x40; //0x40; // LL
    DestStr[i++] = 0x1F; //0x1F;//LH
    DestStr[i++] = 0x00; //HL
    DestStr[i++] = 0x00; //HH

    // 10.BlockAlign   	== NumChannels * BitsPerSample/8
    DestStr[i++] = 0x01; //0x02;//0x01; // L
    DestStr[i++] = 0x00; //H
    // 11.BitsPerSample
    DestStr[i++] = 0x08; //0x10;//0x08; // L
    DestStr[i++] = 0x00; //H
    // 12.data string
    memcpy(DestStr + i, "data", 4);
    i += 4;
    // 13 .datasize
    Filesize = (inFilesize << 3); // ����16 �� 36 wave �ļ���С
    DestStr[i++] = Filesize; // LL
    DestStr[i++] = (Filesize >> 8); //LH
    DestStr[i++] = (Filesize >> 16); //HL
    DestStr[i++] = (Filesize >> 24); //HH

    return i;
}
#endif

//-----------  starttime[6]
u8 CurrentTime_Judge( u8 *startTime , u8 *endTime)
{
    u32 daycaculate_current = 0, daycaculate_start = 0, daycaculate_end = 0;
    u32 secondcaculate_current = 0, secondcaculate_start = 0, secondcaculate_end = 0;


    daycaculate_start = ((startTime[0] >> 4) * 10 + (startTime[0] & 0x0f)) * 365 + ((startTime[1] >> 4) * 10 + (startTime[1] & 0x0f)) * 30 + ((startTime[2] >> 4) * 10 + (startTime[2] & 0x0f));
    secondcaculate_start = ((startTime[3] >> 4) * 10 + (startTime[3] & 0x0f)) * 60 + ((startTime[4] >> 4) * 10 + (startTime[4] & 0x0f)) * 60 + ((startTime[5] >> 4) * 10 + (startTime[5] & 0x0f));


    daycaculate_end = ((endTime[0] >> 4) * 10 + (endTime[0] & 0x0f)) * 365 + ((endTime[1] >> 4) * 10 + (endTime[1] & 0x0f)) * 30 + ((endTime[2] >> 4) * 10 + (endTime[2] & 0x0f));
    secondcaculate_end = ((endTime[3] >> 4) * 10 + (endTime[3] & 0x0f)) * 60 + ((endTime[4] >> 4) * 10 + (endTime[4] & 0x0f)) * 60 + ((endTime[5] >> 4) * 10 + (endTime[5] & 0x0f));

    daycaculate_current = (time_now.year) * 365 + time_now.month * 30 + time_now.day;
    secondcaculate_current = time_now.hour * 60 + time_now.min * 60 + time_now.sec;

    if((daycaculate_current > daycaculate_start) && (daycaculate_current < daycaculate_end))
    {
        return  true;
    }
    else if((secondcaculate_current >= secondcaculate_start) && (secondcaculate_current <= secondcaculate_end))
    {
        return true;
    }
    else
        return false;



}

void CycleRail_Judge(u8 *LatiStr, u8 *LongiStr)
{
    /*
        γ��û�в�ֵ    1γ��  111km
        40��γ���� 1����Ϊ  85.3km   (��������)
    */
    u8 i = 0;
    u32 Latitude = 0, Longitude = 0;
    u32 DeltaLatiDis = 0, DeltaLongiDis = 0, CacuDist = 0;
    u8  InOutState = 0; //   0 ��ʾ in   1  ��ʾOut

    //  1. get value
    Latitude = (LatiStr[0] << 24) + (LatiStr[1] << 16) + (LatiStr[2] << 8) + LatiStr[3];
    Longitude = (LongiStr[0] << 24) + (LongiStr[1] << 16) + (LongiStr[2] << 8) + LongiStr[3];



    for(i = 0; i < 8; i++)
    {
        InOutState = 0;
        memset((u8 *)&Rail_Cycle, 0, sizeof(Rail_Cycle));
        //Api_RecordNum_Read(Rail_cycle,i+1, (u8*)&Rail_Cycle,sizeof(Rail_Cycle));
        memcpy((u8 *)&Rail_Cycle, (u8 *)&Rail_Cycle_multi[i], sizeof(Rail_Cycle));
        // rt_kprintf("\r\n\r\n Բ��Χ�� ��Ч״̬:%d  TYPE: %d    atrri=%d  lati: %d  longiti:%d  radicus:%d  maxspd: %d  keepdur:%d \r\n",Rail_Cycle.Effective_flag,Rail_Cycle.Area_ID,Rail_Cycle.Area_attribute,Rail_Cycle.Center_Latitude,Rail_Cycle.Center_Longitude,Rail_Cycle.Radius,Rail_Cycle.MaxSpd,Rail_Cycle.KeepDur);


        if(Rail_Cycle.Effective_flag == 1)
        {

            DeltaLatiDis = abs(Latitude - Rail_Cycle.Center_Latitude) / 9; //  a/1000000*111000=a/9.009	����һ����õ����� �ٳ��� 111000 �׵õ�ʵ�ʾ���

            DeltaLongiDis = abs(Longitude - Rail_Cycle.Center_Longitude) * 853 / 10000; // a/1000000*85300=a 853/10000 m

            CacuDist = sqrt((DeltaLatiDis * DeltaLatiDis) + (DeltaLongiDis * DeltaLongiDis));

            rt_kprintf("\r\n  TemperLati  %d  TemperLongi	%d	  Centerlati %d  center longi %d\r\n", Latitude, Longitude, Rail_Cycle.Center_Latitude, Rail_Cycle.Center_Longitude);
            rt_kprintf("\r\n  he=%d heng=%d   shu=%d   juli=%d\r\n", abs(Longitude - Rail_Cycle.Center_Longitude), DeltaLongiDis, DeltaLatiDis, CacuDist);

            if(DeltaLatiDis > Rail_Cycle.Radius )
            {
                // ���γ�Ⱦ������ �뾶�϶���
                InOutState = 1;
            }
            else
            {
                DeltaLongiDis = abs(Longitude - Rail_Cycle.Center_Longitude) * 853 / 10000; // a/1000000*85300=a 853/10000 m
                if(DeltaLongiDis > Rail_Cycle.Radius )
                {
                    // ������Ⱦ�����ڰ뾶�϶���
                    InOutState = 1;
                }
                else  //  �������������
                    CacuDist = sqrt((DeltaLatiDis * DeltaLatiDis) + (DeltaLongiDis * DeltaLongiDis));
            }


            // 1. �ж�����
            if(Rail_Cycle.Area_attribute & 0x0001) //Bit 0 ����ʱ��
            {
                if(CurrentTime_Judge(Rail_Cycle.StartTimeBCD, Rail_Cycle.EndTimeBCD) == false)
                {
                    rt_kprintf("\r\n ʱ��û�������� \r\n");
                    return;
                }
                //continue;
            }
            if(Rail_Cycle.Area_attribute & 0x0002) //Bit 1 ����
            {
                if(Spd_Using > Rail_Cycle.MaxSpd)
                {
                    StatusReg_SPD_WARN(); //  ���ٱ���״̬
                    rt_kprintf("\r\n  �趨Χ�����ٱ���\r\n");
                }
                else
                    StatusReg_SPD_NORMAL();
                //continue;
            }
            if(Rail_Cycle.Area_attribute & 0x0004) //Bit 2 �����򱨾�����ʻԱ
            {


                //continue;
            }
            if(Rail_Cycle.Area_attribute & 0x0008) //Bit 3 �����򱨾���ƽ̨
            {
                if((InOutState == 0) && (CacuDist < Rail_Cycle.Radius ) && (Rail_Cycle.MaxSpd > (Speed_gps / 10)))
                {
                    Warn_Status[1] |= 0x10; // �������򱨾�
                    InOut_Object.TYPE = 1; //Բ������
                    InOut_Object.ID = i; //  ID
                    InOut_Object.InOutState = 0; //  ������
                    rt_kprintf("\r\n -----Բ�ε���Χ��--�뱨��");
                    break;
                }
                //continue;
            }
            if(Rail_Cycle.Area_attribute & 0x0010) //Bit 4 �����򱨾���˾��
            {
                ;
                //continue;
            }
            if((Rail_Cycle.Area_attribute & 0x0020) && (Rail_Cycle.MaxSpd > (Speed_gps / 10))) //Bit 5 �����򱨾���ƽ̨
            {
                if((InOutState == 1) || (CacuDist > Rail_Cycle.Radius ))
                {
                    Warn_Status[1] |= 0x10; // �������򱨾�
                    InOut_Object.TYPE = 1; //Բ������
                    InOut_Object.ID = i; //  ID
                    InOut_Object.InOutState = 1; //  ������
                    rt_kprintf("\r\n -----Բ�ε���Χ��--������");
                    break;
                }

                //continue;
            }
        }

    }



}

void RectangleRail_Judge(u8 *LatiStr, u8 *LongiStr)
{
    u8 i = 0;
    u32 Latitude = 0, Longitude = 0;
    //	u32 DeltaLatiDis=0,DeltaLongiDis=0,CacuDist=0;
    u8	InOutState = 1;	//	 0 ��ʾ in	 1	��ʾOut


    //  1. get value
    Latitude = (LatiStr[0] << 24) + (LatiStr[1] << 16) + (LatiStr[2] << 8) + LatiStr[3];
    Longitude = (LongiStr[0] << 24) + (LongiStr[1] << 16) + (LongiStr[2] << 8) + LongiStr[3];


    // rt_kprintf("\r\n  1---TemperLati  %d  TemperLongi	%d	 res %d\r\n",Latitude,Longitude,InOutState);

    for(i = 0; i < 8; i++)
    {
        InOutState = 1;
        // Api_RecordNum_Read(Rail_rect,i+1, (u8*)&Rail_Rectangle, sizeof(Rail_Rectangle));
        memcpy((u8 *)&Rail_Rectangle, (u8 *)&Rail_Rectangle_multi[i], sizeof(Rail_Rectangle));

        if(Rail_Rectangle.Effective_flag == 1)
        {

            //  rt_kprintf("\r\n\r\n �жϾ�����Χ�� ��Ч:%d ID: %d  atrri=%X  leftlati: %d  leftlongiti:%d    rightLati:%d   rightLongitu: %d	\r\n",Rail_Rectangle.Effective_flag,i+1,Rail_Rectangle.Area_attribute,Rail_Rectangle.LeftUp_Latitude,Rail_Rectangle.LeftUp_Longitude,Rail_Rectangle.RightDown_Latitude,Rail_Rectangle.RightDown_Longitude);
            if((Latitude > Rail_Rectangle.RightDown_Latitude) && (Latitude < Rail_Rectangle.LeftUp_Latitude) && (Longitude > Rail_Rectangle.LeftUp_Longitude) && (Longitude < Rail_Rectangle.RightDown_Longitude))
                InOutState = 0;

            //rt_kprintf("\r\n  TemperLati  %d  TemperLongi  %d   res %d\r\n",Latitude,Longitude,InOutState);

            // 1. �ж�����
            if(Rail_Rectangle.Area_attribute & 0x0001) //Bit 0 ����ʱ��
            {

                //continue;
            }
            if(Rail_Rectangle.Area_attribute & 0x0002) //Bit 1 ����
            {

                //continue;
            }
            if(Rail_Rectangle.Area_attribute & 0x0004) //Bit 2 �����򱨾�����ʻԱ
            {


                //continue;
            }
            if(Rail_Rectangle.Area_attribute & 0x0008) //Bit 3 �����򱨾���ƽ̨
            {
                if(InOutState == 0)
                {
                    Warn_Status[1] |= 0x10; // �������򱨾�
                    InOut_Object.TYPE = 2; //��������
                    InOut_Object.ID = i; //	ID
                    InOut_Object.InOutState = 0; //  ������
                    rt_kprintf("\r\n -----���ε���Χ��--�뱨��");
                    break;
                }
                //continue;
            }
            if(Rail_Rectangle.Area_attribute & 0x0010) //Bit 4 �����򱨾���˾��
            {


                // continue;
            }
            if(Rail_Rectangle.Area_attribute & 0x0020) //Bit 5 �����򱨾���ƽ̨
            {
                if(InOutState == 1)
                {
                    Warn_Status[1] |= 0x10; // �������򱨾�
                    InOut_Object.TYPE = 2; //��������
                    InOut_Object.ID = i; //	ID
                    InOut_Object.InOutState = 1; //  ������
                    rt_kprintf("\r\n -----���ε���Χ��--������");
                    break;
                }

                // continue;
            }


        }

    }




}
#if  0
void RouteRail_Judge(u8 *LatiStr, u8 *LongiStr)
{
    /*
        γ��û�в�ֵ    1γ��  111km
        40��γ���� 1����Ϊ  85.3km   (��������)
    */
    u8 i = 0;
    u8 route_cout = 0, seg_count = 0, seg_num = 0;
    u32 Latitude = 0, Longitude = 0;
    // u32 DeltaLatiDis=0,DeltaLongiDis=0,CacuDist=0;
    // u8  InOutState=0;   //   0 ��ʾ in   1  ��ʾOut
    u32  Route_Status = 0; // ÿ��bit ��ʾ һ��·�� ƫ��״̬Ĭ��Ϊ0
    u32  Segment_Status = 0; //  ��ǰ��·�У���Ӧ�˵�ƫ������� Ĭ��Ϊ0
    u32  Distance = 0;
    //     u8    InAreaJudge=0; //  �ж��Ƿ����ж����� bit 0 ���ȷ�Χ bit  1 γ�ȷ�Χ
    u32  Distance_Array[6]; //�洢������·����С���룬Ĭ���Ǹ�����ֵ

    //  1. get value
    Latitude = (LatiStr[0] << 24) + (LatiStr[1] << 16) + (LatiStr[2] << 8) + LatiStr[3];
    Longitude = (LongiStr[0] << 24) + (LongiStr[1] << 16) + (LongiStr[2] << 8) + LongiStr[3];

    // rt_kprintf("\r\n ��ǰ---->  Latitude:   %d     Longitude: %d\r\n",Latitude,Longitude);

    //  2.  Judge
    for(route_cout = 0; route_cout < Route_Mum; route_cout++) // ��ȡ·��
    {

        // 2.1  --------   ��ȡ·��-----------
        memset((u8 *)&ROUTE_Obj, 0, sizeof(ROUTE_Obj)); //  clear all  first
        DF_ReadFlash(DF_Route_Page + route_cout, 0, (u8 *)&ROUTE_Obj, sizeof(ROUTE_Obj));
        DF_delay_us(20);
        //rt_kprintf("\r\n -----> ROUTE_Obj.RouteID:   %d \r\n",ROUTE_Obj.Route_ID);
        // 2.2  -----  �ж��Ƿ���Ч  -------
        if((ROUTE_Obj.Effective_flag == 1) && (ROUTE_Obj.Points_Num > 1)) //  �ж��Ƿ���Ч���йյ㣬����Ч������
        {
            // 2.2.0    ��ǰ�ξ��븶��һ�������ֵ
            for(i = 0; i < 6; i++)
                Distance_Array[i] = ROUTE_DIS_Default;
            // 2.2.1      �������
            seg_num = ROUTE_Obj.Points_Num - 1; // ��·����Ŀ
            //  2.2.2    �ж�·����ÿһ�ε�״̬
            Segment_Status = 0; // ������ж�״̬��ÿ����·���¿�ʼһ��
            for(seg_count = 0; seg_count < seg_num; seg_count++)
            {
                if((ROUTE_Obj.RoutePoints[seg_count + 1].POINT_Latitude == 0) && (ROUTE_Obj.RoutePoints[seg_count + 1].POINT_Longitude == 0))
                {
                    rt_kprintf("\r\n  �õ�Ϊ0 ��jump\r\n");
                    continue;
                }
                //----- ��ʼ���������, ��û�������ں�����������ж�
                Distance_Array[seg_count] = Distance_Point2Line(Latitude, Longitude, ROUTE_Obj.RoutePoints[seg_count].POINT_Latitude, ROUTE_Obj.RoutePoints[seg_count].POINT_Longitude, ROUTE_Obj.RoutePoints[seg_count + 1].POINT_Latitude, ROUTE_Obj.RoutePoints[seg_count + 1].POINT_Longitude);

            }
            //=========================================================
            //  2.4 ------  ��ӡ��ʾ���룬�ҳ���С��ֵ----
            Distance = Distance_Array[0]; // ��С����
            for(i = 0; i < 6; i++)
            {
                if(Distance >= Distance_Array[i])
                    Distance = Distance_Array[i];
                // rt_kprintf("\r\n  Distance[%d]=%d",i,Distance_Array[i]);
            }
            rt_kprintf("\r\n MinDistance =%d  Width=%d \r\n", Distance, (ROUTE_Obj.RoutePoints[seg_num].Width >> 1));	 //

            if(Distance < ROUTE_DIS_Default)
            {
                //  ---- ��·�ο�����Ա�
                if(Distance > (ROUTE_Obj.RoutePoints[seg_num].Width >> 1))
                {
                    rt_kprintf("\r\n ·��ƫ��\r\n");
                    Segment_Status |= (1 << seg_num); //  ����Ӧ��bit  ��λ
                }
            }

            //
        }
        // 2.4  ���� 2.2 ����жϵ���·��״̬
        if(Segment_Status)
            Route_Status |= (1 << route_cout); //  ����Ӧ��bit  ��λ
    }
    // 3.  Result
    if(Route_Status)
    {
        if( (Warn_Status[1] & 0x80) == 0) //  �����ǰû��������ô��ʱ�ϱ�
        {
            PositionSD_Enable();
            Current_UDP_sd = 1;
        }

        Warn_Status[1] |= 0x80; // ·��ƫ������
        rt_kprintf("\r\n    ·��ƫ������ !\r\n");

    }
    else
    {
        if (Warn_Status[1] & 0x80) //  �����ǰû��������ô��ʱ�ϱ�
        {
            PositionSD_Enable();
            Current_UDP_sd = 1;
        }

        Warn_Status[1] &= ~0x80; // ·��ƫ������
    }


}

//--------  D�㵽ֱ�߾������-------
/*
     P1(x1,y1)   P2(x2,y2)  ,�ѵ�P(x1,y2)��Ϊ����ԭ�㣬��x1=0��y2=0��

     ��ô����P1��P2 ȷ����ֱ�߷���(����ʽ)Ϊ:
             (x-x1)/(x2-x1) =(y-y1)/(y2-y1)                          (1)

    ע:  ��׼ʽֱ�߷���Ϊ AX+BY+C=0;
             ��ôƽ��������һ��P(x0,y0) ��ֱ�ߵľ����ʾΪ
             d=abs(Ax0+By0+C)/sqrt(A^2+B^2)

    ���аѷ���ʽ(1) ת���ɱ�׼ʽΪ:
            (y2-y1)x+(x1-x2)y+x1(y1-y2)+y1(x2-x1)=0;

   ���ڵ�(x1,y2)Ϊԭ��  ��x1=0��y2=0��  P1(0,y1) , P2(x2,0)
    ����   A=-y1 ,  B=-x2, C=y1x2
    ��ô ֱ�ߵķ���:
                  -y1x-x2y+y1x2=0;  (2)

  =>     d=abs(-y1x0-x2y0+y1x2)/sqrt(y1^2+x2^2)       (3)

         ���� (3)  Ϊ����Ӧ�õĹ�ʽ

        ע:  �ȸ��ݾ�γ���ۺϼ���� x0��y0��x1,y1,x2,y2  ����ֵ��λΪ: ��
=>  �����ж�:
           ����(2) �������  �� P1(0,y1) , P2(x2,0) ������ֱ֪�ߴ�ֱ������ֱ�߷���
              P1(0,y1) :      x2x-y1y+y1^2=0  (4)
              P2(x2,0) :      x2x-y1y-x2^2=0  (5)

          ��� y1 >=0      ֱ��(4)    ��ֱ��(5)  ���ϱ�
          ��ô �����߶������ڵ��жϷ�����
                       (4) <=  0    ��  (5)  >=0
       ��
           ��� y1 <=0      ֱ��(5)    ��ֱ��(4)  ���ϱ�
          ��ô �����߶������ڵ��жϷ�����
                       (4) >=  0    ��  (5)  <=0
   //------------------------------------------------------------------------------------------

       γ��û�в�ֵ    1γ��  111km
       40��γ���� 1����Ϊ  85.3km   (��������)

       X ��Ϊ ����(longitude) ��ֵ
       Y ��Ϊγ�� (latitude)  ��ֵ


   //------------------------------------------------------------------------------------------
*/

u32   Distance_Point2Line(u32 Cur_Lat, u32  Cur_Longi, u32 P1_Lat, u32 P1_Longi, u32 P2_Lat, u32 P2_Longi)
{
    //   ���뵱ǰ�� �����ص㵽����ֱ�ߵľ���
    long  x0 = 0, y0 = 0, Line4_Resualt = 0, Line5_Resualt = 0; // ��λ: ��
    long  y1 = 0;
    long  x2 = 0;
    long   distance = 0;
    // long  Rabs=0;
    //      long  Rsqrt=0;
    //  long  DeltaA1=0,DeltaA2=0,DeltaO1=0,DeltaO2=0; //  DeltaA : Latitude     DeltaO:  Longitude
    // u32   Line4_Resualt2=0,Line5_Resualt2=0;
    double   fx0 = 0, fy0 = 0, fy1 = 0, fx2 = 0;
    double   FLine4_Resualt2 = 0, FLine5_Resualt2 = 0, fRabs = 0, fRsqrt = 0;

    // 0.   �ȴ��Ե��ж�
    //   DeltaA1=abs(Cur_Lat-P1_Lat);
    //   DeltaA2=abs(Cur_Lat-P2_Lat);
    //	DeltaO1=abs(Cur_Lat-P1_Longi);
    //  DeltaO2=abs(Cur_Lat-P2_Longi);
    /* if((DeltaA1>1000000) &&(DeltaA2>1000000))
         {
             rt_kprintf("\r\n  Latitude ��̫��\r\n");
             return   ROUTE_DIS_Default;
     }
      if((DeltaO1>1000000) &&(DeltaO2>1000000))
         {
             rt_kprintf("\r\n  Longitude ��̫��\r\n");
             return   ROUTE_DIS_Default;
     }
    */
    // 1.  ��ȡ  P1(0,y1)   P2(x2,0) ,��P(x0,y0)    P(x1,y2)Ϊԭ��  ��x1=0��y2=0��  P1(0,y1) , P2(x2,0)
    x2 = abs(P2_Longi - P1_Longi); // a/1000000*85300=a 853/10000 m =a x 0.0853
    if(P2_Longi < P1_Longi)
        x2 = 0 - x2;
    fx2 = (double)((double)x2 / 1000);
    //rt_kprintf("\r\n P2_L=%d,P1_L=%d   delta=%d \r\n",P2_Longi,P1_Longi,(P2_Longi-P1_Longi));
    // if(P2_Longi
    y1 = abs(P2_Lat - P1_Lat); //  a/1000000*111000=a/9.009	����һ����õ����� �ٳ��� 111000 �׵õ�ʵ�ʾ���
    if(P2_Lat < P1_Lat)
        y1 = 0 - y1;
    fy1 = (double)((double)y1 / 1000);
    //rt_kprintf("\r\n P2_LA=%d,P1_LA=%d   delta=%d \r\n",P2_Lat,P1_Lat,(P2_Lat-P1_Lat));

    //   rt_kprintf("\r\n ��֪��������: P1(0,%d)   P2(%d,0) \r\n", y1,x2);
    //    ��ǰ��
    x0 = abs(Cur_Longi - P1_Longi);
    if(Cur_Longi < P1_Longi)
        x0 = 0 - x0;
    fx0 = (double)((double)x0 / 1000);
    //rt_kprintf("\r\n Cur_L=%d,P1_L=%d   delta=%d \r\n",Cur_Longi,P1_Longi,(Cur_Longi-P1_Longi));

    y0 = abs(Cur_Lat - P2_Lat); //  a/1000000*111000=a/9.009
    if(Cur_Lat < P2_Lat)
        y0 = 0 - y0;
    fy0 = (double)((double)y0 / 1000);
    // rt_kprintf("\r\n Cur_La=%d,P2_La=%d   delta=%d \r\n",Cur_Lat,P2_Lat,(Cur_Lat-P2_Lat));
    //   rt_kprintf("\r\n��ǰ������: P0(%d,%d)    \r\n", x0,y0);
    // 2. �ж�y1  �Ĵ�С�� ����� P1(0,y1)   P2(x2,0) ,����ֱ֪�ߵķ��̣����ж�
    //     ��ǰ���Ƿ���·�δ�ֱ��Χ��

    //  2.1   ����ǰ����룬 �� P1(0,y1)   �� ֱ�߷���(4)  ������
    Line4_Resualt = (x2 * x0) - (y1 * y0) + (y1 * y1);
    FLine4_Resualt2 = fx2 * fx0 - fy1 * fy0 + fy1 * fy1;
    //     rt_kprintf("\r\n Line4=x2*x0-y1*y0+y1*y1=(%d)*(%d)-(%d)*(%d)+(%d)*(%d)=%ld     x2*x0=%d    y1*y0=%d   y1*y1=%d  \r\n",x2,x0,y1,y0,y1,y1,Line4_Resualt,x2*x0,y1*y0,y1*y1);
    //     rt_kprintf("\r\n FLine4=fx2*fx0-fy1*fy0+fy1*fy1=(%f)*(%f)-(%f)*(%f)+(%f)*(%f)=%f      fx2*fx0=%f    fy1*fy0=%f   fy1*fy1=%f  \r\n",fx2,fx0,fy1,fy0,fy1,fy1,FLine4_Resualt2,fx2*fx0,fy1*fy0,fy1*fy1);

    //   2.2   ����ǰ����룬 ��P2(x2,0) �� ֱ�߷���(5)  ������
    Line5_Resualt = (x2 * x0) - y1 * y0 - x2 * x2;
    FLine5_Resualt2 = fx2 * fx0 - fy1 * fy0 - fx2 * fx2;
    //rt_kprintf("\r\n Line5=x2*x0-y1*y0-x2*x2=(%d)*(%d)-(%d)*(%d)-(%d)*(%d)=%ld     Se : %ld   \r\n",x2,x0,y1,y0,x2,x2,Line5_Resualt,Line5_Resualt2);
    //    rt_kprintf("\r\n FLine5=fx2*fx0-fy1*fy0-fx2*fx2=(%f)*(%f)-(%f)*(%f)-(%f)*(%f)=%f      fx2*fx0=%f    fy1*fy0=%f   fx2*fx2=%f  \r\n",fx2,fx0,fy1,fy0,fx2,fx2,FLine5_Resualt2,fx2*fx0,fy1*fy0,fx2*fx2);
    // rt_kprintf("\r\n  Line4_Resualt=%d     Line5_Resualt=%d  \r\n",Line4_Resualt,Line5_Resualt);

    if(fy1 >= 0)    //  ֱ��(4) ���Ϸ�
    {

        //   2.3   �ж�����    (4) <=  0    ��  (5)  >=0     // �ж�����ȡ��
        if((FLine4_Resualt2 > 0) || (FLine5_Resualt2 < 0))
            return   ROUTE_DIS_Default;      //  �������������������ֵ
    }
    else
    {
        //  ֱ��(5)

        //   2.4   �ж�����     (4) >=  0    ��  (5)  <=0     // �ж�����ȡ��
        if((FLine4_Resualt2 < 0) || (FLine5_Resualt2 > 0))
            return   ROUTE_DIS_Default;      //  �������������������ֵ

    }

    rt_kprintf("\r\n In judge area \r\n");
    //rt_kprintf("\r\n   Current== Latitude:   %d     Longitude: %d     Point1== Latitude:   %d     Longitude: %d     Point2== Latitude:   %d     Longitude: %d\r\n",Cur_Lat,Cur_Longi,P1_Lat,P1_Longi,P2_Lat,P2_Longi);

    //  3. ����ֵ�����ʵ�ʾ���
#if 0
    x2 = x2 * 0.0853; // a/1000000*85300=a 853/10000 m =a x 0.0853
    y1 = y1 / 9; //  a/1000000*111000=a/9.009	����һ����õ����� �ٳ��� 111000 �׵õ�ʵ�ʾ���
    x0 = x0 * 0.0853;
    y0 = y0 / 9; //  a/1000000*111000=a/9.009
#else
    fx2 = fx2 * 0.0853; // a/1000000*85300=a 853/10000 m =a x 0.0853
    fy1 = fy1 / 9; //  a/1000000*111000=a/9.009	����һ����õ����� �ٳ��� 111000 �׵õ�ʵ�ʾ���
    fx0 = fx0 * 0.0853;
    fy0 = fy0 / 9; //  a/1000000*111000=a/9.009
#endif

    //  4. �������
    //Rabs=0-y1*x0-x2*y0+y1*x2;
    // rt_kprintf("\r\n Test -y1*x0=%d -y0*x2=%d  y1*x2=%d   Rabs=%d  \r\n",0-y1*x0,0-y0*x2,0-y1*x2,Rabs);
#if 0
    Rabs = abs(-y1 * x0 - x2 * y0 + y1 * x2);
    Rsqrt = sqrt(y1 * y1 + x2 * x2);
    // distance=abs(-y1*x0-x2*y0-y1*x2)/sqrt(y1*y1+x2*x2);
    distance = Rabs / Rsqrt;
    // rt_kprintf("\r\n Rabs=%d    Rsqrt=%d   d=%d",Rabs,Rsqrt,distance);
#else
    fRabs = abs(-fy1 * fx0 - fx2 * fy0 + fy1 * fx2);
    fRsqrt = sqrt(fy1 * fy1 + fx2 * fx2);
    // distance=abs(-y1*x0-x2*y0-y1*x2)/sqrt(y1*y1+x2*x2);
    distance = (long) ((fRabs / fRsqrt) * 1000);
    // rt_kprintf("\r\n Rabs=%d    Rsqrt=%d   d=%d",Rabs,Rsqrt,distance);
#endif


    return   distance;
}
#endif

unsigned short int File_CRC_Get(void)
{

    u8   buffer_temp[514];
    unsigned short int i = 0;
    u16  packet_num = 0, leftvalue = 0; // 512    per packet
    u32  File_size = 0;

    DF_TAKE;
    memset(buffer_temp, 0, sizeof(buffer_temp));

    //  ��ȡ  �ļ�ͷ��Ϣ
    SST25V_BufferRead( buffer_temp, ISP_StartArea,  256 );
    File_size = (buffer_temp[114] << 24) + (buffer_temp[115] << 16) + (buffer_temp[116] << 8) + buffer_temp[117];

    leftvalue = File_size % 512;
    rt_kprintf("\r\n �ļ���С: %d Bytes  leftvalue=%d \r\n", File_size, leftvalue);
    FileTCB_CRC16 = (buffer_temp[134] << 8) + buffer_temp[135];
    rt_kprintf("\r\n Read CRC16: 0x%X Bytes\r\n", FileTCB_CRC16);

    // OutPrint_HEX("1stpacket",buffer_temp,256);

    if(leftvalue)   // ���� 512
        packet_num = (File_size >> 9) + 1;
    else
        packet_num = (File_size >> 9);


    for(i = 0; i < packet_num; i++)
    {
        if(i == 0) //��һ��
        {
            Last_crc = 0; // clear first
            crc_fcs = 0;
            SST25V_BufferRead(buffer_temp, ISP_StartArea + 256, 512); // 0x001000+256=0x001100   ISP_StartArea+256
            delay_ms(50);
            WatchDog_Feed();
            Last_crc = CRC16_1(buffer_temp, 512, 0xffff);
            //rt_kprintf("\r\n                  i=%d,Last_crc=0x%X",i+1,Last_crc);

            //rt_kprintf("\r\n //----------   %d     packet    len=%d  ",i+1,512);
            //OutPrint_HEX("1stpacket",buffer_temp,512);
        }
        else if(i == (packet_num - 1)) //���һ��
        {
            SST25V_BufferRead(buffer_temp, ISP_StartArea + 256 + i * 512, leftvalue);
            delay_ms(50);
            WatchDog_Feed();
            // rt_kprintf("\r\n //----------   %d     packet    len=%d  ",i+1,leftvalue);
            // OutPrint_HEX("endstpacket",buffer_temp,leftvalue);
            crc_fcs = CRC16_1(buffer_temp, leftvalue, Last_crc);
            rt_kprintf("\r\n                  i=%d,Last_crc=0x%X ReadCrc=0x%X ", i + 1, crc_fcs, FileTCB_CRC16);
        }
        else
        {
            // �м�İ�
            SST25V_BufferRead(buffer_temp, ISP_StartArea + 256 + i * 512, 512);
            delay_ms(50);
            WatchDog_Feed();
            // rt_kprintf("\r\n //----------   %d	 packet    len=%d  ",i+1,512);
            // OutPrint_HEX("midstpacket",buffer_temp,512);
            Last_crc = CRC16_1(buffer_temp, 512, Last_crc);
            // rt_kprintf("\r\n                 i=%d,Last_crc=0x%X",i+1,Last_crc);
        }
    }

    DF_RELEASE;
    rt_kprintf("\r\n  У���� 0x%X \r\n", crc_fcs);

    if(FileTCB_CRC16 == crc_fcs)
    {
        SST25V_BufferRead( buffer_temp, 0x001000, 100 );
        buffer_temp[32] = ISP_BYTE_CrcPass;    //-----   �ļ�У��ͨ��
        SST25V_BufferWrite( buffer_temp, 0x001000, 100);
        rt_kprintf("\r\n  У����ȷ! \r\n", crc_fcs);
        return true;
    }
    else
    {
        rt_kprintf("\r\n  У��ʧ��! \r\n", crc_fcs);
        return false;
    }
}
//FINSH_FUNCTION_EXPORT(File_CRC_Get, File_CRC_Get);

//-----------------------------------------------------------------------------------
void Spd_Exp_Wr(void)
{
    u8  content[40];
    u8  wr_add = 0, i = 0, FCS = 0;

    memset(content, 0, sizeof(content));

    memcpy(content + wr_add, JT808Conf_struct.Driver_Info.DriveName, 18);
    wr_add += 18;
    memcpy(content + wr_add, speed_Exd.ex_startTime, 6);
    wr_add += 6;
    memcpy(content + wr_add, speed_Exd.ex_endTime, 6);
    wr_add += 6;
    content[wr_add++] = speed_Exd.current_maxSpd / 10;

    FCS = 0;
    for ( i = 0; i < 32; i++ )
    {
        FCS ^= content[i];
    }			  //���ϱ����ݵ�����
    content[wr_add++] = FCS;	  // ��31�ֽ�

    Api_DFdirectory_Write(spd_warn, (u8 *)content, 32);
    //----------- debug -----------------------
    if(GB19056.workstate == 0)
        rt_kprintf("\r\n ���ٱ���  %X-%X-%X %X:%X:%X,MaxSpd=%d\r\n", speed_Exd.ex_endTime[0], speed_Exd.ex_endTime[1], speed_Exd.ex_endTime[2], speed_Exd.ex_endTime[3], speed_Exd.ex_endTime[4], speed_Exd.ex_endTime[5], speed_Exd.current_maxSpd);
    //--------- clear status ----------------------------
    Spd_ExpInit();

}

//E:\work_xj\F4_BD\�����г���¼�ǹ���(��Э��)\2-21 RT-Thread_NewBoard-LCD2-Jiance\bsp\stm32f407vgt6_RecDrv\app_712\lcd\Menu_0_3_Sim.c
void CAN_struct_init(void)
{
    //-------  protocol variables
    CAN_trans.can1_sample_dur = 0; // can1 �ɼ����  ms
    CAN_trans.can1_trans_dur = 0; // can1  �ϱ���� s
    CAN_trans.can1_enable_get = 0; // 500ms

    CAN_trans.can2_sample_dur = 0; // can2 �ɼ����  ms
    CAN_trans.can2_trans_dur = 0; // can2  �ϱ���� s

    //   u8      canid_1[8];// ԭʼ����
    CAN_trans.canid_1_Filter_ID = 0; // �����ж���
    //u8      canid_1_Rxbuf[400]; // ����buffer
    CAN_trans.canid_1_RxWr = 0; // дbuffer�±�
    CAN_trans.canid_1_SdWr = 0;
    CAN_trans.canid_1_ext_state = 0; // ��չ֡״̬
    CAN_trans.canid_1_sample_dur = 10; // ��ID �Ĳɼ����
    CAN_trans.canid_ID_enableGet = 0;

    //------- system variables
    CAN_trans.canid_timer = 0; //��ʱ��
    CAN_trans.canid_0705_sdFlag = 0; // ���ͱ�־λ

}

void  CAN_send_timer(void)
{
    u16 i = 0, datanum = 0;;
    if(CAN_trans.can1_trans_dur > 0)
    {
        CAN_trans.canid_timer++;
        // if( CAN_trans.canid_timer>=CAN_trans.can1_trans_dur)
        if( CAN_trans.canid_timer >= 4)
        {
            CAN_trans.canid_timer = 0;
            //------  �ж���û��������
            if(CAN_trans.canid_1_RxWr)
            {
                datanum = (CAN_trans.canid_1_RxWr >> 3);
                memcpy(CAN_trans.canid_1_Sdbuf, CAN_trans.canid_1_Rxbuf, CAN_trans.canid_1_RxWr);
                CAN_trans.canid_1_SdWr = CAN_trans.canid_1_RxWr;
                for(i = 0; i < datanum; i++)
                    CAN_trans.canid_1_ID_SdBUF[i] = CAN_trans.canid_1_ID_RxBUF[i];

                CAN_trans.canid_1_RxWr = 0; // clear
                CAN_trans.canid_0705_sdFlag = 1;
            }

        }

    }
    else
    {
        CAN_trans.canid_0705_sdFlag = 0;
        CAN_trans.canid_timer = 0;
    }
}


void  JT808_Related_Save_Process(void)
{


    // 1. VDR  08H  Data  Save
    if(VdrData.H_08_saveFlag == 1)
    {
        WatchDog_Feed();
        //OutPrint_HEX("08H save",VdrData.H_08_BAK,126);
        if( GB19056.workstate == 0)
            rt_kprintf("\r\n 08H save  %d \r\n", Vdr_Wr_Rd_Offset.V_08H_Write);
        vdr_creat_08h(Vdr_Wr_Rd_Offset.V_08H_Write, VdrData.H_08_BAK, 126);
        Vdr_Wr_Rd_Offset.V_08H_Write++; //  д��֮���ۼӣ��Ͳ��ñ�����
        vdr_cmd_writeIndex_save(0x08, Vdr_Wr_Rd_Offset.V_08H_Write);
        VdrData.H_08_saveFlag = 0;
        return;
    }
    // 2.  VDR  09H  Data  Save
    if(VdrData.H_09_saveFlag == 1)
    {
        WatchDog_Feed();
        // OutPrint_HEX("09H save",VdrData.H_09,666);
        if( GB19056.workstate == 0)
            rt_kprintf("\r\n 09H save  %d\r\n", Vdr_Wr_Rd_Offset.V_09H_Write);
        vdr_creat_09h(Vdr_Wr_Rd_Offset.V_09H_Write, VdrData.H_09, 666);
        Vdr_Wr_Rd_Offset.V_09H_Write++; // //  д��֮���ۼӣ��Ͳ��ñ�����
        vdr_cmd_writeIndex_save(0x09, Vdr_Wr_Rd_Offset.V_09H_Write);
        memset(VdrData.H_09 + 6, 0x0, 660);	 // Ĭ���� 0xFF
        VdrData.H_09_saveFlag = 0;
        return;
    }
    //  3. VDR  10H  Data  Save
    if(VdrData.H_10_saveFlag == 1)
    {
        WatchDog_Feed();
        // OutPrint_HEX("10H save",VdrData.H_10,234);
        if( GB19056.workstate == 0)
            rt_kprintf("\r\n 10H save  %d\r\n", Vdr_Wr_Rd_Offset.V_10H_Write);
        vdr_creat_10h(Vdr_Wr_Rd_Offset.V_10H_Write, VdrData.H_10, 234);
        Vdr_Wr_Rd_Offset.V_10H_Write++; // //  д��֮���ۼӣ��Ͳ��ñ�����
        vdr_cmd_writeIndex_save(0x10, Vdr_Wr_Rd_Offset.V_10H_Write);
        VdrData.H_10_saveFlag = 0;
        return;
    }
    //  4.  VDR  11H  Data  Save
    if(VdrData.H_11_saveFlag)  //  1 : �洢����    2: �洢������
    {
        WatchDog_Feed();
        // OutPrint_HEX("11H save",VdrData.H_11,50);
        if( GB19056.workstate == 0)
        {
            rt_kprintf("\r\n 11H save   %d \r\n", Vdr_Wr_Rd_Offset.V_11H_Write);
            // OutPrint_HEX("11H save",VdrData.H_11,50);
        }
        vdr_creat_11h(Vdr_Wr_Rd_Offset.V_11H_Write, VdrData.H_11, 50);
        if(VdrData.H_11_saveFlag == 1)
            Vdr_Wr_Rd_Offset.V_11H_Write++; // //  д��֮���ۼӣ��Ͳ��ñ�����

        WatchDog_Feed();
        delay_ms(20);
        vdr_cmd_writeIndex_save(0x11, Vdr_Wr_Rd_Offset.V_11H_Write);
        VdrData.H_11_saveFlag = 0;
        return;
    }
    //  5.  VDR  12H  Data  Save
    if(VdrData.H_12_saveFlag == 1)
    {
        WatchDog_Feed();
        //OutPrint_HEX("12H save",VdrData.H_12,25);
        if( GB19056.workstate == 0)
            rt_kprintf("\r\n 12H save  %d \r\n", Vdr_Wr_Rd_Offset.V_12H_Write);
        vdr_creat_12h(Vdr_Wr_Rd_Offset.V_12H_Write, VdrData.H_12, 25);
        Vdr_Wr_Rd_Offset.V_12H_Write++; // //  д��֮���ۼӣ��Ͳ��ñ�����
        vdr_cmd_writeIndex_save(0x12, Vdr_Wr_Rd_Offset.V_12H_Write);
        VdrData.H_12_saveFlag = 0;

        // rt_kprintf("\r\n дIC ��  ��¼  \r\n");
        return;
    }
    //  6.  VDR  13H  Data  Save
    if(VdrData.H_13_saveFlag == 1)
    {
        WatchDog_Feed();
        // OutPrint_HEX("13H save",VdrData.H_13,7);
        if( GB19056.workstate == 0)
            rt_kprintf("\r\n 13H save %d \r\n", Vdr_Wr_Rd_Offset.V_13H_Write);
        vdr_creat_13h(Vdr_Wr_Rd_Offset.V_13H_Write, VdrData.H_13, 7);
        Vdr_Wr_Rd_Offset.V_13H_Write++; // //  д��֮���ۼӣ��Ͳ��ñ�����
        vdr_cmd_writeIndex_save(0x13, Vdr_Wr_Rd_Offset.V_13H_Write);
        VdrData.H_13_saveFlag = 0;
        return;
    }
    //  7.  VDR  14H  Data  Save
    if(VdrData.H_14_saveFlag == 1)
    {
        WatchDog_Feed();
        //OutPrint_HEX("14H save",VdrData.H_14,7);
        if( GB19056.workstate == 0)
            rt_kprintf("\r\n 14H save  %d\r\n", Vdr_Wr_Rd_Offset.V_14H_Write);
        vdr_creat_14h(Vdr_Wr_Rd_Offset.V_14H_Write, VdrData.H_14, 7);
        Vdr_Wr_Rd_Offset.V_14H_Write++; // //  д��֮���ۼӣ��Ͳ��ñ�����
        vdr_cmd_writeIndex_save(0x14, Vdr_Wr_Rd_Offset.V_14H_Write);
        VdrData.H_14_saveFlag = 0;
        return;
    }
    // 8.  VDR  15H  Data save
    if(VdrData.H_15_saveFlag == 1)
    {
        //OutPrint_HEX("15H save",VdrData.H_15,133;
        if( GB19056.workstate == 0)
            rt_kprintf("\r\n 15H save  %d\r\n", Vdr_Wr_Rd_Offset.V_15H_Write);
        vdr_creat_15h(Vdr_Wr_Rd_Offset.V_15H_Write, VdrData.H_15, 133);
        Vdr_Wr_Rd_Offset.V_15H_Write++; // //  д��֮���ۼӣ��Ͳ��ñ�����
        vdr_cmd_writeIndex_save(0x15, Vdr_Wr_Rd_Offset.V_15H_Write);
        VdrData.H_15_saveFlag = 0;
    }
    //-----------------  ���ٱ��� ----------------------
    if(speed_Exd.excd_status == 2)
    {
        Spd_Exp_Wr();
        return;
    }

    //	 ��ʱ�洢���
    if((Vehicle_RunStatus == 0x01) && (DistanceWT_Flag == 1))
    {
        //  �����������ʻ�����У�ÿ255 ��洢һ���������
        //rt_kprintf("\r\n distance --------\r\n");
        DistanceWT_Flag = 0; // clear
        DF_Write_RecordAdd(Distance_m_u32, DayStartDistance_32, TYPE_DayDistancAdd);
        JT808Conf_struct.DayStartDistance_32 = DayStartDistance_32;
        JT808Conf_struct.Distance_m_u32 = Distance_m_u32;
        return;
    }

    //--------------------------------------------------------------

}


/***********************************************************
* Function:
* Description:
* Input:
* Input:
* Output:
* Return:
* Others:
***********************************************************/
void Tired_Check( void )
{

    //	2.	�ж���ʻ״̬
    if((Speed_gps >= 60) || (Speed_cacu >= 60))	// ��¼����֤ʱ���������� 1km/h
    {
        //	11 H   ������ʻ��ʼʱ��   ����ʼλ��	 ��60s ��ʼ���ʻ��ʼ
        if(VDR_TrigStatus.Running_state_enable == 0)
        {
            VdrData.H_08_counter = time_now.sec; // start
        }
        VDR_TrigStatus.Running_state_enable = 1; //  ������ʻ״̬
    }
    if((Speed_gps < 60) && (Speed_cacu < 60))
    {
        //		  11 H	   ���    ����4 Сʱ ��   �ٶȴӴ󽵵� 0  ���Ŵ�����ʱ��¼
        if(VDR_TrigStatus.Running_state_enable == 1)
        {
            Different_DriverIC_End_Process();
        }
        VDR_TrigStatus.Running_state_enable = 0; //  ����ֹͣ״̬
    }


    //   2.   ��ʻ��ʱ�ж�
    // 2.1    H_11   Start    acc on Ϊ 0 ʱ��ʼ��ʱ��¼
    Different_DriverIC_Start_Process();
    // 2.2    acc on  counter     -------------ƣ�ͼ�ʻ��� -----------------------
    Different_DriverIC_Checking();
    //--------------------------------------------------------------------------------
    //------------------------------------------------
}



/*
    ��ӡ��� HEX ��Ϣ��Descrip : ������Ϣ ��instr :��ӡ��Ϣ�� inlen: ��ӡ����
*/
void OutPrint_HEX(u8 *Descrip, u8 *instr, u16 inlen )
{
    u32  i = 0;
    rt_kprintf("\r\n %s:", Descrip);
    for( i = 0; i < inlen; i++)
        rt_kprintf("%02X ", instr[i]);
    rt_kprintf("\r\n");
}

void  dur(u8 *content)
{
    sscanf(content, "%d", (u32 *)&Current_SD_Duration);
    rt_kprintf("\r\n �ֶ������ϱ�ʱ���� %d s \r\n", Current_SD_Duration);

    JT808Conf_struct.DURATION.Default_Dur = Current_SD_Duration;
    JT808Conf_struct.DURATION.Sleep_Dur = Current_SD_Duration; //  ���ó�һ��
    Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));

}
FINSH_FUNCTION_EXPORT(dur, dur);

void adjust_ok(int in)
{

    if(in > 1)
        return;
    if(in == 1)
    {
        JT808Conf_struct.DF_K_adjustState = 1;
        ModuleStatus |= Status_Pcheck;
    }
    else
    {
        JT808Conf_struct.DF_K_adjustState = 0;
        ModuleStatus &= ~Status_Pcheck;
    }
    rt_kprintf("adjust_ok(%d)\r\n", in);
    Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
}
FINSH_FUNCTION_EXPORT(adjust_ok, set_adjust_ok  1: ok 0: notok);

void spd_type(int  in)
{
    if(in > 1)
        return;

    if(in == 0)
    {
        JT808Conf_struct.Speed_GetType = 0;
        JT808Conf_struct.DF_K_adjustState = 0;
        ModuleStatus &= ~Status_Pcheck;

        rt_kprintf("spd_type: gps_speed get\r\n");
    }
    else if(in == 1)
    {
        JT808Conf_struct.Speed_GetType = 1;
        JT808Conf_struct.DF_K_adjustState = 0;
        ModuleStatus &= ~Status_Pcheck;

        rt_kprintf("spd_type: sensor_speed get\r\n");
    }
    Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));

}
//FINSH_FUNCTION_EXPORT(spd_type, spd_type  gps->0  sensor->1);

void  plus_num(u32 value)
{
    JT808Conf_struct.Vech_Character_Value = value;

    // rt_kprintf("plus_num(%d)\r\n",value);
    Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));

}
//FINSH_FUNCTION_EXPORT(plus_num, set_plus_num);


void chepai(u8 *instr)
{
    memset(Vechicle_Info.Vech_Num, 0, sizeof(Vechicle_Info.Vech_Num));
    memcpy(Vechicle_Info.Vech_Num, instr, 8);
    DF_WriteFlashSector(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
    WatchDog_Feed();
    DF_WriteFlashSector(DF_VehicleBAK_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
    WatchDog_Feed();
    DF_WriteFlashSector(DF_VehicleBAK2_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
}
FINSH_FUNCTION_EXPORT(chepai, set_chepai);

void  vin_set(u8 *instr)
{
    //����VIN
    memset(Vechicle_Info.Vech_VIN, 0, sizeof(Vechicle_Info.Vech_VIN));
    memcpy(Vechicle_Info.Vech_VIN, instr, strlen((const char *)instr));
    DF_WriteFlashSector(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
    WatchDog_Feed();
    DF_WriteFlashSector(DF_VehicleBAK_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
    WatchDog_Feed();
    DF_WriteFlashSector(DF_VehicleBAK2_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));

    rt_kprintf("\r\n �ֶ�����vin:%s \r\n", instr);

}
//FINSH_FUNCTION_EXPORT(vin_set, vin_set );
#if 0
void  current(void)
{
    PositionSD_Enable();
    Current_UDP_sd = 1;
}
//FINSH_FUNCTION_EXPORT(current, current );
#endif

void  link_mode(u8 *instr)
{
    if(instr[0] == '1')
    {
        Vechicle_Info.Link_Frist_Mode = 1;
        rt_kprintf("\r\n Mainlink:%s \r\n", instr);
    }
    else if(instr[0] == '0')
    {
        Vechicle_Info.Link_Frist_Mode = 0;
        rt_kprintf("\r\n DNSR :%s \r\n", instr);
    }

    DF_WriteFlashSector(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
    WatchDog_Feed();
    DF_WriteFlashSector(DF_VehicleBAK_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));

    WatchDog_Feed();
    DF_WriteFlashSector(DF_VehicleBAK2_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));

}
FINSH_FUNCTION_EXPORT(link_mode, link_mode );

void  redial(void)
{
    DataLink_EndFlag = 1; //AT_End();
    // rt_kprintf("\r\n Redial\r\n");
}
//FINSH_FUNCTION_EXPORT(redial, redial);

void  port_main(u8 *instr)
{
    sscanf(instr, "%d", (u32 *)&RemotePort_main);
    rt_kprintf("\r\n�������˿�=%d!", RemotePort_main);
    SysConf_struct.Port_main = RemotePort_main;
    Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));

    DataLink_MainSocket_set(RemoteIP_main, RemotePort_main, 0);
    DataLink_EndFlag = 1; //AT_End();

}
FINSH_FUNCTION_EXPORT(port_main, port_main);

void dnsr_main(u8 *instr)
{
    u16  len = 0;

    len = strlen((const char *)instr);

    if(len != 0)
    {
        memset(DomainNameStr, 0, sizeof(DomainNameStr));
        memset(SysConf_struct.DNSR, 0, sizeof(DomainNameStr));
        memcpy(DomainNameStr, (char *)instr, len);
        memcpy(SysConf_struct.DNSR, (char *)instr, len);
        Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
        //----- ���� GSM ģ��------
        DataLink_DNSR_Set(SysConf_struct.DNSR, 1);
        //--------    �����Ȩ�� -------------------
        idip("clear");
    }

}
FINSH_FUNCTION_EXPORT(dnsr_main, dnsr_main);

void password(u8 in)
{

    Login_Menu_Flag = in;  // clear  first flag 	 Login_Menu_Flag=0;     //  �������Ϊ0
    DF_WriteFlashSector(DF_LOGIIN_Flag_offset, 0, &Login_Menu_Flag, 1);
    rt_kprintf("\r\n  password=%d \r\n", Login_Menu_Flag);

    if(Login_Menu_Flag == 0)
    {
        JT808Conf_struct.Regsiter_Status = 0; //��Ҫ����ע��
        pMenuItem = &Menu_0_0_password;
        pMenuItem->show();
    }
    else
    {
        pMenuItem = &Menu_1_Idle;
        pMenuItem->show();
    }
}
FINSH_FUNCTION_EXPORT(password, password);


void  print_power(u8 *instr)
{
    if(instr[0] == '0')
    {
        Print_power_Q5_enable = 0;
        lcd_update_all();
        //rt_kprintf("\r\n  printer poweroff\r\n");
    }

    if(instr[0] == '1')
    {
        Print_power_Q5_enable = 1;
        lcd_update_all();
        // rt_kprintf("\r\n printer poweron\r\n");
    }
}
//FINSH_FUNCTION_EXPORT(print_power, print_power[1|0]);


void buzzer_onoff(u8 in)
{

    GPIO_InitTypeDef GPIO_InitStructure;

    if(0 == in)
    {
        GPIO_StructInit(&GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; 			//ָ����������
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;		//ģʽΪ����
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//Ƶ��Ϊ����
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;		//�����Ա��ʡ����
        GPIO_Init(GPIOA, &GPIO_InitStructure);

    }

    if(1 == in)
    {
        //-----------------  hardware  0x101    5   Beep -----------------
        /*�����ýṹ���еĲ��ֳ�Ա����������£��û�Ӧ�����ȵ��ú���PPP_SturcInit(..)
        ����ʼ������PPP_InitStructure��Ȼ�����޸�������Ҫ�޸ĵĳ�Ա���������Ա�֤����
        ��Ա��ֵ����Ϊȱʡֵ������ȷ���롣
         */

        GPIO_StructInit(&GPIO_InitStructure);

        /*����GPIOA_Pin_5����ΪTIM2_Channel1 PWM���*/
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; 			//ָ����������
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;		//ģʽ����Ϊ���ã�
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//Ƶ��Ϊ����
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;		//��������PWM������Ӱ��
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        //GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_TIM2); //����GPIOA_Pin1ΪTIM2_Ch2
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_TIM2); //����GPIOA_Pin5ΪTIM2_Ch1,
    }


}
//FINSH_FUNCTION_EXPORT(buzzer_onoff, buzzer_onoff[1|0]);

void provinceid(u8 *strin)
{
    sscanf(strin, "%d", (u32 *)&Vechicle_Info.Dev_ProvinceID);

    DF_WriteFlashSector(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
    WatchDog_Feed();
    DF_WriteFlashSector(DF_VehicleBAK_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));

    WatchDog_Feed();
    DF_WriteFlashSector(DF_VehicleBAK2_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
    // rt_kprintf("\r\n ��������ʡ��ID: %d \r\n",Vechicle_Info.Dev_ProvinceID);
}
//FINSH_FUNCTION_EXPORT(provinceid, provinceid);

void cityid(u8 *strin)
{
    sscanf(strin, "%d", (u32 *)&Vechicle_Info.Dev_CityID);
    DF_WriteFlashSector(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
    WatchDog_Feed();
    DF_WriteFlashSector(DF_VehicleBAK_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));

    WatchDog_Feed();
    DF_WriteFlashSector(DF_VehicleBAK2_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
    /// rt_kprintf("\r\n ������������ID: %d \r\n",Vechicle_Info.Dev_CityID);
}
//FINSH_FUNCTION_EXPORT(cityid, cityid);

void ata_enable(u8 value)
{
    //rt_kprintf("\r\n  ata_enable=%d \r\n",value);

    JT808Conf_struct.Auto_ATA_flag = value;
    Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
}
//FINSH_FUNCTION_EXPORT(ata_enable, ata_enable[1|0]);

void  type_vech(u8 type)
{
    // Vechicle_Info.Vech_Type_Mark=type;

    if(type == 1)
    {
        //rt_kprintf("\r\n  ������������Ϊ:  ����һΣ\r\n");
        memset(Vechicle_Info.Vech_Type, 0, sizeof(Vechicle_Info.Vech_Type));
        memcpy(Vechicle_Info.Vech_Type, "����һΣ", 8);

    }
    else if(type == 2)
    {
        //rt_kprintf("\r\n  ������������Ϊ:  ����\r\n");
        memset(Vechicle_Info.Vech_Type, 0, sizeof(Vechicle_Info.Vech_Type));
        memcpy(Vechicle_Info.Vech_Type, "����", 4);
    }


    DF_WriteFlashSector(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
    WatchDog_Feed();
    DF_WriteFlashSector(DF_VehicleBAK_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));

    WatchDog_Feed();
    DF_WriteFlashSector(DF_VehicleBAK2_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));

}
//FINSH_FUNCTION_EXPORT(type_vech, type_vech[1|0]);


#if 1
void write_read(u32 write , u32 read)
{
    cycle_write = write;
    cycle_read = read;
    DF_Write_RecordAdd(cycle_write, cycle_read, TYPE_CycleAdd);
    rt_kprintf("\r\n  write=%d    read=%d     \r\n", cycle_write, cycle_read);
}
FINSH_FUNCTION_EXPORT(write_read, write_read(write, read));
void qwrite_read(void)
{
    rt_kprintf("\r\n query write=%d    read=%d    mangQu_read_reg=%d  max=%d   \r\n", cycle_write, cycle_read, mangQu_read_reg, Max_CycleNum);
}
FINSH_FUNCTION_EXPORT(qwrite_read, qwrite_read(write, read));
#endif


#ifdef SPD_WARN_SAVE
void rd_spdwarn(void)
{
    u8 i = 0;
    u8  Read_ChaosuData[32];


    for(i = 0; i < ExpSpdRec_write; i++)
    {
        Api_DFdirectory_Read(spd_warn, Read_ChaosuData, 32, 0, i); // ��new-->old  ��ȡ
        OutPrint_HEX("���ټ�¼����", Read_ChaosuData, 32);
    }
    rt_kprintf("\r\n �ֶ����ٲ�ѯ\r\n");
}
FINSH_FUNCTION_EXPORT(rd_spdwarn, rd_spdwarn(0));
#endif

// C.  Module
