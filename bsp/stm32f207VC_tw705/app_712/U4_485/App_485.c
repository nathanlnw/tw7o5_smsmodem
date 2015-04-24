/*
       APP_485.C
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
#include "spi_sd.h"
#include "Usbh_conf.h"
#include <dfs_posix.h>
#include <finsh.h>



/*
                  �������
*/
_MultiTake     MultiTake;	  //  ��·����״̬λ
Camera_state   CameraState;



//---------------------------Photo Related Function --------------------------------------

void Camera_Init(void)
{
    CameraState.block_counter = 0; // clear
    CameraState.status = other;
    CameraState.create_Flag = 0;
    CameraState.Camera_Number = 1;
}
void Camera_End(u8 value)
{
    CameraState.block_counter = 0;
    CameraState.status = other;
    CameraState.camera_running = 0;
    CameraState.timeoutCounter = 0;
    CameraState.OperateFlag = 0;
    CameraState.create_Flag = 0;
    _485_RXstatus._485_receiveflag = IDLE_485;

    //--  add later---
    if(value == 0)
        Power_485CH1_OFF;   //   off  power

}

void  Camera_powerOn_timer(void)
{
    if(CameraState.camera_running == 2)
    {
        CameraState.power_on_timer++;
        if(CameraState.power_on_timer >= 4)
        {
            CameraState.camera_running = 1;
            CameraState.status = enable;
            CameraState.TX_485const_Enable = 1;
            _485_RXstatus._485_receiveflag = IDLE_485;
#ifdef LCD_5inch
            DwinLCD_work_Disable() ;  //  ֹͣ5 ����
#endif
            CameraState.power_on_timer = 0;
        }

    }

}

u8 Camera_Start(u8  CameraNum)
{
    if(1 == BD_ISP.ISP_running)  // Զ�̸��¹����в�������
        return false;

    //----------------------------------------------------
    Photo_TakeCMD_Update(CameraNum);
    //---------------------------------
    Photo_FetchCMD_Update(CameraNum);

    CameraState.create_Flag = 0;

    //-------------------------------------------------------
    //if((CameraState.camera_running==0)||(Photo_sdState.photo_sending==enable))// �ڲ����䱨��ͼƬ�������û��ͼƬ����������ִ��
    if((CameraState.camera_running == 0) && (Photo_sdState.photo_sending == 0)) // �ڲ����䱨��ͼƬ�������û��ͼƬ����������ִ��
    {
        CameraState.camera_running = 2; // enable camrea  power on
        CameraState.power_on_timer = 0;

        //----- power on  camra  -----------------
        Power_485CH1_ON;
        WatchDog_Feed();
        delay_ms(200);
        // -------------------------------------

        return true;
    }
    else
    {
        CameraState.status = other;
        return false;
    }
}

