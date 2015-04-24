#include <rtthread.h>
#include <rthw.h>
#include "stm32f2xx.h"
#include "usart.h"
#include "board.h"
#include <serial.h>

#include  <stdlib.h>
#include  <stdio.h>
#include  <string.h>
#include "App_moduleConfig.h"
#include "Device_808.h"
#include  "Vdr.h"


#define   SYSID            0xE637      //0xE63B  
/*
                        0x0000   -----   0x00FF  �������з���
                        0x0100   -----   0x0FFF  ��Ʒ������
                        0x1000   -----   0xF000  Զ��������
                       */



ALIGN(RT_ALIGN_SIZE)
SYS_CONF          SysConf_struct;   //  ϵͳ����

ALIGN(RT_ALIGN_SIZE)
JT808_CONF       JT808Conf_struct;    //  JT 808   �������
JT808_CONF       JT808_struct_Bak;    //  JT808 ���ģʽ���ñ���
JT808_CONF       JT808_struct_Bak2;   //  JT808  bak 2

ALIGN(RT_ALIGN_SIZE)
TIRED_CONF      TiredConf_struct;    //  ƣ�ͼ�ʻ�������





//----------  Basic  Config---------------------------
u8      DeviceNumberID[13];//="800130100001";    // ����DeviceID    ---- �ӱ����ͨ��
u8      SimID_12D[13]; // ����ID  ���� 12 λ ��λ Ϊ 0

u8          RemoteIP_Dnsr[4] = {255, 255, 255, 255};
u8		RemoteIP_main[4] = {113, 31, 28, 101}; //{125,38,185,88};//{113,31,28,101 };//{113,31,92,200};//���{60,28,50,210}; �ӱ����ͨ 113,31,28,100
u16		RemotePort_main = 8201; //���9131;   �ӱ����ͨ 8201             //test tianjin
u8		RemoteIP_aux[4] = {60, 28, 50, 210}; //{60,28,50,210}
u16		RemotePort_aux = 4000;
//           Link2  Related
u8      Remote_Link2_IP[4] = {60, 28, 50, 210};
u16     Remote_Link2_Port = 9131;



u8           APN_String[30] = "UNINET"; //"CMNET";   //  �ӱ����ͨ  �ƶ��Ŀ�
u8           DomainNameStr[50] = "up.gps960.net"; ; // ����  ���ͨup.gps960.com //jt1.gghypt.net
u8           DomainNameStr_aux[50] = "jt2.gghypt.net";   //"www.sina.com";//jt2.gghypt.net
u16         ACC_on_sd_Duration = 30;  //  ACC ������ʱ�� �ϱ���ʱ����
u16         ACC_off_sd_Duration = 60;  //  ACC �ر�ʱ���ϱ���ʱ����
u8          TriggerSDsatus = 0x80; // �����������ϱ�״̬λ






u32	     Current_SD_Duration = 20; //GPS ��Ϣ�����ʱ����
u32      Current_SD_Distance = 100; // GPS ��Ϣ�����ϱ�����
u32      DistanceAccumulate = 0;  // �����ϱ��ۼ���
u8		 Current_State = 0; //��ʽΪ0 �� ��ʾΪ1		 // �ϱ�ʵʱ��־λ��Ϣ	 Ԥ��DF ��

u16      StopLongDuration = 15300; //255minutes 15300s   //����ͣ�������ʱ��
u16      Stop_counter = 0;             //����ͣ������ʱ������

u8      EmergentWarn = 0;             // ��������

//-------------- Vehicle Recorder Setting --------------------

//---------------------------   ��ʻԱ��Ϣ  ------------------------------
//u8     DriverCard_ID[18]="000000000000000000";  // ��ʻԱ��ʻ֤���� 18λ
//u8     DriveName[21]="����";                    // ��ʻԱ ����

//-----------------  ������Ϣ ------------------------------------------
//u8     Vech_VIN[17]="00000000000000000";        // ����VIN��
//u8     Vech_Num[12]="��A00001";	                // ���ƺ�
//u8     Vech_Type[12]="000000000000";            // ��������


//-----------------  ������Ϣ ------------------------------------------
//VechINFO  Vechicle_Info;



u8     Vechicle_TYPE = 1;               //   ��������    1:���ͻ���  2: С�ͻ���  3:���Ϳͳ�  4: ���Ϳͳ�   5:С�Ϳͳ�
u8     OnFire_Status = 0;                    //   1 : ACC ���������     0 :  ACC  �ػ�������
u8     Login_Status = 0x02;                  //   01H:��¼��02H���˳���03H��������ʻԱ
u8     Powercut_Status = 0x01;               //01H:�ϵ磬02H���ϵ�

u8     Settingchg_Status = 0x00;                 /*
												82H:���ó�����Ϣ��84H������״̬��
												C2H:���ü�¼��ʱ��
												C3H:���ü�¼���ٶ�����ϵ��
											*/

//u16    DaySpdMax=0;                                //  ��������ٶ�
//u16    DayDistance=0;                              //  ������ʻ����

//------ ����ش� ------

//--------------  �����ϱ�  --------------------
u32  former_distance_meter = 0;   //   ��һ�ξ���    ����ش�ʱ������
u32  current_distance_meter = 0;  //   ��ǰ����

//---------  SytemCounter ------------------
u32  Systerm_Reset_counter = 0;
u8   DistanceWT_Flag = 0; //  д��̱�־λ
u8   SYSTEM_Reset_FLAG = 0;      // ϵͳ��λ��־λ
u32  Device_type = 0x00000001; //Ӳ������   STM32103  ��A1
u32  Firmware_ver = 0x0000001; // ����汾
u8   ISP_resetFlag = 0;      //Զ��������λ��־λ



//     �ն��������     1  ���汾˵��
void ProductAttribute_init(void)
{
    ProductAttribute._1_DevType = 0x07; //  �͡�����Σ
    memcpy(ProductAttribute._2_ProducterID, "7_1_2", 5);
    memset(ProductAttribute._3_Dev_TYPENUM, 0, sizeof(ProductAttribute._3_Dev_TYPENUM));
    memcpy(ProductAttribute._3_Dev_TYPENUM, "TW703_BD", 8);

    //�ն�ID ��SIM�� ICCID �ڳ�ʼ��ʱ��ȡ


    ProductAttribute._6_HardwareVer_Len = 14;
    memset(ProductAttribute._7_HardwareVer, 0, sizeof(ProductAttribute._7_HardwareVer));
    memcpy(ProductAttribute._7_HardwareVer, "TW703_BD-HW1.0", 14);

    ProductAttribute._8_SoftwareVer_len = 14;
    memset(ProductAttribute._9_SoftwareVer, 0, sizeof(ProductAttribute._9_SoftwareVer));
    memcpy(ProductAttribute._9_SoftwareVer, "TW703_BD-SW1.0", 14);

    ProductAttribute._10_FirmWareVer_len = 16;
    memset(ProductAttribute._11_FirmWare, 0, sizeof(ProductAttribute._11_FirmWare));
    memcpy(ProductAttribute._11_FirmWare, "TW703_BD-Firm1.0", 16);

    ProductAttribute._12_GNSSAttribute = 0x03; //   ֧��GPS ��λ ������λ

    ProductAttribute._13_ComModuleAttribute = 0x01; // ֧��GPRS ͨ��

};



/*
          ����ϵͳĿ¼
*/
void    Create_Sys_Directory(void)
{


    //  ��¼
    Api_MediaIndex_Init();    //  ��ý������   ͼƬ+  ����
}


/*
       ϵͳ������Ϣд��
*/
u8  SysConfig_init(void)
{

    //  1. Stuff
    //   ϵͳ�汾
    SysConf_struct.Version_ID = SYSID;
    //   APN
    memset((u8 *)SysConf_struct.APN_str, 0 , sizeof(SysConf_struct.APN_str));
    memcpy(SysConf_struct.APN_str, (u8 *)APN_String, strlen((const char *)APN_String));
    //   ����main
    memset((u8 *)SysConf_struct.DNSR, 0 , sizeof(SysConf_struct.DNSR));
    memcpy(SysConf_struct.DNSR, (u8 *)DomainNameStr, strlen((const char *)DomainNameStr));
    //   ����aux
    memset((u8 *)SysConf_struct.DNSR_Aux, 0 , sizeof(SysConf_struct.DNSR_Aux));
    memcpy(SysConf_struct.DNSR_Aux, (u8 *)DomainNameStr_aux, strlen((const char *)DomainNameStr_aux));



    //   �� IP   +  �˿�
    memcpy(SysConf_struct.IP_Main, (u8 *)RemoteIP_main, 4);
    SysConf_struct.Port_main = RemotePort_main;
    //   ���� IP   +  �˿�
    memcpy(SysConf_struct.IP_Aux, (u8 *)RemoteIP_aux, 4);
    SysConf_struct.Port_Aux = RemotePort_aux;

    //   LINK2 +      �˿�
    memcpy(SysConf_struct.Link2_IP, (u8 *)Remote_Link2_IP, 4);
    SysConf_struct.Link2_Port = Remote_Link2_Port;



    //	 IC ������IP   TCP �˿� UDP  �˿�
    memcpy(SysConf_struct.BD_IC_main_IP, (u8 *)RemoteIP_aux, 4);
    SysConf_struct.BD_IC_TCP_port = RemotePort_aux;
    SysConf_struct.BD_IC_UDP_port = 29;


    //  �����������ϱ�״̬
    SysConf_struct.TriggerSDsatus = TriggerSDsatus;
    //   ACC  on   off  ����
    SysConf_struct.AccOn_Dur = ACC_on_sd_Duration;
    SysConf_struct.AccOff_Dur = ACC_off_sd_Duration;
    //    2. Operate
    return(Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct)));

}

