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
#include "App_moduleConfig.h"



//---------------��������-------------------------------------------
//���ĸ��ֽ�  0x81: 320x240      0x82 : 640 x480
u8   Take_photo[10] = {0x40, 0x40, 0x61, 0x81, 0x02, 0X00, 0X00, 0X02, 0X0D, 0X0A}; ; //----  ������������
u8   Fectch_photo[10] = {0x40, 0x40, 0x62, 0x81, 0x02, 0X00, 0XFF, 0XFF, 0X0D, 0X0A};; //----- ����ȡͼ����


u8    _485_CameraData_Enable = 0; // ��ͼƬ���ݹ���   1: data come  0:   no data
u8 	 _485_content[600];
u16	 _485_content_wr = 0;



static  u16	 PackageLen = 0; //��¼ÿ�ν��յ�byte��
u8  OpenDoor_StartTakeFlag = 0; // �����ؿ�ʼ����״̬ ��	��ʼ����ʱ	set   1   ���ս�����  0
u8   Opendoor_transFLAG = 0x02;	 //	���Ŵ����պ��Ƿ��ϴ���־λ


//----------- _485 rx-----
ALIGN(RT_ALIGN_SIZE)
u8    _485_dev_rx[_485_dev_SIZE];
u16  _485dev_wr = 0;




struct rt_device  Device_485;
MSG_Q_TYPE  _485_MsgQue_sruct;
_485REC_Struct 	 _485_RXstatus;




//=====================================================

void  Photo_TakeCMD_Update(u8 CameraNum)
{
    Take_photo[4] = CameraNum;  // Set take  Camra  Num
    Take_photo[5] = 0x00;
}

void  Photo_FetchCMD_Update(u8 CameraNum)
{
    Fectch_photo[4] = CameraNum; // Set  fetch  Camra  Num
    Fectch_photo[5] = 0x00;
}

void _485_delay_ms(u16 j)
{
    while(j--)
    {
        delay_us(1000);
    }

}

/* write one character to serial, must not trigger interrupt */
void rt_hw_485_putc(const char c)
{
    USART_SendData(USART2,  c);
    while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET) {}
}

void rt_hw_485_Output_Data(const char *Instr, unsigned int len)
{
    unsigned int  Info_len485 = 0;

    Info_len485 = len;
    /* empty console output */
    TX_485const;
    _485_delay_ms(1);
    //--------  add by  nathanlnw ---------
    while (Info_len485)
    {
        rt_hw_485_putc (*Instr++);
        Info_len485--;
    }
    //--------  add by  nathanlnw  --------
    _485_delay_ms(3);
    RX_485const;
}

static u8  CHKendTake_ReadyToSend(void)
{
    /*  ������·���գ�ͬʱ�����ϱ�����   */


    if(Max_CameraNum == CameraState.Camera_Number)
    {
        /*
          Taking End, Start Transfering
         */
        MultiTake.Taking = 0;	// Taking  State  Over

        //------ �жϳ��ſ��������Ƿ��ϴ�״̬ ------
        if((OpenDoor_StartTakeFlag == 1) && (Opendoor_transFLAG == 0))
        {
            MultiTake_End();	// ���ſ��ز��ϴ�
            OpenDoor_StartTakeFlag = 0;
        }
        else
            Check_MultiTakeResult_b4Trans();

        return true;
    }
    else
        return false;
}

