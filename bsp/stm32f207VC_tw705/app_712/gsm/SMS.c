/*
     SMS.C
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
#include  "App_moduleConfig.h"
#include  "App_gsm.h"
#include  "SMS.h"


SMS_Style   SMS_Service;    //  ��Ϣ���

u8 SMS_Destion[15];   //  ���Ͷ�ϢĿ�����



/*********************************************************************************
*��������:void SMS_timer(u8 *instr,u16 len)
*��������:���Ŵ����������������Ҫ��һ��1��Ķ�ʱ��������ã����ں���"SMS_Process"�Ķ�ʱ�����
*��    ��:none
*��    ��:none
*�� �� ֵ:none
*��    ��:������
*��������:2013-05-29
*---------------------------------------------------------------------------------
*�� �� ��:
*�޸�����:
*�޸�����:
*********************************************************************************/
void SMS_timer(void)
{
    if(SMS_Service.SMS_waitCounter)
        SMS_Service.SMS_waitCounter--;
}


/*********************************************************************************
*��������:void SMS_protocol(u8 *instr,u16 len)
*��������:���Ŵ����������������Ҫ��һ���߳�������ã�������ش���(���Ŷ�ȡ��ɾ�����Զ��������)
*��    ��:none
*��    ��:none
*�� �� ֵ:none
*��    ��:������
*��������:2013-05-29
*---------------------------------------------------------------------------------
*�� �� ��:
*�޸�����:
*�޸�����:
*********************************************************************************/
void SMS_Process(void)
{
    u16   	ContentLen = 0;
    //	u16 		i,j,k;
    char *pstrTemp;
    if(SMS_Service.SMS_waitCounter)
        return;
    //-----------  ���Ŵ������ --------------------------------------------------------
    //---------------------------------
    if(SMS_Service.SMS_read)	   // ��ȡ����
    {
        memset(SMS_Service.SMSAtSend, 0, sizeof(SMS_Service.SMSAtSend));
        /*
        strcpy( ( char * ) SMS_Service.SMSAtSend, "AT+CMGR=" );
        if ( SMS_Service.SMIndex > 9 )
        {
        	SMS_Service.SMSAtSend[8] = ( SMS_Service.SMIndex >> 4 ) + 0x30;
        	SMS_Service.SMSAtSend[9] = ( SMS_Service.SMIndex & 0x0f ) + 0x30;
        	SMS_Service.SMSAtSend[10] = 0x0d;
        	SMS_Service.SMSAtSend[11] = 0x0a;
        }
        else
        {
        	SMS_Service.SMSAtSend[8] = ( SMS_Service.SMIndex & 0x0f ) + 0x30;
        	SMS_Service.SMSAtSend[9] = 0x0d;
        	SMS_Service.SMSAtSend[10] = 0x0a;
        }
        rt_kprintf("\r\n%s",SMS_Service.SMSAtSend);
        */
        ///
        sprintf(SMS_Service.SMSAtSend, "AT+CMGR=%d\r\n", SMS_Service.SMIndex);
        rt_kprintf("%s", SMS_Service.SMSAtSend);
        ///
        rt_hw_gsm_output( ( char * ) SMS_Service.SMSAtSend );
        SMS_Service.SMS_read--;
        SMS_Service.SMS_waitCounter = 3;
    }
    //-------------------------------
    //       ���Ͷ�Ϣȷ��
    else if(SMS_Service.SMS_sendFlag == 1)
    {
        //#ifdef SMS_TYPE_PDU
        if(SMS_Service.SMS_come == 1)
        {
            memset(SMS_Service.SMSAtSend, 0, sizeof(SMS_Service.SMSAtSend));
            ///����400�ֽڿռ�
            pstrTemp = rt_malloc(400);
            memset(pstrTemp, 0, 400);
            ///���ַ�����ʽ��Ŀ�ĵ绰��������ΪPDU��ʽ�ĺ���
            SetPhoneNumToPDU(SMS_Service.Sms_Info.TPA, SMS_Service.SMS_destNum, sizeof(SMS_Service.Sms_Info.TPA));
            ///����PDU��ʽ��������
            ContentLen = AnySmsEncode_NoCenter(SMS_Service.Sms_Info.TPA, GSM_UCS2, SMS_Service.SMS_sd_Content, strlen(SMS_Service.SMS_sd_Content), pstrTemp);
            //ContentLen=strlen(pstrTemp);
            ///��Ӷ���β�����"esc"
            pstrTemp[ContentLen] = 0x1A;    // message  end
            //////
            sprintf( ( char * ) SMS_Service.SMSAtSend, "AT+CMGS=%d\r\n", (ContentLen - 2) / 2);
			  if(GB19056.workstate==3)
            rt_kprintf("%s", SMS_Service.SMSAtSend);
            rt_hw_gsm_output( ( char * ) SMS_Service.SMSAtSend );
            rt_thread_delay(50);
            //////
            //rt_kprintf("%s",pstrTemp);
            if(GB19056.workstate==3)
            rt_device_write( &dev_vuart, 0, pstrTemp, strlen(pstrTemp) );
            rt_hw_gsm_output_Data( ( char * ) pstrTemp, ContentLen + 1);
            rt_free( pstrTemp );
            pstrTemp = RT_NULL;
        }
        //#else
        else
        {
            memset(SMS_Service.SMSAtSend, 0, sizeof(SMS_Service.SMSAtSend));
            strcpy( ( char * ) SMS_Service.SMSAtSend, "AT+CMGS=\"" );
            //strcat(SMS_Service.SMSAtSend,"8613820554863");// Debug
            strcat(SMS_Service.SMSAtSend, SMS_Service.SMS_destNum);
            strcat(SMS_Service.SMSAtSend, "\"\r\n");
               if(GB19056.workstate==3)
            rt_kprintf("\r\n%s", SMS_Service.SMSAtSend);
            rt_hw_gsm_output( ( char * ) SMS_Service.SMSAtSend );

            rt_thread_delay(50);
            ContentLen = strlen(SMS_Service.SMS_sd_Content);
            SMS_Service.SMS_sd_Content	[ContentLen] = 0x1A;    // message  end
          if(GB19056.workstate==3)
            rt_kprintf("%s", SMS_Service.SMS_sd_Content);
            rt_hw_gsm_output_Data( ( char * ) SMS_Service.SMS_sd_Content, ContentLen + 1);
        }
        //#endif
        SMS_Service.SMS_sendFlag = 0; // clear
        SMS_Service.SMS_waitCounter = 3;
    }

}