void SysConfig_Read(void)
{
    if( Api_Config_read(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct)) == false) //��ȡϵͳ������Ϣ
        rt_kprintf("\r\nConfig_ Read Error\r\n");


    memset((u8 *)APN_String, 0 , sizeof(APN_String));
    memcpy((u8 *)APN_String, SysConf_struct.APN_str, strlen((const char *)SysConf_struct.APN_str));
    //   ����
    memset((u8 *)DomainNameStr, 0 , sizeof(DomainNameStr));
    memcpy((u8 *)DomainNameStr, SysConf_struct.DNSR, strlen((const char *)SysConf_struct.DNSR));
    //   ����aux
    memset((u8 *)DomainNameStr_aux, 0 , sizeof(DomainNameStr_aux));
    memcpy((u8 *)DomainNameStr_aux, SysConf_struct.DNSR_Aux, strlen((const char *)SysConf_struct.DNSR_Aux));



    //   �� IP   +  �˿�
    memcpy((u8 *)RemoteIP_main, SysConf_struct.IP_Main, 4);
    RemotePort_main = SysConf_struct.Port_main;
    //   ���� IP   +  �˿�
    memcpy((u8 *)RemoteIP_aux, SysConf_struct.IP_Aux, 4);
    RemotePort_aux = SysConf_struct.Port_Aux;
    //  Link2
    memcpy((u8 *)Remote_Link2_IP, SysConf_struct.Link2_IP, 4);
    Remote_Link2_Port = SysConf_struct.Link2_Port;

    //  �����������ϱ�״̬
    TriggerSDsatus = SysConf_struct.TriggerSDsatus;
    //   ACC  on   off  ����
    ACC_on_sd_Duration = SysConf_struct.AccOn_Dur;
    ACC_off_sd_Duration = SysConf_struct.AccOff_Dur;


}


/*
       JT808    Related
*/
void JT808_DURATION_Init(void)
{
    JT808Conf_struct.DURATION.Heart_Dur = 60;     // ���������ͼ��
    JT808Conf_struct.DURATION.TCP_ACK_Dur = 20;   //  TCP Ӧ��ʱ
    JT808Conf_struct.DURATION.TCP_ReSD_Num = 3;   //  TCP �ط�����
    JT808Conf_struct.DURATION.TCP_ACK_Dur = 20;   //  UDP Ӧ��ʱ
    JT808Conf_struct.DURATION.UDP_ReSD_Num = 5;   //  UDP �ط�����
    JT808Conf_struct.DURATION.NoDrvLogin_Dur = 40; //  ��ʻԱû��½ʱ�ķ��ͼ��
    JT808Conf_struct.DURATION.Sleep_Dur = 30;     //  ����ʱ�ϱ���ʱ����
    JT808Conf_struct.DURATION.Emegence_Dur = 20;  //  ��������ʱ�ϱ�ʱ����
    JT808Conf_struct.DURATION.Default_Dur = 30;   //  ȱʡ������ϱ���ʱ����
    JT808Conf_struct.DURATION.SD_Delta_maxAngle = 60; // �յ㲹�������Ƕ�
    JT808Conf_struct.DURATION.IllgleMovo_disttance = 300; // �Ƿ��ƶ���ֵ
}

void JT808_SendDistances_Init(void)
{
    JT808Conf_struct.DISTANCE.Defalut_DistDelta = 200;  // Ĭ�϶���ش�����
    JT808Conf_struct.DISTANCE.NoDrvLogin_Dist = 300;    // ��ʻԱδ��¼ʱ�ش�����
    JT808Conf_struct.DISTANCE.Sleep_Dist = 500;         // ����������ϱ��Ķ���ش�
    JT808Conf_struct.DISTANCE.Emergen_Dist = 100;       // ��������������ϱ��Ķ���ش�
}

//------------------------- �ն����ݷ��ͷ�ʽ -----------------------
void JT808_SendMode_Init(void)
{
    JT808Conf_struct.SD_MODE.DUR_TOTALMODE = 1; // ʹ�ܶ�ʱ����
    JT808Conf_struct.SD_MODE.Dur_DefaultMode = 1; //  ȱʡ��ʽ�ϱ�
    JT808Conf_struct.SD_MODE.Dur_EmegencMode = 0;
    JT808Conf_struct.SD_MODE.Dur_NologinMode = 0;
    JT808Conf_struct.SD_MODE.Dur_SleepMode = 0;

    JT808Conf_struct.SD_MODE.DIST_TOTALMODE = 0;
    JT808Conf_struct.SD_MODE.Dist_DefaultMode = 0;
    JT808Conf_struct.SD_MODE.Dist_EmgenceMode = 0;
    JT808Conf_struct.SD_MODE.Dist_NoLoginMode = 0;
    JT808Conf_struct.SD_MODE.Dist_SleepMode = 0;


    JT808Conf_struct.SD_MODE.Send_strategy = 0;
}

//------------------------------------------------------------------
void  JT808_RealTimeLock_Init(void)
{
    // Ԥ��ֵĬ��ֵ
    JT808Conf_struct.RT_LOCK.Lock_state = 0;
    JT808Conf_struct.RT_LOCK.Lock_Dur = 20;
    JT808Conf_struct.RT_LOCK.Lock_KeepDur = 300;
    JT808Conf_struct.RT_LOCK.Lock_KeepCnter = 0;
}

void  Vehicleinfo_Init(void)
{
    memset((u8 *)&Vechicle_Info, 0, sizeof(Vechicle_Info));
    //-----------------------------------------------------------------------
    memcpy(Vechicle_Info.Vech_VIN, "00000000000000000", 17);
    memcpy(Vechicle_Info.Vech_Num, "δ֪����", 8);
    memcpy(Vechicle_Info.Vech_Type, "δ֪��", 6);
    memcpy(Vechicle_Info.ProType, "TW705", 5);
    Vechicle_Info.Dev_ProvinceID = 13; // Ĭ��ʡID   0      13  �ӱ�ʡ
    Vechicle_Info.Dev_CityID = 101;    // Ĭ����ID   0		 ʯ��ׯ
    Vechicle_Info.Dev_Color = 1;     // Ĭ����ɫ    // JT415    1  �� 2 �� 3 �� 4 �� 9����
    //Vechicle_Info.loginpassword_flag=0;
    Vechicle_Info.Link_Frist_Mode = 0; //     0  : dnsr first     1: mainlink  first

    DF_WriteFlashSector(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));

    WatchDog_Feed();
    DF_WriteFlashSector(DF_VehicleBAK_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));

    WatchDog_Feed();
    DF_WriteFlashSector(DF_VehicleBAK2_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));

}

u8     JT808_Conf_init( void )
{
    u8  FirstUseDate[6] = {0x14, 0x05, 0x08, 0x08, 0x30, 0x26};
    //  1.  clear
    memset((u8 *) & (JT808Conf_struct), 0, sizeof(JT808Conf_struct)); 	 // ��ʻԱ��Ϣ


    //   2.  Stuff
    JT808_DURATION_Init();
    //  JT808Conf_struct.
    JT808_SendDistances_Init();
    JT808_SendMode_Init();
    JT808Conf_struct.LOAD_STATE = 0; //  ����״̬
    JT808Conf_struct.PositionSd_Stratage = 0; // ����ACC ״̬

    memset((u8 *)JT808Conf_struct.ConfirmCode, 0, sizeof(JT808Conf_struct.ConfirmCode));
    memcpy((u8 *)JT808Conf_struct.ConfirmCode, "012345\x00", 7); //  ��Ȩ��

    JT808Conf_struct.Regsiter_Status = 0; //  ע��״̬

    memset((u8 *)JT808Conf_struct.LISTEN_Num, 0, sizeof(JT808Conf_struct.LISTEN_Num));
    memcpy((u8 *)JT808Conf_struct.LISTEN_Num, "10086", 5); //  ��������



    memset((u8 *)JT808Conf_struct.SMS_RXNum, 0, sizeof(JT808Conf_struct.SMS_RXNum));
    memcpy((u8 *)JT808Conf_struct.SMS_RXNum, "106220801", 9); //  ��������

    JT808Conf_struct.Vech_Character_Value = 6240; // ����ϵ��  �ٶ�����ϵ��



    memset((u8 *)JT808Conf_struct.FirstSetupDate, 0, sizeof(JT808Conf_struct.FirstSetupDate));
    memcpy((u8 *)JT808Conf_struct.FirstSetupDate, FirstUseDate, 6); // �״ΰ�װʱ��


    memset((u8 *)JT808Conf_struct.DeviceOnlyID, 0, sizeof(JT808Conf_struct.DeviceOnlyID));
    memcpy((u8 *)JT808Conf_struct.DeviceOnlyID, "00000010000000000000001", 23); //   �г���¼�ǵ�ΨһID

    JT808Conf_struct.Msg_Float_ID = 0; // ��Ϣ��ˮ��




    JT808Conf_struct.Distance_m_u32 = 0;          //  ��ʻ��¼����ʻ���  ��λ: ��
    JT808Conf_struct.DayStartDistance_32 = 0;   //  ÿ�����ʼ�����Ŀ

    JT808Conf_struct.Speed_warn_MAX = 200;         //  �ٶȱ�������
    JT808Conf_struct.Spd_Exd_LimitSeconds = 10; //  ���ٱ�������ʱ������ s
    JT808Conf_struct.Speed_GetType = 0;           //  ��¼�ǻ�ȡ�ٶȵķ�ʽ  00  gpsȡ�ٶ�  01 ��ʾ�Ӵ�����ȥ�ٶ�
    JT808Conf_struct.DF_K_adjustState = 0; // ����ϵ���Զ�У׼״̬˵��  1:�Զ�У׼��    0:��δ�Զ�У׼


    JT808Conf_struct.OutGPS_Flag = 1;   //  0  Ĭ��  1  ���ⲿ��Դ����
    JT808Conf_struct.concuss_step = 40;
    JT808Conf_struct.Auto_ATA_flag = 0; // �������Զ�����

    JT808_RealTimeLock_Init();   //  ʵʱ��������


    memset((u8 *) & (JT808Conf_struct.StdVersion), 0, sizeof(JT808Conf_struct.StdVersion)); // ��׼���Ұ汾
    memcpy((u8 *)(JT808Conf_struct.StdVersion.stdverStr), "GB/T19056-2011", 14); // ��׼�汾
    JT808Conf_struct.StdVersion.MdfyID = 0x02; //�޸ĵ���



    memset((u8 *) & (JT808Conf_struct.Driver_Info), 0, sizeof(JT808Conf_struct.Driver_Info)); 	 // ��ʻԱ��Ϣ
    //--------------------------------------------------------------------------

    JT808Conf_struct.Driver_Info.Effective_Date[0] = 0x20;
    JT808Conf_struct.Driver_Info.Effective_Date[1] = 0x07;
    JT808Conf_struct.Driver_Info.Effective_Date[2] = 0x01;
    memcpy(JT808Conf_struct.Driver_Info.DriverCard_ID, "000000000000000000", 18);
    memcpy(JT808Conf_struct.Driver_Info.DriveName, "δ֪", 4);
    memcpy(JT808Conf_struct.Driver_Info.Drv_CareerID, "00000000000000000000", 20);
    memcpy(JT808Conf_struct.Driver_Info.Comfirm_agentID, "000000000000000", 16);

    //    3. Operate
    return(Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct)));

}