void  OpenDoor_TakePhoto(void)
{
    //------------------------------ ���س�������״̬��� -----------------------------
    if(DoorLight_StatusGet())  // PA1
    {
        DoorOpen.currentState = 1;
        //--------------------------------------
        if( (Car_Status[2] & 0x20) == 0x00)
        {
            Car_Status[2] |= 0x20;
            PositionSD_Enable();
            Current_UDP_sd = 1;
            //rt_kprintf("\r\n  ǰ�ſ���\r\n");
        }
        Car_Status[2] |= 0x20; //  Bit(13)     Set  1  ��ʾ  ǰ�ſ�

        //--------------------------------------
    }
    else
    {
        DoorOpen.currentState = 0;
        // rt_kprintf( "\r\n   �ŵ�!\r\n ");
        //-------------------------------
        if( Car_Status[2] & 0x20)
        {
            Car_Status[2] &= ~0x20;
            PositionSD_Enable();
            Current_UDP_sd = 1;
            //rt_kprintf("\r\n  ǰ�Źر�\r\n");
        }
        Car_Status[2] &= ~0x20; //  Bit(4)     Set  0  ��ʾ  ����  0 Ӫ��״̬

        //-------------------------------
    }
    if((DoorOpen.currentState != DoorOpen.BakState) && (DataLink_Status()))
    {
        if((CameraState.status == other) && (Photo_sdState.photo_sending == 0) && (0 == MultiTake.Taking) && (0 == MultiTake.Transfering))
        {
            //----------  ��·����ͷ���� -------------
            MultiTake_Start();
            //Camera_Start(1);
            if(GB19056.workstate == 0)
                rt_kprintf( "\r\n   take!\r\n ");
        }
    }
    DoorOpen.BakState = DoorOpen.currentState; //  update state
}

void  _485_RxHandler(u8 data)
{
    //      Large   LCD   Data

    rt_interrupt_enter( );
    _485_dev_rx[_485dev_wr++] = data;


    switch(_485_RXstatus._485_receiveflag)
    {
    case  IDLE_485:
        if((0xAA != _485_dev_rx[0]) && (0x40 != _485_dev_rx[0]))
        {
            //    �ж� ��һ���ֽ��Ƿ�Ϸ�������ֱ�����
            _485dev_wr = 0;
            break;
        }
        switch (_485dev_wr)
        {
        case  2 :
            if((0x55 != _485_dev_rx[1]) && (0x40 != _485_dev_rx[1]))
                _485dev_wr = 0;
            break;
        case   3:
            //----------  Check  LCD  --------------------------
            if((0xAA == _485_dev_rx[0]) && (0x55 == _485_dev_rx[1]))
            {
                //AA 55 08 83 30 00 02 1E FF FF 00                     08 �ǳ���(����1BYTE ָ��2 BYTES  ��ַ)
                _485_RXstatus._485_RxLen = _485_dev_rx[2];
                _485_RXstatus._485_receiveflag = LARGE_LCD;
            }
            else if(0x40 != _485_dev_rx[0])
                _485dev_wr = 0;
            break;
        case    8:      //        -------  Camera  Data  -----------
            if((0x40 == _485_dev_rx[0]) && (0x40 == _485_dev_rx[1]) && (0x63 == _485_dev_rx[2]))
            {
                //  40  40  63  FF  FF  00  02           00  02  �ǳ���  (С��)
                _485_RXstatus._485_RxLen = ((u16)_485_dev_rx[6] << 8) + (u16)_485_dev_rx[5];
                //----------------------------------
                _485dev_wr = 0; //clear now  , the bytes  receiving  later  is  pic data
                _485_RXstatus._485_receiveflag = CAMERA_Related; //  return Idle
            }
            else
                _485dev_wr = 0; // clear
            break;
        }


        break;
    case  LARGE_LCD:
        if(_485dev_wr >= (_485_RXstatus._485_RxLen + 3)) //  AA  55  08
        {
            //  send msg_queue
            _485_MsgQue_sruct.info = _485_dev_rx;
            _485_MsgQue_sruct.len = _485dev_wr;
            _485dev_wr = 0; // clear
            _485_RXstatus._485_receiveflag = IDLE_485;
        }
        break;
    case   CAMERA_Related:
        if(_485dev_wr >= _485_RXstatus._485_RxLen)
        {
            memset(_485_content, 0, sizeof(_485_content));
            //--------------------------------------------------
            memcpy(_485_content, _485_dev_rx, _485_RXstatus._485_RxLen);
            _485_content_wr = _485_RXstatus._485_RxLen; // Packet info len
            _485_CameraData_Enable = 1; // ͼƬ���ݹ�����
            //---------------------------------------------------
            _485dev_wr = 0; // clear
            _485_RXstatus._485_receiveflag = IDLE_485;
        }
        break;
    default:
        _485dev_wr = 0;
        _485_RXstatus._485_receiveflag =	IDLE_485;
        break ;
    }

    rt_interrupt_leave( );
}