///���ӷ��Ͷ�����������ݣ�����λ���Ͷ�Ϣ��ǣ��ɹ�����true��ʧ�ܷ���false
u8 Add_SMS_Ack_Content(char *instr, u8 ACKflag)
{
    if(ACKflag == 0)
        return false;

    if(strlen(instr) + strlen(SMS_Service.SMS_sd_Content) < sizeof(SMS_Service.SMS_sd_Content))
    {
        strcat((char *)SMS_Service.SMS_sd_Content, instr);
        SMS_Service.SMS_sendFlag = 1;
        return true;
    }
    return false;
}

/*********************************************************************************
*��������:void SMS_protocol(u8 *instr,u16 len)
*��������:���յ����ź����������
*��    ��:instrԭʼ�������ݣ�len����
*��    ��:none
*�� �� ֵ:none
*��    ��:������
*��������:2013-05-29
*---------------------------------------------------------------------------------
*�� �� ��:
*�޸�����:
*�޸�����:
*********************************************************************************/
void   SMS_protocol (u8 *instr, u16 len, u8  ACKstate)  //  ACKstate
{
    char	sms_content[60];		///����������"()"֮�������
    char	sms_ack_data[60];		///����ÿ���������'#'������������
    u8	u8TempBuf[6];
    u16	i = 0, j = 0, res = 0;
    u16  cmdLen, u16Temp;
    char *p_Instr;
    char *pstrTemp, *pstrTempStart, *pstrTempEnd;

    /*
        if(SMS_ACK_none==ACKstate)
        {
    		rt_kprintf( "\r\n �յ�gprs TXT��Ϣ: " );
    		rt_device_write( &dev_vuart, 0, instr, len);
        }
    */

    // һ.  ��������
    //  TW703#SET#C{��A55555}#V{12345678901234567}#M{0}#S{12345678901}#K{1}
    if(strncmp((char *)instr, "#SET", 4) == 0)
    {
        //  1.  ���ƺ�
        if((instr[5] == 'C') && (instr[6] == '{'))
        {
            memset((u8 *)&Vechicle_Info.Vech_Num, 0, sizeof(Vechicle_Info.Vech_Num));	//clear
            rt_memcpy(Vechicle_Info.Vech_Num, instr + 7, 8);
            DF_WriteFlashSector(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
            WatchDog_Feed();
            DF_WriteFlashSector(DF_VehicleBAK_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));

            WatchDog_Feed();
            DF_WriteFlashSector(DF_VehicleBAK2_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
        }

        // 2 .  VIN
        if((instr[17] == 'V') && (instr[18] == '{'))
        {
            memset(sms_content, 0, sizeof(sms_content));
            memcpy(sms_content, instr + 19, 17);
            vin_set(sms_content);
        }
        // 3.  ����ģʽ
        if((instr[38] == 'M') && (instr[39] == '{'))
        {

            // ����
            memset(DomainNameStr, 0, sizeof(DomainNameStr));
            memset(SysConf_struct.DNSR, 0, sizeof(DomainNameStr));
            if(instr[40] == '0')
            {
                memcpy(DomainNameStr, "jt1.gghypt.net", 14);
                memcpy(SysConf_struct.DNSR, "jt1.gghypt.net", 14);
            }
            else if(instr[40] == '1')
            {
                memcpy(DomainNameStr, "up.gps960.com", 13);
                memcpy(SysConf_struct.DNSR, "up.gps960.com", 13);
            }
            Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
            //----- ���� GSM ģ��------
            DataLink_DNSR_Set(SysConf_struct.DNSR, 1);

            // �˿�
            if(instr[40] == '0')
            {
                port_main("7008");
            }
            else
            {
                port_main("8201");
            }

        }
        // 4.  SIM����  ���ֻ�����
        if((instr[43] == 'S') && (instr[44] == '{'))
        {
            memset(SimID_12D, '0', sizeof(SimID_12D));
            memcpy(SimID_12D + 1, instr + 45, 11);;
            DF_WriteFlashSector(DF_SIMID_12D, 0, SimID_12D, 13);
            SIMID_Convert_SIMCODE(); //  ת��
        }
        //  5. ��������
        if((instr[58] == 'K') && (instr[59] == '{'))
        {
            if(instr[60] == '1')
                type_vech(1);

            else if(instr[60] == '2')
                type_vech(2);



            //------ end process
            if(ACKstate == SMS_ACK_none)
                SD_ACKflag.f_CentreCMDack_0001H = 2 ; //DataLink_EndFlag=1; //AT_End(); �ȷ��ؽ���ٹҶ�
            else
                DataLink_EndFlag = 1; //AT_End();
            //--------    �����Ȩ�� -------------------
            idip("clear");
            password(1);

            memset(SMS_Service.SMS_sd_Content, 0, sizeof(SMS_Service.SMS_sd_Content));
            strcpy(SMS_Service.SMS_sd_Content, "TW705");
            strcat(SMS_Service.SMS_sd_Content, "#");
            strcat(SMS_Service.SMS_sd_Content, "SETOK");
            memcpy(SMS_Service.SMS_sd_Content + strlen(SMS_Service.SMS_sd_Content), instr + 4, 58);
            Add_SMS_Ack_Content("", ACKstate);
        }
        return;
    }

    //  ��.  Normal   state

    //SYSID		///�޸ĸ�ֵ������flash
    ///Ӧ����Ű�ͷ����
    memset(SMS_Service.SMS_sd_Content, 0, sizeof(SMS_Service.SMS_sd_Content));
    strcpy(SMS_Service.SMS_sd_Content, Vechicle_Info.Vech_Num);
    strcat(SMS_Service.SMS_sd_Content, "#"); // Debug
    strcat(SMS_Service.SMS_sd_Content, SimID_12D); // Debug
    //strcat(SMS_Service.SMS_sd_Content,"#TW705");
    if(Vechicle_Info.Vech_Type_Mark == 1)
        strcat(SMS_Service.SMS_sd_Content, "#K1"); // �汾��Ϣ
    else if(Vechicle_Info.Vech_Type_Mark == 2)
        strcat(SMS_Service.SMS_sd_Content, "#H1"); // �汾��Ϣ
    else
        strcat(SMS_Service.SMS_sd_Content, "#QT"); // QT �汾��Ϣ

    /*************************������Ϣ****************************/
    p_Instr = (char *)instr;
    for(i = 0; i < len; i++)
    {
        pstrTemp = strchr(p_Instr, '#');					///���������Ƿ����
        //instr++;
        if(pstrTemp)
        {
            p_Instr = pstrTemp + 1;
            pstrTempStart = strchr((char *)pstrTemp, '(');			///�����������ݿ�ʼλ��
            pstrTempEnd = strchr((char *)pstrTemp, ')');			///�����������ݽ���λ��
            if((NULL == pstrTempStart) || (NULL == pstrTempEnd))
            {
                break;
            }
            if(GB19056.workstate) 
                rt_kprintf("\r\n���������ʽ��Ч !");
            ///��ȡ��������
            memset(sms_ack_data, 0, sizeof(sms_ack_data));
            memcpy(sms_ack_data, pstrTemp, pstrTempEnd - pstrTemp + 1);

            ///��ȡ�����������������Լ���������
            pstrTempStart++;
            pstrTemp++;
            cmdLen = pstrTempEnd - pstrTempStart;
            memset(sms_content, 0, sizeof(sms_content));
            rt_memcpy(sms_content, pstrTempStart, cmdLen);

            ///��������ƥ��
            if(strncmp(pstrTemp, "DNSR", 4) == 0)				///  1. ��������
            {
                if(cmdLen <= sizeof(DomainNameStr))
                {
                    if(pstrTemp[4] == '1')		///������
                    {
                        //rt_kprintf("\r\n���������� !");
                        memset(DomainNameStr, 0, sizeof(DomainNameStr));
                        memset(SysConf_struct.DNSR, 0, sizeof(DomainNameStr));
                        memcpy(DomainNameStr, (char *)pstrTempStart, cmdLen);
                        memcpy(SysConf_struct.DNSR, (char *)pstrTempStart, cmdLen);
                        Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
                        //----- ���� GSM ģ��------
                        DataLink_DNSR_Set(SysConf_struct.DNSR, 1);

                        ///
                        Add_SMS_Ack_Content(sms_ack_data, ACKstate);

                        //------- add on 2013-6-6
                        if(ACKstate == SMS_ACK_none)
                            SD_ACKflag.f_CentreCMDack_0001H = 2 ; //DataLink_EndFlag=1; //AT_End(); �ȷ��ؽ���ٹҶ�
                        else
                            DataLink_EndFlag = 1; //AT_End();

                        //--------    �����Ȩ�� -------------------
                        idip("clear");

                    }
                    else if(pstrTemp[4] == '2')	///��������
                    {
                        //rt_kprintf("\r\n���ñ������� !");
                        memset(DomainNameStr_aux, 0, sizeof(DomainNameStr_aux));
                        memset(SysConf_struct.DNSR_Aux, 0, sizeof(DomainNameStr_aux));
                        memcpy(DomainNameStr_aux, (char *)pstrTempStart, cmdLen);
                        memcpy(SysConf_struct.DNSR_Aux, (char *)pstrTempStart, cmdLen);
                        Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
                        //----- ���� GSM ģ��------
                        DataLink_DNSR2_Set(SysConf_struct.DNSR_Aux, 1);

                        ///
                        Add_SMS_Ack_Content(sms_ack_data, ACKstate);
                    }
                    else
                    {
                        continue;
                    }
                }
            }
            else if(strncmp(pstrTemp, "PORT", 4) == 0)			///2. ���ö˿�
            {
                j = sscanf(sms_content, "%u", &u16Temp);
                if(j)
                {
                    if(pstrTemp[4] == '1')		///���˿�
                    {
                        //rt_kprintf("\r\n�������˿�=%d!",u16Temp);
                        RemotePort_main = u16Temp;
                        SysConf_struct.Port_main = u16Temp;
                        Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
                        //----- ���� GSM ģ��------
                        DataLink_MainSocket_set(RemoteIP_main, RemotePort_main, 0);
                        ///
                        Add_SMS_Ack_Content(sms_ack_data, ACKstate);

                        //------- add on 2013-6-6
                        if(ACKstate == SMS_ACK_none)
                            SD_ACKflag.f_CentreCMDack_0001H = 2 ; //DataLink_EndFlag=1; //AT_End(); �ȷ��ؽ���ٹҶ�
                        else
                            DataLink_EndFlag = 1; //AT_End();

                        //--------    �����Ȩ�� -------------------
                        idip("clear");

                    }
                    else if(pstrTemp[4] == '2')	///���ö˿�
                    {
                        //rt_kprintf("\r\n���ñ��ö˿�=%d!",u16Temp);
                        RemotePort_aux = u16Temp;
                        SysConf_struct.Port_Aux = u16Temp;
                        Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
                        //----- ���� GSM ģ��------
                        DataLink_AuxSocket_set(RemoteIP_aux, RemotePort_aux, 1);
                        ///
                        Add_SMS_Ack_Content(sms_ack_data, ACKstate);
                    }
                    else
                    {
                        continue;
                    }
                }

            }
            else if(strncmp(pstrTemp, "DUR", 3) == 0)				///3. �޸ķ��ͼ��
            {
                j = sscanf(sms_content, "%u", &u16Temp);
                if(j)
                {

                    //rt_kprintf("\r\n�޸ķ��ͼ��! %d",u16Temp);
                    dur(sms_content);

                    ///
                    Add_SMS_Ack_Content(sms_ack_data, ACKstate);
                }
            }
            else if(strncmp(pstrTemp, "DEVICEID", 8) == 0)			///4. �޸��ն�ID
            {
                if(cmdLen <= sizeof(DeviceNumberID))
                {
                    //rt_kprintf("\r\n�޸��ն�ID  !");
                    memset(DeviceNumberID, 0, sizeof(DeviceNumberID));
                    memcpy(DeviceNumberID, pstrTempStart, cmdLen);
                    DF_WriteFlashSector(DF_DeviceID_offset, 0, DeviceNumberID, 13);
                    ///
                    Add_SMS_Ack_Content(sms_ack_data, ACKstate);

                    //--------    �����Ȩ�� -------------------
                    idip("clear");

                }
                else
                {
                    continue;
                }
            }    //DeviceNumberID
            else if(strncmp(pstrTemp, "SIMID", 5) == 0)			///4. �޸�SIMID
            {
                if(cmdLen <= sizeof(SimID_12D))
                {
                    //rt_kprintf("\r\n�޸�SIM_ID  !");
                    memset(SimID_12D, 0, sizeof(SimID_12D));
                    memcpy(SimID_12D, pstrTempStart, cmdLen);
                    DF_WriteFlashSector(DF_SIMID_12D, 0, SimID_12D, 13);
                    SIMID_Convert_SIMCODE(); //  ת��

                    ///
                    Add_SMS_Ack_Content(sms_ack_data, ACKstate);

                    //--------    �����Ȩ�� -------------------
                    idip("clear");

                }
                else
                {
                    continue;
                }
            }    //DeviceNumberID
            else if(strncmp(pstrTemp, "PHONENUM", 8) == 0)			///4. �޸�SIMID
            {
                if(cmdLen == 11)
                {
                    //rt_kprintf("\r\n�޸�PHONENUM_ID  !");
                    memset(SimID_12D, 0, sizeof(SimID_12D));
                    SimID_12D[0] = '0'; //    ��һλ��д 0
                    memcpy(SimID_12D + 1, pstrTempStart, cmdLen);
                    DF_WriteFlashSector(DF_SIMID_12D, 0, SimID_12D, 13);
                    SIMID_Convert_SIMCODE(); //  ת��

                    ///
                    Add_SMS_Ack_Content(sms_ack_data, ACKstate);
                    //--------    �����Ȩ�� -------------------
                    idip("clear");

                }
                else
                {
                    continue;
                }
            }    //DeviceNumberID
            else if(strncmp(pstrTemp, "IP", 2) == 0)				///5.����IP��ַ
            {
                j = sscanf(sms_content, "%u.%u.%u.%u", (u32 *)&u8TempBuf[0], (u32 *)&u8TempBuf[1], (u32 *)&u8TempBuf[2], (u32 *)&u8TempBuf[3]);
                //j=str2ip(sms_content, u8TempBuf);
                if(j == 4)
                {
                    //rt_kprintf("\r\n����IP��ַ!");
                    if(pstrTemp[2] == '1')
                    {
                        memcpy(SysConf_struct.IP_Main, u8TempBuf, 4);
                        memcpy(RemoteIP_main, u8TempBuf, 4);
                        SysConf_struct.Port_main = RemotePort_main;
                        Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
                        //rt_kprintf("\r\n���������������� IP: %d.%d.%d.%d : %d ",RemoteIP_main[0],RemoteIP_main[1],RemoteIP_main[2],RemoteIP_main[3],RemotePort_main);
                        //-----------  Below add by Nathan  ----------------------------
                        DataLink_MainSocket_set(RemoteIP_main, RemotePort_main, 0);
                        ///
                        Add_SMS_Ack_Content(sms_ack_data, ACKstate);

                        //------- add on 2013-6-6
                        if(ACKstate == SMS_ACK_none)
                            SD_ACKflag.f_CentreCMDack_0001H = 2 ; //DataLink_EndFlag=1; //AT_End(); �ȷ��ؽ���ٹҶ�
                        else
                            DataLink_EndFlag = 1;

                        //--------    �����Ȩ�� -------------------
                        idip("clear");

                    }
                    else if(pstrTemp[2] == '2')
                    {
                        memcpy(SysConf_struct.IP_Aux, u8TempBuf, 4);
                        memcpy(RemoteIP_aux, u8TempBuf, 4);
                        SysConf_struct.Port_Aux = RemotePort_aux;
                        Api_Config_write(config, ID_CONF_SYS, (u8 *)&SysConf_struct, sizeof(SysConf_struct));
                        //rt_kprintf("\r\n�������ñ��÷����� IP: %d.%d.%d.%d : %d ",RemoteIP_aux[0],RemoteIP_aux[1],RemoteIP_aux[2],RemoteIP_aux[3],RemotePort_aux);
                        //-----------  Below add by Nathan  ----------------------------
                        DataLink_AuxSocket_set(RemoteIP_aux, RemotePort_aux, 1);
                        ///
                        Add_SMS_Ack_Content(sms_ack_data, ACKstate);
                    }
                }
            }
            if(strncmp(pstrTemp, "MODE", 4) == 0)			///6. ���ö�λģʽ
            {
                if(strncmp(sms_content, "BD", 2) == 0)
                {
                    gps_mode("1");
                }
                if(strncmp(sms_content, "GP", 2) == 0)
                {
                    gps_mode("2");
                }
                if(strncmp(sms_content, "GN", 2) == 0)
                {
                    gps_mode("3");
                }
                Add_SMS_Ack_Content(sms_ack_data, ACKstate);
            }
            else if(strncmp(pstrTemp, "VIN", 3) == 0)				///7.���ó���VIN
            {
                vin_set(sms_content);
                Add_SMS_Ack_Content(sms_ack_data, ACKstate);
            }
            else if(strncmp(pstrTemp, "TIREDCLEAR", 10) == 0)		///8.���ƣ�ͼ�ʻ��¼
            {
                index_write(0x11, 0);
                Add_SMS_Ack_Content(sms_ack_data, ACKstate);
            }
            else if(strncmp(pstrTemp, "SPDLIMIT", 8) == 0)		///8+1.���ƣ�ͼ�ʻ��¼
            {
                Limit_max_SateFlag = sms_content[0] - 0x30;
                DF_WriteFlashSector(DF_LimitSPEED_offset, 0, &Limit_max_SateFlag, 1);
                Add_SMS_Ack_Content(sms_ack_data, ACKstate);
            }
            else if(strncmp(pstrTemp, "DISCLEAR", 8) == 0)			///9������
            {
                DayStartDistance_32 = 0;
                Distance_m_u32 = 0;
                //              Api_Config_Recwrite_Large(jt808,0,(u8*)&JT808Conf_struct,sizeof(JT808Conf_struct));
                DF_Write_RecordAdd(Distance_m_u32, DayStartDistance_32, TYPE_DayDistancAdd);
                JT808Conf_struct.DayStartDistance_32 = DayStartDistance_32;
                JT808Conf_struct.Distance_m_u32 = Distance_m_u32;
                Add_SMS_Ack_Content(sms_ack_data, ACKstate);
            }
            else if(strncmp(pstrTemp, "SPDCLEAR", 8) == 0) //  ���ٱ������
            {
#ifdef SPD_WARN_SAVE
                //-----  Record update----
                ExpSpdRec_write = 0;
                ExpSpdRec_write = 0;
                DF_Write_RecordAdd(ExpSpdRec_write, ExpSpdRec_write, TYPE_ExpSpdAdd);
#endif

                Add_SMS_Ack_Content(sms_ack_data, ACKstate);

            }
            else if(strncmp(pstrTemp, "RESET", 5) == 0)			///10.�ն�����
            {
                reset();
            }
            else if(strncmp(pstrTemp, "RELAY", 5) == 0)			///11.�̵�������
            {
                if(sms_content[0] == '0')
                    debug_relay("0");
                if(sms_content[0] == '1')
                    debug_relay("1");

                Add_SMS_Ack_Content(sms_ack_data, ACKstate);
            }
            else if(strncmp(pstrTemp, "TAKE", 4) == 0)				//12./����
            {
                switch(sms_content[0])
                {
                case '1':
                    takephoto("1");
                    break;
                case '2':
                    takephoto("2");
                    break;
                case '3':
                    takephoto("3");
                    break;
                case '4':
                    takephoto("4");
                    break;
                }
                Add_SMS_Ack_Content(sms_ack_data, ACKstate);
            }
            else if(strncmp(pstrTemp, "PLAY", 4) == 0)				///13.��������
            {
                TTS_Get_Data(sms_content, strlen(sms_content));
                Add_SMS_Ack_Content(sms_ack_data, ACKstate);
            }
            else if(strncmp(pstrTemp, "QUERY", 5) == 0)			///14.����״̬��ѯ
            {
                switch(sms_content[0])
                {
                case '0':  // �ն˺�  ������  ���˿�

                    strcat((char *)SMS_Service.SMS_sd_Content, sms_ack_data);
                    strcat(SMS_Service.SMS_sd_Content, "#"); // �ָ�����
                    strcat(SMS_Service.SMS_sd_Content, DeviceNumberID); // �ն�ID
                    strcat(SMS_Service.SMS_sd_Content, "#"); // �ָ�����
                    strcat(SMS_Service.SMS_sd_Content, DomainNameStr); // �ָ�����
                    strcat(SMS_Service.SMS_sd_Content, "#"); // �ָ�����
                    sprintf(SMS_Service.SMS_sd_Content + strlen(SMS_Service.SMS_sd_Content), "%d", RemotePort_main);

                    break;
                case '1':  // MODE    ��IP   ���˿�
                    strcat((char *)SMS_Service.SMS_sd_Content, sms_ack_data);
                    strcat(SMS_Service.SMS_sd_Content, "#"); // �ָ�����
                    if(Vechicle_Info.Link_Frist_Mode == 1)
                        strcat(SMS_Service.SMS_sd_Content, "MODE(mainlink)");
                    else
                        strcat(SMS_Service.SMS_sd_Content, "MODE(dnsr)");
                    strcat(SMS_Service.SMS_sd_Content, "#"); // �ָ�����
                    IP_Str(SMS_Service.SMS_sd_Content + strlen(SMS_Service.SMS_sd_Content), *( u32 * ) RemoteIP_main);
                    strcat(SMS_Service.SMS_sd_Content, "#"); // �ָ�����
                    sprintf(SMS_Service.SMS_sd_Content + strlen(SMS_Service.SMS_sd_Content), "%d", RemotePort_main);

                    break;
                case '2':
                    strcat(SMS_Service.SMS_sd_Content, "#"); // �ָ�����
                    strcat(SMS_Service.SMS_sd_Content, Posit_ASCII.Lat_ASCII); // �ָ�����
                    strcat(SMS_Service.SMS_sd_Content, "#"); // �ָ�����
                    strcat(SMS_Service.SMS_sd_Content, Posit_ASCII.Longi_ASCII); // �ָ�����
                    strcat(SMS_Service.SMS_sd_Content, "#"); // �ָ�����
                    strcat(SMS_Service.SMS_sd_Content, &Posit_ASCII.AV_ASCII); // �ָ�����
                    strcat(SMS_Service.SMS_sd_Content, "#"); // �ָ�����
                    if(GpsStatus.Position_Moule_Status == 1)
                    {
                        strcat(SMS_Service.SMS_sd_Content, "BD");
                    }
                    else if(GpsStatus.Position_Moule_Status == 2)
                    {
                        strcat(SMS_Service.SMS_sd_Content, "GPS");
                    }
                    else if(GpsStatus.Position_Moule_Status == 3)
                    {
                        strcat(SMS_Service.SMS_sd_Content, "BD+GPS");
                    }
                    strcat(SMS_Service.SMS_sd_Content, "#"); // �ָ�����
                    sprintf(SMS_Service.SMS_sd_Content + strlen(SMS_Service.SMS_sd_Content), "%d", ModuleSQ);

                    break;
                case '3':  // ʡ��ID  ����ID
                    strcat(SMS_Service.SMS_sd_Content, "#provinceid("); // �ָ�����
                    sprintf(SMS_Service.SMS_sd_Content + strlen(SMS_Service.SMS_sd_Content), "%d", Vechicle_Info.Dev_ProvinceID);
                    strcat(SMS_Service.SMS_sd_Content, ")#"); // �ָ�����
                    strcat(SMS_Service.SMS_sd_Content, "cityid("); // �ָ�����
                    sprintf(SMS_Service.SMS_sd_Content + strlen(SMS_Service.SMS_sd_Content), "%d)", Vechicle_Info.Dev_CityID);
                    strcat(SMS_Service.SMS_sd_Content, "#Online("); // �ָ�����
                    sprintf(SMS_Service.SMS_sd_Content + strlen(SMS_Service.SMS_sd_Content), "%d)", DataLink_Status());
                    strcat(SMS_Service.SMS_sd_Content, "#HWver("); // �ָ�����
                    sprintf(SMS_Service.SMS_sd_Content + strlen(SMS_Service.SMS_sd_Content), "%d)", HardWareVerion);
                    break;
                case '4': 		 //�ٶ� ������
                    strcat(SMS_Service.SMS_sd_Content, "#SpdType("); // �ָ�����
                    sprintf(SMS_Service.SMS_sd_Content + strlen(SMS_Service.SMS_sd_Content), "%d)", JT808Conf_struct.Speed_GetType);
                    strcat(SMS_Service.SMS_sd_Content, "#Adjust("); // �ָ�����
                    sprintf(SMS_Service.SMS_sd_Content + strlen(SMS_Service.SMS_sd_Content), "%d)", JT808Conf_struct.DF_K_adjustState);
                    strcat(SMS_Service.SMS_sd_Content, "#PlusNum("); // �ָ�����
                    sprintf(SMS_Service.SMS_sd_Content + strlen(SMS_Service.SMS_sd_Content), "%d)", JT808Conf_struct.Vech_Character_Value);
                    break;
                case '5':
                    strcat(SMS_Service.SMS_sd_Content, "#gpsspd("); // �ָ�����
                    sprintf(SMS_Service.SMS_sd_Content + strlen(SMS_Service.SMS_sd_Content), "%d km/h)", Speed_gps / 10);
                    strcat(SMS_Service.SMS_sd_Content, "#sensor("); // �ָ�����
                    sprintf(SMS_Service.SMS_sd_Content + strlen(SMS_Service.SMS_sd_Content), "%d km/h)", Speed_cacu / 10);

                    break;
                }

                if(ACKstate)
                    SMS_Service.SMS_sendFlag = 1; // ���ͷ��ض�Ϣ��־λ
            }
            else if(strncmp(pstrTemp, "ISP", 3) == 0)				///15.Զ������IP �˿�
            {
                ;
                Add_SMS_Ack_Content(sms_ack_data, ACKstate);
            }
            else if(strncmp(pstrTemp, "PLATENUM", 8) == 0)
            {
                //rt_kprintf("���ƺ�:%s", sms_content);
                memset((u8 *)&Vechicle_Info.Vech_Num, 0, sizeof(Vechicle_Info.Vech_Num));	//clear
                rt_memcpy(Vechicle_Info.Vech_Num, sms_content, strlen(sms_content));
                DF_WriteFlashSector(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
                WatchDog_Feed();
                DF_WriteFlashSector(DF_VehicleBAK_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));

                WatchDog_Feed();
                DF_WriteFlashSector(DF_VehicleBAK2_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));

                Add_SMS_Ack_Content(sms_ack_data, ACKstate);

                Login_Menu_Flag = 1;	 // clear  first flag
                DF_WriteFlashSector(DF_LOGIIN_Flag_offset, 0, &Login_Menu_Flag, 1);

                //--------    �����Ȩ�� -------------------
                idip("clear");
            }
            else if(strncmp(pstrTemp, "COLOUR", 6) == 0)
            {
                j = sscanf(sms_content, "%d", &u16Temp);
                if(j)
                {

                    Vechicle_Info.Dev_Color = u16Temp;
                    //rt_kprintf("\r\n ������ɫ: %s ,%d \r\n",sms_content,Vechicle_Info.Dev_Color);
                    DF_WriteFlashSector(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));

                    WatchDog_Feed();
                    DF_WriteFlashSector(DF_VehicleBAK_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));

                    WatchDog_Feed();
                    DF_WriteFlashSector(DF_VehicleBAK2_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
                    Add_SMS_Ack_Content(sms_ack_data, ACKstate);
                }
            }
            else if(strncmp(pstrTemp, "CONNECT", 6) == 0)
            {
                j = sscanf(sms_content, "%d", &u16Temp);
                if(j)
                {

                    Vechicle_Info.Link_Frist_Mode = u16Temp;
                    //rt_kprintf("\r\n �״����ӷ�ʽ %s ,%d \r\n",sms_content,Vechicle_Info.Link_Frist_Mode);
                    DF_WriteFlashSector(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));

                    WatchDog_Feed();
                    DF_WriteFlashSector(DF_VehicleBAK_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));

                    WatchDog_Feed();
                    DF_WriteFlashSector(DF_VehicleBAK2_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
                    Add_SMS_Ack_Content(sms_ack_data, ACKstate);
                    //--------    �����Ȩ�� -------------------
                    idip("clear");
                }
            }
            else if(strncmp(pstrTemp, "CLEARREGIST", 11) == 0)
            {
                //--------    �����Ȩ�� -------------------
                idip("clear");
                DEV_regist.Enable_sd = 1; // set ����ע���־λ
                DataLink_EndFlag = 1;
                Add_SMS_Ack_Content(sms_ack_data, ACKstate); //  ack  state
            }
            //  20
            else if(strncmp(pstrTemp, "PASSWORD", 8) == 0) //��������ͨ��
            {
                j = sscanf(sms_content, "%d", &u16Temp);
                if(j)
                {
                    Login_Menu_Flag = (u8)u16Temp;	 // clear  first flag
                    DF_WriteFlashSector(DF_LOGIIN_Flag_offset, 0, &Login_Menu_Flag, 1);

                    Add_SMS_Ack_Content(sms_ack_data, ACKstate); //  ack  state
                    if(Login_Menu_Flag)
                    {
                        //--------    �����Ȩ�� -------------------
                        idip("clear");
                        DEV_regist.Enable_sd = 1; // set ����ע���־λ
                        DataLink_EndFlag = 1;
                    }
                }
            }
            else if(strncmp(pstrTemp, "RECOVER", 7) == 0)
            {
                //------  ����ظ���������
                Login_Menu_Flag = 0;	 // clear	first flag
                DF_WriteFlashSector(DF_LOGIIN_Flag_offset, 0, &Login_Menu_Flag, 1);

                Add_SMS_Ack_Content(sms_ack_data, ACKstate); //  ack  state
                Systerm_Reset_counter = Max_SystemCounter - 30; //���ض��ź�ָ���������

            } //provinceid
            else if(strncmp(pstrTemp, "PROVINCEID", 10) == 0)
            {
                provinceid(sms_content);
                Add_SMS_Ack_Content(sms_ack_data, ACKstate);

            }
            else if(strncmp(pstrTemp, "CITYID", 6) == 0)
            {
                cityid(sms_content);
                Add_SMS_Ack_Content(sms_ack_data, ACKstate);

            }
            else if(strncmp(pstrTemp, "ATAENABLE", 9) == 0)
            {
                j = sscanf(sms_content, "%d", &u16Temp);
                if(j)
                {
                    ata_enable(u16Temp);
                    Add_SMS_Ack_Content(sms_ack_data, ACKstate);
                }

            }
            else if(strncmp(pstrTemp, "TYPEVECH", 8) == 0)			///14.����״̬��ѯ
            {
                if(sms_content[0] == '1')
                {
                    type_vech(1);
                    Add_SMS_Ack_Content(sms_ack_data, ACKstate);
                }
                if(sms_content[0] == '2')
                {
                    type_vech(2);
                    Add_SMS_Ack_Content(sms_ack_data, ACKstate);
                }
            }

            if(strncmp(pstrTemp, "PROTYPE", 7) == 0)			 ///�����ն��ͺ�
            {
                rt_kprintf("\r\n	PROTYPE \r\n");
                switch(sms_content[0])
                {
                case '3':
                    product_type("TW703");
                    Add_SMS_Ack_Content(sms_ack_data, ACKstate);
                    break;
                case  '5':
                    product_type("TW705");
                    Add_SMS_Ack_Content(sms_ack_data, ACKstate);
                    break;
                default:
                    break;
                }
            }

            if(strncmp(pstrTemp, "SPDTYPE", 6) == 0)
            {
                j = sscanf(sms_content, "%d", &u16Temp);
                if(j)
                {
                    spd_type(u16Temp);//   0: GPS �ٶ�   1:  �������ٶ�
                    Add_SMS_Ack_Content(sms_ack_data, ACKstate);

                }
            }
            else if(strncmp(pstrTemp, "ADJUST", 6) == 0)
            {
                j = sscanf(sms_content, "%d", &u16Temp);
                if(j)
                {
                    adjust_ok(u16Temp);//   0: GPS �ٶ�   1:  �������ٶ�
                    Add_SMS_Ack_Content(sms_ack_data, ACKstate);

                }
            }
            else if(strncmp(pstrTemp, "PLUSNUM", 6) == 0)
            {
                j = sscanf(sms_content, "%d", &u16Temp);
                if(j)
                {
                    plus_num(u16Temp);//	0: GPS �ٶ�   1:  �������ٶ�
                    Add_SMS_Ack_Content(sms_ack_data, ACKstate);

                }
            }
            else if(strncmp(pstrTemp, "DATACLEAR", 9) == 0)		///����洢����
            {
                write_read(0, 0);
                Add_SMS_Ack_Content(sms_ack_data, ACKstate);
            }
            else if(strncmp(pstrTemp, "PLAYSET", 7) == 0)				///3. �޸ķ��ͼ��
            {
                j = sscanf(sms_content, "%u", &u16Temp);
                if(j)
                {
                    Warn_Play_controlBit = j;
                    DF_WriteFlashSector(DF_WARN_PLAY_Page, 0, &Warn_Play_controlBit, 1);
                    Add_SMS_Ack_Content(sms_ack_data, ACKstate);
                }
            }
            //-----------------------------------------------------------------------------------
        }
        else
        {
            break;
        }

    }
}