void TIRED_DoorValue_Init(void)
{
    TiredConf_struct.TiredDoor.Door_DrvKeepingSec = 14400; // ���ұ�׼�� 3Сʱ
    TiredConf_struct.TiredDoor.Door_MinSleepSec = 1200;   //
    TiredConf_struct.TiredDoor.Door_DayAccumlateDrvSec = 28800; //8Сʱ
    TiredConf_struct.TiredDoor.Door_MaxParkingSec = 7200; // 2 Сʱ
    TiredConf_struct.TiredDoor.Parking_currentcnt = 0; // ͣ��״̬������
}

void TIRED_DoorValue_Read(void)   //��ʱ��Ҫע�� ��3���ط�Ҫ��� 0
{
    Api_Config_read(tired_config, 0, (u8 *)&TiredConf_struct, sizeof(TiredConf_struct));
}


u8   TIRED_CONF_Init(void)
{
    //  1.  clear
    memset((u8 *) & (TiredConf_struct), 0, sizeof(TiredConf_struct)); 	 // ��ʻԱ��Ϣ

    // 2. stuff
    TIRED_DoorValue_Init();

    //    2. Operate
    return(Api_Config_write(tired_config, 0, (u8 *)&TiredConf_struct, sizeof(TiredConf_struct)));

}

/*
         �¼�
*/
//-----------------------------------------------------------------
void Event_Write_Init(void)
{
    u8 len_write = 8;
    //�¼�д��
    len_write = 8;
    EventObj.Event_ID = 1; //�¼�ID
    EventObj.Event_Len = len_write; //���� 4*2
    EventObj.Event_Effective = 1; //�¼���Ч
    memset(EventObj.Event_Str, 0, sizeof(EventObj.Event_Str));
    memcpy(EventObj.Event_Str, "�������", EventObj.Event_Len);
    Api_RecordNum_Write(event_808, EventObj.Event_ID, (u8 *)&EventObj, sizeof(EventObj));

    len_write = 16;
    EventObj.Event_ID = 2; //�¼�ID
    EventObj.Event_Len = len_write; //���� 8*2
    memset(EventObj.Event_Str, 0, sizeof(EventObj.Event_Str));
    memcpy(EventObj.Event_Str, "����װ��׼������", EventObj.Event_Len);
    Api_RecordNum_Write(event_808, EventObj.Event_ID, (u8 *)&EventObj, sizeof(EventObj));

    EventObj.Event_ID = 3; //�¼�ID
    EventObj.Event_Len = len_write; //���� 8*2
    memset(EventObj.Event_Str, 0, sizeof(EventObj.Event_Str));
    memcpy(EventObj.Event_Str, "ƽ������һ��˳��", EventObj.Event_Len);
    Api_RecordNum_Write(event_808, EventObj.Event_ID, (u8 *)&EventObj, sizeof(EventObj));

    EventObj.Event_ID = 4; //�¼�ID
    EventObj.Event_Len = len_write; //���� 8*2
    memset(EventObj.Event_Str, 0, sizeof(EventObj.Event_Str));
    memcpy(EventObj.Event_Str, "ָ���ص��δ����", EventObj.Event_Len);
    Api_RecordNum_Write(event_808, EventObj.Event_ID, (u8 *)&EventObj, sizeof(EventObj));

    EventObj.Event_ID = 5; //�¼�ID
    EventObj.Event_Len = len_write; //���� 8*2
    memset(EventObj.Event_Str, 0, sizeof(EventObj.Event_Str));
    memcpy(EventObj.Event_Str, "ָ���ص����˽Ӵ�", EventObj.Event_Len);
    Api_RecordNum_Write(event_808, EventObj.Event_ID, (u8 *)&EventObj, sizeof(EventObj));

    EventObj.Event_ID = 6; //�¼�ID
    EventObj.Event_Len = len_write; //���� 8*2
    EventObj.Event_Effective = 0; //�¼���Ч
    memset(EventObj.Event_Str, 0, sizeof(EventObj.Event_Str));
    memcpy(EventObj.Event_Str, "�����޷���ϵ����", EventObj.Event_Len);
    Api_RecordNum_Write(event_808, EventObj.Event_ID, (u8 *)&EventObj, sizeof(EventObj));

    len_write = 14;
    EventObj.Event_ID = 7; //�¼�ID
    EventObj.Event_Len = len_write; //���� 7*2
    memset(EventObj.Event_Str, 0, sizeof(EventObj.Event_Str));
    memcpy(EventObj.Event_Str, "������������", EventObj.Event_Len);
    Api_RecordNum_Write(event_808, EventObj.Event_ID, (u8 *)&EventObj, sizeof(EventObj));

    EventObj.Event_ID = 8; //�¼�ID
    EventObj.Event_Len = len_write; //���� 7*2
    memset(EventObj.Event_Str, 0, sizeof(EventObj.Event_Str));
    memcpy(EventObj.Event_Str, "�м������ٻػ�", EventObj.Event_Len);
    Api_RecordNum_Write(event_808, EventObj.Event_ID, (u8 *)&EventObj, sizeof(EventObj));
}

void Event_Read(void)
{
    u8 i = 0;
    for(i = 0; i < 8; i++)
    {
        memset(EventObj.Event_Str, 0, sizeof(EventObj.Event_Str));
        Api_RecordNum_Read(event_808, i + 1, (u8 *)&EventObj, sizeof(EventObj));

        EventObj_8[i].Event_ID = EventObj.Event_ID;
        EventObj_8[i].Event_Len = EventObj.Event_Len;
        EventObj_8[i].Event_Effective = EventObj.Event_Effective;
        memcpy(EventObj_8[i].Event_Str, EventObj.Event_Str, sizeof(EventObj.Event_Str));
        //	memcpy(DisInfor_Affair[i],EventObj.Event_Str,20);
        //rt_kprintf("\r\n�¼�ID:%d  ����:%d  �Ƿ���Ч:%d(1��ʾ0����ʾ) Info: %s",EventObj.Event_ID,EventObj.Event_Len,EventObj.Event_Effective,EventObj.Event_Str);
    }
}


void Event_Init(u8  Intype)
{
    u8 i = 0;

    if(Intype == 0)
    {
        EventObj.Event_Len = 8;
        memset(EventObj.Event_Str, 0, sizeof(EventObj.Event_Str));
        memcpy(EventObj.Event_Str, "����·��", 8);
        EventObj.Event_Effective = 1;
    }
    else if(Intype == 1)
    {
        EventObj.Event_Len = 0;
        memset(EventObj.Event_Str, 0, sizeof(EventObj.Event_Str));
        EventObj.Event_Effective = 0;
    }

    for(i = 0; i < 8; i++)
    {
        EventObj.Event_ID = i + 1;
        Api_RecordNum_Write(event_808, EventObj.Event_ID, (u8 *)&EventObj, sizeof(EventObj));
    }
}

