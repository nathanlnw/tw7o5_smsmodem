/*
    App Gsm uart
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
#include "App_gsm.h"
#include "SMS_PDU.h"
#include "SMS.h"

#define GSM_GPIO			GPIOC
#define GSM_TX_PIN			GPIO_Pin_10
#define GSM_TX_PIN_SOURCE	GPIO_PinSource10

#define GSM_RX_PIN			GPIO_Pin_11
#define GSM_RX_PIN_SOURCE	GPIO_PinSource11


u8 Module_Type = M69_GSM;


#ifdef  M50_GSM
//----------  EM310 �Դ�Э��ջ���  -----------------------------
//   1.  ͨ�������״̬ ��ʼ������
flash char  M50_SIM_Check_Str[] = "AT+QCCID\r\n"; //  ���SIM���Ĵ���
flash char  M50_IMSI_Check_str[] = "AT+CIMI\r\n";
flash char  M50_Signal_Intensity_str[] = "AT+CSQ\r\n"; // �ź�ǿ�� ������
flash char  M50_CommAT_Str1[] = "AT\r\n";
flash char  M50_CommAT_Str2[] = "ATE0\r\n";
flash char  M50_CommAT_Str3[] = "AT+QSPN?\r\n";
flash char  M50_CommAT_Str4[] = "AT+QAUDCH=2\r\n";         // ������Ƶͨ��  ��Ĭ��    2   ����
flash char  M50_CommAT_Str5[] = "AT+QMIC=0,10\r\n"; // ������Ƶmic ����ͨ�� ѡ���һ·
flash char  M50_CommAT_Str6[] = "AT+QIMUX=1\r\n";  //���ö���·ģʽ
flash char  M50_CommAT_Str7[] = "AT+CLVL=80\r\n"; // ����������  --û����Ƶ���� û��
flash char  M50_CommAT_Str8[] = "AT+CRSL=80\r\n"; //;��������
flash char  M50_CommAT_Str9[] = "ATL2\r\n"; // ���ü�������������
flash char  M50_CommAT_Str11[] = "AT+QMEDVL=80\r\n"; //"AT%RECFDEL\r\n";//"AT\r\n"; //
flash char  M50_CommAT_Str12[] = "AT+CMGF=0\r\n"; // PDU ģʽ
flash char  M50_CommAT_Str13[] = "AT+CPMS=\"SM\",\"SM\",\"SM\"\r\n";
flash char  M50_CommAT_Str14[] = "AT+CNMI=2,1\r\n";


//    2.   ͨ��ģ���½������·�������
flash char       M50_DialInit1[]                          = "AT+CGATT=1\r\n";
flash char	M50_DialInit2[]				=	"AT+CGACT=1,1\r\n";
flash char	M50_DialInit3[]				=	"AT+CGPADDR=1\r\n";
flash char	M50_DialInit4[] 			       =	"AT\r\n";  // �鿴��ǰIP
char	M50_Dialinit_DNSR[50] 			=	"AT+QIDNSGIP=\"";     //  ���� up.gps960.com
char	M50_Dialinit_DNSR2[50] 			=	"AT+QIDNSGIP=\"";     //  ����


static u8   M50_Mainlink_send_over = 0; // ����·�Ѿ�������Ϣ
flash u8	M50_Send_TCP[20] = "AT+QISEND=1,%d\r\n";
flash u8	M50_Send_ISP[20] = "AT+QISEND=2,%d\r\n";

static u8	M50_DialStr_Link1_demo[90] = "AT+QIOPEN=%d,\"%s\",\"%d.%d.%d.%d\",\"%d\"\r\n"; //"ATD*99***1#\r\n"; AT%IPOPENX=1,"UDP","117.11.126.248",7106
static u8   M50_DialStr_LinkAux_demo[90] = "AT+QIOPEN=%d,\"%s\",\"%d.%d.%d.%d\",\"%d\"\r\n"; //"ATD*99***1#\r\n"; AT%IPOPENX=1,"UDP","117.11.126.248",7106
static u8   M50_DialStr_IC_card_demo[90] = "AT+QIOPEN=%d,\"%s\",\"%d.%d.%d.%d\",\"%d\"\r\n"; //"ATD*99***1#\r\n"; AT%IPOPENX=1,"UDP","117.11.126.248",7106

static u8	M50_DialStr_Link1[90];
static u8   M50_DialStr_LinkAux[90];
static u8   M50_DialStr_IC_card[90];

flash char  M50_CutDataLnk_str2[] = "AT+QICLOSE=1\r\n";


//  M66  Rec voice
flash char M50_VoiceRec_config[] = "ATI\r\n"; // ����¼����ʽ��¼������
//  0C: 28 ���� 18�� 504
flash char M50_VoiceRec_start[] = "AT+QAUDRD=1,\"voice.amr\",3\r\n"; // 1: start rec    3: amr format
flash char M50_VoiceRec_stop[] = "AT+QAUDRD=0,\"voice.amr\",3\r\n";
flash char M50_VoiceRec_file_size[] = "AT+QFLST\r\n";
flash char M50_VoiceRec_delete[] = "AT+QFDEL=\"voice.amr\"\r\n";
flash char M50_VoiceRec_fileopen[] = "AT+QFOPEN=\"voice.amr\"\r\n";

static u8  M50_VoiceRec_sd_getAT[50] = "AT+QFREAD=%s,%d\r\n";
static u8  M50_VoiceRec_fileclose[] = "AT+QFCLOSE=%s\r\n";

#endif



#ifdef   M69_GSM

//----------  EM310 �Դ�Э��ջ���  -----------------------------
//   1.  ͨ�������״̬ ��ʼ������
flash char  SIM_Check_Str[] = "AT%TSIM\r\n"; //  ���SIM���Ĵ���
flash char  IMSI_Check_str[] = "AT+CIMI\r\n";
flash char  Signal_Intensity_str[] = "AT+CSQ\r\n"; // �ź�ǿ�� ������
flash char  CommAT_Str1[] = "ATV1\r\n";
flash char  CommAT_Str2[] = "ATE0\r\n";
flash char  CommAT_Str3[] = "AT+COPS?\r\n";
flash char  CommAT_Str4[] = "AT%SNFS=1\r\n";         // ������Ƶ���ͨ��ѡ�� �ڶ�·
flash char  CommAT_Str5[] = "AT%NFI=1,10,0,0\r\n"; // ������Ƶ����ͨ�� ѡ��� ��·
flash char  CommAT_Str6[] = "AT+CLVL=6\r\n";
flash char  CommAT_Str7[] = "AT%NFV=5\r\n"; // ����������  --û����Ƶ���� û��
flash char  CommAT_Str8[] = "AT%NFO=1,6,0\r\n"; //�����������
flash char  CommAT_Str9[] = "AT%VLB=1\r\n"; //ATI
flash char  CommAT_Str10[] = "AT%NFW=1\r\n";  //   ������Ƶ����
flash char  CommAT_Str11[] = "AT+CGMR\r\n"; //"AT%RECFDEL\r\n";//"AT\r\n"; //
flash char  CommAT_Str12[] = "AT+CMGF=0\r\n"; 		///PDUģʽ
flash char  CommAT_Str13[] = "AT+CPMS=\"SM\",\"SM\",\"SM\"\r\n";
flash char  CommAT_Str14[] = "AT+CNMI=1,2\r\n"; 		///ֱ�ӽ��������
flash char  CommAT_Str15[] = "AT+CMGD=1,4\r\n";
flash char  CommAT_Str16[] = "AT+CSCA?\r\n";
flash char  CommAT_Str17[] = "AT+CCID\r\n";
flash char  CommAT_Str18[] = "AT+VGR=9\r\n"; // ͨ���������


//    2.   ͨ��ģ���½������·�������
flash char       DialInit1[]                          =    "AT+CREG?\r\n"; // "AT+CGACT=1,1\r\n"; S
flash char	DialInit2[]				=	"AT%IOMODE=1,2,1\r\n";
flash char	DialInit3[]				=	"AT%ETCPIP=\"user\",\"gprs\"\r\n";
flash char	DialInit4[] 			       =	"AT%ETCPIP?\r\n";
char	Dialinit_DNSR[50] 			=	"AT%DNSR=\"";     //  ���� up.gps960.com
char	Dialinit_DNSR2[50] 			=	"AT%DNSR=\"";     //  ����

flash char	DialInit6[] 			       =	"AT%NFO=1,6,0\r\n";
flash char	DialInit7[] 			       =	"AT\r\n";
flash char	DialInit8[] 			       =	"AT\r\n";//"AT+CGPADDR\r\n";

flash char	Open_TCP[] = "AT%ETCPIP=\"user\",\"gprs\"\r\n";

static u8	Send_TCP[18] = "AT%IPSENDX=1,\"";
static u8	Send_ISP[18] = "AT%IPSENDX=2,\"";
static u8	DialStr_Link1[90] = "AT%IPOPENX=1,\"TCP\",\""; //"ATD*99***1#\r\n"; AT%IPOPENX=1,"UDP","117.11.126.248",7106
static u8   DialStr_LinkAux[50] = "AT%IPOPENX=1,\"TCP\",\""; //"ATD*99***1#\r\n"; AT%IPOPENX=1,"UDP","117.11.126.248",7106
static u8	Dialinit_APN[40] = "AT+CGDCONT=1,\"IP\",\"";		//CMNET\"\r\n"; 	 // oranges Access Point Name
static u8   DialStr_IC_card[50] = "AT%IPOPENX=2,\"TCP\",\""; //  Link2




flash char  CutDataLnk_str1[] = "AT%IPCLOSE=3\r\n";
flash char  CutDataLnk_str2[] = "AT%IPCLOSE=2\r\n";
flash char  CutDataLnk_str3[] = "AT%IPCLOSE=1\r\n";
flash char  CutDataLnk_str4[] = "AT%IPCLOSE=5\r\n";
flash char  CutDataLnk_str5[] = "ATH\r\n";

#endif

//  M66  Rec voice
flash char VoiceRec_config[] = "AT%RECCFG=\"AMR\",0\r\n"; // ����¼����ʽ��¼������
flash char VoiceRec_start[] = "AT%RECSTART=\""; //at%recstart="test1.amr",10
flash char VoiceRec_stop[] = "AT%RECSTOP\r\n"; 	 // %RECSTOP: 10,6824
flash char  VoiceRec_getdata[] = "AT%RECFGET=\"";        //at%recfget="test1.amr",100,100
flash char  VoiceRec_delFile[] = "AT%RECFDEL\r\n"; //  AT%RECFDEL="test1.amr"

//-------  struct  variables -------------
GSM_POWER   GSM_PWR;
static IMSI_GET       IMSIGet;
COMM_AT               CommAT;
DATA_DIAL             DataDial;


ALIGN(RT_ALIGN_SIZE)
u8     GSM_rx[GSMRX_SIZE];
u16     GSM_rx_Wr = 0;
u16     gsm_rx_infolen = 0;
u8      gsm_rx_linknum = 0;
u8      gsm_rdy_2_rxEnable = 0; // �ȴ�����



u16     info_len = 0;
static u8  former_byte = 0;

static GSM_typeBUF GSM_INT_BUFF;
static GSM_typeBUF GSM_RX_BUFF;

u8  GSM_AsciiTx[GSM_AsciiTX_SIZE];
u16   GSM_AsciiTx_len = 0;

//-----  1 s timer  related below   ------
u16   one_second_couner = 0;
u8     Enable_UDP_sdFlag = 0;


u8    Dial_jump_State = 0; // ���Ź�����  ״̬��ת��־�� DialInit5  DialInit6 �� DialInit7 ����ת���
u8    Dnsr_state = 0; //  DNSR ״̬   1: ��ʾ�����������ɹ���ǰ����


//------- GPPRS ������� ----------------
//u8 Datalink_close=0;  //�ҶϺ��ٵ�½

//--------   �绰����ָʾ -------------------
u8  Ring_counter = 0; // ring   ���绰������ָʾ
u8  Calling_ATA_flag = 0; //     �����绰����


//-------- TCP2 send ---------
u8     TCP2_ready_dial = 0;
u16    Ready_dial_counter2 = 0;
u16    TCP2_notconnect_counter = 0;
u8     TCP2_Connect = 0;
u8	  TCP2_sdFlag = 0;		//��ʱ����GPSλ����Ϣ��־
u16    TCP2_sdDuration = 50;
u8      TCP2_Coutner = 0; // ��ʱ������
u8      TCP2_login = 0;     // TCP ���������Ӻ�ı�־λ
u8       Online_error_counter = 0; // ���������������,�������ִ��������


//    TTS   ���
TTS              TTS_Var;  //  TTS ���ͱ���
ALIGN(RT_ALIGN_SIZE)
u8                AT_TTS[800];
u16               TTS_Len = 0;


//  Voice  Record

// u8  Debug_gsmnoack=0;
void Dial_Stage(T_Dial_Stage  Stage);
static void GSM_Process(u8 *instr, u16 len);
u32 GSM_HextoAscii_Convert(u8 *SourceHex, u16 SouceHexlen, u8 *Dest);

//=========================================================================
//    TTS  realated
u8    TTS_Get_Data(u8 *Instr, u16 LEN)    //  return   0   : OK     return   1 : busy
{
    // 1. check status
    if(TTS_Var.Playing)
    {
        memset(TTS_Var.HEX_BUF, 0, sizeof((const char *)TTS_Var.HEX_BUF));
        TTS_Var.HEX_len = LEN;
        memcpy(TTS_Var.HEX_BUF, Instr, LEN);
        TTS_Var.Save = 1;
        return  TTS_BUSY;

    }
    //  2.   HEX to  ASCII convert
    memset(TTS_Var.ASCII_BUF, 0, sizeof((const char *)TTS_Var.ASCII_BUF));

#ifdef M50_GSM
    if(Module_Type)
    {
        memcpy(TTS_Var.ASCII_BUF, Instr, LEN);
        TTS_Var.ASCII_Len = LEN ; // ����
    }
    else
#endif
    {
        TTS_Var.ASCII_Len = GSM_HextoAscii_Convert(Instr, LEN, TTS_Var.ASCII_BUF);
        TTS_Var.ASCII_Len = (LEN << 1); // ���ȳ��� 2
    }

    //  3. cacu  timeout value
    TTS_Var.TimeOut_limt = LEN / 3 + 3; // 3����ÿ��+ 3  �뱣��ʱ��
    TTS_Var.TimeCounter = 0;
    //  4.  ready to play
    TTS_Var.NeedtoPlay = 1;
    return  TTS_OK;
}
u8    TTS_Data_Play(void)
{
    u16   i = 0;
    //  1.  check status
    if(TTS_Var.Playing)
        return  TTS_BUSY;
    else if(TTS_Var.Save) //  2.  check  save  status
    {
        TTS_Var.Save = 0;
        //  3. HEX to  ASCII
        memset(TTS_Var.ASCII_BUF, 0, sizeof((const char *)TTS_Var.ASCII_BUF));
#ifdef M50_GSM
        if(Module_Type)
        {
            memcpy(TTS_Var.ASCII_BUF, TTS_Var.HEX_BUF, TTS_Var.HEX_len);
            TTS_Var.ASCII_Len = TTS_Var.HEX_len ; // ����
        }
        else
#endif
        {
            TTS_Var.ASCII_Len = GSM_HextoAscii_Convert(TTS_Var.HEX_BUF, TTS_Var.HEX_len, TTS_Var.ASCII_BUF);
            TTS_Var.ASCII_Len = (TTS_Var.HEX_len << 1); // ���ȳ��� 2
        }
        //  4. cacu  timeout value
        TTS_Var.TimeOut_limt = TTS_Var.HEX_len / 3 + 3; // 3����ÿ��+ 3  �뱣��ʱ��
        TTS_Var.TimeCounter = 0;
        //  5.  ready to play
        TTS_Var.NeedtoPlay = 1;
    }

    //   6.   play process
    if(( TTS_Var.NeedtoPlay	== 1) && (print_rec_flag == 0)) // Ϊ�˼�С���������ӡʱ�򲻲���
    {
        Speak_ON;   // ������
        TTS_Var.Playing = 1;
        //  head
        memset(AT_TTS, 0, sizeof(AT_TTS));
#ifdef M50_GSM
        if(Module_Type)
            strcat(AT_TTS, "AT+QTTS=2,\"");
        else
#endif
            strcat(AT_TTS, "AT%TTS=2,3,6,\"");
        TTS_Len = strlen(AT_TTS)	;
        //  info
        memcpy(AT_TTS + TTS_Len, TTS_Var.ASCII_BUF, TTS_Var.ASCII_Len);
        TTS_Len += TTS_Var.ASCII_Len;
        //  tail
#ifdef M50_GSM
        if(Module_Type)
        {
            memcpy((char *)AT_TTS + TTS_Len, "\",4\r\n", 5); // tail
            TTS_Len += 5;
        }
        else
#endif
        {
            memcpy((char *)AT_TTS + TTS_Len, "\"\r\n", 3); // tail
            TTS_Len += 3;
        }
        if( GB19056.workstate == 0)
        {
            for(i = 0; i < TTS_Len; i++)
                rt_kprintf("%c", AT_TTS[i]);
        }

        rt_hw_gsm_output_Data(AT_TTS, TTS_Len);

        WatchDog_Feed();
        delay_ms(30); // rt_thread_delay(RT_TICK_PER_SECOND/8);

        //---------------------
        TTS_Var.NeedtoPlay = 0;
    }
    return TTS_OK;
}

void   TTS_Play_End(void)
{
    TTS_Var.Playing = 0;
    TTS_Var.TimeCounter = 0;

    Speak_OFF;	   // �رչ���
}
void TTS_Exception_TimeLimt(void)     //  ��λ: s
{
    //  TTS  �����쳣 ��ʱ������
    if(TTS_Var.Playing)
    {
        TTS_Var.TimeCounter++;
        if(TTS_Var.TimeCounter > TTS_Var.TimeOut_limt)
        {
            TTS_Var.Playing = 0;
            TTS_Var.TimeCounter = 0;
            Speak_OFF;
        }
    }
}

u8  TTS_ACK_Error_Process(void)
{
    // ����TTS  ������������󷵻�Error �쳣
    if(TTS_Var.Playing)
    {
        TTS_Var.Playing = 0;
        TTS_Var.TimeCounter = 0;
        return true;
    }
    else
        return false;
}

void TTS_play(u8 *instr)
{
    TTS_Get_Data(instr, strlen((const char *)instr));
    // rt_kprintf("\r\n    �ֶ���������: %s\r\n",instr);
}
FINSH_FUNCTION_EXPORT(TTS_play, TTS play);

void gsm_power_cut(void)
{
    GSM_PWR.GSM_powerCounter = 0;
    GSM_PWR.GSM_power_over = 3;  // enable  GSM reset
    rt_kprintf(" \r\n  GSM ģ��׼���ر�!  \r\n");

}
//FINSH_FUNCTION_EXPORT(gsm_power_cut, gsm_power_cut);



void AT_cmd_send_TimeOUT(void)
{
    // work  in    1  sencond
    if(CommAT.AT_cmd_sendState == enable)
    {
        CommAT.AT_cmd_send_timeout++;
        if(CommAT.AT_cmd_send_timeout > 70)
        {
            gsm_power_cut();
            CommAT.AT_cmd_send_timeout = 0;
            CommAT.AT_cmd_sendState = 0;
            rt_kprintf("AT  noack  reset GSM module\r\n");
        }
    }
}

void GSM_CSQ_timeout(void)
{
    CSQ_counter++;
    if(CSQ_counter >= CSQ_Duration)
    {
        CSQ_counter = 0;
        CSQ_flag = 1;
    }

}

u8 GSM_CSQ_Query(void)
{
    if((GSM_PWR.GSM_power_over > 0) && (GSM_PWR.GSM_power_over <= 2))
    {
        if((CSQ_flag == 1) && (MediaObj.Media_transmittingFlag == 0))
        {
            CSQ_flag = 0;
            delay_ms(100);
            rt_hw_gsm_output("AT+CSQ\r\n");    //����ź�ǿ��
            if(GB19056.workstate == 0)
                rt_kprintf("AT+CSQ\r\n");
            return true;
        }
    }
    return false;
}




//-------------------------------------------------------------------------------
void  DataLink_MainSocket_set(u8 *IP, u16  PORT, u8 DebugOUT)
{
#ifdef M50_GSM

    if(Module_Type)
    {
        memset((char *)M50_DialStr_Link1, 0, sizeof(M50_DialStr_Link1)); // clear
        sprintf((char *)M50_DialStr_Link1, M50_DialStr_Link1_demo, 1, "TCP", IP[0], IP[1], IP[2], IP[3], PORT);

        if(DebugOUT)
        {
            rt_kprintf("		   Main Initial  Str:");
            rt_kprintf((char *)M50_DialStr_Link1);
        }
    }
    else
#endif
    {
        memset((char *)DialStr_Link1 + 20, 0, sizeof(DialStr_Link1) - 20);
        IP_Str((char *)DialStr_Link1 + 20, *( u32 * ) IP);

        strcat((char *) DialStr_Link1, "\"," );
        sprintf((char *)DialStr_Link1 + strlen((char const *)DialStr_Link1), "%u\r\n", PORT);

        if(DebugOUT)
        {
            rt_kprintf("		   Main Initial  Str:");
            rt_kprintf((char *)DialStr_Link1);
        }
    }
}

void  DataLink_AuxSocket_set(u8 *IP, u16  PORT, u8 DebugOUT)
{
#ifdef M50_GSM

    if(Module_Type)
    {
        memset((char *)M50_DialStr_LinkAux, 0, sizeof(M50_DialStr_LinkAux)); // clear
        sprintf((char *)M50_DialStr_LinkAux, M50_DialStr_LinkAux_demo, 1, "TCP", IP[0], IP[1], IP[2], IP[3], PORT);

        if(DebugOUT)
        {
            rt_kprintf("\r\n	��IP   DialString : ");
            rt_kprintf((char *)M50_DialStr_LinkAux);
        }

    }
    else
#endif
    {
        memset((char *)DialStr_LinkAux + 20, 0, sizeof(DialStr_LinkAux) - 20);
        IP_Str((char *)DialStr_LinkAux + 20, *( u32 * ) IP);

        strcat((char *) DialStr_LinkAux, "\"," );
        sprintf((char *)DialStr_LinkAux + strlen((char const *)DialStr_LinkAux), "%u\r\n", PORT);

        if(DebugOUT)
        {
            rt_kprintf("\r\n  ��IP   DialString : ");
            rt_kprintf((char *)DialStr_LinkAux);
        }
    }
}
void  DataLink_IC_Socket_set(u8 *IP, u16  PORT, u8 DebugOUT)
{
#ifdef M50_GSM
    if(Module_Type)
    {
        memset((char *)M50_DialStr_IC_card, 0, sizeof(M50_DialStr_IC_card)); // clear
        sprintf((char *)M50_DialStr_IC_card, M50_DialStr_IC_card_demo, 2, "TCP", IP[0], IP[1], IP[2], IP[3], PORT);
        if(DebugOUT)
        {
            rt_kprintf("\r\n IC  DialString : ");
            rt_kprintf((char *)M50_DialStr_IC_card);
        }

    }
    else
#endif
    {
        memset((char *)DialStr_IC_card + 20, 0, sizeof(DialStr_IC_card) - 20);
        IP_Str((char *)DialStr_IC_card + 20, *( u32 * ) IP);

        strcat((char *) DialStr_IC_card, "\"," );
        sprintf((char *)DialStr_IC_card + strlen((char const *)DialStr_IC_card), "%u\r\n", PORT);

        if(DebugOUT)
        {
            rt_kprintf("\r\n IC  DialString : ");
            rt_kprintf((char *)DialStr_IC_card);
        }
    }
}

void  DataLink_IspSocket_set(u8 *IP, u16  PORT, u8 DebugOUT)
{
    u8 *regstr[30];

    memset((char *)regstr, 0, sizeof(regstr));
    IP_Str((char *)regstr, *( u32 * ) IP);

    sprintf(( char *)regstr, IP);

    if(DebugOUT)
        rt_kprintf("		  Aux Socket : %s\r\n", regstr);
}

void  DataLink_APN_Set(u8 *apn_str, u8 DebugOUT)
{

    memset(Dialinit_APN + APN_initSTR_LEN, 0, sizeof(Dialinit_APN) - APN_initSTR_LEN);
    memcpy(Dialinit_APN + APN_initSTR_LEN, apn_str, strlen((char const *)apn_str));
    strcat( (char *)Dialinit_APN, "\"\r\n" );

    if(DebugOUT)
    {
        rt_kprintf("\r\n APN ���� :  ");
        rt_kprintf((const char *)Dialinit_APN);
    }

}


void  DataLink_DNSR_Set(u8 *Dns_str, u8 DebugOUT)
{
#ifdef M50_GSM
    memset(M50_Dialinit_DNSR + 13, 0, sizeof(M50_Dialinit_DNSR) - 13);
    memcpy(M50_Dialinit_DNSR + 13, Dns_str, strlen((char const *)Dns_str));
    strcat( M50_Dialinit_DNSR, "\"\r\n" );
#endif

    memset(Dialinit_DNSR + 9, 0, sizeof(Dialinit_DNSR) - 9);
    memcpy(Dialinit_DNSR + 9, Dns_str, strlen((char const *)Dns_str));
    strcat( Dialinit_DNSR, "\"\r\n" );

    if(DebugOUT)
    {
        rt_kprintf("\r\n		 ����1 ���� :	 ");
        rt_kprintf(Dialinit_DNSR);
    }
}


void  DataLink_DNSR2_Set(u8 *Dns_str, u8 DebugOUT)
{
#ifdef M50_GSM
    memset(M50_Dialinit_DNSR2 + 13, 0, sizeof(M50_Dialinit_DNSR2) - 13);
    memcpy(M50_Dialinit_DNSR2 + 13, Dns_str, strlen((char const *)Dns_str));
    strcat( M50_Dialinit_DNSR2, "\"\r\n" );
#endif

    memset(Dialinit_DNSR2 + 9, 0, sizeof(Dialinit_DNSR2) - 9);
    memcpy(Dialinit_DNSR2 + 9, Dns_str, strlen((char const *)Dns_str));
    strcat( Dialinit_DNSR2, "\"\r\n" );

    if(DebugOUT)
    {
        rt_kprintf("\r\n		Aux  �������� :	 ");
        rt_kprintf(Dialinit_DNSR2);
    }

}

void Gsm_RegisterInit(void)
{
    //--------   Power  Related   ---------
    GSM_PWR.GSM_PowerEnable = 1; // ��ʼʹ��
    GSM_PWR.GSM_powerCounter = 0;
    GSM_PWR.GSM_power_over = 0;

    // --------  IMSI  -------------------
    memset((u8 *)&IMSIGet, 0, sizeof(IMSIGet));

    //---------- COMM AT ----------------
    memset((u8 *)&CommAT, 0, sizeof(CommAT));


    //----------- Data Dial ----------------
    memset((u8 *)&DataDial, 0, sizeof(DataDial));
    DataDial.start_dial_stateFLAG = 1; //important

    //---  Network Setting  Default ----
    DataLink_APN_Set(APN_String, 0);  // apn
    DataLink_DNSR_Set(DomainNameStr, 0);   // DNSR  MG323  û��
    DataLink_DNSR2_Set(DomainNameStr_aux, 0);
    DataLink_MainSocket_set(RemoteIP_main, RemotePort_main, 0);

    //  DataLink_AuxSocket_set(RemoteIP_aux, RemotePort_aux,1);
    Dial_Stage(Dial_Idle);
}


/* write one character to serial, must not trigger interrupt */
void rt_hw_gsm_putc(const char c)
{
    /*
    	to be polite with serial console add a line feed
    	to the carriage return character
    */
    //if (c=='\n')rt_hw_gps_putc('\r');
    //USART_SendData(UART4,  c);
    //while (!(UART4->SR & USART_FLAG_TXE));
    //UART4->DR = (c & 0x1FF);
    USART_SendData( UART4, c );
    while( USART_GetFlagStatus( UART4, USART_FLAG_TC ) == RESET )
    {
    }

}