void  Pic_Data_Process(void)
{
    u8  tmp[40];
    u8   pic_buf[600];
    u16 i = 0;

    DF_TAKE;
    // 1.    Judge   last  package
    PackageLen = _485_content_wr;
    if(PackageLen < PageSIZE)
        CameraState.last_package = 1;
    else if((CameraState.last_package == 0) && (_485_content[510] == 0xFF) && (_485_content[511] == 0xD9))
    {
        CameraState.last_package = 1;
    }
    //  2.    Block ++
    CameraState.block_counter++;
    //   3.    Check   first   packet
    if(CameraState.create_Flag == 1)  // ����ǵ�һ���򴴽��ļ� hhmmss_x.jpg
    {
        CameraState.create_Flag = 0; // clear
        memset(tmp, 0, sizeof(tmp));
        memset(PictureName, 0, sizeof(PictureName));
        //-----------  ����ͼƬ�ļ�����  -------------
        /*
         ÿ��ͼƬռ32��page    ���е�1��page ΪͼƬ���������127��PageΪͼƬ����
         SST25 ���ٸ�������ͼƬ����
        */
        if(CameraState.Camera_Number == 1)
        {
            pic_current_page = PicStart_offset; //��ʼpage �̶�Ϊ������ʼ��ַ
            //����һ��64K����������ͼƬ�洢

            Api_DFdirectory_Delete(camera_1);

        }
        else if(CameraState.Camera_Number == 2)
        {
            pic_current_page = PicStart_offset2; //��ʼpage �̶�Ϊ������ʼ��ַ
            //����һ��64K����������ͼƬ�洢
            Api_DFdirectory_Delete(camera_2);
            ;
        }
        else if(CameraState.Camera_Number == 3)
        {
            pic_current_page = PicStart_offset3; //��ʼpage �̶�Ϊ������ʼ��ַ
            //����һ��64K����������ͼƬ�洢
            Api_DFdirectory_Delete(camera_3);
        }
        else if(CameraState.Camera_Number == 4)
        {
            pic_current_page = PicStart_offset4; //��ʼpage �̶�Ϊ������ʼ��ַ
            //����һ��64K����������ͼƬ�洢
            Api_DFdirectory_Delete(camera_4);
        }
        DF_delay_ms(150);
        WatchDog_Feed();
        pic_current_page++;  // ͼƬ���ݴ� �ڶ���page ��ʼ ��һ��Page �洢����ͼƬ����
        pic_PageIn_offset = 0; // ҳ��ƫ�����
        pic_size = 0; // ���ͼƬ��С
        //------------------------------------------
        memset(PictureName, 0, sizeof(PictureName));
        sprintf((char *)PictureName, "%d%d%d-%d.jpg", time_now.hour, time_now.min, time_now.sec, CameraState.Camera_Number);
        if(GB19056.workstate == 0)
            rt_kprintf("\r\n              ����ͼƬ����: %s \r\n           ", PictureName);
        WatchDog_Feed();


        WatchDog_Feed();
        // -----    дͼƬ���� -------
        Save_MediaIndex(0, PictureName, CameraState.Camera_Number, 0);
    }

    //  4.   ��д�洢ͼƬ��������  --------------------
    WatchDog_Feed();
    DF_WriteFlashDirect(pic_current_page, 0, _485_content, PackageLen); // дһ��һ��Page 512Bytes
    delay_ms(150);
    //  rt_kprintf(" \r\n ---- pkg=%d  \r\n",CameraState.block_counter);

    //---  read compare
    memset(pic_buf, 0, 600);
    DF_ReadFlash(pic_current_page, 0, pic_buf, PackageLen);
    delay_ms(10);
    for(i = 0; i < PackageLen; i++)
    {
        if(pic_buf[i] != _485_content[i])
        {
            if(GB19056.workstate == 0)
                rt_kprintf(" \r\n write--read Error");
            // rt_kprintf(" \r\n ----read not equal write  where i=%d  Rd[i]=%2X  WR[i]=%2X \r\n",i,pic_buf[i],_485_content[i]);
            DF_WriteFlashDirect(pic_current_page, 0, _485_content, PackageLen); // ��дһ��һ��Page 512Bytes
            delay_ms(100);
            break;
        }
    }
    pic_size += PackageLen; // ͼƬ��С�ۼ�
    pic_current_page++; //дһҳ��һ
    // pic_PageIn_offset+=PackageLen;
    //  DF_delay_ms(50);
    //   5.   ���һ�� �������ս���
    if(CameraState.last_package == 1)
    {
        memset(_485_content, 0, sizeof(_485_content));
        _485_content_wr = 0;

        //-------------  ͼƬ���ս��� ��ش���  ------------------------------------
        //  1. дͼƬ����
        if(CameraState.Camera_Number == 1)
            pic_current_page = PicStart_offset; //����ͼƬ��ʼpage
        else if(CameraState.Camera_Number == 2)
            pic_current_page = PicStart_offset2; //����ͼƬ��ʼpage
        else if(CameraState.Camera_Number == 3)
            pic_current_page = PicStart_offset3; //����ͼƬ��ʼpage
        else if(CameraState.Camera_Number == 4)
            pic_current_page = PicStart_offset4; //����ͼƬ��ʼpage
        PictureName[18] = CameraState.Camera_Number;
        memcpy(PictureName + 19, (u8 *)&pic_size, 4);
        DF_WriteFlashDirect(pic_current_page, 0, PictureName, 23);
        DF_delay_ms(8);

        //  5.1   ����ͼƬ��д��¼
        //--------------------------------------------------------------------------
        if(GB19056.workstate == 0)
            rt_kprintf("\r\n        PicSize: %d Bytes\r\n    Camera  %d   End\r\n", pic_size, CameraState.Camera_Number);
        CameraState.SingleCamra_TakeResualt_BD = 0;	  //  ��·����ͷ����
        SD_ACKflag.f_BD_CentreTakeAck_0805H = 1; //  ����������������Ӧ��
        //----------  Normal process ---------------------
        Camera_End(MultiTake.Taking);


        // 5.2   ������ɺ�����û�ж�· ��-----------Multi Take process--------------------
        if(1 == MultiTake.Taking)
        {
            switch(CameraState.Camera_Number)
            {
            case  1:
                //-------- old process---------------
                MultiTake.TakeResult[0] = Take_Success; // ��ʾ��һ·����ͷ����ʧ��
                //--------- new   process-------------
                MultiTake.Take_retry = 0;
                //-------------------------------------------
                if(CHKendTake_ReadyToSend())// ����Ƿ�������������·
                    break;
                //----------������һ·����ͷ-----------
                CameraState.Camera_Number = 2;
                //-------------------------
                Camera_Start(CameraState.Camera_Number);

                break;
            case  2:
                //-------- old process---------------
                MultiTake.TakeResult[1] = Take_Success; // ��ʾ��һ·����ͷ����ʧ��
                //--------- new   process-------------
                MultiTake.Take_retry = 0;
                //-------------------------------------------
                if(CHKendTake_ReadyToSend())// ����Ƿ�������������·
                    break;
                //----------������һ·����ͷ-----------
                CameraState.Camera_Number = 3;
                //-------------------------
                Camera_Start(CameraState.Camera_Number);

                break;
            case  3:
                //-------- old process---------------
                MultiTake.TakeResult[2] = Take_Success; // ��ʾ��һ·����ͷ����ʧ��
                //--------- new   process-------------
                MultiTake.Take_retry = 0;
                //-------------------------------------------
                if(CHKendTake_ReadyToSend())// ����Ƿ�������������·
                    break;
                //----------������һ·����ͷ-----------
                CameraState.Camera_Number = 4;
                //-------------------------
                Camera_Start(CameraState.Camera_Number);

                break;
            case  4:
                //-------- old process---------------
                MultiTake.TakeResult[3] = Take_Success; // ��ʾ��һ·����ͷ����ʧ��
                //--------- new   process-------------
                MultiTake.Take_retry = 0;

                //-------------------------------------------
                if(CHKendTake_ReadyToSend())// ����Ƿ�������������·
                    break;

                break;
            default:
                MultiTake_End();
                break;

            }

        }
        else if((0 == MultiTake.Taking) && (0 == MultiTake.Transfering))
        {
            //------ �жϳ��ſ��������Ƿ��ϴ�״̬ ------
            if((OpenDoor_StartTakeFlag == 1) && (Opendoor_transFLAG == 0))
            {
                MultiTake_End();   // ���ſ��ز��ϴ�
                OpenDoor_StartTakeFlag = 0;
            }
            else
            {
                //rt_kprintf("\r\n Single Camera !\r\n");
                if(CameraState.Camera_Take_not_trans == 0)
                    Photo_send_start(CameraState.Camera_Number);  //�ڲ��Ƕ�·���յ����������Ϳ����ϴ���
                else
                    CameraState.Camera_Take_not_trans = 0;
            }
        }
        //  ���ս���
    }
    else
    {

        //------- change state  -------
        CameraState.status = transfer;
        CameraState.OperateFlag = 0; // clear

        CameraState.TX_485const_Enable = 1; // ����485 ����
        _485_RXstatus._485_receiveflag = IDLE_485;
        memset(_485_content, 0, sizeof(_485_content));
        _485_content_wr = 0;
        //rt_kprintf("\r\n  One Packet Over!\r\n");

    }
    DF_RELEASE;
}