/*********************************************************************************
*��������:u8 SMS_Rx_Text(char *instr,char *strDestNum)
*��������:���յ�TEXT��ʽ�Ķ��Ŵ�����
*��    ��:instr ԭʼ�������ݣ�strDestNum���յ�����Ϣ�ķ��ͷ�����
*��    ��:none
*�� �� ֵ:	1:������ɣ�
			0:��ʾʧ��
*��    ��:������
*��������:2013-05-29
*---------------------------------------------------------------------------------
*�� �� ��:
*�޸�����:
*�޸�����:
*********************************************************************************/
u8 SMS_Rx_Text(char *instr, char *strDestNum)
{
    u16 len;
    u8 ret = 0;
    len = strlen(strDestNum);
    memset( SMS_Service.SMS_destNum, 0, sizeof( SMS_Service.SMS_destNum ) );
    if(len > sizeof( SMS_Service.SMS_destNum ))
    {
        len = sizeof( SMS_Service.SMS_destNum );
    }
    memcpy(SMS_Service.SMS_destNum, strDestNum, len);
    rt_kprintf( "\r\n Rx_Text ��Ϣ��Դ����:%s", SMS_Service.SMS_destNum ); 

    len = strlen(instr);
    rt_kprintf( "\r\n �����յ���Ϣ: " );
    rt_device_write( &dev_vuart, 0, instr, len);

    //  tw705 �ظ��ľ��� TW705  �յ�703 Ҳ��705
    if(( strncmp( (char *)instr, "TW705#", 6 ) == 0 ) || ( strncmp( (char *)instr, "TW703#", 6 ) == 0 ))                                            //�����޸�UDP��IP�Ͷ˿�
    {
        //-----------  �Զ��� ��Ϣ�����޸� Э�� ----------------------------------
        SMS_protocol( instr + 5, len - 5 , SMS_ACK_msg);
        ret = 1;
    }
    SMS_Service.SMS_read		= 0;
    SMS_Service.SMS_waitCounter = 0;
    SMS_Service.SMS_come		= 0;
    //SMS_Service.SMS_delALL		= 1;
    return ret;
}