void rt_hw_gsm_output(const char *str)
{
    //   u16  len=0;
    /* empty console output */
    //--------  add by  nathanlnw ---------

    while (*str)
    {
        rt_hw_gsm_putc (*str++);
    }
    CommAT.AT_cmd_sendState = enable;
    /* len=strlen(str);
       while( len )
    {
    	USART_SendData( UART4, *str++ );
    	while (!(UART4->SR & USART_FLAG_TXE));
    	UART4->DR = (*str++ & 0x1FF);
    	len--;
    }  */
    //--------  add by  nathanlnw  --------
}

void rt_hw_gsm_output_Data(u8 *Instr, u16 len)
{
    unsigned int  infolen = 0;

    infolen = len;

    //--------  add by  nathanlnw ---------
    while (infolen)
    {
        rt_hw_gsm_putc (*Instr++);
        infolen--;
    }
    //--------  add by  nathanlnw  --------
    CommAT.AT_cmd_sendState = enable;
}
void Dial_Stage(T_Dial_Stage  Stage)
{
    // set the AT modem stage
    //if (DataDial.Dial_step == Stage) return;	// no change
    //
    DataDial.Dial_step = Stage;				//
    DataDial.Dial_step_RetryTimer = Dial_Dial_Retry_Time;
    DataDial.Dial_step_Retry = 0;
}
void  GSM_RxHandler(u8 data)
{
    static  u16  i = 0;

    rt_interrupt_enter( );
#ifdef M50_GSM
    if(gsm_rx_infolen)
    {
        if(gsm_rdy_2_rxEnable == 1) // first  rx
        {
            GSM_INT_BUFF.gsm_wr = 0; // clear first
            gsm_rdy_2_rxEnable = 0;
        }
        gsm_rx_infolen--;

        GSM_INT_BUFF.gsm_content[GSM_INT_BUFF.gsm_wr++] = data;

        if(gsm_rx_infolen == 0)	// rx end
        {

            switch(gsm_rx_linknum)
            {
            case  1:  // mainlink
                memcpy(GSM_HEX, GSM_INT_BUFF.gsm_content, GSM_INT_BUFF.gsm_wr);
                GSM_HEX_len = GSM_INT_BUFF.gsm_wr;
                Receive_DataFlag = 1;
                gsm_rx_linknum = 0;
                break;
            case  2:  // aux link
                memcpy(GSM_HEX, GSM_INT_BUFF.gsm_content, GSM_INT_BUFF.gsm_wr);
                GSM_HEX_len = GSM_INT_BUFF.gsm_wr;
                Receive_DataFlag = 1;
                gsm_rx_linknum = 0;
                break;
            }
            GSM_INT_BUFF.gsm_wr = 0;
        }
        else   //  rx  not  end
        {
            //---------------------------------------------------------
            if( GSM_INT_BUFF.gsm_wr == GSM_TYPEBUF_SIZE )
            {
                GSM_INT_BUFF.gsm_wr = 0;
            }
            GSM_INT_BUFF.gsm_content[GSM_INT_BUFF.gsm_wr ] = 0;
            //--------------------------------------------------------
        }

    }
    else
#endif
        if( ( data == 0x0a) && (former_byte == 0x0d ) ) /*����0d 0a ��������*/
        {
            GSM_INT_BUFF.gsm_content[GSM_INT_BUFF.gsm_wr++] = data;
            if( GSM_INT_BUFF.gsm_wr < 1400 )
            {

                /* ���ж����жϲ�����*/
#ifdef M50_GSM
                if( strncmp( GSM_INT_BUFF.gsm_content, "+RECEIVE:", 9 ) == 0 ) //+RECEIVE: 1, 29
                {
                    /*����������Ϣ,���������Ż���pdst*/
                    i = sscanf(GSM_INT_BUFF.gsm_content, "+RECEIVE: %d, %d", &gsm_rx_linknum, &gsm_rx_infolen );
                    if( i != 2 )
                    {
                        gsm_rx_linknum  = 0;
                        gsm_rx_infolen = 0;
                    }
                    gsm_rdy_2_rxEnable = 1;

                }
                else
#endif
                    rt_mq_send( &mq_GSM, (void *)&GSM_INT_BUFF, GSM_INT_BUFF.gsm_wr + 2 );
            }

            GSM_INT_BUFF.gsm_wr = 0;
        }
        else
        {
            GSM_INT_BUFF.gsm_content[GSM_INT_BUFF.gsm_wr++] = data;
            if( GSM_INT_BUFF.gsm_wr == GSM_TYPEBUF_SIZE )
            {
                GSM_INT_BUFF.gsm_wr = 0;
            }
            GSM_INT_BUFF.gsm_content[GSM_INT_BUFF.gsm_wr] = 0;
        }
    former_byte = data;
    rt_interrupt_leave( );

}