static rt_err_t   Device_485_init( rt_device_t dev )
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;


    //  1 . Clock

    /* Enable USART2 and GPIOA clocks */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    /* Enable USART2 clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    //   2.  GPIO
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Connect alternate function */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

    //  3.  Interrupt
    /* Enable the USART2 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    //   4.  uart  Initial
    USART_InitStructure.USART_BaudRate = 57600;  //485
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);

    /* Enable USART */
    USART_Cmd(USART2, ENABLE);
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    /* -------------- 485  ������� -----------------	*/
    /*
    	   STM32 Pin	 		   Remark
    		 PC4		  		    485_Rx_Tx ����   0:Rx    1: Tx
    		 PD0		  		    485 ��Դ	1: ON  0:OFF
    */

    /*  �ܽų�ʼ�� ����Ϊ ������� */

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

    // ------------- PD10     --------------------------------
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_8;		//--------- 485 �����õĵ�
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    Power_485CH1_OFF;  //  485  �ص�

    //------------------- PC4------------------------------
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4;				 //--------- 485const	�շ�������
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    RX_485const;
    return RT_EOK;
}

static rt_err_t Device_485_open( rt_device_t dev, rt_uint16_t oflag )
{
    return RT_EOK;
}
static rt_err_t Device_485_close( rt_device_t dev )
{
    return RT_EOK;
}
static rt_size_t Device_485_read( rt_device_t dev, rt_off_t pos, void *buff, rt_size_t count )
{

    return RT_EOK;
}
static rt_size_t Device_485_write( rt_device_t dev, rt_off_t pos, const void *buff, rt_size_t count )
{
    unsigned int  Info_len485 = 0;
    const char		*p	= (const char *)buff;


    Info_len485 = (unsigned int)count;
    /* empty console output */
    TX_485const;
    _485_delay_ms(1);
    //--------  add by  nathanlnw ---------
    while (Info_len485)
    {
        rt_hw_485_putc (*p++);
        Info_len485--;
    }
    //--------  add by  nathanlnw  --------
    _485_delay_ms(3);
    RX_485const;
    return RT_EOK;
}
static rt_err_t Device_485_control( rt_device_t dev, rt_uint8_t cmd, void *arg )
{
    return RT_EOK;
}

/* init 485 */
void _485_startup(void)
{

    Device_485.type	= RT_Device_Class_Char;
    Device_485.init	= Device_485_init;
    Device_485.open	=  Device_485_open;
    Device_485.close	=  Device_485_close;
    Device_485.read	=  Device_485_read;
    Device_485.write	=  Device_485_write;
    Device_485.control = Device_485_control;

    rt_device_register( &Device_485, "485", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE );
    rt_device_init( &Device_485 );

}