/*
          ��Ϣ
*/
//----------------------------------------------------------------
void MSG_BroadCast_Write_Init(void)
{
    u8 len_write = 8;
    MSG_BroadCast_Obj.INFO_TYPE = 1; //����
    MSG_BroadCast_Obj.INFO_LEN = len_write; //���� 4*2
    MSG_BroadCast_Obj.INFO_PlyCancel = 1; //�㲥
    MSG_BroadCast_Obj.INFO_SDFlag = 1; //���ͱ�־λ
    MSG_BroadCast_Obj.INFO_Effective = 1; //��ʾ��Ч
    memset(MSG_BroadCast_Obj.INFO_STR, 0, sizeof(MSG_BroadCast_Obj.INFO_STR));
    memcpy(MSG_BroadCast_Obj.INFO_STR, "����Ԥ��", MSG_BroadCast_Obj.INFO_LEN);
    Api_RecordNum_Write(msg_broadcast, MSG_BroadCast_Obj.INFO_TYPE, (u8 *)&MSG_BroadCast_Obj, sizeof(MSG_BroadCast_Obj));

    MSG_BroadCast_Obj.INFO_TYPE = 2; //����
    memset(MSG_BroadCast_Obj.INFO_STR, 0, sizeof(MSG_BroadCast_Obj.INFO_STR));
    memcpy(MSG_BroadCast_Obj.INFO_STR, "������Ϣ", MSG_BroadCast_Obj.INFO_LEN);
    Api_RecordNum_Write(msg_broadcast, MSG_BroadCast_Obj.INFO_TYPE, (u8 *)&MSG_BroadCast_Obj, sizeof(MSG_BroadCast_Obj));

    MSG_BroadCast_Obj.INFO_TYPE = 3; //����
    memset(MSG_BroadCast_Obj.INFO_STR, 0, sizeof(MSG_BroadCast_Obj.INFO_STR));
    memcpy(MSG_BroadCast_Obj.INFO_STR, "��ͨ��Ϣ", MSG_BroadCast_Obj.INFO_LEN);
    Api_RecordNum_Write(msg_broadcast, MSG_BroadCast_Obj.INFO_TYPE, (u8 *)&MSG_BroadCast_Obj, sizeof(MSG_BroadCast_Obj));

    MSG_BroadCast_Obj.INFO_TYPE = 4; //����
    memset(MSG_BroadCast_Obj.INFO_STR, 0, sizeof(MSG_BroadCast_Obj.INFO_STR));
    memcpy(MSG_BroadCast_Obj.INFO_STR, "��ʳ��Ϣ", MSG_BroadCast_Obj.INFO_LEN);
    Api_RecordNum_Write(msg_broadcast, MSG_BroadCast_Obj.INFO_TYPE, (u8 *)&MSG_BroadCast_Obj, sizeof(MSG_BroadCast_Obj));

    MSG_BroadCast_Obj.INFO_Effective = 0; //��ʾ��Ч
    MSG_BroadCast_Obj.INFO_TYPE = 5; //����
    memset(MSG_BroadCast_Obj.INFO_STR, 0, sizeof(MSG_BroadCast_Obj.INFO_STR));
    memcpy(MSG_BroadCast_Obj.INFO_STR, "��¼��Ϣ", MSG_BroadCast_Obj.INFO_LEN);
    Api_RecordNum_Write(msg_broadcast, MSG_BroadCast_Obj.INFO_TYPE, (u8 *)&MSG_BroadCast_Obj, sizeof(MSG_BroadCast_Obj));

    MSG_BroadCast_Obj.INFO_TYPE = 6; //����
    memset(MSG_BroadCast_Obj.INFO_STR, 0, sizeof(MSG_BroadCast_Obj.INFO_STR));
    memcpy(MSG_BroadCast_Obj.INFO_STR, "�¼���Ϣ", MSG_BroadCast_Obj.INFO_LEN);
    Api_RecordNum_Write(msg_broadcast, MSG_BroadCast_Obj.INFO_TYPE, (u8 *)&MSG_BroadCast_Obj, sizeof(MSG_BroadCast_Obj));

    MSG_BroadCast_Obj.INFO_TYPE = 7; //����
    memset(MSG_BroadCast_Obj.INFO_STR, 0, sizeof(MSG_BroadCast_Obj.INFO_STR));
    memcpy(MSG_BroadCast_Obj.INFO_STR, "ʱ����Ϣ", MSG_BroadCast_Obj.INFO_LEN);
    Api_RecordNum_Write(msg_broadcast, MSG_BroadCast_Obj.INFO_TYPE, (u8 *)&MSG_BroadCast_Obj, sizeof(MSG_BroadCast_Obj));

    MSG_BroadCast_Obj.INFO_TYPE = 8; //����
    memset(MSG_BroadCast_Obj.INFO_STR, 0, sizeof(MSG_BroadCast_Obj.INFO_STR));
    memcpy(MSG_BroadCast_Obj.INFO_STR, "������Ϣ", MSG_BroadCast_Obj.INFO_LEN);
    Api_RecordNum_Write(msg_broadcast, MSG_BroadCast_Obj.INFO_TYPE, (u8 *)&MSG_BroadCast_Obj, sizeof(MSG_BroadCast_Obj));
}

void MSG_BroadCast_Read(void)
{
    u8 i = 0;

    for(i = 0; i < 8; i++)
    {
        memset(MSG_BroadCast_Obj.INFO_STR, 0, sizeof(MSG_BroadCast_Obj.INFO_STR));
        Api_RecordNum_Read(msg_broadcast, i + 1, (u8 *)&MSG_BroadCast_Obj, sizeof(MSG_BroadCast_Obj));

        MSG_Obj_8[i].INFO_TYPE = MSG_BroadCast_Obj.INFO_TYPE;
        MSG_Obj_8[i].INFO_LEN = MSG_BroadCast_Obj.INFO_LEN;
        MSG_Obj_8[i].INFO_PlyCancel = MSG_BroadCast_Obj.INFO_PlyCancel;
        MSG_Obj_8[i].INFO_SDFlag = MSG_BroadCast_Obj.INFO_SDFlag;
        MSG_Obj_8[i].INFO_Effective = MSG_BroadCast_Obj.INFO_Effective;
        memcpy(MSG_Obj_8[i].INFO_STR, MSG_BroadCast_Obj.INFO_STR, sizeof(MSG_BroadCast_Obj.INFO_STR));

        //memcpy(DisInfor_Menu[i],MSG_BroadCast_Obj.INFO_STR,20);
        //rt_kprintf("\r\n ��ϢTYPE:%d  ����:%d  �Ƿ�㲥:%d �Ƿ���ʾ��Ч:%d(1��ʾ0����ʾ) Info: %s",MSG_BroadCast_Obj.INFO_TYPE,MSG_BroadCast_Obj.INFO_LEN,MSG_BroadCast_Obj.INFO_PlyCancel,MSG_BroadCast_Obj.INFO_Effective,MSG_BroadCast_Obj.INFO_STR);
    }
}

void MSG_BroadCast_Init(u8  Intype)
{
    u8 i = 0;

    if(Intype == 0)
    {
        MSG_BroadCast_Obj.INFO_LEN = 8;
        MSG_BroadCast_Obj.INFO_Effective = 1;
        memset(MSG_BroadCast_Obj.INFO_STR, 0, sizeof(MSG_BroadCast_Obj.INFO_STR));
        memcpy(MSG_BroadCast_Obj.INFO_STR, "�������", 8);
    }
    else if(Intype == 1)
    {
        MSG_BroadCast_Obj.INFO_LEN = 0;
        MSG_BroadCast_Obj.INFO_Effective = 0;
        memset(MSG_BroadCast_Obj.INFO_STR, 0, sizeof(MSG_BroadCast_Obj.INFO_STR));
    }

    for(i = 0; i < 8; i++)
    {
        MSG_BroadCast_Obj.INFO_TYPE = i + 1;
        Api_RecordNum_Write(msg_broadcast, MSG_BroadCast_Obj.INFO_TYPE, (u8 *)&MSG_BroadCast_Obj, sizeof(MSG_BroadCast_Obj));
    }
}

/*
      �绰��
*/
void PhoneBook_Read(void)
{
    u8 i = 0;

    for(i = 0; i < 8; i++)
    {
        Api_RecordNum_Read(phonebook, i + 1, (u8 *)&PhoneBook_8[i], sizeof(PhoneBook));
        //rt_kprintf("\r\n\r\n �绰�� TYPE: %d   Numlen=%d  Num: %s   UserLen: %d  UserName:%s \r\n",PhoneBook.CALL_TYPE,PhoneBook.NumLen,PhoneBook.NumberStr,PhoneBook.UserLen,PhoneBook.UserStr);
    }
}
void PhoneBook_Init(u8  Intype)
{
    u8 i = 0;

    if(Intype == 0)
    {
        PhoneBook.Effective_Flag = 1; //��Ч��־λ
        PhoneBook.CALL_TYPE = 2; //���Ͷ���Ϊ���
        PhoneBook.NumLen = 5;  // ���볤��
        memset(PhoneBook.NumberStr, 0, sizeof(PhoneBook.NumberStr)); // ��������
        memcpy(PhoneBook.NumberStr, "10086", 5);
        PhoneBook.UserLen = 8;		// �û�������
        memset(PhoneBook.UserStr, 0, sizeof(PhoneBook.UserStr)); // �û�������
        memcpy(PhoneBook.UserStr, "�й��ƶ�", 8);

    }
    else if(Intype == 1)
    {
        PhoneBook.Effective_Flag = 0; //��Ч��־λ
        PhoneBook.CALL_TYPE = 2; //���Ͷ���Ϊ���
        PhoneBook.NumLen = 0;  // ���볤��
        memset(PhoneBook.NumberStr, 0, sizeof(PhoneBook.NumberStr));
        PhoneBook.UserLen = 0;
        memset(PhoneBook.UserStr, 0, sizeof(PhoneBook.UserStr));
    }

    for(i = 0; i < 8; i++)
    {
        Api_RecordNum_Write(phonebook, i + 1, (u8 *)&PhoneBook, sizeof(PhoneBook));
    }
}

/*
       Բ��Χ��
*/
//----------------------------------------------------------
void  RailCycle_Init(void)
{
    u8 i = 0;

    Rail_Cycle.Area_ID = 0;
    Rail_Cycle.Area_attribute = 0; // Bit 0  ��ʾ����ʱ��  ���ֶ�Ϊ0 ��ʾΧ��û������
    Rail_Cycle.Center_Latitude = 0;
    Rail_Cycle.Center_Longitude = 0;
    Rail_Cycle.Radius = 100; // �뾶
    Time2BCD(Rail_Cycle.StartTimeBCD);
    Time2BCD(Rail_Cycle.EndTimeBCD);
    Rail_Cycle.MaxSpd = 100; // ����ٶ�
    Rail_Cycle.KeepDur = 30; // ���ٳ���ʱ��
    Rail_Cycle.Rail_Num = 8;
    Rail_Cycle.Effective_flag = 0;

    for(i = 0; i < 8; i++)
    {
        Rail_Cycle.Area_ID = i + 1;
        Api_RecordNum_Write(Rail_cycle, Rail_Cycle.Area_ID, (u8 *)&Rail_Cycle, sizeof(Rail_Cycle));
    }
}
void  RailCycle_Read(void)
{
    u8 i = 0;

    for(i = 0; i < 8; i++)
    {
        Api_RecordNum_Read(Rail_cycle, i + 1, (u8 *)&Rail_Cycle, sizeof(Rail_Cycle));
        //  rt_kprintf("\r\n\r\n Բ��Χ�� TYPE: %d    atrri=%d  lati: %d  longiti:%d  radicus:%d	maxspd: %d  keepdur:%d \r\n",Rail_Cycle.Area_ID,Rail_Cycle.Area_attribute,Rail_Cycle.Center_Latitude,Rail_Cycle.Center_Longitude,Rail_Cycle.Radius,Rail_Cycle.MaxSpd,Rail_Cycle.KeepDur);
    }
}