void  GSM_Buffer_Read_Process(void)
{
    // char ch;
    //-----------------------------------------------------
    rt_err_t	res;

    {
        res = rt_mq_recv( &mq_GSM, (void *)&GSM_RX_BUFF, 1400, 3 ); //�ȴ�100ms,ʵ���Ͼ��Ǳ䳤����ʱ,�100ms
        if( res == RT_EOK )                                                     //�յ�һ������
        {
            GSM_Process(GSM_RX_BUFF.gsm_content, GSM_RX_BUFF.gsm_wr);
        }
    }

}
//------------------------  ASCII    HEX  convert   ---------------------
u8  HexValue (u8 inchar)
{
    switch(inchar)
    {
    case '0':
        return  0;
    case '1':
        return  1;
    case '2':
        return  2;
    case '3':
        return  3;
    case '4':
        return  4;
    case '5':
        return 5;
    case '6':
        return 6;
    case '7':
        return 7;
    case '8':
        return 8;
    case '9':
        return 9;
    case 'A':
        return 0x0A;
    case 'B':

        return 0x0B;
    case 'C':
        return 0x0C;
    case 'D':
        return 0x0D;
    case 'E':
        return 0x0E;
    case 'F':
        return 0x0F;
    default :
        // rt_kprintf("\r\n ת���д���:%c \r\n",inchar);
        return  0xFF;
    }
}
//------------------------------- HEX to ASCII --------------------------------------
u32 GSM_HextoAscii_Convert(u8 *SourceHex, u16 SouceHexlen, u8 *Dest)
{
    u16 len_counter = 0;
    u16 dest_counter = 0;
    u8  c = 0;

    for(len_counter = 0; len_counter < SouceHexlen; len_counter++)
    {
        c = SourceHex[len_counter];

        //---------------- High --------------------------------
        if((c >> 4) >= 10)
            Dest[dest_counter++] = 'A' + (c >> 4) - 10;
        else
            Dest[dest_counter++] = '0' + (c >> 4);

        Dest[dest_counter] = 0x00;

        //----------------- Low --------------------------------
        c = c & 0x0F;
        if(c >= 10)
            Dest[dest_counter++] = 'A' + c - 10;
        else
            Dest[dest_counter++] = '0' + c;
        Dest[dest_counter] = 0x00;
    }

    return dest_counter;
}
u16  GSM_AsciitoHEX_Convert(u8 *Src_str, u16 Src_infolen, u8 *Out_Str)
{
    u16 Counter = 0, Out_Str_Len = 0;
    u8  C = 0;

    Out_Str_Len = 0;
    /*
         if((u8)Src_infolen&0x01)
       	{
       	     rt_kprintf("\r\n      ����ASCII��Ϣ����ȷ!   %u  \r\n",Src_infolen);
       	}
      */
    for(Counter = 0; Counter < Src_infolen; Counter++)
    {
        //--------------------------------------------------
        if((Counter & 0x01) == 1)
        {
            C = HexValue(*(Src_str + Counter));
            if(C != 0xff)
            {
                Out_Str[Out_Str_Len] += C;
                Out_Str_Len++;
            }
            // else
            //  rt_kprintf("\r\n    Convert Error at:   %u  \r\n",Counter);
        }
        else
        {
            C = ((HexValue(*(Src_str + Counter))) << 4);
            Out_Str[Out_Str_Len] = (HexValue(*(Src_str + Counter))) << 4;
            if(C != 0xff)
            {
                Out_Str[Out_Str_Len] = C;
            }
            // else
            //  rt_kprintf("\r\n    Convert Error at2:   %u  \r\n",Counter);
        }

    }
    return Out_Str_Len;

}



void  Data_Send(u8 *DataStr, u16  Datalen, u8  Link_Num)
{
    u16  i = 0, packet_len = 0;
    u8   AT_send_str[20];
    //  4. Send
#ifdef M50_GSM
    if(Module_Type)
    {
        memset(AT_send_str, 0, sizeof(AT_send_str));

        if(Link_Num == 0)
        {
            sprintf((char *)AT_send_str, (char *)M50_Send_TCP, GPRS_infoWr_Tx);
            M50_Mainlink_send_over = 1; // ����·������Ϣ
        }
        else
        {
            sprintf((char *)AT_send_str, (char *)M50_Send_ISP, GPRS_infoWr_Tx);
        }


        rt_hw_gsm_output(AT_send_str);
        delay_ms(100);// ��ģ�鷵�����ʱ��

        if((Photo_sdState.photo_sending == 0) && (GB19056.workstate == 0)) // ����ʱ����������Ϣ
        {
            rt_kprintf(AT_send_str);
            OutPrint_HEX("GsmSend", DataStr, GPRS_infoWr_Tx);
        }
        rt_hw_gsm_output_Data(DataStr, GPRS_infoWr_Tx);
        // rt_hw_gsm_putc (0x1A);
        WatchDog_Feed();
        rt_thread_delay(RT_TICK_PER_SECOND / 10); //DF_delay_ms(100);

    }
    else
#endif
    {
        //  4.1  ����Ҫ���͵���Ϣ����
        memset(GSM_AsciiTx, 0, sizeof(GSM_AsciiTx));
        if(Link_Num == 0)
        {
            strcat((char *)GSM_AsciiTx, Send_TCP);      // head
            packet_len = strlen((const char *)Send_TCP);
        }
        else
        {
            strcat((char *)GSM_AsciiTx, Send_ISP);      // head
            packet_len = strlen((const char *)Send_ISP);
        }
        //  infomation
        WatchDog_Feed();
        GSM_AsciiTx_len = GSM_HextoAscii_Convert( DataStr, Datalen, GSM_AsciiTx + packet_len);
        packet_len += GSM_AsciiTx_len;
        strcat((char *)GSM_AsciiTx, "\"\r\n");  // tail
        WatchDog_Feed();
        packet_len += 3;

        // 4.2 ������Ϣ����1
        //   if(DispContent==2)
        if((Photo_sdState.photo_sending == 0) && (GB19056.workstate == 0)) // ����ʱ����������Ϣ
        {
            for(i = 0; i < packet_len; i++)
                rt_kprintf("%c", GSM_AsciiTx[i]);
        }
        rt_hw_gsm_output_Data(GSM_AsciiTx, packet_len);
        WatchDog_Feed();
        rt_thread_delay(RT_TICK_PER_SECOND / 10); //DF_delay_ms(100);
    }
}