void  MultiTake_Exception(void)
{
    switch(CameraState.Camera_Number)
    {
    case  1:
        MultiTake.Take_retry++;
        if(MultiTake.Take_retry >= 3)
        {
            //-------- old process---------------
            MultiTake.TakeResult[0] = Take_Fail; // ��ʾ��һ·����ͷ����ʧ��
            //--------- new   process-------------
            MultiTake.Take_retry = 0;
            //----------������һ·����ͷ-----------
            CameraState.Camera_Number = 2;
            //-------------------------
            Camera_Start(CameraState.Camera_Number);
        }
        else
        {
            // continue operate this  camera
            Camera_Start(CameraState.Camera_Number);
        }

        break;
    case  2:
        MultiTake.Take_retry++;
        if(MultiTake.Take_retry >= 3)
        {
            //-------- old process---------------
            MultiTake.TakeResult[1] = Take_Fail; // ��ʾ��һ·����ͷ����ʧ��
            //--------- new   process-------------
            MultiTake.Take_retry = 0;
            //----------������һ·����ͷ-----------
            CameraState.Camera_Number = 3;
            //-------------------------
            Camera_Start(CameraState.Camera_Number);
        }
        else
        {
            // continue operate this  camera
            Camera_Start(CameraState.Camera_Number);
        }

        break;
    case  3:
        MultiTake.Take_retry++;
        if(MultiTake.Take_retry >= 3)
        {
            //-------- old process---------------
            MultiTake.TakeResult[2] = Take_Fail; // ��ʾ��һ·����ͷ����ʧ��
            //--------- new   process-------------
            MultiTake.Take_retry = 0;
            //----------������һ·����ͷ-----------
            CameraState.Camera_Number = 4;
            //-------------------------
            Camera_Start(CameraState.Camera_Number);
        }
        else
        {
            // continue operate this  camera
            Camera_Start(CameraState.Camera_Number);
        }

        break;
    case  4:
        MultiTake.Take_retry++;
        if(MultiTake.Take_retry >= 3)
        {
            //-------- old process---------------
            MultiTake.TakeResult[3] = Take_Fail; // ��ʾ��һ·����ͷ����ʧ��
            //--------- new   process-------------
            MultiTake.Take_retry = 0;
            /*
            Taking End, Start Transfering
             */

            Check_MultiTakeResult_b4Trans();
        }
        else
        {
            // continue operate this  camera
            Camera_Start(CameraState.Camera_Number);
        }

        break;
    default:
        MultiTake_End();   // clear and  exit Mutitake state
        break;
    }





}


void  Camra_Take_Exception(void)
{
    if(CameraState.camera_running == 1)
    {
        if((CameraState.timeoutCounter++) >= 3) // ÿ����һ���������ֻ�е������ͳ���3��û���ز���Ϊʧ��
        {
            if(CameraState.timeoutCounter == 4)
            {
                if(GB19056.workstate == 0)
                    rt_kprintf("\r\n  Camera %d  Error\r\n", CameraState.Camera_Number);
                Power_485CH1_OFF;  // ��һ·485�ĵ�			�ص繤��
            }

            if(CameraState.timeoutCounter == 5)
            {
                Power_485CH1_ON;  // ��һ·485�ĵ�		   ��	�繤��
            }

            if(CameraState.timeoutCounter == 7)
            {
                //------------  Normal  Process  --------------
                //FeedDog;
                Camera_End(MultiTake.Taking);

                //  Power_485CH1_OFF;  // ��һ·485�ĵ�			�ص繤��
                WatchDog_Feed();
                delay_ms(500);  // end--duration--new start
                WatchDog_Feed();
                Power_485CH1_ON;  // ��һ·485�ĵ�		   ��	�繤��
                delay_ms(1000);  // end--duration--new start
                WatchDog_Feed();
                //------------  Multi Take Process  ------------
                if(1 == MultiTake.Taking)
                {
                    MultiTake_Exception();
                }
                else
                {
                    //  ��������
                    CameraState.SingleCamera_TakeRetry++;
                    if(CameraState.SingleCamera_TakeRetry >= 3)
                    {
                        CameraState.SingleCamera_TakeRetry = 0; //clear
                        if(GB19056.workstate == 0)
                            rt_kprintf("\r\n     ��·���ճ���������!\r\n");
                        CameraState.SingleCamra_TakeResualt_BD = 1;
                        Power_485CH1_OFF;  // ��һ·485�ĵ�			�ص繤��
                    }
                    else
                    {
                        // continue operate this  camera
                        Camera_Start(CameraState.Camera_Number);
                    }
                }
            }

        }
    }




}

void MultiTake_Start(void)
{
    u8 i = 0;

    MultiTake.Taking = 1;   // enable
    MultiTake.Transfering = 0;
    MultiTake.CurretCameraNum = 1;
    for(i = 0; i < Max_CameraNum; i++)
        MultiTake.TakeResult[i] = Take_idle;
    MultiTake.Take_retry = 0;
    MultiTake.Take_success_counter = 0;
    //----------------------------------
    CameraState.Camera_Number = 1; // �� 1  Start from  1  Camera
    Camera_Start(CameraState.Camera_Number);  // ��ʼִ������

    //------------------
    CameraState.SingleCamera_TakeRetry = 0; // add later

}

