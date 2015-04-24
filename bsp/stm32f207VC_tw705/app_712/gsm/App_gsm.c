/*
     APP_GSM.C
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
#include "App_moduleConfig.h"
#include "App_gsm.h"



/* gsm thread */

ALIGN(RT_ALIGN_SIZE)
char gsm_thread_stack[2048];
struct rt_thread gsm_thread;

struct rt_semaphore gsmRx_sem;


#ifdef RT_USING_DEVICE
struct rt_device  Device_GSM;
#endif

/* ��ʱ���Ŀ��ƿ� */
static rt_timer_t timer_gsm;


//-----  GSM  ��Ϣ����------
u8  GSM_HEX[1024];
u16  GSM_HEX_len = 0;


//----- gsm_thread  rx   app_thread  data  related -----
//static  MSG_Q_TYPE  gsm_rx_app_infoStruct;  //  gsm  ���մ� app �������ݽṹ

ALIGN(RT_ALIGN_SIZE)
uint8_t					GSM_rawinfo[GSM_RAWINFO_SIZE];
struct rt_messagequeue	mq_GSM;




/*
    Ӧ�����
*/
u8  DataLink_Online = 0; //  DataLink  ���߱�־    ��ǰ��GPRS_Online
u8  DataLink_EndFlag = 0;       // Close_DataLink
u8  DataLink_EndCounter = 0;
u8   PositionInfo_sdFlag = 0;   // ���Ͷ�λ��Ϣ��־
u8   Datalink_close = 0; //�ҶϺ��ٵ�½
u8     Current_UDP_sd = 0; // ��ʱ�ϱ� ��־λ






u8  COPS_Couter = 0;           // COPS  ���ش���

u16   CSQ_counter = 0;
u16   CSQ_Duration = 288;  //��ѯCSQ �Ķ�ʱ���
u8  CSQ_flag = 1;
u8  ModuleSQ = 0; //GSM ģ���ź�ǿ����ֵ
u8	ModuleStatus = 0;  //����״̬

u8  Light = 0;
u8   Mocule_Data_Come = 0; // ģ���յ�����

u8   Send_DataFlag = 0; // ����GSM data Flag
u8   LinkNum = 0; // ͨ����·    0    LINK 1    1   LINK
u8   Receive_DataFlag = 0; // ��������
u8   Redial_reset_save = 0; // ��������ǰ�洢����
/*
    Ӧ����غ���
*/



void   DialLink_TimeOut_Clear(void)
{
    DataDial.start_dial_stateFLAG = 0;
    DataDial.start_dial_counter = 0;
}

void   DialLink_TimeOut_Enable(void)
{
    DataDial.start_dial_stateFLAG = 1;
    DataDial.start_dial_counter = 0;
}


u8   DataLink_Status(void)
{
    if(DataLink_Online)
        return   DataLink_Online  ;
    else
        return   0;

}
void DataLinkOK_Process(void)    // �������ӳɹ��������״̬����
{

    DataLink_Online = 1;
    ModuleStatus |= Status_GPRS;
    rt_kprintf("   \r\n  -------ͨ����·�����ɹ�!-----  \r\n");
}

u8  Close_DataLink(void)
{
    DataLink_EndFlag = 1;
    return true;
}


u8   PositionSD_Enable(void)
{
    PositionInfo_sdFlag = 1;
    return true;
}

u8   PositionSD_Disable(void)
{
    PositionInfo_sdFlag = 0;
    return true;
}

u8   PositionSD_Status(void)
{
    if(PositionInfo_sdFlag)
        return   PositionInfo_sdFlag  ;
    else
        return   0;
}


u8   Stop_Communicate(void)
{
    Datalink_close = 1; //�ҶϺ��ٵ�½
    return true;
}


u8   NO_ACKtimer(void)
{
    // �ж�����Ӧ��
    //------- no  ack  process -----
    if((Send_Rdy4ok == 2) && ( ReadCycle_status == RdCycle_SdOver))
    {
        ACK_timer++;
        if( ACK_timer >= 40)
        {
            ACK_timer = 0;
            cycle_read = mangQu_read_reg; //   ��ԭread���·���
            ReadCycle_status = RdCycle_Idle;
            // rt_kprintf("\r\n MQ_true Noack rensend! \r\n");
        }

    }
    return true;
}

void    GSM_SD_MsgQueue_to_APP (void)
{
    // 1.   Rx Process
    if (Mocule_Data_Come != 1)   return;
    // 2.   send  msgqueue
    Mocule_Data_Come = 0;
    //rt_712Rule_MsgQue_Post(&gsm2app808_mq,(u8*)GSM_HEX,GSM_HEX_len);

}