void End_Datalink(void)
{
    if(1 == DataLink_EndFlag)
    {
        TCP2_Connect = 0;
        DataLink_Online = disable; // off line

        DataLink_EndCounter++;
        if(DataLink_EndCounter == 2)
        {
            //rt_kprintf(" Ready to escape gprs \r\n");
#ifdef M50_GSM
            if(Module_Type)
                rt_hw_gsm_output(M50_CutDataLnk_str2);
            else
#endif
                rt_hw_gsm_output(CutDataLnk_str2);
            if(GB19056.workstate == 0)
                rt_kprintf(CutDataLnk_str2);
            DataDial.Dial_step_Retry = 0;
            DataDial.Dial_step_RetryTimer = Dial_Dial_Retry_Time;
        }
        else if(DataLink_EndCounter == 3)
        {
#ifdef M50_GSM
            if(Module_Type)
                rt_hw_gsm_output("AT+QICLOSE=2\r\n");
            else
#endif
                rt_hw_gsm_output(CutDataLnk_str3);	// �رյ�Internet
            //  rt_kprintf(CutDataLnk_str3);	// �رյ�Internet
            DataDial.Dial_step_Retry = 0;
            DataDial.Dial_step_RetryTimer = Dial_Dial_Retry_Time;

        }
        else if(DataLink_EndCounter == 5)
        {
#ifdef M50_GSM
            if(Module_Type)
                rt_hw_gsm_output("AT+QICLOSE=5\r\n");
            else
#endif
                rt_hw_gsm_output(CutDataLnk_str4);
            //rt_kprintf(CutDataLnk_str4);
            DataDial.Dial_step_Retry = 0;
            DataDial.Dial_step_RetryTimer = Dial_Dial_Retry_Time;
        }
        if(DataLink_EndCounter == 8)
        {
            rt_hw_gsm_output(CutDataLnk_str5);
            if(GB19056.workstate == 0)
                rt_kprintf(CutDataLnk_str5);
            DataDial.Dial_step_Retry = 0;
            DataDial.Dial_step_RetryTimer = Dial_Dial_Retry_Time;
        }
        else if(DataLink_EndCounter > 10)
        {
            DataLink_EndCounter = 0;
            DataLink_EndFlag = 0;
            //-----------------------------
            CommAT.Initial_step = 0;
            CommAT.Total_initial = 0;
            Redial_Init();
            ModuleStatus &= ~Status_GPRS;
            if(GB19056.workstate == 0)
                rt_kprintf(" Redial Start\r\n");
            //-----------------------------
        }
    }

}

void  ISP_Timer(void)
{
    if((BD_ISP.ISP_running == 1) && DataLink_Status())
    {
        BD_ISP.ISP_runTimer++;
        if(BD_ISP.ISP_runTimer > 350)
        {
            BD_ISP.ISP_runTimer = 0;
            BD_ISP.ISP_running = 0; //  clear
            SD_ACKflag.f_BD_ISPResualt_0108H = 2;
            rt_kprintf("\r\n �����·���ʱ������ʧ��\r\n");

        }
    }

}

u8  GPRS_GSM_PowerON(void)
{
    /*
              EM310���ػ�����
              powerkey ���� 50ms ����  Ȼ��������50ms�ػ�
    */
    if(GSM_PWR.GSM_power_over)
        return 1;

    GSM_PWR.GSM_powerCounter += 50;
#if 0
    if((GSM_PWR.GSM_powerCounter >= 10) && (GSM_PWR.GSM_powerCounter < 300))
    {
        GPIO_ResetBits(GPIOD, GPRS_GSM_Power);   // �ص�
        GPIO_ResetBits(GPIOD, GPRS_GSM_PWKEY);     //  PWK ��
        // rt_kprintf(" step 1\r\n");
    }
    if((GSM_PWR.GSM_powerCounter >= 300) && (GSM_PWR.GSM_powerCounter < 400))
    {
        GPIO_SetBits(GPIOD, GPRS_GSM_Power);   //  ����
        GPIO_SetBits(GPIOD, GPRS_GSM_PWKEY);  //  PWK��
        //  rt_kprintf(" step 2\r\n");

    }
    if((GSM_PWR.GSM_powerCounter >= 400) && (GSM_PWR.GSM_powerCounter < 700))
    {
        GPIO_SetBits(GPIOD, GPRS_GSM_Power);   //  ����
        GPIO_ResetBits(GPIOD, GPRS_GSM_PWKEY);   //  PWK ��
        //rt_kprintf(" step 3\r\n");
        gps_onoff(1);  // Gps module Power on	GPS ģ�鿪��
    }
    if((GSM_PWR.GSM_powerCounter >= 700) && (GSM_PWR.GSM_powerCounter < 900))
    {
        GPIO_SetBits(GPIOD, GPRS_GSM_PWKEY);  //  PWK��
        //rt_kprintf(" step 4\r\n");
    }
    if((GSM_PWR.GSM_powerCounter >= 900) && (GSM_PWR.GSM_powerCounter < 1100))
    {
        GPIO_ResetBits(GPIOD, GPRS_GSM_PWKEY);     //  PWK ��
        //rt_kprintf(" step 5\r\n");
    }
    if(GSM_PWR.GSM_powerCounter >= 1200)
    {
        rt_kprintf("         %s", "--GPRS Power over\r\n ");
        GSM_PWR.GSM_PowerEnable = 0;
        GSM_PWR.GSM_powerCounter = 0;
        GSM_PWR.GSM_power_over = 5; // ready ati
        CSQ_Duration = 200; // ��ѯ����ӳ�

        // -----  ack  timeout    clear
        CommAT.AT_cmd_send_timeout = 0;
        CommAT.AT_cmd_sendState = 0;
        //-------add for re g
    }

#endif

#ifdef M50_GSM

    if((GSM_PWR.GSM_powerCounter >= 10) && (GSM_PWR.GSM_powerCounter < 300))
    {
        GPIO_ResetBits(GPIOD, GPRS_GSM_Power);	 // �ص�
        GPIO_ResetBits(GPIOD, GPRS_GSM_PWKEY);		//	PWK ��

    }
    if((GSM_PWR.GSM_powerCounter >= 300) && (GSM_PWR.GSM_powerCounter < 400))
    {
        GPIO_SetBits(GPIOD, GPRS_GSM_Power);	//	����
        GPIO_SetBits(GPIOD, GPRS_GSM_PWKEY);  //  PWK��
        gps_onoff(1);  // Gps module Power on   GPS ģ�鿪��
    }
    if((GSM_PWR.GSM_powerCounter >= 400) && (GSM_PWR.GSM_powerCounter < 700))
    {
        GPIO_SetBits(GPIOD, GPRS_GSM_Power);   //  ����
        GPIO_SetBits(GPIOD, GPRS_GSM_PWKEY);  //  PWK��
    }
    if((GSM_PWR.GSM_powerCounter >= 700) && (GSM_PWR.GSM_powerCounter < 800))
    {
        GPIO_ResetBits(GPIOD, GPRS_GSM_PWKEY);		//	PWK ��
    }
    if(GSM_PWR.GSM_powerCounter >= 900)
    {
        rt_kprintf("		  %s", "--GPRS Power over\r\n ");
        GSM_PWR.GSM_PowerEnable = 0;
        GSM_PWR.GSM_powerCounter = 0;
        GSM_PWR.GSM_power_over = 5;
        //-------add for re g
    }



#endif
    return   0;
}

void Moudule_Init_Select(u8 *dest, flash char *M50, flash char *M69)
{
#ifdef M50_GSM
    if(Module_Type)
        memcpy(dest, M50, strlen(M50));
    else
#endif
        memcpy(dest, M69, strlen(M69));
    rt_hw_gsm_output((const char *)dest);
}

void GPRS_GSM_PowerOFF_Working(void)
{
    if(GSM_PWR.GSM_power_over != 3)
        return ;

    GSM_PWR.GSM_powerCounter++;

    if(GSM_PWR.GSM_powerCounter <= 3)
    {
        GPIO_SetBits(GPIOD, GPRS_GSM_Power);   //  ����
        GPIO_SetBits(GPIOD, GPRS_GSM_PWKEY);     //  PWK ��
        //	   rt_kprintf("\r\n �ص� --�� 300ms\r\n");
    }
    if((GSM_PWR.GSM_powerCounter > 3) && (GSM_PWR.GSM_powerCounter <= 20))
    {
        GPIO_SetBits(GPIOD, GPRS_GSM_Power);   //  ����
        GPIO_ResetBits(GPIOD, GPRS_GSM_PWKEY);     //  PWK ��
        //  rt_kprintf("\r\n �ص� --��\r\n");
    }

    if(GSM_PWR.GSM_powerCounter > 20)
    {
        GSM_PWR.GSM_powerCounter = 0;
        GSM_PWR.GSM_power_over = 0;
        DataLink_Online = 0;        // �Ͽ�����
        ModuleStatus &= ~Status_GPRS;
        rt_kprintf("\r\n ��ģ�����ת��  ����ģʽ\r\n");
    }

}

void GSM_Module_TotalInitial(u8 Invalue)
{
    u8 cmd_str[50];

    //----------  Total_initial ------------
    if(CommAT.Total_initial == 1)
    {
        if( CommAT.Execute_enable)
        {
            memset(cmd_str, 0, sizeof(cmd_str));
            switch(CommAT.Initial_step)
            {
            case 0:
                Moudule_Init_Select(cmd_str, M50_CommAT_Str1, CommAT_Str1);
                //rt_hw_gsm_output((const char*)cmd_str);
                CommAT.Initial_step++;
                break;
            case 1:
                Moudule_Init_Select(cmd_str, M50_CommAT_Str2, CommAT_Str2); 
                //rt_hw_gsm_output((const char*)cmd_str);
                CommAT.Initial_step++;
                break;

            case 2:/* Query Operator */

                memcpy(cmd_str, CommAT_Str3, strlen(CommAT_Str3));  //  ͨ������ 
                rt_hw_gsm_output((const char *)cmd_str);
                CommAT.Initial_step++; 
                break;
            case 3:
                Moudule_Init_Select(cmd_str, M50_CommAT_Str4, CommAT_Str4);
                // rt_hw_gsm_output((const char*)cmd_str);
                CommAT.Initial_step++;
                break;
            case 4:
                Moudule_Init_Select(cmd_str, M50_CommAT_Str5, CommAT_Str5);
                // rt_hw_gsm_output((const char*)cmd_str);
                CommAT.Initial_step++;
                break;
            case 5:
                Moudule_Init_Select(cmd_str, M50_CommAT_Str8, CommAT_Str8);
                rt_hw_gsm_output((const char *)cmd_str);
                CommAT.Initial_step++;
                break;
            case 6:
                Moudule_Init_Select(cmd_str, M50_CommAT_Str7, CommAT_Str7);
                //rt_hw_gsm_output((const char*)cmd_str);
                CommAT.Initial_step++;
                break;
            case 7:
                Moudule_Init_Select(cmd_str, M50_CommAT_Str11, CommAT_Str11);
                // rt_hw_gsm_output((const char*)cmd_str);
                CommAT.Initial_step++;
                break;
            case 8:
                Moudule_Init_Select(cmd_str, M50_CommAT_Str9, CommAT_Str9);
                // rt_hw_gsm_output((const char*)cmd_str);
                CommAT.Initial_step++;
                break;
            case 9:
                Moudule_Init_Select(cmd_str, CommAT_Str16, CommAT_Str10);
                //  rt_hw_gsm_output((const char*)cmd_str);
                CommAT.Initial_step++;
                break;
            case 10:
                //  ��������¼������
                Moudule_Init_Select(cmd_str, M50_CommAT_Str6, VoiceRec_config);
                // rt_hw_gsm_output((const char*)cmd_str);
                CommAT.Initial_step++;
                break;
            case 11:
                Moudule_Init_Select(cmd_str, M50_CommAT_Str12, CommAT_Str12);
                //rt_hw_gsm_output((const char*)cmd_str);
                CommAT.Initial_step++;
                break;
            case 12:
                Moudule_Init_Select(cmd_str, M50_CommAT_Str13, CommAT_Str13);
                // rt_hw_gsm_output((const char*)cmd_str);
                CommAT.Initial_step++;
                break;
            case 13:
                Moudule_Init_Select(cmd_str, M50_CommAT_Str14, CommAT_Str14);
                // rt_hw_gsm_output((const char*)cmd_str);
                CommAT.Initial_step++;
                break;
            case 14:
                memcpy(cmd_str, CommAT_Str15, strlen(CommAT_Str15));
                /// rt_hw_gsm_output((const char*)cmd_str);
                CommAT.Initial_step++;
                break;
            case 15:
                Moudule_Init_Select(cmd_str, M50_CommAT_Str1, CommAT_Str16);
                //  rt_hw_gsm_output((const char*)cmd_str);
                CommAT.Initial_step++;
                break;
            case 16:
                memcpy(cmd_str, Signal_Intensity_str, strlen(Signal_Intensity_str));
                rt_hw_gsm_output((const char *)cmd_str);
                break;
            case 17://  CCID /
                Moudule_Init_Select(cmd_str, M50_CommAT_Str1, CommAT_Str17);
                // rt_hw_gsm_output((const char*)cmd_str);

                    CommAT.Initial_step++;
                break;
            case 18://  �ź�ǿ�� /

                Moudule_Init_Select(cmd_str, M50_CommAT_Str1, CommAT_Str16); 
                // rt_hw_gsm_output((const char*)cmd_str);
                CommAT.Initial_step++;
                break;
            case 19:
                Moudule_Init_Select(cmd_str, M50_CommAT_Str1, CommAT_Str18);
                //  rt_hw_gsm_output((const char*)cmd_str);
                CommAT.Initial_step++;
                break;
            case 20:
            case 21:
            case 22:
                CommAT.Initial_step++;
                break;
            case 23:/*��ʼ�ܲ���*/
                //if((DispContent) && (GB19056.workstate == 0))
                  //  rt_kprintf("AT_Start\r\n");
                CommAT.Initial_step = 0;
                CommAT.Total_initial = 0;

                //DataDial.Dial_ON = enable; //  ����  Data   ״̬
                //DataDial.Pre_Dial_flag = 1;  // Convert to  DataDial State
                //Dial_Stage(Dial_DialInit0); 
				rt_kprintf("\r\n  ��Ϣ��ʼ��OK!  \r\n");
				// ����˵��:
				//-----------------------------------------
				rt_kprintf("\r\n                      -----------------------------------------------------------------------------\r\n");
				rt_kprintf("\r\n                      ����˵��:");				
				rt_kprintf("\r\n                       1. ʹ��ǰҪ����Ŀ���ֻ����� ");
				rt_kprintf("\r\n                            ��ʽΪ: sms_dest(\"11λ�ֻ�����\")");                
				rt_kprintf("\r\n                       2. ��д������Ϣ ");
				rt_kprintf("\r\n                            ��ʽΪ: txt_sms(\"С�ڵ���70���ַ�������\")");				
				rt_kprintf("\r\n                           ���ͳɹ���������ʾ���ͳɹ���");
				rt_kprintf("\r\n                       3. �ն��յ����Ż���ʾ������ʾ�����Ϣ");
			     rt_kprintf("\r\n                      -----------------------------------------------------------------------------\r\n");
				//------------------------------------------
                break;
            default:
                break;

            }
            if((DispContent) && (GB19056.workstate == 0) && (CommAT.Initial_step < 20))
                rt_kprintf(cmd_str);
            CommAT.Execute_enable = 0;
        }
    }

}