void MultiTake_End(void)
{
    u8 i = 0;

    if(MultiTake.Taking)
        Power_485CH1_OFF;

    MultiTake.Taking = 0;
    MultiTake.Transfering = 0;
    MultiTake.CurretCameraNum = 1;
    for(i = 0; i < Max_CameraNum; i++)
        MultiTake.TakeResult[i] = Take_idle;
    MultiTake.Take_retry = 0;
    MultiTake.Take_success_counter = 0;

}


u8   Camera_Take_Enable(void)
{
    //  ������״̬���У� û�ж�·���պͶ�·���������£���������
    if((CameraState.status == other) && (0 == MultiTake.Taking) && (0 == MultiTake.Transfering))
        return  true;
    else
        return false;
}

u8  Check_MultiTakeResult_b4Trans(void)
{
    u8 i = 0;

    MultiTake.Take_success_counter = 0;
    for(i = 0; i < Max_CameraNum; i++)
    {
        if(Take_Success == MultiTake.TakeResult[i])
        {
            MultiTake.Take_success_counter++;
        }
    }
    // Read_picNum=MultiTake.Take_success_counter-1; // 0  ������һ��
    if(MultiTake.Take_success_counter)
    {
        MultiTake.Transfering = 1; // Set Transfering
        MultiTake.Take_success_counter--;  // ��ʼ�ϴ��͵ü� 1
        //----------- �������ճɹ�������ͷ���  �ϱ�����Ҫ���-----------
        for(i = 0; i < Max_CameraNum; i++)
        {
            if(Take_Success == MultiTake.TakeResult[i])
            {
                CameraState.Camera_Number = i + 1;
                MultiTake.TakeResult[i] = Take_idle; // clear state
                break;
            }
        }

        if(MultiTake.Taking)
            Power_485CH1_OFF;
        MultiTake.Taking = 0; // Taking  State  Over

        //rt_kprintf("\r\n            ��·��ʼ Camera=%d  \r\n",CameraState.Camera_Number);
        Photo_send_start(CameraState.Camera_Number);   //��ʼ׼������
        //rt_kprintf("\r\n            ��ý���¼��ϴ�MediaObj.SD_media_Flag=%d , MediaObj.SD_Eventstate=%d  \r\n",MediaObj.SD_media_Flag,MediaObj.SD_Eventstate);
        return  1;
    }
    else
    {
        MultiTake_End();
        return  0;
    }

}

void Send_const485(u8  flag)
{

    if(!CameraState.TX_485const_Enable)
        return;

    switch(CameraState.status)
    {
    case enable:
        rt_device_write(&Device_485, 0, (char const *)Take_photo, 10); // ������������
        // rt_hw_485_Output_Data(Take_photo, 10);  // ������������
        CameraState.OperateFlag = 1;
        CameraState.create_Flag = 1; // ��Ҫ������ͼƬ�ļ�
        CameraState.last_package = 0;
        if(GB19056.workstate == 0)
            rt_kprintf("\r\n  Camera: %d  ����\r\n", CameraState.Camera_Number);
        CameraState.timeoutCounter = 0; // send clear
        break;
    case  transfer:
        //----------_485_content_wr=0;
        _485_RXstatus._485_receiveflag = IDLE_485;
        // rt_kprintf("\r\n ����ͼƬ��ȡ����\r\n");
        rt_device_write(&Device_485, 0, (char const *)Fectch_photo, 10); // ����ȡͼ����
        //   rt_hw_485_Output_Data(Fectch_photo, 10);  // ������������
        CameraState.OperateFlag = 1;
        CameraState.timeoutCounter = 0; // sendclear
        //rt_kprintf("\r\n  ����ȡͼ����\r\n");
        break;
    case  other	:

        break;
    default:
        break;
    }
    CameraState.TX_485const_Enable = 0; // clear
}
void takephoto(u8 *str)
{

}

//-------------------------------------------------------------------------