/*********************************************************************************
*��������:u8 SMS_Rx_PDU(char *instr,u16 len)
*��������:���յ�PDU��ʽ�Ķ��Ŵ�����
*��    ��:instr ԭʼ�������ݣ�len���յ�����Ϣ���ȣ���λΪ�ֽ�
*��    ��:none
*�� �� ֵ:	1:������ɣ�
			0:��ʾʧ��
*��    ��:������
*��������:2013-05-29
*---------------------------------------------------------------------------------
*�� �� ��:
*�޸�����:
*�޸�����:
*********************************************************************************/
u8 SMS_Rx_PDU(char *instr, u16 len)
{
    char *pstrTemp;
    u8 ret = 0;

    //////
    memset( SMS_Service.SMS_destNum, 0, sizeof( SMS_Service.SMS_destNum ) );
    pstrTemp = (char *)rt_malloc(200);	///���Ž������������ݣ��������ΪGB��
    memset(pstrTemp, 0, 200);
    if(GB19056.workstate==3)
    {
        rt_kprintf( "\r\n ����ԭʼ��Ϣ: " );
        rt_device_write( &dev_vuart, 0, GSM_rx, len );
    }

    len = GsmDecodePdu(GSM_rx, len, &SMS_Service.Sms_Info, pstrTemp);
    GetPhoneNumFromPDU( SMS_Service.SMS_destNum,  SMS_Service.Sms_Info.TPA, sizeof(SMS_Service.Sms_Info.TPA));

    //memcpy( SMS_Service.SMS_destNum, SMS_Service.Sms_Info.TPA,sizeof( SMS_Service.SMS_destNum ) );
    // if(GB19056.workstate)
    {   
		rt_kprintf( "\r\n        �յ�����Ϣ! ");
        rt_kprintf( "\r\n                ��Դ����:%s \r\n", SMS_Service.SMS_destNum );
        rt_kprintf( "\r\n                ��Ϣ����:" );
        rt_device_write( &dev_vuart, 0, pstrTemp, len );
		rt_kprintf( "\r\n  ");
    }
    //rt_hw_console_output(GSM_rx);
    //  ��Ϣ���մ���---------------- 
    /*
    if(( strncmp( (char *)pstrTemp, "TW705#", 6 ) == 0 ) || ( strncmp( (char *)pstrTemp, "TW703#", 6 ) == 0 ))                                            //�����޸�UDP��IP�Ͷ˿�
    {
        //-----------  �Զ��� ��Ϣ�����޸� Э�� ----------------------------------
        SMS_protocol( pstrTemp + 5, len - 5 , SMS_ACK_msg);
        ret = 1;
    }*/
     
	ret = 1;
    SMS_Service.SMS_read		= 0;
    SMS_Service.SMS_waitCounter = 3;
    SMS_Service.SMS_come		= 1;
    //SMS_Service.SMS_delALL		= 1;
    rt_free( pstrTemp );
    pstrTemp = RT_NULL;
    return ret;
}