/*
       ����Χ��
*/
void  RailRect_Init(void)
{
    u8 i = 0;

    Rail_Rectangle.Area_ID = 0;
    Rail_Rectangle.Area_attribute = 0; // Bit 0  ��ʾ����ʱ��  ���ֶ�Ϊ0 ��ʾΧ��û������
    Rail_Rectangle.LeftUp_Latitude = 0; // ����
    Rail_Rectangle.LeftUp_Longitude = 0;
    Rail_Rectangle.RightDown_Latitude = 0; //  ����
    Rail_Rectangle.RightDown_Longitude = 0;
    Time2BCD(Rail_Rectangle.StartTimeBCD);
    Time2BCD(Rail_Rectangle.EndTimeBCD);
    Rail_Rectangle.MaxSpd = 100; // ����ٶ�
    Rail_Rectangle.KeepDur = 30; // ���ٳ���ʱ��
    Rail_Rectangle.Rail_Num = 8;
    Rail_Rectangle.Effective_flag = 0;

    for(i = 0; i < 8; i++)
    {
        Rail_Rectangle.Area_ID = i + 1;
        Api_RecordNum_Write(Rail_rect, Rail_Rectangle.Area_ID, (u8 *)&Rail_Rectangle, sizeof(Rail_Rectangle));
    }

}

void  RailRect_Read(void)
{
    u8 i = 0;

    for(i = 0; i < 8; i++)
    {
        Api_RecordNum_Read(Rail_rect, i + 1, (u8 *)&Rail_Rectangle, sizeof(Rail_Rectangle));
        //  rt_kprintf("\r\n\r\n ������Χ�� TYPE: %d    atrri=%d  leftlati: %d  leftlongiti:%d    rightLati:%d   rightLongitu: %d  \r\n",Rail_Rectangle.Area_ID,Rail_Rectangle.Area_attribute,Rail_Rectangle.LeftUp_Latitude,Rail_Rectangle.LeftUp_Longitude,Rail_Rectangle.RightDown_Latitude,Rail_Rectangle.RightDown_Longitude);
    }
}

/*
       �����Χ��
*/
void  RailPolygen_Init(void)
{
    u8 i = 0;

    Rail_Polygen.Area_ID = 0;
    Rail_Polygen.Area_attribute = 0; // Bit 0  ��ʾ����ʱ��  ���ֶ�Ϊ0 ��ʾΧ��û������
    Time2BCD(Rail_Polygen.StartTimeBCD);
    Time2BCD(Rail_Polygen.EndTimeBCD);
    Rail_Polygen.MaxSpd = 100; // ����ٶ�
    Rail_Polygen.KeepDur = 30; // ���ٳ���ʱ��
    Rail_Polygen.Acme_Num = 3;
    Rail_Polygen.Acme1_Latitude = 10; //����1
    Rail_Polygen.Acme1_Longitude = 10;
    Rail_Polygen.Acme2_Latitude = 20; //����2
    Rail_Polygen.Acme2_Longitude = 20;
    Rail_Polygen.Acme3_Latitude = 30; //����3
    Rail_Polygen.Acme3_Longitude = 30;
    Rail_Polygen.Effective_flag = 0;

    for(i = 0; i < 8; i++)
    {
        Rail_Polygen.Area_ID = i + 1;
        Api_RecordNum_Write(Rail_polygen, Rail_Polygen.Area_ID, (u8 *)&Rail_Polygen, sizeof(Rail_Polygen));
    }
}

void  RailPolygen_Read(void)
{
    u8 i = 0;

    for(i = 0; i < 8; i++)
    {
        Api_RecordNum_Read(Rail_polygen, i + 1, (u8 *)&Rail_Polygen, sizeof(Rail_Polygen));
        //  rt_kprintf("\r\n\r\n �����Χ�� TYPE: %d   1lat: %d  1long:%d    2lat:%d   2long: %d  3lat:%d 3long:%d \r\n",Rail_Polygen.Area_ID,Rail_Polygen.Acme1_Latitude,Rail_Polygen.Acme1_Longitude,Rail_Polygen.Acme2_Latitude,Rail_Polygen.Acme2_Longitude,Rail_Polygen.Acme3_Latitude,Rail_Polygen.Acme3_Longitude);
    }
}

/*
        �յ�����    (Maybe Null)
*/





/*
        ·������Χ��
*/
void  RouteLine_Obj_init(void)
{
    u8 i = 0;

    ROUTE_Obj.Route_ID = 0;
    ROUTE_Obj.Route_attribute = 0; // Bit 0  ��ʾ����ʱ��  ���ֶ�Ϊ0 ��ʾΧ��û������
    Time2BCD(ROUTE_Obj.StartTimeBCD);
    Time2BCD(ROUTE_Obj.EndTimeBCD);
    ROUTE_Obj.Points_Num = 3;
    for(i = 0; i < 3; i++)
    {
        ROUTE_Obj.RoutePoints[i].POINT_ID = i + 1;
        ROUTE_Obj.RoutePoints[i].Line_ID = i;
        ROUTE_Obj.RoutePoints[i].POINT_Latitude = i + 300;
        ROUTE_Obj.RoutePoints[i].POINT_Longitude = i + 500;
        ROUTE_Obj.RoutePoints[i].Width = 20;
        ROUTE_Obj.RoutePoints[i].Atribute = 0; // 0 ��ʾδ����
        ROUTE_Obj.RoutePoints[i].TooLongValue = 100;
        ROUTE_Obj.RoutePoints[i].TooLessValue = 50;
        ROUTE_Obj.RoutePoints[i].MaxSpd = 60;
        ROUTE_Obj.RoutePoints[i].KeepDur = 3;
        ROUTE_Obj.Effective_flag = 0;
    }

}

void  RouteLine_Init(void)
{
    u8 i = 0;

    RouteLine_Obj_init();

    for(i = 0; i < 8; i++)
    {
        ROUTE_Obj.Route_ID = i + 1;
        Api_RecordNum_Write(route_line, ROUTE_Obj.Route_ID, (u8 *)&ROUTE_Obj, sizeof(ROUTE_Obj));
    }
}

void  RouteLine_Read(void)
{
    u8 i = 0;

    for(i = 0; i < 8; i++)
    {
        Api_RecordNum_Read(route_line, i + 1, (u8 *)&ROUTE_Obj, sizeof(ROUTE_Obj));
        //  rt_kprintf("\r\n\r\n ·�� TYPE: %d   pointsNum:%d   p1lat: %d  p1long:%d    p2lat:%d   p2long: %d  p3lat:%d p3long:%d \r\n",ROUTE_Obj.Route_ID,ROUTE_Obj.Points_Num,ROUTE_Obj.RoutePoints[0].POINT_Latitude,ROUTE_Obj.RoutePoints[0].POINT_Longitude,ROUTE_Obj.RoutePoints[1].POINT_Latitude,ROUTE_Obj.RoutePoints[1].POINT_Longitude,ROUTE_Obj.RoutePoints[2].POINT_Latitude,ROUTE_Obj.RoutePoints[2].POINT_Longitude);
    }
}

/*
       ����
*/
#if 0
void Question_Read(void)
{
    //������Ϣ	 ����
    Api_RecordNum_Read(ask_quesstion, 1, (u8 *)&ASK_Centre, sizeof(ASK_Centre));
    rt_kprintf("\r\n��־λ:%d  ��ˮ��:%d  ��Ϣ����:%d �ظ�ID:%d", ASK_Centre.ASK_SdFlag, ASK_Centre.ASK_floatID, ASK_Centre.ASK_infolen, ASK_Centre.ASK_answerID);
    if(ASK_Centre.ASK_SdFlag == 1)
    {
        ASK_Centre.ASK_SdFlag = 0;
        rt_kprintf("\r\n��Ϣ����: %s", ASK_Centre.ASK_info);
        rt_kprintf("\r\n��ѡ��: %s", ASK_Centre.ASK_answer);
    }
}
#endif


/*
        �ı���Ϣ
*/

void TEXTMsg_Read (void)
{
    u8 i = 0, min = 0, max = 0;

    for(i = 0; i <= 7; i++)
    {
        Api_RecordNum_Read(text_msg, i + 1, (u8 *)&TEXT_Obj, sizeof(TEXT_Obj));
        memcpy((u8 *)&TEXT_Obj_8bak[i], (u8 *)&TEXT_Obj, sizeof(TEXT_Obj));
        //rt_kprintf("\r\n�ı���Ϣ ����:%d  ��ϢTYPE:%d  ����:%d",TEXT_Obj_8bak[i].TEXT_mOld,TEXT_Obj_8bak[i].TEXT_TYPE,TEXT_Obj_8bak[i].TEXT_LEN);
    }

    //����һ������
    max = TEXT_Obj_8bak[0].TEXT_TYPE;
    for(i = 0; i <= 7; i++)
    {
        if(TEXT_Obj_8bak[i].TEXT_TYPE > max)
            max = TEXT_Obj_8bak[i].TEXT_TYPE;
    }
    TextInforCounter = max;
    //rt_kprintf("\r\n  �����������  max=%d,TextInforCounter=%d",max,TextInforCounter);


    //�ҳ������һ����Ϣ
    min = TEXT_Obj_8bak[0].TEXT_TYPE;
    //rt_kprintf("\r\n  ����ǰ  ����һ����Ϣ���  min=%d",min);
    for(i = 0; i <= 7; i++)
    {
        if((TEXT_Obj_8bak[i].TEXT_TYPE < min) && (TEXT_Obj_8bak[i].TEXT_TYPE > 0))
            min = TEXT_Obj_8bak[i].TEXT_TYPE;
    }
    //rt_kprintf("\r\n	�����	����һ����Ϣ���  min=%d",min);

    if(max < 1)return;

    if(max <= 8)
    {
        for(i = 1; i <= max; i++)
        {
            Api_RecordNum_Read(text_msg, max - i + 1, (u8 *)&TEXT_Obj, sizeof(TEXT_Obj));
            memcpy((u8 *)&TEXT_Obj_8[i - 1], (u8 *)&TEXT_Obj, sizeof(TEXT_Obj));
            //rt_kprintf("\r\n(<8)��ϢTYPE:%d  ����:%d",TEXT_Obj_8[i-min].TEXT_TYPE,TEXT_Obj_8[i-min].TEXT_LEN);
        }
    }
    else
    {
        max = max % 8;
        //rt_kprintf("\r\n max%8=%d",max);

        if(max == 0)
        {
            for(i = 0; i < 8; i++)
            {
                Api_RecordNum_Read(text_msg, (7 - i), (u8 *)&TEXT_Obj, sizeof(TEXT_Obj));
                memcpy((u8 *)&TEXT_Obj_8[i], (u8 *)&TEXT_Obj, sizeof(TEXT_Obj));
                //rt_kprintf("\r\n(8*n)i=%d ��ϢTYPE:%d  ����:%d",i,TEXT_Obj_8[i].TEXT_TYPE,TEXT_Obj_8[i].TEXT_LEN);
            }
        }
        else
        {
            for(i = 0; i < max; i++)
            {
                Api_RecordNum_Read(text_msg, (max - i), (u8 *)&TEXT_Obj, sizeof(TEXT_Obj));
                memcpy((u8 *)&TEXT_Obj_8[i], (u8 *)&TEXT_Obj, sizeof(TEXT_Obj));
                //rt_kprintf("\r\n(1)��ϢTYPE:%d  ����:%d",TEXT_Obj_8[i].TEXT_TYPE,TEXT_Obj_8[i].TEXT_LEN);
            }
            for(i = 7; i >= max; i--)
            {
                Api_RecordNum_Read(text_msg, i + 1, (u8 *)&TEXT_Obj, sizeof(TEXT_Obj));
                memcpy((u8 *)&TEXT_Obj_8[max + 7 - i], (u8 *)&TEXT_Obj, sizeof(TEXT_Obj));
                //rt_kprintf("\r\n(2)��ϢTYPE:%d  ����:%d",TEXT_Obj_8[max+7-i].TEXT_TYPE,TEXT_Obj_8[max+7-i].TEXT_LEN);
            }
        }
    }
}