void Redial_Init(void)
{
    //----------- Data Dial ----------------
    memset((u8 *)&DataDial, 0, sizeof(DataDial));
    DataDial.start_dial_stateFLAG = 1; //important
    DataDial.Dial_ON = 0; //  ����  Data   ״̬
    //DataDial.Pre_Dial_flag = 1;  // Convert to  DataDial State
    Dial_Stage(Dial_DialInit0);
}

void  Dial_step_Single_10ms_timer(void)
{
    //    û������״̬�£��� �ɹ���½�Ϻ󲻽��д���
    if( (DataDial.Dial_ON == 0) || (DataDial.Dial_step == Dial_Idle) )
        return;

    if (DataDial.Dial_step_RetryTimer >= 3)
        DataDial.Dial_step_RetryTimer -= 3;
    else
        DataDial.Dial_step_RetryTimer = 0;
}

void  Get_GSM_HexData(u8  *Src_str, u16 Src_infolen, u8 link_num)
{
    // u16   i=0;
    //  1.  Check wether   Instr   is  need   to  convert     Exam:  ASCII_2_HEX
    GSM_HEX_len = GSM_AsciitoHEX_Convert(Src_str, Src_infolen, GSM_HEX);
    //  2 .  Realse   sem
    Receive_DataFlag = 1;
}
void Moudule_DialStr_Select(u8 *dest, flash char *M50, flash char *M69)
{
#ifdef M50_GSM
    if(Module_Type)
        memcpy(dest, M50, strlen(M50));
    else
#endif
        memcpy(dest, M69, strlen(M69));
}

void DataLink_Process(void)
{
    u8  len = 0, i = 0;
    u8  cmd_str[50];

    // state  filter  1:   û������״̬�£��� �ɹ���½�Ϻ󲻽��д���
    if( (DataDial.Dial_ON == 0) || (DataDial.Dial_step == Dial_Idle) )
        return;
    // state  filter  2:  	������ʱ��û�е�
    if (DataDial.Dial_step_RetryTimer )
        return;						   // not time to retry
    // state  filter  3:
    if (DataDial.Dial_step_Retry >= Dial_Step_MAXRetries)
    {

        if((DataDial.Dial_step == Dial_DNSR1) || (DataDial.Dial_step == Dial_DNSR2))
        {
            DataDial.Dial_step++;
        }
        else
        {
            //---------------------------------------
            DataDial.Connect_counter++;
            if( DataDial.Connect_counter > 4)
            {
                //DataDial.Pre_Dial_flag = 1; 	 //--- ���²���
                DataDial.Pre_dial_counter = 0;
                //rt_kprintf("\r\n  RetryDialcounter>=4 ���²���\r\n");
            }
            Dial_Stage(Dial_DialInit0);    // Clear   from  Dial  start
        }
        DataDial.Dial_step_Retry = 0;
        DataDial.Dial_step_RetryTimer = 0;
        //  rt_kprintf("\r\nDataDial.Dial_step_Retry>= Dial_Step_MAXRetries ,redial \r\n");
        return;
    }
    //----------  ����������2�� ʧ���л���������
    if((DataDial.Dial_step == Dial_MainLnk) && (DataDial.Dial_step_Retry > 2))
    {
        i = 0;
        if(Dnsr_state == 0)
        {
            // rt_thread_delay(10);
#ifdef M50_GSM
            if(Module_Type)
                rt_hw_gsm_output(M50_CutDataLnk_str2);
            else
#endif
                rt_hw_gsm_output("AT%IPCLOSE=1\r\n");
            WatchDog_Feed();
            delay_ms(100);//rt_thread_delay(10);
            Dial_Stage(Dial_AuxLnk);
            DataLink_AuxSocket_set(RemoteIP_aux, RemotePort_main, 0);

        }
    }



    //  work  on
    if(DataDial.Dial_ON)
    {
        switch(DataDial.Dial_step)
        {
        case Dial_DialInit0:
#ifdef M50_GSM
            if(Module_Type)
                rt_hw_gsm_output(M50_DialInit4);
            else
#endif
                rt_hw_gsm_output(DialInit1);
            //-----------------------------------------
            DataDial.start_dial_stateFLAG = 1;
            //-----------------------------------------
            DataDial.Dial_step_RetryTimer = Dial_Dial_Retry_Time;
            DataDial.Dial_step_Retry++;
            //  Debug

            memset(cmd_str, 0, sizeof(cmd_str));
            Moudule_DialStr_Select(cmd_str, M50_DialInit4, DialInit1);
            len = strlen((const char *)cmd_str);
            if(GB19056.workstate == 0)
            {
                for(i = 0; i < len; i++)
                    rt_kprintf("%c", cmd_str[i]);
            }
            break;
        case Dial_DialInit1:
#ifdef M50_GSM
            if(Module_Type)
                rt_hw_gsm_output(M50_DialInit1);
            else
#endif
                rt_hw_gsm_output((const char *)Dialinit_APN);
            DataDial.Dial_step_RetryTimer = Dial_max_Timeout;
            DataDial.Dial_step_Retry++;

            memset(cmd_str, 0, sizeof(cmd_str));
            Moudule_DialStr_Select(cmd_str, M50_DialInit1, Dialinit_APN);
            len = strlen((const char *)cmd_str);
            if(GB19056.workstate == 0)
            {
                for(i = 0; i < len; i++)
                    rt_kprintf("%c", cmd_str[i]);
            }
            break;
        case Dial_DialInit2:
#ifdef M50_GSM
            if(Module_Type)
                rt_hw_gsm_output(Dialinit_APN);
            else
#endif
                rt_hw_gsm_output(DialInit2);
            DataDial.Dial_step_RetryTimer = Dial_Timeout;
            DataDial.Dial_step_Retry++;

            memset(cmd_str, 0, sizeof(cmd_str));

            Moudule_DialStr_Select(cmd_str, Dialinit_APN, DialInit2);
            len = strlen((const char *)cmd_str);
            if(GB19056.workstate == 0)
            {
                for(i = 0; i < len; i++)
                    rt_kprintf("%c", cmd_str[i]);
            }
            break;
        case  Dial_DialInit3:
#ifdef M50_GSM
            if(Module_Type)
                rt_hw_gsm_output(M50_DialInit2);
            else
#endif
                rt_hw_gsm_output(DialInit3);
            DataDial.Dial_step_RetryTimer = Dial_max_Timeout;
            DataDial.Dial_step_Retry++;

            memset(cmd_str, 0, sizeof(cmd_str));

            Moudule_DialStr_Select(cmd_str, M50_DialInit2, DialInit3);
            len = strlen((const char *)cmd_str);
            if(GB19056.workstate == 0)
            {
                for(i = 0; i < len; i++)
                    rt_kprintf("%c", cmd_str[i]);
            }

            break;
        case Dial_DialInit4:
#ifdef M50_GSM
            if(Module_Type)
                rt_hw_gsm_output(M50_DialInit3);
            else
#endif
                rt_hw_gsm_output(DialInit6); // AT
            DataDial.Dial_step_RetryTimer = Dial_Dial_Retry_Time;
            DataDial.Dial_step_Retry++;

            memset(cmd_str, 0, sizeof(cmd_str));

            Moudule_DialStr_Select(cmd_str, M50_DialInit3, DialInit6);
            len = strlen((const char *)cmd_str);

            if(GB19056.workstate == 0)
            {
                for(i = 0; i < len; i++)
                    rt_kprintf("%c", cmd_str[i]);
            }
            break;

        case Dial_DialInit5:
#ifdef M50_GSM
            if(Module_Type)
                rt_hw_gsm_output(DialInit1);
            else
#endif
                rt_hw_gsm_output(DialInit4);
            DataDial.Dial_step_RetryTimer = Dial_Dial_Retry_Time;
            DataDial.Dial_step_Retry++;
            memset(cmd_str, 0, sizeof(cmd_str));
            Moudule_DialStr_Select(cmd_str, DialInit1, DialInit4);
            len = strlen((const char *)cmd_str);
            if(GB19056.workstate == 0)
            {
                for(i = 0; i < len; i++)
                    rt_kprintf("%c", cmd_str[i]);
            }

            break;
        case Dial_DialInit6:
#ifdef M50_GSM
            if(Module_Type)
                rt_hw_gsm_output(M50_CommAT_Str1);
            else
#endif
                rt_hw_gsm_output(DialInit8);  // Query  IP
            DataDial.Dial_step_RetryTimer = Dial_Dial_Retry_Time;
            DataDial.Dial_step_Retry++;
            memset(cmd_str, 0, sizeof(cmd_str));

            Moudule_DialStr_Select(cmd_str, M50_CommAT_Str1, DialInit8);
            len = strlen((const char *)cmd_str);
            if(GB19056.workstate == 0)
            {
                for(i = 0; i < len; i++)
                    rt_kprintf("%c", cmd_str[i]);
            }
            break;
        case Dial_DNSR1:
#ifdef M50_GSM
            if(Module_Type)
                rt_hw_gsm_output(M50_Dialinit_DNSR);
            else
#endif
                rt_hw_gsm_output(Dialinit_DNSR);  // main DNSR
            KorH_check();
            DataDial.Dial_step_RetryTimer = Dial_max_Timeout;
            DataDial.Dial_step_Retry++;
            if(GB19056.workstate == 0)
                rt_kprintf("\r\n   ----- Link  main  DNSR ----\r\n");
            memset(cmd_str, 0, sizeof(cmd_str));

            Moudule_DialStr_Select(cmd_str, M50_Dialinit_DNSR, Dialinit_DNSR);
            len = strlen((const char *)cmd_str);
            if(GB19056.workstate == 0)
            {
                for(i = 0; i < len; i++)
                    rt_kprintf("%c", cmd_str[i]);
            }
            Dial_jump_State = 7;
            Dnsr_state = 1; //��ʾDNSR ״̬�²���
            break;
        case Dial_DNSR2:
#ifdef M50_GSM
            if(Module_Type)
                rt_hw_gsm_output(M50_Dialinit_DNSR2);
            else
#endif
                rt_hw_gsm_output(Dialinit_DNSR2);  // Aux DNSR
            DataDial.Dial_step_RetryTimer = Dial_max_Timeout;
            DataDial.Dial_step_Retry++;
            if(GB19056.workstate == 0)
                rt_kprintf("\r\n   ----- Link  Aux  DNSR ----\r\n");
            memset(cmd_str, 0, sizeof(cmd_str));
            Moudule_DialStr_Select(cmd_str, M50_Dialinit_DNSR2, Dialinit_DNSR2);
            len = strlen((const char *)cmd_str);
            if(GB19056.workstate == 0)
            {
                for(i = 0; i < len; i++)
                    rt_kprintf("%c", cmd_str[i]);
            }
            Dial_jump_State = 8;
            Dnsr_state = 1; //��ʾDNSR ״̬�²���
            break;
        case Dial_DialInit7:
#ifdef M50_GSM
            if(Module_Type)
                rt_hw_gsm_output(M50_CommAT_Str1);
            else
#endif
                rt_hw_gsm_output(DialInit8);  // Query  IP
            DataDial.Dial_step_RetryTimer = Dial_Dial_Retry_Time;
            DataDial.Dial_step_Retry++;
            memset(cmd_str, 0, sizeof(cmd_str));

            Moudule_DialStr_Select(cmd_str, M50_CommAT_Str1, DialInit8);
            len = strlen((const char *)cmd_str);
            if(GB19056.workstate == 0)
            {
                for(i = 0; i < len; i++)
                    rt_kprintf("%c", cmd_str[i]);
            }
            break;
        case Dial_MainLnk:   // rt_hw_gsm_output(DialStr_Link1);
            if( Dnsr_state == 0) //��ʾDNSR ״̬�²���
            {
                if(GB19056.workstate == 0)
                    rt_kprintf("\r\n Dial  orginal   Mainlink\r\n");
                DataLink_MainSocket_set(RemoteIP_main, RemotePort_main, 0);
            }

            DataDial.Dial_step_RetryTimer = 800;
            DataDial.Dial_step_Retry++;


            memset(cmd_str, 0, sizeof(cmd_str));

            Moudule_DialStr_Select(cmd_str, M50_DialStr_Link1, DialStr_Link1);
            len = strlen((const char *)cmd_str);
            for(i = 0; i < len; i++)
            {
                rt_hw_gsm_putc (cmd_str[i]);
            }
            if(GB19056.workstate == 0)
            {
                for(i = 0; i < len; i++)
                {
                    rt_kprintf("%c", cmd_str[i]);
                }
            }
            break;
        case Dial_AuxLnk:   // rt_hw_gsm_output(DialStr_IC_card);
            DataDial.Dial_step_RetryTimer = 1000;
            DataDial.Dial_step_Retry++;

            memset(cmd_str, 0, sizeof(cmd_str));

            Moudule_DialStr_Select(cmd_str, M50_DialStr_LinkAux, DialStr_LinkAux);
            len = strlen((const char *)cmd_str);
            for(i = 0; i < len; i++)
            {
                rt_hw_gsm_putc (cmd_str[i]);
            }
            if(GB19056.workstate == 0)
            {
                for(i = 0; i < len; i++)
                {
                    rt_kprintf("%c", cmd_str[i]);
                }
            }
            break;
        case   Dial_ISP:
            DataDial.Dial_step_RetryTimer = Dial_max_Timeout;
            DataDial.Dial_step_Retry++;

            memset(cmd_str, 0, sizeof(cmd_str));

            Moudule_DialStr_Select(cmd_str, M50_DialStr_IC_card, DialStr_IC_card);
            len = strlen((const char *)cmd_str);
            for(i = 0; i < len; i++)
            {
                rt_hw_gsm_putc (cmd_str[i]);
            }
            rt_kprintf("\r\n	----- Link	IC ----\r\n");
            for(i = 0; i < len; i++)
            {
                rt_kprintf("%c", cmd_str[i]);
            }
            break;
        default:
            break;
        }

    }



}