void sms_dest(u8 *str) 
{

    u8 i = 0, value = 0;
    u8 reg_str[20];

    memset(reg_str, 0, sizeof(reg_str));
    if (strlen((const char *)str) == 0)
    {
        rt_kprintf("\r\n ��ϢĿ�ĺ���Ϊ : ");
        for(i = 0; i < 11; i++)
            rt_kprintf("%c", SMS_Destion[i]);
        rt_kprintf("\r\n");
        return ;
    }
    else
    {
        //---- check -------
        memcpy(reg_str, str, strlen((const char *)str));
        if(strlen((const char *)reg_str) == 11) //  �����ж�
        {
            for(i = 0; i < 11; i++)
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
                rt_kprintf("\r\n ��ϢĿ�ĺ��벻�Ϸ�!  \r\n");
                return ;
            }

        }
        else
        {
            rt_kprintf("\r\ndevice_LenError\r\n");
            rt_kprintf("\r\n ��ϢĿ�ĺ��볤�Ȳ���ȷ!  \r\n"); 
            return ;
        }

        memset(SMS_Destion, 0, sizeof(SMS_Destion));
        memcpy(SMS_Destion, reg_str, 11);
        DF_WriteFlashSector(DF_DeviceID_offset, 0, SMS_Destion, 13);
        delay_ms(80);
        DF_ReadFlash(DF_DeviceID_offset, 0, SMS_Destion, 13);
        rt_kprintf("\r\ndevice_OK(");
        for(i = 0; i < 11; i++)
            rt_kprintf("%c", SMS_Destion[i]);
        rt_kprintf(")\r\n");
        rt_kprintf("\r\n��ϢĿ�ĺ���Ϊ :%s\r\n", SMS_Destion);
        rt_kprintf("\r\n");
        return ;
    }
}
FINSH_FUNCTION_EXPORT(sms_dest, sms_destnum set);