void TEXTMSG_Write(u8 num, u8 new_state, u8 len, u8 *str)
{
    //  д������Ϣ
    u8 pos_1_8 = 0; //,i=0;
    //�¼�д��
    TEXT_Obj.TEXT_mOld = new_state; //�Ƿ���������Ϣ
    TEXT_Obj.TEXT_TYPE = num; // 1
    TEXT_Obj.TEXT_LEN = len; //��Ϣ����
    memset(TEXT_Obj.TEXT_STR, 0, sizeof(TEXT_Obj.TEXT_STR));
    memcpy(TEXT_Obj.TEXT_STR, str, TEXT_Obj.TEXT_LEN);

    if(num % 8)
        pos_1_8 = TEXT_Obj.TEXT_TYPE % 8;
    else
        pos_1_8 = 8;
    Api_RecordNum_Write(text_msg, pos_1_8, (u8 *)&TEXT_Obj, sizeof(TEXT_Obj));
}

void TEXTMSG_Write_Init(void)
{
    u8  i = 0;

    TEXT_Obj.TEXT_mOld = 1; //�Ƿ���������Ϣ
    TEXT_Obj.TEXT_TYPE = 1; // 1
    TEXT_Obj.TEXT_LEN = 0; //��Ϣ����
    memset(TEXT_Obj.TEXT_STR, 0, sizeof(TEXT_Obj.TEXT_STR));

    for(i = 0; i < 8; i++)
    {
        //-------------------------------------------------------
        if(i == 0)
            TEXT_Obj.TEXT_mOld = 1; //�Ƿ���������Ϣ
        else
            TEXT_Obj.TEXT_mOld = 0; //�Ƿ���������Ϣ
        //-------------------------------------------------------
        TEXT_Obj.TEXT_TYPE = i + 1; // 1
        Api_RecordNum_Write(text_msg, TEXT_Obj.TEXT_TYPE, (u8 *)&TEXT_Obj, sizeof(TEXT_Obj));

    }
}


/*
     ��ý���������õ�ʱ���ٲ��� write / read   ��ʼ�������ô���
*/




void  BD_EXT_initial(void)
{
    //    ��������
    BD_EXT.GNSS_Mode = 0x02;   //   ˫ģ
    BD_EXT.GNSS_Baud = 0x01;   //   9600
    BD_EXT.BD_OutputFreq = 0x01; // 1000ms
    BD_EXT.BD_SampleFrea = 1; //
    BD_EXT.GNSS_Baud = 0x01; //  9600
    //-----  ��̨��� ----------------------
    BD_EXT.Termi_Type = 0x0001; //  �ն�����
    BD_EXT.Software_Ver = 0x0100; //  Ver  1.00
    BD_EXT.GNSS_Attribute = 0x54444244; // TDBD
    BD_EXT.GSMmodule_Attribute = 0x48554157; // HUAW
    BD_EXT.Device_Attribute = 0x00000001; //  �ն�����

    //   CAN   �������
    BD_EXT.CAN_1_Mode = 0xC0000014;  //  CAN  ģʽ  01:����ģʽ  10  : ��ͨģʽ   11:  ��Ĭģʽ
    /*  bit31  ����  bit 30 -29   ģʽ   bit 15-0   ������0x14   <=> 20k*/
    BD_EXT.CAN_1_ID = 0x01;
    BD_EXT.CAN_1_Type = 0; // ��չ֡
    //     BD_EXT.CAN_1_Duration=1;  // 0   ��ʾֹͣ
    BD_EXT.CAN_2_Mode = 0xC0000014; //   CAN  ģʽ
    BD_EXT.CAN_2_ID = 0x02;
    BD_EXT.CAN_2_Type = 0; // ��չ֡
    // BD_EXT.CAN_2_Duration=1;  // 0   ��ʾֹͣ
    BD_EXT.Collision_Check = 0x0101;   //     �ر���  0.1g    4ms

    //   λ�ø�����Ϣ
    // 1. �ź�ǿ��
    BD_EXT.FJ_SignalValue = 0x0000; //  �ź�ǿ��   ���ֽ� 0 �����ֽ�  ��4Ϊ X2 gprsǿ�� ����4λ ���ǿ���
    //  2. �Զ���״̬��ģ�����ϴ�
    BD_EXT.FJ_IO_1 = 0x00;
    BD_EXT.FJ_IO_2 = 0x00;
    BD_EXT.AD_0 = 0x00; //  2 Byte
    BD_EXT.AD_1 = 0x00; //   2 Byte

    BD_EXT.Close_CommunicateFlag = disable;
    BD_EXT.Trans_GNSS_Flag = disable;

    Api_Config_Recwrite_Large(BD_ext_config, 0, (u8 *)&BD_EXT, sizeof(BD_EXT));
    DF_delay_ms(5);


}

void BD_EXT_Write(void)
{
    Api_Config_Recwrite_Large(BD_ext_config, 0, (u8 *)&BD_EXT, sizeof(BD_EXT));
    DF_delay_ms(15);
}

void BD_EXT_Read(void)
{
    Api_Config_read(BD_ext_config, 0, (u8 *)&BD_EXT, sizeof(BD_EXT));
}

void  BD_list(void)
{
    u8  BD_str[20];

    //-----list -----
    rt_kprintf("\r\n -------������չ��Ϣ���------\r\n ");
    rt_kprintf("\r\n\r\n	 �ն�����:    0x%08X      \r\n       ����汾:   0x%08X  \r\n", BD_EXT.Termi_Type, BD_EXT.Software_Ver);
    rt_kprintf("\r\n\r\n	 GNNS����:   %04s      \r\n       GSM����:  %04s    \r\n   �ն�����:  0x%08X \r\n\r\n    ", (char *)&BD_EXT.GNSS_Attribute, (char *)&BD_EXT.GSMmodule_Attribute, BD_EXT.Device_Attribute);

    memset(BD_str, 0, sizeof(BD_str));
    switch(BD_EXT.CAN_1_Mode & 0x60000000)
    {
    case 0x20000000:
        memcpy(BD_str, "����ģʽ", 8);
        break;
    case 0x40000000:
        memcpy(BD_str, "��ͨģʽ", 8);
        break;
    case 0x6000000:
        memcpy(BD_str, "��Ĭģʽ", 8);
        break;

    }
    rt_kprintf("\r\n   CAN1 :\r\n            CAN1  Mode:    %s      \r\n             CAN1  ID:    0x%08X      \r\n\r\n ", BD_str, BD_EXT.CAN_1_ID);

    memset(BD_str, 0, sizeof(BD_str));
    switch(BD_EXT.CAN_2_Mode & 0x60000000)
    {
    case 0x20000000:
        memcpy(BD_str, "����ģʽ", 8);
        break;
    case 0x40000000:
        memcpy(BD_str, "��ͨģʽ", 8);
        break;
    case 0x6000000:
        memcpy(BD_str, "��Ĭģʽ", 8);
        break;

    }
    //   rt_kprintf("\r\n   CAN2 :\r\n            CAN2  Mode:    %s      \r\n             CAN2  ID:    0x%08X      \r\n\r\n ",BD_str,BD_EXT.CAN_2_ID);



}


void  SendMode_ConterProcess(void)         //  ��ʱ���ʹ������
{
    //   ���ͷ�ʽ����������
    //  1. ������������
    JT808Conf_struct.DURATION.Heart_SDCnter++;
    if(JT808Conf_struct.DURATION.Heart_SDCnter > JT808Conf_struct.DURATION.Heart_Dur) //�������������õļ��
    {
        JT808Conf_struct.DURATION.Heart_SDCnter = 0;
        JT808Conf_struct.DURATION.Heart_SDFlag = 1;
    }
    //  2. ���ͳ�ʱ�ж�
#if 0
    if(1 == JT808Conf_struct.DURATION.TCP_SD_state)
    {
        JT808Conf_struct.DURATION.TCP_ACK_DurCnter++;
        if(JT808Conf_struct.DURATION.TCP_ACK_DurCnter > JT808Conf_struct.DURATION.TCP_ACK_Dur) //����Ӧ��ʱ
        {
            JT808Conf_struct.DURATION.TCP_ACK_DurCnter = 0;
            JT808Conf_struct.DURATION.Heart_SDFlag = 1;       //���·���
            JT808Conf_struct.DURATION.TCP_ReSD_cnter++;
            if(JT808Conf_struct.DURATION.TCP_ReSD_cnter > JT808Conf_struct.DURATION.TCP_ReSD_Num) //���·��ʹ����ж�
            {
                JT808Conf_struct.DURATION.TCP_ReSD_cnter = 0;
                Close_DataLink();   // AT_End();	   //�Ҷ�GPRS����

            }
        }

    }
#endif
}