static void GSM_Process(u8 *instr, u16 len)
{
    u8	ok = false, dnsrok = false;
    u8	error = false;
    u8	failed = false;
    u8	connect = false, connect_2 = false;
    u16  i = 0, j = 0, q = 0; //,len=0;//j=0;
    u8 reg_str[80];
    //----------------------  Debug -------------------------
    // if(DispContent==2)
    memset(GSM_rx, 0, sizeof((const char *)GSM_rx));
    memcpy(GSM_rx, instr, len);

    if(GB19056.workstate==3)
    {
        rt_kprintf("\r\n");
        for(i = 0; i < len; i++)
            rt_kprintf("%c", GSM_rx[i]);
    }

    // -----  ack  timeout    clear
    CommAT.AT_cmd_send_timeout = 0;
    CommAT.AT_cmd_sendState = 0;

    //-------------------------------------------------------------------------------------------------------------------

#ifdef M50_GSM
    if (strncmp((char *)GSM_rx, "Quectel_M50", 11) == 0)
    {
        /*
        	 ģ�����������ˣ���Щ״̬Ҫ��ԭ����Ӧ״̬ ��-- ��Ҫ������
         */
        Module_Type = M50_GSM;
        GSM_PWR.GSM_PowerEnable = 0;
        GSM_PWR.GSM_powerCounter = 0;
        GSM_PWR.GSM_power_over = 1;


        DataLink_APN_Set(APN_String, 1);  // apn
        DataLink_DNSR_Set(DomainNameStr, 0);   // DNSR	MG323  û��
        DataLink_DNSR2_Set(DomainNameStr_aux, 0);
        DataLink_MainSocket_set(RemoteIP_main, RemotePort_main, 0);

        if(GB19056.workstate )
            rt_kprintf("\r\n   ģ��������: Quectel_M50\r\n");
    }
#endif
    if (strncmp((char *)GSM_rx, "M69", 2) == 0) 
    {
        Module_Type = M69_GSM;
        GSM_PWR.GSM_PowerEnable = 0;
        GSM_PWR.GSM_powerCounter = 0;
        GSM_PWR.GSM_power_over = 1;

        DataLink_APN_Set(APN_String, 0);  // apn
        DataLink_DNSR_Set(DomainNameStr, 0);   // DNSR	MG323  û��
        DataLink_DNSR2_Set(DomainNameStr_aux, 0);
        DataLink_MainSocket_set(RemoteIP_main, RemotePort_main, 0);
        if(GB19056.workstate==2 )
            rt_kprintf("\r\n   ģ��������: M69 \r\n");
    }  
    if(strncmp((char *)GSM_rx, "+CMGS:", 6) == 0) //+CMGS: 2
    {
         rt_kprintf("\r\n����Ϣ���ͳɹ�:%s",GSM_rx+6);
    }

    if(strncmp((char *)GSM_rx, "%IPDATA:1", 9) == 0) // UDP	  �����ݹ�����
    {
        //exam:	%IPDATA:1,16,"6473616466617365"
        for(i = 0; i < 20; i++) //  ��ǰ20���ֽ����ҵ�һ��"
        {
            if(GSM_rx[i] == '"')
                break;
        }
        info_len = len - i - 5;
        WatchDog_Feed();
        Get_GSM_HexData(GSM_rx + i + 1, info_len, 0);
        goto RXOVER;
    }
    if((strncmp((char *)GSM_rx, "%TTS: 0", 7) == 0) || (strncmp((char *)GSM_rx, "+QTTS:0", 7) == 0))
    {
        // M50

        TTS_Play_End();
        if(GB19056.workstate == 0)
            rt_kprintf("\r\n   TTS  �������\r\n");
        Speak_OFF;
    }
#ifdef M50_GSM
    else if(strncmp((char *)GSM_rx, "SEND OK", 7) == 0)	 // ��· 1  TCP ����OK
    {
        if(M50_Mainlink_send_over)
        {
            M50_Mainlink_send_over = 0;
        }
        // Recorder_JudgeOK();
    }
#endif
#ifdef  SMS_ENABLE
    //--------      SMS  service  related Start  -------------------------------------------
    //+CMTI: "SM",1            +CMTI: "SM",1
    if( strncmp( (char *)GSM_rx, "+CMTI: \"SM\",", 12 ) == 0 )
    {
        if(GB19056.workstate == 0)
            rt_kprintf( "\r\n�յ�����:" );
        j = sscanf( GSM_rx + 12, "%d", &i );
        if( j )
        {
            SMS_Rx_Notice(i);
        }
    }
    else if( strncmp( (char *)GSM_rx, "+CMT: ", 6 ) == 0 )
    {
        if(GSM_rx[6] == ',')		///PDUģʽ
        {
            if( RT_EOK == rt_mq_recv( &mq_GSM, (void *)&GSM_RX_BUFF, GSM_TYPEBUF_SIZE, RT_TICK_PER_SECOND ) )   //�ȴ�1000ms,ʵ���Ͼ��Ǳ䳤����ʱ,�1000ms
            {
                memset( GSM_rx, 0, sizeof( GSM_rx ) );
                memcpy( GSM_rx, GSM_RX_BUFF.gsm_content, GSM_RX_BUFF.gsm_wr );
                len = GSM_RX_BUFF.gsm_wr;

                SMS_Rx_PDU(GSM_rx, len);
            }
        }
        else					///TEXTģʽ
        {
            j	= 0;
            q	= 0;
            memset( reg_str, 0, sizeof( reg_str ) );
            for( i = 6; i < len; i++ )
            {
                if( ( j == 1 ) && ( GSM_rx[i] != '"' ) )
                {
                    reg_str[q++] = GSM_rx[i];
                }
                if( GSM_rx[i] == '"' )
                {
                    j++;
                    if(j > 1)
                        break;
                }
            }
            if( RT_EOK == rt_mq_recv( &mq_GSM, (void *)&GSM_RX_BUFF, GSM_TYPEBUF_SIZE, RT_TICK_PER_SECOND ) )   //�ȴ�1000ms,ʵ���Ͼ��Ǳ䳤����ʱ,�1000ms
            {
                memset( GSM_rx, 0, sizeof( GSM_rx ) );
                memcpy( GSM_rx, GSM_RX_BUFF.gsm_content, GSM_RX_BUFF.gsm_wr );
                len = GSM_RX_BUFF.gsm_wr;
                SMS_Rx_Text(GSM_rx, reg_str);
            }
        }

    }

#ifdef SMS_TYPE_PDU
    else if( strncmp( (char *)GSM_rx, "+CMGR:", 6 ) == 0 )
    {
        if( RT_EOK == rt_mq_recv( &mq_GSM, (void *)&GSM_RX_BUFF, GSM_TYPEBUF_SIZE, RT_TICK_PER_SECOND ) )   //�ȴ�1000ms,ʵ���Ͼ��Ǳ䳤����ʱ,�1000ms
        {
            memset( GSM_rx, 0, sizeof( GSM_rx ) );
            memcpy( GSM_rx, GSM_RX_BUFF.gsm_content, GSM_RX_BUFF.gsm_wr );
            len = GSM_RX_BUFF.gsm_wr;

            SMS_Rx_PDU(GSM_rx, len);
        }
    }
#else
    else if( strncmp( (char *)GSM_rx, "+CMGR:", 6 ) == 0 )
    {
        //+CMGR: "REC UNREAD","8613602069191", ,"13/05/16,13:05:29+35"
        // ��ȡҪ���ض�Ϣ��Ŀ�ĺ���
        j	= 0;
        q	= 0;
        memset( reg_str, 0, sizeof( reg_str ) );
        for( i = 6; i < 50; i++ )
        {
            if( ( j == 3 ) && ( GSM_rx[i] != '"' ) )
            {
                reg_str[q++] = GSM_rx[i];
            }
            if( GSM_rx[i] == '"' )
            {
                j++;
            }
        }
        if( RT_EOK == rt_mq_recv( &mq_GSM, (void *)&GSM_RX_BUFF, GSM_TYPEBUF_SIZE, RT_TICK_PER_SECOND ) )   //�ȴ�1000ms,ʵ���Ͼ��Ǳ䳤����ʱ,�1000ms
        {
            memset( GSM_rx, 0, sizeof( GSM_rx ) );
            memcpy( GSM_rx, GSM_RX_BUFF.gsm_content, GSM_RX_BUFF.gsm_wr );
            len = GSM_RX_BUFF.gsm_wr;
            SMS_Rx_Text(GSM_rx, reg_str);
        }
    }
#endif

#endif
    if(strncmp((char *)GSM_rx, "%IPSENDX:1", 10) == 0)	 // ��· 1  TCP ����OK
    {
        //exam:	%IPSENDX:1,15
        //--------��Ϣ��� ���� --------
        JT808Conf_struct.Msg_Float_ID++;
        WatchDog_Feed();
    }
    else if(strncmp((char *)GSM_rx, "%IPCLOSE: 2", 11) == 0) // ISP close
    {
        if(GB19056.workstate == 0)
        {
            rt_kprintf("\r\n");
            rt_kprintf("%s", GSM_rx);
            rt_kprintf("\r\n");
        }
    }
    else if(strncmp((char *)GSM_rx, "IPCLOSE", 5) == 0)
    {
        DataLink_Online = 0;
        ModuleStatus &= ~Status_GPRS;
        // rt_kprintf("\r\nclose  all!\r\n");
       // DataDial.Pre_Dial_flag = 1;
    }
    else if((strncmp((char *)GSM_rx, "%IPCLOSE:", 9) == 0) || (strncmp((char *)GSM_rx, "1, CLOSED", 9) == 0)) //1, CLOSED
    {
        DataLink_Online = 0;
        ModuleStatus &= ~Status_GPRS;
        if(GB19056.workstate == 0)
            rt_kprintf("\r\nTCP���ӹر�!\r\n");
        if(CallState == CallState_Idle)		 //  ��Ҫ��ͨ����ʱ�����
        {
            rt_hw_gsm_output("ATH\r\n");
            DataLink_EndFlag = 1;
            DEV_Login.Operate_enable = 1; //���¼�Ȩ
        }

    }
#ifdef M50_GSM
    if(((DataDial.Dial_step == Dial_DNSR1) || (DataDial.Dial_step == Dial_DNSR2)) && (len > 8))
    {
        /*
        if(sscanf(psrc,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]) == 4)
        	{
        	memset( pcurr_socket->ip_addr, 0, 15 );
        	memcpy( pcurr_socket->ip_addr, psrc, len );

        	rt_kprintf( "\ndns ip=%s", pcurr_socket->ip_addr );
            }
           */
        i = str2ip((char *)GSM_rx, RemoteIP_Dnsr);
        if (i <= 3)
            failed = true;
        else
        {
            //memcpy((char*)SysConf_struct.IP_Main,RemoteIP_main,4);
            // SysConf_struct.Port_main=RemotePort_main;
            //Api_Config_write(config,ID_CONF_SYS,(u8*)&SysConf_struct,sizeof(SysConf_struct));
            DataLink_MainSocket_set(RemoteIP_Dnsr, RemotePort_main, 0);
            dnsrok = true;
            if(GB19056.workstate == 0)
                rt_kprintf("\r\n   ���������ɹ�:%s\r\n", GSM_rx);

            Dial_Stage(Dial_MainLnk);  //--   �л�������
        }
    }
#endif
    if(strncmp((char *)GSM_rx, "%DNSR:", 6) == 0)
    {
        //%DNSR:113.31.28.100
        //%DNSR:0.0.0.0
        if(strncmp((char *)GSM_rx, "%DNSR:0.0.0.0", 13) == 0)
        {
            if((Dial_jump_State == 7) || (Dial_jump_State == 8))
                Dial_jump_State = 0;
            Dnsr_state = 0; //��ʾDNSR ״̬�²���

        }
        else
        {


            i = str2ip((char *)GSM_rx + 6, RemoteIP_Dnsr);
            if (i <= 3) failed = true;
            //memcpy((char*)SysConf_struct.IP_Main,RemoteIP_main,4);
            // SysConf_struct.Port_main=RemotePort_main;
            //Api_Config_write(config,ID_CONF_SYS,(u8*)&SysConf_struct,sizeof(SysConf_struct));
            DataLink_MainSocket_set(RemoteIP_Dnsr, RemotePort_main, 0);
            if(GB19056.workstate == 0)
                rt_kprintf("\r\n   ���������ɹ�\r\n");

            Dial_Stage(Dial_MainLnk);  //--   �л�������
            return ;
        }
    }
    if(strncmp((char *)GSM_rx, "+COPS: 0,0,", 11) == 0)
    {
        // M50    �ƶ��� +COPS: 0,0,"CHINA MOBILE"
        // M50   ��ͨ��  +COPS: 0,0,"CHINA UNICOM GSM"
        // ��ͨ��	      +COPS: 0,0,"CHINA UNICOM",0
        //�ƶ���	     +COPS: 0,0,"CHINA MOBILE",0
        //ע���Ŀ�	  +COPS: 0
        if(strncmp((char *)GSM_rx + 12, "CHINA UNICOM", 12) == 0)
        {
            memset(APN_String, 0, sizeof(APN_String));
            memcpy(APN_String, "UNINET", 6);
            memset((u8 *)SysConf_struct.APN_str, 0 , sizeof(SysConf_struct.APN_str));
            memcpy(SysConf_struct.APN_str, (u8 *)APN_String, strlen((const char *)APN_String));
            Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));

            //��ͨ��
            memset((char *)Dialinit_APN, 0, sizeof(Dialinit_APN));
            strcat((char *)Dialinit_APN, "AT+CGDCONT=1,\"IP\",\"UNINET\"\r\n");
        }
        else if(strncmp((char *)GSM_rx + 12, "CHINA MOBILE", 12) == 0)
        {
            memset(APN_String, 0, sizeof(APN_String));
            memcpy(APN_String, "CMNET", 5);
            memset((u8 *)SysConf_struct.APN_str, 0 , sizeof(SysConf_struct.APN_str));
            memcpy(SysConf_struct.APN_str, (u8 *)APN_String, strlen((const char *)APN_String));
            Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));

            // �ƶ���
            memset(Dialinit_APN, 0, sizeof(Dialinit_APN));
            strcat(Dialinit_APN, "AT+CGDCONT=1,\"IP\",\"CMNET\"\r\n");
        }
        CommAT.Initial_step++;
        //-------------------------------------------------------------------------


    }
    else if(strncmp((char *)GSM_rx, "+COPS: 0", 8) == 0) //----�ÿ���ע����
    {
        COPS_Couter++;
        if(COPS_Couter >= 3)
        {
            COPS_Couter = 0;
            CommAT.Initial_step++;
            if(GB19056.workstate == 0)
                rt_kprintf("\r\n  SIM��������������\r\n");
        }
    }
    else if(strncmp((char *)GSM_rx, "OK", 2) == 0)
    {
        ok = true;
        if((DispContent) && (GB19056.workstate == 0))
            rt_kprintf(" OK\r\n");
        //-------------------------------------------
#ifdef REC_VOICE_ENABLE
        VOC_REC_filedel();
#endif
        //   Online  state  OK  ,clear Error Counter
        if(DataLink_Status())
        {
            Online_error_counter = 0; // clear
        }

    }
    else if (strcmp((char *)GSM_rx, "NO DIALTONE") == 0) failed = true;
    else if (strcmp((char *)GSM_rx, "NO ANSWER") == 0) failed = true;
    if (strncmp((char *)GSM_rx, "NO CARRIER", 10) == 0)
    {
        //  NO CARRIER
        CallState = CallState_Idle;
        Speak_OFF;// �رչ���
        failed = true;
        // rt_kprintf("\r\n Callstate=Idle\r\n");
    }
    if(strcmp((char *)GSM_rx, "BUSY") == 0)
    {
        // ZHinfotype=ZH_01H;//  ��̨���ն˷�������ָʾ
        //subtype=0x12;	   //�Ҷ�
        CallState = CallState_Idle;
        failed = true;
        // Off_speaker;		// �رչ���
        //Clip_flag=0;
        //Clip_counter=0;

    }
    else if(strncmp((char *)GSM_rx, "+CSQ:", 5) == 0)	//��ǿ��Ϣ
    {
        //+CSQ: 27,99
        if ( GSM_rx[7] == ',' )
        {
            ModuleSQ = GSM_rx[6] - 0x30;
        }
        else if ( GSM_rx[8] == ',' )
        {
            ModuleSQ = ( GSM_rx[6] - 0x30 ) * 10 + GSM_rx[7] - 0x30;

            if ( ModuleSQ == 99 )
            {
                ModuleSQ = 0;
            }
        }
        //--------------------------------------
        //  You must   register on
        if(CommAT.Total_initial == 1)
        {
            if((ModuleSQ > 7) && (CommAT.Initial_step == 16))
            {
                CommAT.Initial_step++;
                if(GB19056.workstate == 0)
                    rt_kprintf("\r\n CSQ check Pass\r\n");
            }
        }

    }

    if (strncmp((char *)GSM_rx, "%TSIM", 5) == 0)	 //-----  %TSIM 1 %TSIM 1
    {
        if(GSM_rx[6] == '0')
        {
            IMSIGet.imsi_error_count++;
            if(IMSIGet.imsi_error_count > 10)
            {
                IMSIGet.Get_state = 2;
                IMSIGet.imsi_error_count = 0;
                GSM_PWR.GSM_power_over = 0;;
                IMSIGet.Get_state = 0;
                // EM310_StartFLAG=0;
                rt_kprintf("\r\nû�м�⵽SIM��\r\n");
            }
        }
        else if(GSM_rx[6] == '1')
        {
            IMSIGet.Get_state = 1;
            //rt_kprintf("\r\n  ��⵽SIM��\r\n");
        }
    }


    if (strncmp((char *)GSM_rx, "89860", 5) == 0)	 //-----  %TSIM 1 %TSIM 1
    {
        // M50
        ok = 0; // �������±�
        for(i = 0; i < 10; i++)
        {
            ProductAttribute._5_Sim_ICCID[ok] = ((GSM_rx[2 * i] - 0x30) << 4) + (GSM_rx[1 + 2 * i] - 0x30);
            ok++;
        }
        IMSIGet.Get_state = 1;
        rt_kprintf("\r\n  ��⵽SIM��\r\n");
    }
    else if (strncmp((char *)GSM_rx, "460", 3) == 0)	//----- Nathan Add
    {

        CommAT.Total_initial = 1;	// ���в�������
        //---  add for Test  -----
        if(GB19056.workstate == 0)
            rt_kprintf("\r\n��ȡIMSI ����:%s \r\n", GSM_rx);
        memcpy((char *)IMSI_CODE, (char *)GSM_rx, 15);
        IMSIGet.imsi_error_count = 0;
        IMSIGet.Get_state = 1;
        GSM_PWR.GSM_power_over = 2;   //  get imsi
        IMSIGet.Get_state = 0;
        CommAT.Total_initial = 1;	// ���в�������
    }
    if (strncmp((char *)GSM_rx, "+CCID: \"", 8) == 0)
    {
        //+CCID: "89860109520220884603"    AT+CCID
        //  ��ȡ�ն�����  ��ICCID �ֶ�
        ok = 0; // �������±�
        for(i = 0; i < 10; i++)
        {
            ProductAttribute._5_Sim_ICCID[ok] = ((GSM_rx[8 + 2 * i] - 0x30) << 4) + (GSM_rx[9 + 2 * i] - 0x30);
            ok++;
        }
    }
    else if(strncmp((char *)GSM_rx, "RING", 4) == 0) //�绰  RING
    {
        if((JT808Conf_struct.Auto_ATA_flag == 1) && (Calling_ATA_flag == 0))
        {

            Ring_counter++;
            if(Ring_counter >= 3)
            {
                Calling_ATA_flag = 1;
                Ring_counter = 0;
            }
        }
    }
    else if ((strlen((char *)GSM_rx) >= 7) && (strncmp((char *)GSM_rx, "CONNECT FAIL", 12) == 0))
    {

        DataLink_EndFlag = 1; //AT_End();

    }
    else if ((strlen((char *)GSM_rx) >= 7) && (strncmp((char *)GSM_rx, "CONNECT", 7) == 0))
    {
        connect = true;
    }

    //1, CONNECT OK
    if ((strncmp((char *)GSM_rx, "1, CONNECT OK", 13) == 0))
    {
        connect = true;
    }
    else if ((strncmp((char *)GSM_rx, "2, CONNECT OK", 13) == 0))
    {
        connect_2 = true;
    }
    if((strncmp((char *)GSM_rx, "ERROR: 13", 9) == 0) || (strncmp((char *)GSM_rx, "ERROR:13", 8) == 0) || (strncmp((char *)GSM_rx, "ERROR:35", 8) == 0)) //ERROR:13
    {
        Online_error_counter++;
        if(Online_error_counter > 3)
        {
            //  rt_kprintf("\r\n  error 13  >3  disc\r\n");
            DataLink_EndFlag = 1; //AT_End();
            Online_error_counter = 0;
        }

    }
    if(strncmp((char *)GSM_rx, "ERROR:13", 8) == 0) //ERROR:13
    {
        Online_error_counter++;
        if(Online_error_counter > 3)
        {
            // rt_kprintf("\r\n  error 13  >3  disc\r\n");
            DataLink_EndFlag = 1; //AT_End();
            Online_error_counter = 0;
        }

    }
    else  //ERROR: 14
        if((strncmp((char *)GSM_rx, "ERROR:30", 9) == 0) || (strncmp((char *)GSM_rx, "ERROR:20", 9) == 0) || (strncmp((char *)GSM_rx, "ERROR:14", 14) == 0) || (strncmp((char *)GSM_rx, "ERROR:19", 14) == 0)\
                || (strncmp((char *)GSM_rx, "ERROR:6", 8) == 0) || (strncmp((char *)GSM_rx, "ERROR:7", 7) == 0) || (strncmp((char *)GSM_rx, "ERROR:8", 8) == 0) || (strncmp((char *)GSM_rx, "ERROR:10", 9) == 0) || (strncmp((char *)GSM_rx, "ERROR:35", 8) == 0))
        {
            if (Dial_jump_State == 7)
            {
#ifdef M50_GSM
                if(Module_Type)
                    rt_hw_gsm_output(M50_CutDataLnk_str2);
                else
#endif
                    rt_hw_gsm_output("AT%IPCLOSE=1\r\n");
                rt_thread_delay(10);
                Dial_Stage(Dial_DNSR1);  // �����Ծ��ready to  connect aux DNS
                // rt_kprintf("\r\n   DNSR1  ���� �ɹ�-->����ʧ��\r\n");
                Dnsr_state = 0; //��ʾDNSR ״̬�²���
                //-------------  End  need	process ---------------------------------------
                memset(GSM_rx, 0, sizeof(GSM_rx));
                GSM_rx_Wr = 0;
                return;
            }
            else if(Dial_jump_State == 8)
            {
#ifdef M50_GSM
                if(Module_Type)
                    rt_hw_gsm_output(M50_CutDataLnk_str2);
                else
#endif
                    rt_hw_gsm_output("AT%IPCLOSE=1\r\n");
                rt_thread_delay(10);
                Dial_Stage(Dial_MainLnk);  // ready to  connect mainlink
                // rt_kprintf("\r\n   DNSR2  ���� �ɹ�-->����ʧ��\r\n");
                Dial_jump_State = 0;
                Dnsr_state = 0; //��ʾDNSR ״̬�²���
                //-------------  End  need	process ---------------------------------------
                memset(GSM_rx, 0, sizeof(GSM_rx));
                GSM_rx_Wr = 0;
                return;
            }
            else if(DataDial.Dial_step == Dial_MainLnk)  //mainlink  to  auxlink
            {
                // GSM_PutStr("AT%IPCLOSE=1\r\n");
                //  AT_Stage(AT_Dial_AuxLnk);//AT_Dial_AuxLnk
                rt_thread_delay(10);
                Dial_Stage(Dial_AuxLnk);//  ready to connect auxlink

                //-----------  Below add by Nathan  ----------------------------
                DataLink_AuxSocket_set(RemoteIP_aux, RemotePort_main, 0);
                //-------------  End  need	process ---------------------------------------
                memset(GSM_rx, 0, sizeof(GSM_rx));
                GSM_rx_Wr = 0;
                return;
            }
            //--------------------------------
            else
            {
                Online_error_counter++;
                if(Online_error_counter > 3)
                {
                    DataLink_EndFlag = 1; //AT_End();
                    Online_error_counter = 0;
                }
            }
        }
        else if (strncmp((char *)GSM_rx, "ERROR", 5) == 0)
        {
            error = true;
            CallState = CallState_Idle;
            //----------------------
            if(TTS_ACK_Error_Process() == true)
            {
                rt_kprintf("\r\n  TTS ack  error \r\n");
                Speak_OFF;
            }
            else if(DataLink_Status())       //   Online  state  Error    , End Link and Redial
            {
                Online_error_counter++;
                if(Online_error_counter > 3)
                {
                    DataLink_EndFlag = 1; //AT_End();
                    Online_error_counter = 0;
                }
                // rt_kprintf("\r\n Online error\r\n");
            }
            if(GB19056.workstate == 0)
                rt_kprintf("\r\n ERROR\r\n");
        }
        else if (strncmp((char *)GSM_rx, "Unknown", 7) == 0)
            error = true;

    //-------------  End  need	process ---------------------------------------