void txt_sms(u8 *str)
{
   u8 local_str[150];
   u8  len=0;

   memset(local_str,0,sizeof(local_str));
   strcat((char *)local_str, str);
   len=strlen(local_str);
   
   if(11!=strlen(SMS_Destion))
  {	 
	   rt_kprintf("\r\n ��Ϣ�������������11 λ�ֻ�����\r\n", str);
       
      return;
  }
   if(len>70)
  {	 
	   rt_kprintf("\r\n  ��Ϣ���ݹ�����������С�ڵ���70���ַ�\r\n", str);
       
      return;
  }
   memset(SMS_Service.SMS_destNum,0,sizeof(SMS_Service.SMS_destNum)); 
   memcpy(SMS_Service.SMS_destNum,SMS_Destion,strlen(SMS_Destion));
   SMS_Service.SMS_come=1;
   memset(SMS_Service.SMS_sd_Content, 0, sizeof(SMS_Service.SMS_sd_Content));
   strcat((char *)SMS_Service.SMS_sd_Content, str);
   SMS_Service.SMS_sendFlag = 1;


   rt_kprintf("\r\n            ��ϢĿ�ĺ���Ϊ :%s\r\n", SMS_Destion); 
   rt_kprintf("\r\n            ������Ϣ����Ϊ :%s\r\n", str);

}
FINSH_FUNCTION_EXPORT(txt_sms, txt_sms); 