void Gsm_rxAppData_SemRelease(u8 *instr, u16 inlen, u8 link_num)
{
    /* release semaphore to let finsh thread rx data */
    Send_DataFlag = 1;
    //gsm_rx_app_infoStruct.info=instr;
    //gsm_rx_app_infoStruct.len=inlen;
    //gsm_rx_app_infoStruct.link_num=link_num;
    LinkNum = link_num;
}


//=====================================================================
/*
          Thread   Initial    ,   Device      Related
*/
//=====================================================================

static rt_err_t   Device_GSM_init( rt_device_t dev )
{
    //    һ. ---------  GSM  Hardware  initial  ---------------------
    rt_hw_gsm_init();
    //    ��.   GSM ״̬�Ĵ�����ʼ��
    //    Gsm_RegisterInit();   //  init register states    ,then  it  will  power on  the module
    return RT_EOK;
}

static rt_err_t Device_GSM_open( rt_device_t dev, rt_uint16_t oflag )
{
    return RT_EOK;
}
static rt_err_t Device_GSM_close( rt_device_t dev )
{
    return RT_EOK;
}
static rt_size_t Device_GSM_read( rt_device_t dev, rt_off_t pos, void *buff, rt_size_t count )
{

    return RT_EOK;
}
static rt_size_t Device_GSM_write( rt_device_t dev, rt_off_t pos, const void *buff, rt_size_t count )
{
    //     Data_Send(GPRS_info,GPRS_infoWr_Tx);
    Data_Send((u8 *)buff, (u16)count, (u8)pos);
    return RT_EOK;
}


/*
                   Descirption               cmd
                     APN                        0x01
                     main_socket            0x02
                     aux_socket             0x03
                     isp_socket               0x04
                     DNSR1                    0x05
                     DNSR2                    0x06
                     power_on                0x11
                     at_init                     0x12
                     dial_gprs                 0x13
                     query_online            0x21
                     send_gprsdata         0x22

*/
static rt_err_t Device_GSM_control( rt_device_t dev, rt_uint8_t cmd, void *arg )
{
    /*
      SOCKET    *socket = (SOCKET *)arg;
      SOCKET   reg_socket;

      reg_socket = *socket;

      switch(cmd)
      {
      case  APN ://                        0x01
          DataLink_APN_Set((u8 *)arg, 1);
          break;
      case  main_socket://             0x02
          //SOCKET type arg
          DataLink_MainSocket_set(reg_socket.ip, reg_socket.port, 1);
          break;
      case  aux_socket://             0x03
          DataLink_AuxSocket_set(reg_socket.ip, reg_socket.port, 1);
          break;
      case isp_socket://               0x04
          DataLink_IspSocket_set(reg_socket.ip, reg_socket.port, 1);
          break;
      case DNSR1 ://                   0x05
          DataLink_DNSR_Set((u8 *)arg, 1);
          break;
      case DNSR2 ://                   0x06
          DataLink_DNSR2_Set((u8 *)arg, 1);
          break;
      case power_on ://               0x11
          if(GPRS_GSM_PowerON())
              return 	RT_EOK;    // GSM ģ�鿪�����
          else
              return	RT_EBUSY;  //�����ϵ������
      case at_init://                     0x12
          CommAT.Total_initial = 1;
          CommAT.Initial_step = 0;
          break;
      case dial_gprs ://                0x13
          // rt_kprintf("AT_Start\r\n");
          CommAT.Initial_step = 0;
          CommAT.Total_initial = 0;

          DataDial.Dial_ON = enable; //  ����  Data   ״̬
          DataDial.Pre_Dial_flag = 1;  // Convert to  DataDial State 
          break;
      case query_online://            0x21
          if(DataLink_Status())
              return RT_EOK;
          else
              return RT_ERROR;
      }
    */ 
    return RT_EOK;
}