RXOVER:
    memset(GSM_rx, 0, sizeof(GSM_rx));
    GSM_rx_Wr = 0;

    //---------------------  Function  Process Below --------------------------
    if ((DataDial.Dial_step != Dial_MainLnk) && failed)
    {
        Dial_Stage(Dial_Idle);
        return;
    }

    switch (DataDial.Dial_step)
    {
    case Dial_DialInit0:
        if (ok)  Dial_Stage(Dial_DialInit1);
        break;
    case Dial_DialInit1   :
        if (ok) Dial_Stage(Dial_DialInit2);
        break;
    case Dial_DialInit2	:
        if (ok)   Dial_Stage(Dial_DialInit3);
        break;
    case Dial_DialInit3	:
        if (ok)   Dial_Stage(Dial_DialInit4);
        break;
    case Dial_DialInit4	:
        if (ok)  Dial_Stage(Dial_DialInit5);
        break;
    case Dial_DialInit5	:
        if (ok)  Dial_Stage(Dial_DialInit6);
        break;
    case Dial_DialInit6	:
        if (ok)
            /*
                                          ��ʼ����ɺ�ѡ���״����ӵķ�ʽ
                          */
            if(Vechicle_Info.Link_Frist_Mode == 1)
                Dial_Stage(Dial_MainLnk);
            else
                Dial_Stage(Dial_DNSR1);

        /*
                 #ifdef MULTI_LINK
                       Dial_Stage(Dial_DNSR1);   // ������
        #else
              Dial_Stage(Dial_MainLnk);
              #endif
              */
        break;
    case Dial_DNSR1	:
        if (dnsrok)
        {
            //Dial_Stage(Dial_DNSR2);
            rt_kprintf("\r\n DNSR1 SEND over\r\n");
            goto LIANJIE;
        }
        break;
    case Dial_DNSR2	:
        if (dnsrok)
        {
            goto LIANJIE;
        }
        break;
    case Dial_MainLnk		:
    case Dial_AuxLnk	    :
        if (failed || error)
        {
            DataDial.Dial_step_RetryTimer = Dial_Dial_Retry_Time;				// try again in 6 seconds
            break;
        }
        if (connect)
        {
LIANJIE:        //  below  run once  since  online
            if(DataDial.Dial_step == Dial_MainLnk)   //mainlink
                rt_kprintf("\r\n���ӳɹ�TCP---\r\n");
            if(DataDial.Dial_step == Dial_AuxLnk)   //auxlink
                rt_kprintf("\r\n Aux ���ӳɹ�TCP---\r\n");
            //     1.   ��½�ɹ�����ز���
            // <--  ע��״̬
            if(1 == JT808Conf_struct.Regsiter_Status)
            {
                DEV_Login.Operate_enable = 1;
                // if(DEV_Login.Sd_counter==0)
                //   DEV_Login.Enable_sd=1;
                DEV_Login.Sd_counter = 0;
                DEV_Login.Enable_sd = 1;
            }
            else
            {
                JT808Conf_struct.Regsiter_Status = 0;
                if(DEV_regist.Sd_counter == 0)
                    DEV_regist.Enable_sd = 1;
            }

            // connect = true;
            //  -----  Data  Dial Related ------
            if((DataDial.Dial_ON) && (DataDial.Dial_step < Max_DIALSTEP))
            {
                Dial_Stage(Dial_Idle);
                DataDial.Dial_step_Retry = 0;
                DataDial.Dial_step_RetryTimer = 0;
            }
            //-----------------------------------
            //   DataDial.Dial_ON=0;
            DataLink_Online = enable;
            DialLink_TimeOut_Clear();

#ifdef	MULTI_LINK
            TCP2_ready_dial = 1;
#endif

            //--------------------------------------------------------------------->
            Dial_Stage(Dial_Idle); //  state  convert
        }
        //------------------------------------------
        break;
    case Dial_ISP:
        if (failed || error)
        {
            DataDial.Dial_step_RetryTimer = Dial_Dial_Retry_Time;
            break;
        }
        if (connect_2)
        {
            Dial_Stage(Dial_Idle); //	In EM310 Mode
            TCP2_Connect = 1;
            TCP2_sdFlag = 1;
            TCP2_ready_dial = 0;
            DataDial.Dial_ON = 0; //
        }
        break;
    default 			:
        Dial_Stage(Dial_Idle);
        DataDial.Dial_step_Retry = 0;
        break;
    }
    //-----------
}