#if 0
u8 SMS_Tx_Text(char *strDestNum, char *s)
{
    u16 len;
    char *pstrTemp;

    memset(SMS_Service.SMSAtSend, 0, sizeof(SMS_Service.SMSAtSend));
    strcpy( ( char * ) SMS_Service.SMSAtSend, "AT+CMGS=\"" );
    //strcat(SMS_Service.SMSAtSend,"8613820554863");// Debug
    strcat(SMS_Service.SMSAtSend, strDestNum);
    strcat(SMS_Service.SMSAtSend, "\"\r\n");

    rt_kprintf("\r\n%s", SMS_Service.SMSAtSend);
    rt_hw_gsm_output( ( char * ) SMS_Service.SMSAtSend );

    rt_thread_delay(50);

    pstrTemp = rt_malloc(150);
    memset(pstrTemp, 0, 150);
    len = strlen(s);
    memcpy(pstrTemp, s, len);
    pstrTemp[len++] = 0x1A;    // message  end

    ///���͵�����Ϣ
    rt_device_write( &dev_vuart, 0, pstrTemp, len);
    ///���͵�GSMģ��
    rt_hw_gsm_output_Data(pstrTemp, len);
    rt_free( pstrTemp );
    pstrTemp = RT_NULL;
    return 1;
}
//FINSH_FUNCTION_EXPORT(SMS_Tx_Text, SMS_Tx_Text);