//----------
void  Rails_Routline_Read(void)
{
    u16  i = 0;

    //-----------��ȡΧ��״̬-------

    for(i = 0; i < 8; i++)
    {
        Api_RecordNum_Read(Rail_rect, i + 1, (u8 *)&Rail_Rectangle_multi[i], sizeof(Rail_Rectangle));
        delay_ms(2);
        Api_RecordNum_Read(Rail_cycle, i + 1, (u8 *)&Rail_Cycle_multi[i], sizeof(Rail_Cycle));


    }

}


//-----------------------------------------------------------------

void  FirstRun_Config_Write(void)
{
    //   �����״θ�����д���ò���

    //  rt_kprintf("\r\n  sizeof(sysconfig): %d   sizeof(jt808): %d    sizeof(tiredconfig): %d   \r\n",sizeof(SysConf_struct),sizeof(JT808Conf_struct),sizeof(TiredConf_struct));
    SysConfig_init();   //  д��ϵͳ������Ϣ
    TIRED_CONF_Init(); //  д��ƣ�ͼ�ʻ���������Ϣ
    JT808_Conf_init();   //  д�� JT808   ������Ϣ
    Api_WriteInit_var_rd_wr();
    BD_EXT_initial();


    Vehicleinfo_Init();// д�복����Ϣ
    Event_Write_Init();
    MSG_BroadCast_Write_Init();
    PhoneBook_Init(0);
    RailCycle_Init();
    RailRect_Init();
    RailPolygen_Init();
    RouteLine_Init();
    TEXTMSG_Write_Init();

    //---- add special -----------
    Login_Menu_Flag = 0;   //  �������Ϊ0
    DF_WriteFlashSector(DF_LOGIIN_Flag_offset, 0, &Login_Menu_Flag, 1);
    Limit_max_SateFlag = 1; //ʹ��
    DF_WriteFlashSector(DF_LimitSPEED_offset, 0, &Limit_max_SateFlag, 1);


}
//-----------------------------------------------------------------
void SetConfig(void)
{
    //u8 i=0;//,len_write=0;
    //	u32 j=0;

    rt_kprintf("\r\nSave Config\r\n");
    // 1.  ��ȡconfig ����      0 :�ɹ�    1 :  ʧ��
    Api_Config_read(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
    //rt_kprintf("\r\nRead Save SYSID\r\n");
    //  2. ��ȡ�ɹ�  ���ж�  �汾ID
    if(SysConf_struct.Version_ID != SYSID) //SYSID)   //  check  wether need  update  or not
    {
        rt_kprintf("\r\n ID not Equal   Saved==0x%X ,  Read==0x%X !\r\n", SYSID, SysConf_struct.Version_ID);
        SysConf_struct.Version_ID = SYSID; // update  ID
        //  2.1  ����ʹ��

        DF_WriteFlashSector(DF_WARN_PLAY_Page, 0, &Warn_Play_controlBit, 1);

        //  2.2  ����д��
        FirstRun_Config_Write();   // ��߸����� SYSID
        vdr_erase();

    }
    else
        rt_kprintf("\r\n Config Already Exist!\r\n");
}

void ReadConfig(void)
{
    u16   res[3];

    DF_delay_ms(500);

    //  1.   read  read to  compare  judge
    //-------- JT808 �������ö�ȡ���ԣ�����Ƶ��������Ҫ������Ҫ���⴦��
    DF_ReadFlash(JT808Start_offset, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
    DF_delay_ms(80); 	// large content delay

    DF_ReadFlash(JT808_BakSetting_offset, 0, (u8 *)&JT808_struct_Bak, sizeof(JT808_struct_Bak));
    DF_delay_ms(80); 	// large content delay

    DF_ReadFlash(JT808_Bak2Setting_offset, 0, (u8 *)&JT808_struct_Bak2, sizeof(JT808_struct_Bak2));
    DF_delay_ms(80); 	// large content delay

    // 2. compare
    /*
            note:   res[0] == org cmp  bak    res[1]== bak  cmp  bak2    res[2]== bak2  cmp  org

            ---org --<seg1>--  bak ---<seg2>----bak2 ---<seg3>---
            |-----------<---------------<----------------------|
     */
    res[0] = memcmp((u8 *)&JT808Conf_struct, (u8 *)&JT808_struct_Bak, sizeof(JT808_struct_Bak));
    res[1] = memcmp((u8 *)&JT808_struct_Bak, (u8 *)&JT808_struct_Bak2, sizeof(JT808_struct_Bak));
    res[2] = memcmp((u8 *)&JT808_struct_Bak2, (u8 *)&JT808Conf_struct, sizeof(JT808_struct_Bak));

    // 3. judge
    if(res[0] && res[1] && res[2]) // ȫ������
    {
        rt_kprintf("\r\n JT808 ȫ��ʧ��!  need all recover\r\n");
        JT808_Conf_init();
    }
    else if(res[0] && res[1]) //    seg1  seg2  ������˵��  BAK error
    {
        // org  bak2 ---ok      bak---error
        if((u8)(JT808Conf_struct.DURATION.Default_Dur >> 24) != 0xFF) // �ж���ȷ���ǲ��� FF
        {

            DF_WriteFlashSector(JT808_BakSetting_offset, 0, (u8 *)&JT808Conf_struct, sizeof(JT808_struct_Bak));
            rt_kprintf("\r\n JT808 BAK error ,correct ok");

        }
        else
        {
            rt_kprintf("\r\n need all recover 1");
            JT808_Conf_init();
        }

    }
    else if(res[0] && res[2]) //  seg1  seg3    ������˵�� BAK2  error
    {
        // org  bak  ---ok       bak2 -----error
        if((u8)(JT808Conf_struct.DURATION.Default_Dur >> 24) != 0xFF) // �ж���ȷ���ǲ��� FF
        {

            DF_WriteFlashSector(JT808_Bak2Setting_offset, 0, (u8 *)&JT808Conf_struct, sizeof(JT808_struct_Bak));
            rt_kprintf("\r\n JT808 BAK2 error ,correct ok");

        }
        else
        {
            rt_kprintf("\r\n need all recover 2");
            JT808_Conf_init();
        }

    }
    else if(res[1] && res[2]) //  seg2  seg3	 ������˵�� org  error
    {
        //  bak  bak2 --ok     org---error
        if((u8)(JT808_struct_Bak.DURATION.Default_Dur >> 24) != 0xFF) // �ж���ȷ���ǲ��� FF
        {

            DF_WriteFlashSector(JT808Start_offset, 0, (u8 *)&JT808_struct_Bak, sizeof(JT808_struct_Bak));
            rt_kprintf("\r\n JT808 org error ,correct ok");

        }
        else
        {
            rt_kprintf("\r\n need all recover 3");
            JT808_Conf_init();
        }

    }
    else
        rt_kprintf("\r\n JT808 ��ȡУ��ɹ�! \r\n");
    //-------------------------------------------------------------------------------------

    SysConfig_Read();  //��ȡϵͳ������Ϣ
    TIRED_DoorValue_Read();

    Event_Read();
    MSG_BroadCast_Read();
    PhoneBook_Read();
    // RailCycle_Read();
    //RailPolygen_Read();
    //RailRect_Read();
    //RouteLine_Read();
    TEXTMsg_Read();
    BD_EXT_Read();
    Api_Read_var_rd_wr();

    //  Vechicle  compare
    DF_ReadFlash(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));

    WatchDog_Feed();
    DF_ReadFlash(DF_VehicleBAK_Struct_offset, 0, (u8 *)&Vechicle_Info_BAK, sizeof(Vechicle_Info_BAK));

    WatchDog_Feed();
    DF_ReadFlash(DF_VehicleBAK2_Struct_offset, 0, (u8 *)&Vechicle_info_BAK2, sizeof(Vechicle_info_BAK2));

    //  compare
    /*
            note:   res[0] == org cmp  bak    res[1]== bak  cmp  bak2    res[2]== bak2  cmp  org

            ---org --<seg1>--  bak ---<seg2>----bak2 ---<seg3>---
            |-----------<---------------<----------------------|
     */
    res[0] = memcmp((u8 *)&Vechicle_Info, (u8 *)&Vechicle_Info_BAK, sizeof(Vechicle_Info_BAK));
    res[1] = memcmp((u8 *)&Vechicle_Info_BAK, (u8 *)&Vechicle_info_BAK2, sizeof(Vechicle_Info_BAK));
    res[2] = memcmp((u8 *)&Vechicle_info_BAK2, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info_BAK));

    // 3. judge
    if(res[0] && res[1] && res[2])	 // ȫ������
    {
        rt_kprintf("\r\n Vechicleȫ��ʧ��! \r\n");
        rt_kprintf("\r\n need all recover");
        Vehicleinfo_Init();// д�복����Ϣ
    }
    else if(res[0] && res[1])	 //    seg1  seg2  ������˵��  BAK error
    {
        // org  bak2 ---ok 	 bak---error
        if((u8)(Vechicle_Info.Dev_CityID >> 8) != 0xFF) // �ж���ȷ���ǲ��� FF
        {
            DF_WriteFlashSector(DF_VehicleBAK_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
            rt_kprintf("\r\n Vehicle BAK error ,correct ok");
        }
        else
        {
            rt_kprintf("\r\n Vehicle need all recover 1");
            Vehicleinfo_Init();
        }
    }
    else if(res[0] && res[2])	//	seg1  seg3	  ������˵�� BAK2  error
    {
        // org	bak  ---ok		 bak2 -----error
        if((u8)(Vechicle_Info.Dev_CityID >> 8) != 0xFF) // �ж���ȷ���ǲ��� FF
        {
            DF_WriteFlashSector(DF_VehicleBAK2_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
            rt_kprintf("\r\n Vehicle BAK2 error ,correct ok");
        }
        else
        {
            rt_kprintf("\r\n Vehicle need all recover 2");
            Vehicleinfo_Init();
        }

    }
    else if(res[1] && res[2])	//	seg2  seg3	  ������˵�� org  error
    {
        //	bak  bak2 --ok	   org---error
        if((u8)(Vechicle_Info.Dev_CityID >> 8) != 0xFF) // �ж���ȷ���ǲ��� FF
        {
            DF_WriteFlashSector(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info_BAK, sizeof(Vechicle_Info_BAK));
            rt_kprintf("\r\n Vehicle BAK error ,correct ok");
        }
        else
        {
            rt_kprintf("\r\n Vehicle need all recover 3");
            Vehicleinfo_Init();
        }

    }
    else
        rt_kprintf("\r\n Vehicle ��ȡУ��ɹ�! \r\n");
    //---- �豸ID  --------
    memset(SMS_Destion, 0, sizeof(SMS_Destion));
    DF_ReadFlash(DF_DeviceID_offset, 0, (u8 *)SMS_Destion, 11);
    //�������ƺ��Ƿ����ñ�־
    DF_ReadFlash(DF_License_effect, 0, &License_Not_SetEnable, 1);

    if(License_Not_SetEnable == 1)
        rt_kprintf("\r\n������\r\n");
    //  ------ SIM ID  ����ID --------
    memset(SimID_12D, 0, sizeof(SimID_12D));
    DF_ReadFlash(DF_SIMID_12D, 0, (u8 *)SimID_12D, 12);

    //------  ��ȡ ¼��״̬-----------
    DF_ReadFlash(DF_LOGIIN_Flag_offset, 0, &Login_Menu_Flag, 1);

    //  ��ȡ����״̬
    DF_ReadFlash(DF_WARN_PLAY_Page, 0, &Warn_Play_controlBit, 1);

    if(JT808Conf_struct.DF_K_adjustState)
    {
        ModuleStatus |= Status_Pcheck;
    }
    else
    {
        ModuleStatus &= ~Status_Pcheck;
    }
    DF_ReadFlash(DF_LimitSPEED_offset, 0, &Limit_max_SateFlag, 1);
    rt_kprintf("\r\n  Limit_max_stateflag=%d", Limit_max_SateFlag);
    Rails_Routline_Read();

    rt_kprintf("\r\n Read Config Over \r\n");
}
void DefaultConfig(void)
{
    u32 DriveCode32 = 0;
    u8  reg_str[30], i = 0;

    // ---  Ӳ���汾��Ϣ-------------
    HardWareVerion = HardWareGet();
    rt_kprintf("\r\n		        -------Ӳ���汾:%X        B : %d %d %d\r\n", HardWareVerion, (HardWareVerion >> 2) & 0x01, (HardWareVerion >> 1) & 0x01, (HardWareVerion & 0x01));
    if(HardWareVerion == 3)
        rt_kprintf("\r\n		       ��̨�ͺ�: TW705   \r\n");
	
    rt_kprintf("\r\n		��ϢĿ�ĺ���:%s       \r\n", SMS_Destion);
     
}
//FINSH_FUNCTION_EXPORT(DefaultConfig, DefaultConfig);


void KorH_check(void)  // ���˻�����ʾ״̬��ѯ
{
    // �����ж�
    if((strcmp(DomainNameStr, "up.gps960.com") == 0) || (strcmp(DomainNameStr, "up.gps960.net") == 0))
        Vechicle_Info.Vech_Type_Mark = 1;
    else if(strcmp(DomainNameStr, "jt1.gghypt.net") == 0)
        Vechicle_Info.Vech_Type_Mark = 2;
}


/*
�����������ļ�
*/
void SysConfiguration(void)
{
    SetConfig();
    ReadConfig();
    DefaultConfig();
}

void product_type(u8 *instr)
{
    u8 len = strlen(instr);

    if(len != 5)
    {
        rt_kprintf("\r\n len error\r\n");
        return;
    }

    if((strncmp(instr, "TW703", 5) == 0) || (strncmp(instr, "TW705", 5) == 0))
    {
        memcpy(Vechicle_Info.ProType, instr, 5);


        DF_WriteFlashSector(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
        WatchDog_Feed();
        DF_WriteFlashSector(DF_VehicleBAK_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));

        WatchDog_Feed();
        DF_WriteFlashSector(DF_VehicleBAK2_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));

        rt_kprintf("\r\n ���ó�̨�ն�: %s\r\n", Vechicle_Info.ProType);

        //--------	�����Ȩ�� -------------------
        idip("clear");
    }

}
FINSH_FUNCTION_EXPORT(product_type, type set);

void  idip(u8 *str)
{
    u8 Reg_buf[25];

    if (strlen((const char *)str) == 0)
    {
        rt_kprintf("\r\n		   ��ǰ��Ȩ��Ϊ: ");
        rt_kprintf(" %s\r\n		   ��Ȩ�볤��: %d\r\n", JT808Conf_struct.ConfirmCode, strlen((const char *)JT808Conf_struct.ConfirmCode));

        return ;
    }
    else
    {
        if(strncmp((const char *)str, "clear", 5) == 0)
        {
            JT808Conf_struct.Regsiter_Status = 0;
            rt_kprintf("     �ֶ���� ��Ȩ�� !\r\n");
            DEV_Login.Operate_enable = 0;
            DEV_Login.Enable_sd = 0;
            DEV_Login.Sd_counter = 0;
        }
        else
        {
            memset(JT808Conf_struct.ConfirmCode, 0, sizeof(JT808Conf_struct.ConfirmCode));
            memcpy(JT808Conf_struct.ConfirmCode, str, strlen((const char *)str));
            JT808Conf_struct.Regsiter_Status = 1;
            rt_kprintf("     �ֶ�����  ��Ȩ��: %s\r\n", JT808Conf_struct.ConfirmCode);

        }
        memset(Reg_buf, 0, sizeof(Reg_buf));
        memcpy(Reg_buf, JT808Conf_struct.ConfirmCode, 20);
        Reg_buf[20] = JT808Conf_struct.Regsiter_Status;
        Api_Config_Recwrite_Large(jt808, 0, (u8 *)&JT808Conf_struct, sizeof(JT808Conf_struct));
    }
}
FINSH_FUNCTION_EXPORT(idip, id code set);



void deviceid(u8 *str)
{

    u8 i = 0, value = 0;
    u8 reg_str[20];

    memset(reg_str, 0, sizeof(reg_str));
    if (strlen((const char *)str) == 0)
    {
        rt_kprintf("\r\n �ն�IDΪ : ");
        for(i = 0; i < 12; i++)
            rt_kprintf("%c", DeviceNumberID[i]);
        rt_kprintf("\r\n");
        return ;
    }
    else
    {
        //---- check -------
        memcpy(reg_str, str, strlen((const char *)str));
        if(strlen((const char *)reg_str) == 12) //  �����ж�
        {
            for(i = 0; i < 12; i++)
            {
                if(!((reg_str[i] >= '0') && (reg_str[i] <= '9')))
                {
                    value = 1;
                    break;
                }
            }

            if(value)
            {
                rt_kprintf("\r\ndevice_ContentError\r\n");
                rt_kprintf("\r\n �ֶ������ն�ID���Ϸ�!  \r\n");
                return ;
            }

        }
        else
        {
            rt_kprintf("\r\ndevice_LenError\r\n");
            rt_kprintf("\r\n �ֶ������ն�ID ���Ȳ���ȷ!  \r\n");
            return ;
        }

        memset(DeviceNumberID, 0, sizeof(DeviceNumberID));
        memcpy(DeviceNumberID, reg_str, 12);
        DF_WriteFlashSector(DF_DeviceID_offset, 0, DeviceNumberID, 13);
        delay_ms(80);
        DF_ReadFlash(DF_DeviceID_offset, 0, DeviceNumberID, 13);
        rt_kprintf("\r\ndevice_OK(");
        for(i = 0; i < 12; i++)
            rt_kprintf("%c", DeviceNumberID[i]);
        rt_kprintf(")\r\n");
        rt_kprintf("\r\n�ֶ��ն�ID����Ϊ :%s\r\n", DeviceNumberID);
        rt_kprintf("\r\n");
        return ;
    }
}
FINSH_FUNCTION_EXPORT(deviceid, deviceid set);



void simid(u8 *str)
{

    u8 i = 0, value = 0;
    u8 reg_str[20];

    memset(reg_str, 0, sizeof(reg_str));
    if (strlen((const char *)str) == 0)
    {
        rt_kprintf("\r\n ���� SIM_IDΪ : ");
        for(i = 0; i < 12; i++)
            rt_kprintf("%c", SimID_12D[i]);
        rt_kprintf("\r\n");
        return ;
    }
    else
    {
        //---- check -------
        memcpy(reg_str, str, strlen((const char *)str));
        if(strlen((const char *)reg_str) == 12) //  �����ж�
        {
            for(i = 0; i < 12; i++)
            {
                if(!((reg_str[i] >= '0') && (reg_str[i] <= '9')))
                {
                    value = 1;
                    break;
                }
            }

            if(value)
            {
                rt_kprintf("\r\n �ֶ���������SIM_ID���Ϸ�!  \r\n");
                return ;
            }

        }
        else
        {
            rt_kprintf("\r\n �ֶ���������SIM_ID ���Ȳ���ȷ!  \r\n");
            return ;
        }

        memset(SimID_12D, 0, sizeof(SimID_12D));
        memcpy(SimID_12D, reg_str, 12);
        DF_WriteFlashSector(DF_SIMID_12D, 0, SimID_12D, 13);
        delay_ms(80);
        rt_kprintf("\r\n �ֶ��豸����Sim_ID����Ϊ : ");
        DF_ReadFlash(DF_SIMID_12D, 0, SimID_12D, 13);
        SIMID_Convert_SIMCODE();  // ת��
        for(i = 0; i < 12; i++)
            rt_kprintf("%c", SimID_12D[i]);
        rt_kprintf("\r\n");
        return ;
    }
}
FINSH_FUNCTION_EXPORT(simid, simid set);