void  IMSIcode_Get(void)
{


    if(GSM_PWR.GSM_power_over == 1)
    {

        IMSIGet.Checkcounter++;
        if(IMSIGet.Checkcounter > 20)   //  15*30=450ms
        {
            IMSIGet.Checkcounter = 0;
            if(IMSIGet.Get_state == 1)
            {

                rt_hw_gsm_output(IMSI_Check_str);    // ��ѯ IMSI_CODE ����
                if(GB19056.workstate == 0)
                    rt_kprintf(IMSI_Check_str);
            }
            else
            {
#ifdef M50_GSM
                if(Module_Type)
                {
                    rt_hw_gsm_output(M50_SIM_Check_Str);    // �ȼ�� SIM ���Ĵ���
                    if(GB19056.workstate == 0)
                        rt_kprintf(M50_SIM_Check_Str);


                }
                else
#endif
                {
                    rt_hw_gsm_output(SIM_Check_Str);    // �ȼ�� SIM ���Ĵ���
                    if(GB19056.workstate == 0)
                        rt_kprintf(SIM_Check_Str);
                }
            }
        }
    }

    if(GSM_PWR.GSM_power_over == 5)
    {
        IMSIGet.Checkcounter++;
        if(IMSIGet.Checkcounter > 20)   //  15*30=450ms
        {
            IMSIGet.Checkcounter = 0;
            rt_hw_gsm_output("ATI\r\n");    // �ȼ�� SIM ���Ĵ���
            if(GB19056.workstate == 0)
                rt_kprintf("ATI\r\n");
        }

    }

}

u8   GSM_Working_State(void)  //  ��ʾGSM ��������������
{
    if((GSM_PWR.GSM_power_over == 1) || (GSM_PWR.GSM_power_over == 2) || (GSM_PWR.GSM_power_over == 5))
        return  GSM_PWR.GSM_power_over;
    else
        return  0;
}



#if 0
void Rx_in(u8 *instr)
{
    u16  inlen = 0;

    if(strncmp(instr, "can1", 4) == 0)
    {
        Get_GSM_HexData("7E8103003B013601300001864F060000010004000001F40000010102000A0000010204000000000000010302000000000110080000000058FFD11700000111080000006458FFD017EA7E", 148, 0);
        OutPrint_HEX("ģ��1", GSM_HEX, GSM_HEX_len);

    }
    else
    {
        inlen = strlen((const char *)instr);
        Get_GSM_HexData(instr, inlen, 0);
        OutPrint_HEX("ģ��", GSM_HEX, GSM_HEX_len);
    }

}
FINSH_FUNCTION_EXPORT(Rx_in, Rx_in);
#endif

void AT(char *str)
{
    rt_hw_gsm_output((const char *)str);
    rt_hw_gsm_output("\r\n");
    rt_kprintf("%s\r\n", str);

}
FINSH_FUNCTION_EXPORT(AT, AT);



void  rt_hw_gsm_init(void)
{

    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;



    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD, ENABLE );
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_UART4, ENABLE );



    /*uart4 �ܽ�����*/

    /* Configure USART Tx as alternate function  */
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GSM_TX_PIN | GSM_RX_PIN;
    GPIO_Init( GPIOC, &GPIO_InitStructure );

    GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_UART4);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_UART4);



    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_InitStructure.USART_BaudRate = 57600;  // new M66  57600
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(UART4, &USART_InitStructure);
    /* Enable USART */
    USART_Cmd(UART4, ENABLE);
    USART_ITConfig( UART4, USART_IT_RXNE, ENABLE );


    /* -------------- GPRS  GSM	ģ�� EM310 ������� -----------------	*/
    /*
    	   STM32 Pin	 SIM300 gprs  Pin		   Remark
    		 PD12		   Power				  PE2 set 1: Power On  set	0: Poweroff
    		 PD13	   PWRKEY				   ���˷���������	PA5 set 1:Off	   0: On
    */

    /*  �ܽų�ʼ�� ����Ϊ ������� */

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);


    GPIO_InitStructure.GPIO_Pin = GPRS_GSM_Power ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPRS_GSM_PWKEY;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPRS_GSM_RST;		//-----  Reset ��̬���õ�   ����Ч
    GPIO_Init(GPIOD, &GPIO_InitStructure);


    GPIO_InitStructure.GPIO_Pin = Speak_Ctrl;		//-----  ����
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    //====================================================================
    GPIO_ResetBits(GPIOD, GPRS_GSM_RST);  // ��̬���õ� Reset
    GPIO_ResetBits(GPIOD, GPRS_GSM_Power);
    GPIO_ResetBits(GPIOD, GPRS_GSM_PWKEY);   //GPIO_SetBits(GPIOD,GPRS_GSM_PWKEY);

    Speak_OFF;//  �ر���Ƶ����

    /*
        GPIO_InitStructure.GPIO_Pin =GPIO_Pin_2;		//GPS  ��������
       GPIO_Init(GPIOD, &GPIO_InitStructure);

         GPIO_InitStructure.GPIO_Pin =GPIO_Pin_12;		//GPS  ��������
       GPIO_Init(GPIOC, &GPIO_InitStructure);
         GPIO_ResetBits(GPIOC,GPIO_Pin_12);
        GPIO_ResetBits(GPIOD,GPIO_Pin_2);

     // Speak_ON;
     */
}