u8 SMS_Tx_PDU(char *strDestNum, char *s)
{
    u16 len;
    u16 i;
    char *pstrTemp;
    memset(SMS_Service.SMSAtSend, 0, sizeof(SMS_Service.SMSAtSend));
    pstrTemp = rt_malloc(400);
    memset(pstrTemp, 0, 400);
    i = 0;
    SetPhoneNumToPDU(SMS_Service.Sms_Info.TPA, strDestNum, sizeof(SMS_Service.Sms_Info.TPA));
    len = AnySmsEncode_NoCenter(SMS_Service.Sms_Info.TPA, GSM_UCS2, s, strlen(s), pstrTemp);
    //len=strlen(pstrTemp);
    pstrTemp[len++] = 0x1A;    // message  end
    //////
    sprintf( ( char * ) SMS_Service.SMSAtSend, "AT+CMGS=%d\r\n", (len - 2) / 2);
    rt_kprintf("%s", SMS_Service.SMSAtSend);
    rt_hw_gsm_output( ( char * ) SMS_Service.SMSAtSend );
    rt_thread_delay(50);
    //////
    rt_device_write( &dev_vuart, 0, pstrTemp, strlen(pstrTemp) );
    //rt_hw_console_output(pstrTemp);
    rt_hw_gsm_output_Data(pstrTemp, len);
    rt_free( pstrTemp );
    pstrTemp = RT_NULL;
    return 1;
}
//FINSH_FUNCTION_EXPORT(SMS_Tx_PDU, SMS_Tx_PDU);

#endif

/*********************************************************************************
*��������:u8 SMS_Rx_Notice(u16 indexNum)
*��������:ģ���յ��¶���֪ͨ
*��    ��:�¶��ŵ�������
*��    ��:none
*�� �� ֵ:	1:������ɣ�
			0:��ʾʧ��
*��    ��:������
*��������:2013-05-29
*---------------------------------------------------------------------------------
*�� �� ��:
*�޸�����:
*�޸�����:
*********************************************************************************/
u8 SMS_Rx_Notice(u16 indexNum)
{
    SMS_Service.SMIndex = indexNum;
    if(GB19056.workstate)
        rt_kprintf( " index=%d", SMS_Service.SMIndex );
    SMS_Service.SMS_read		= 3;
    SMS_Service.SMS_waitCounter = 1;
    return 1;
}