static void gsm_thread_entry(void *parameter)
{
    u8 atd_str[25];
    rt_size_t  res = RT_ERROR;
    u32 value_15 = 0, value_realtime = 0;
    //     finsh_init(&shell->parser);
    //
    //	step 1:  Init Dataflash

    //  step 2:	 process config data
#ifdef HMI
    HMI_app_init();
#endif
    gps_init();

    //--------------------------------------
    while (1)
    {

        if(GSM_Working_State())
        {
            // 1.  after power  on    get imsi code
            IMSIcode_Get();
            //  2. after get imsi   Comm   AT  initial   start
            GSM_Module_TotalInitial(0);  //GSM_Module_TotalInitial(1);
            // 3. Receivce & Process   Communication  Module   data ----
            GSM_Buffer_Read_Process();
        }
        rt_thread_delay(20);
        if(GSM_Working_State() == 2)
        {
            DataLink_Process();
            //------------------------------------------------
            if (Send_DataFlag == 1)
            {
                res = rt_device_control(&Device_GSM, query_online, NULL);
                if(res == RT_EOK)
                    rt_device_write(&Device_GSM, LinkNum, ( const void *)GPRS_info, (rt_size_t) GPRS_infoWr_Tx);
                Send_DataFlag = 0;

            }
            //����
            if(CallState == CallState_rdytoDialLis)
            {
                CallState = CallState_Dialing;
                memset(atd_str, 0, sizeof(atd_str));
                memcpy(atd_str, "ATD", 3);
                memcpy(atd_str + 3, JT808Conf_struct.LISTEN_Num, strlen((const char *)JT808Conf_struct.LISTEN_Num));
                memcpy(atd_str + 3 + strlen((const char *)JT808Conf_struct.LISTEN_Num), ";\r\n", 3);
                rt_hw_gsm_output(atd_str);
                //rt_kprintf("\r\n����%s\r\n",atd_str);
            }
            //---------  Step timer
            //  Dial_step_Single_10ms_timer();
            //   TTS
            TTS_Data_Play();
            //   Get  CSQ value
            if(GSM_CSQ_Query() == false)
            {
                if(Calling_ATA_flag == 1)
                {
                    delay_ms(10);
                    rt_hw_gsm_output("ATA\r\n");    //����ź�ǿ��
                    if(DispContent)
                        rt_kprintf("ATA\r\n");

                    Calling_ATA_flag = 0;
                }
            }
            //   SMS  Service
            SMS_Process();
        }
    }
}



static void timeout_gsm(void   *parameter)
{
    //  1 second
    //   init  Module related
    GPRS_GSM_PowerON();
    //   Comm AT related
    //if((CommAT.Total_initial==1))
    //   CommAT.Execute_enable=1;      //  enable send   periodic

    //---  Data Link END  --------
    End_Datalink();
    //       ISP timer
    ISP_Timer();
    //       TTS timeout
    TTS_Exception_TimeLimt();
    //      Voice Record
#ifdef REC_VOICE_ENABLE
    VOC_REC_process();
#endif
    //      Dial Link process
    //DialLink_TimeOut_Process();
    //  CSQ
    GSM_CSQ_timeout();
    //  AT cmd timeout
    AT_cmd_send_TimeOUT();
#ifdef SMS_ENABLE
    //  SMS  timer
    SMS_timer();

    NO_ACKtimer(); //  ����Ӧ��timer
#endif
    //    RTC get
    time_now = Get_RTC();
}



/* init gsm */
void _gsm_startup(void)
{
    rt_err_t result;


    //    ��. RT  ��س�ʼ��
    //-------------   sem  init  ---------------------------------
    //  ����DF ��д�ź���
    rt_sem_init( &sem_DF, "sem_DF", 0, 0 );
    rt_sem_release( &sem_DF);


    rt_mq_init( &mq_GSM, "mq_GSM", &GSM_rawinfo[0], 1400 - sizeof( void * ), GSM_RAWINFO_SIZE, RT_IPC_FLAG_FIFO );

    //---------  timer_gsm ----------
    // 1. create  timer     100ms=Dur
    timer_gsm = rt_timer_create("tim_gsm", timeout_gsm, RT_NULL, 100, RT_TIMER_FLAG_PERIODIC); //| RT_TIMER_FLAG_SOFT_TIMER);
    //  2. start timer
    if(timer_gsm != RT_NULL)
        rt_timer_start(timer_gsm);


    result = rt_thread_init(&gsm_thread,
                            "GsmThrd",
                            gsm_thread_entry, RT_NULL,
                            &gsm_thread_stack[0], sizeof(gsm_thread_stack),
                            Prio_GSM, 10);

    if (result == RT_EOK)
    {
        rt_thread_startup(&gsm_thread);
    }

    Device_GSM.type	= RT_Device_Class_Char;
    Device_GSM.init	= Device_GSM_init;
    Device_GSM.open	=  Device_GSM_open;
    Device_GSM.close	=  Device_GSM_close;
    Device_GSM.read	=  Device_GSM_read;
    Device_GSM.write	=  Device_GSM_write;
    Device_GSM.control = Device_GSM_control;

    rt_device_register( &Device_GSM, "GsmDev", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE );
    rt_device_init( &Device_GSM );

}




