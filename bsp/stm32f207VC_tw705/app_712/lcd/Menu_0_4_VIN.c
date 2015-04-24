#include  <string.h>
#include "Menu_Include.h"

#define  VIN_width1  6


static u8 Vin_Modify_Flag = 0;

static u8 VIN_SetFlag = 1, VIN_SetFlag_temp = 1;
static u8 VIN_SetCounter_0 = 0, VIN_SetCounter_1 = 0, VIN_SetCounter_2 = 0;


unsigned char select_vin[] = {0x0C, 0x06, 0xFF, 0x06, 0x0C};
unsigned char ABC_0_9[10][1] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
unsigned char ABC_A_M[13][1] = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M"};
unsigned char ABC_N_Z[13][1] = {"N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"};

unsigned char VIN_Type_flag = 0; //�������ѡ�������ѡ��
unsigned char VIN_Type_Counter = 0; //  0: ����    1:A-M         2:N-Z

DECL_BMP(8, 5, select_vin);

/*=============================================
invert_last=0   ������ʾ
invert_last=1   ����ѡ��׼���޸�
invert_last=2   ����ѡ��׼���޸�, ����
==============================================*/
void VIN_Type_Sel( u8 par, u8 invert_last)
{
    lcd_fill(0);
    lcd_text12(0, 0, (char *)Menu_Vin_Code, VIN_SetFlag - 1, LCD_MODE_SET);
    if(invert_last == 1)
    {
        //rt_kprintf("\r\n(1)����ѡ��VIN_SetFlag_temp=%d",VIN_SetFlag_temp);
        lcd_text12((VIN_SetFlag_temp - 2) * 6, 0, (char *)&Menu_Vin_Code[VIN_SetFlag_temp - 2], 1, LCD_MODE_INVERT);
    }
    else
    {
        if(invert_last == 2)
        {
            //rt_kprintf("\r\n(2)����ѡ��VIN_SetFlag_temp=%d",VIN_SetFlag_temp);
            lcd_text12((VIN_SetFlag_temp - 2) * 6, 0, (char *)&Menu_Vin_Code[VIN_SetFlag_temp - 2], 1, LCD_MODE_INVERT);
        }
        lcd_text12(20, 11, "0-9  A-M  N-Z", 13, LCD_MODE_SET);
        if(par == 0)
        {
            lcd_text12(20, 11, "0-9", 3, LCD_MODE_INVERT);
            lcd_text12(20, 22, "0123456789", 10, LCD_MODE_SET);
        }
        else if(par == 1)
        {
            lcd_text12(20 + 5 * 6, 11, "A-M", 3, LCD_MODE_INVERT);
            lcd_text12(20, 22, "ABCDEFGHIJKLM", 13, LCD_MODE_SET);
        }
        else if(par == 2)
        {
            lcd_text12(20 + 10 * 6, 11, "N-Z", 3, LCD_MODE_INVERT);
            lcd_text12(20, 22, "NOPQRSTUVWXYZ", 13, LCD_MODE_SET);
        }
    }
    lcd_update_all();
}
void VIN_Set(u8 par, u8 type1_2)
{
    lcd_fill(0);
    lcd_text12(0, 0, (char *)Menu_Vin_Code, VIN_SetFlag - 1, LCD_MODE_SET);
    if(Vin_Modify_Flag == 3)
    {
        //rt_kprintf("\r\n(����)����ѡ��VIN_SetFlag_temp=%d",VIN_SetFlag_temp);
        lcd_text12((VIN_SetFlag_temp - 2) * 6, 0, (char *)&Menu_Vin_Code[VIN_SetFlag_temp - 2], 1, LCD_MODE_INVERT);
    }
    if(type1_2 == 1)
    {
        lcd_bitmap(par * VIN_width1, 14, &BMP_select_vin, LCD_MODE_SET);
        lcd_text12(0, 19, "0123456789", 10, LCD_MODE_SET);
    }
    else if(type1_2 == 2)
    {
        lcd_bitmap(par * VIN_width1, 14, &BMP_select_vin, LCD_MODE_SET);
        lcd_text12(0, 19, "ABCDEFGHIJKLM", 13, LCD_MODE_SET);
    }
    else if(type1_2 == 3)
    {
        lcd_bitmap(par * VIN_width1, 14, &BMP_select_vin, LCD_MODE_SET);
        lcd_text12(0, 19, "NOPQRSTUVWXYZ", 13, LCD_MODE_SET);
    }
    lcd_update_all();
}

static void msg( void *p)
{

}
static void show(void)
{
    CounterBack = 0;
    MenuIdle_working = 0; //clear



    VIN_Type_Counter = 0;
    VIN_Type_Sel(VIN_Type_Counter, 0);
    VIN_Type_flag = 1;
    //rt_kprintf("\r\nѡ��Ҫ���������");
}


static void keypress(unsigned int key)
{
    switch(KeyValue)
    {
    case KeyValueMenu:
        if(VIN_Type_flag == 2) //�����˳�����ѡ��
        {
            VIN_Type_flag = 1;
            if(VIN_SetFlag == 1) //��Ҫ�˳�����������ѡ��û����Ҫ��ɾ������
                Vin_Modify_Flag = 1;
            VIN_Type_Sel(VIN_Type_Counter, 0);
        }
        else if((VIN_Type_flag == 1) && (Vin_Modify_Flag == 0)) //����ѡ�����һ��
        {

            //���˵��˳������Ϸ����·�ѡ��Ҫ�޸ĵ�λ��
            //rt_kprintf("\r\n  �� �˵� �޸�����");
            if((VIN_SetFlag > 1) && (VIN_SetFlag < 19))
            {
                Vin_Modify_Flag = 1;

                VIN_SetFlag_temp = VIN_SetFlag;

                VIN_Type_Sel(VIN_Type_Counter, 1);
            }
            else
            {
                memset(Menu_Vin_Code, 0, sizeof(Menu_Vin_Code));
                VIN_SetFlag = 1;
                VIN_SetFlag_temp = 1;
                Vin_Modify_Flag = 0;

                VIN_SetCounter_0 = 0;
                VIN_SetCounter_1 = 0;
                VIN_SetCounter_2 = 0;



                pMenuItem = &Menu_0_loggingin;
                pMenuItem->show();
            }
        }
        else if(Vin_Modify_Flag == 1)
        {
            memset(Menu_Vin_Code, 0, sizeof(Menu_Vin_Code));
            VIN_SetFlag = 1;
            VIN_SetFlag_temp = 1;
            Vin_Modify_Flag = 0;

            VIN_SetCounter_0 = 0;
            VIN_SetCounter_1 = 0;
            VIN_SetCounter_2 = 0;



            pMenuItem = &Menu_0_loggingin;
            pMenuItem->show();
        }

        break;
    case KeyValueOk:
        //rt_kprintf("\r\n (3 �޸�/ 0���)  Vin_Modify_Flag=%d ",Vin_Modify_Flag);
        if(Vin_Modify_Flag == 1)
        {
            Vin_Modify_Flag = 2;

            VIN_Type_Sel(VIN_Type_Counter, 2);
            //rt_kprintf("\r\n �޸���ʾ��  Vin_Modify_Flag=%d ",Vin_Modify_Flag);
        }
        else if(Vin_Modify_Flag == 2)
        {
            VIN_Type_flag = 2; //��������
            Vin_Modify_Flag = 3;
            //------�滻Ҫ�޸ĵ��ַ�---------------------------------
            if((VIN_SetFlag >= 1) && (VIN_SetFlag <= 17))
            {
                if(VIN_Type_Counter == 0)
                    VIN_Set(VIN_SetCounter_0, 1);
                else if(VIN_Type_Counter == 1)
                    VIN_Set(VIN_SetCounter_1, 2);
                else if(VIN_Type_Counter == 2)
                    VIN_Set(VIN_SetCounter_2, 3);
                //rt_kprintf("\r\n VIN_SetFlag_temp=%d Menu_Vin_Code=%s",VIN_SetFlag_temp,Menu_Vin_Code);
            }
        }
        else
        {
            //����
            if(VIN_Type_flag == 1)
            {
                VIN_Type_flag = 2;

                if((VIN_SetFlag >= 1) && (VIN_SetFlag <= 17))
                {
                    if(VIN_Type_Counter == 0)
                        VIN_Set(VIN_SetCounter_0, 1);
                    else if(VIN_Type_Counter == 1)
                        VIN_Set(VIN_SetCounter_1, 2);
                    else if(VIN_Type_Counter == 2)
                        VIN_Set(VIN_SetCounter_2, 3);
                    //rt_kprintf("\r\n��%d��,���ú�%dλ",VIN_Type_Counter,VIN_SetFlag-1);
                }
            }
            //����
            else if(VIN_Type_flag == 2)
            {
                VIN_Type_flag = 3;
                //---------------------------------------
                if((VIN_SetFlag >= 1) && (VIN_SetFlag <= 17))
                {
                    if(VIN_Type_Counter == 0)
                    {
                        if(Vin_Modify_Flag == 3)
                        {
                            Menu_Vin_Code[VIN_SetFlag_temp - 2] = ABC_0_9[VIN_SetCounter_0][0];
                            Vin_Modify_Flag = 0;
                        }
                        else
                        {
                            Menu_Vin_Code[VIN_SetFlag - 1] = ABC_0_9[VIN_SetCounter_0][0];
                            VIN_SetFlag++;
                        }
                        VIN_Set(VIN_SetCounter_0, 1);
                    }
                    else if(VIN_Type_Counter == 1)
                    {
                        if(Vin_Modify_Flag == 3)
                        {
                            Menu_Vin_Code[VIN_SetFlag_temp - 2] = ABC_A_M[VIN_SetCounter_1][0];
                            Vin_Modify_Flag = 0;
                        }
                        else
                        {
                            Menu_Vin_Code[VIN_SetFlag - 1] = ABC_A_M[VIN_SetCounter_1][0];
                            VIN_SetFlag++;
                        }
                        VIN_Set(VIN_SetCounter_1, 2);
                    }
                    else if(VIN_Type_Counter == 2)
                    {
                        if(Vin_Modify_Flag == 3)
                        {
                            Menu_Vin_Code[VIN_SetFlag_temp - 2] = ABC_N_Z[VIN_SetCounter_2][0];
                            Vin_Modify_Flag = 0;
                        }
                        else
                        {
                            Menu_Vin_Code[VIN_SetFlag - 1] = ABC_N_Z[VIN_SetCounter_2][0];
                            VIN_SetFlag++;
                        }
                        VIN_Set(VIN_SetCounter_2, 3);
                    }
                }
                //------------------------------------
                if(VIN_Type_flag == 3)
                {
                    VIN_Type_flag = 1;
                    VIN_SetCounter_0 = 0;
                    VIN_SetCounter_1 = 0;
                    VIN_SetCounter_2 = 0;

                    VIN_Type_Sel(VIN_Type_Counter, 0);
                    //rt_kprintf("\r\n����ѡ��(1_2_3)=%d",VIN_Type_Counter);
                }
            }

            if(VIN_SetFlag == 18) //�������
            {
                VIN_SetFlag = 19;
                if(MENU_set_carinfor_flag == 1)
                {
                    //����VIN
                    memset(Vechicle_Info.Vech_VIN, 0, sizeof(Vechicle_Info.Vech_VIN));
                    memcpy(Vechicle_Info.Vech_VIN, Menu_Vin_Code, 17);
                    DF_WriteFlashSector(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));

                    WatchDog_Feed();
                    DF_WriteFlashSector(DF_VehicleBAK_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
                    WatchDog_Feed();
                    DF_WriteFlashSector(DF_VehicleBAK2_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));

                    //rt_kprintf("\r\nVIN ������ɣ����˵������أ�%s",Menu_Vin_Code);
                }
                lcd_fill(0);
                lcd_text12(0, 5, (char *)Menu_Vin_Code, 17, LCD_MODE_SET);
                lcd_text12(25, 19, "VIN�������", 11, LCD_MODE_SET);
                lcd_update_all();
            }
            else if(VIN_SetFlag == 19)
            {
                VIN_SetFlag = 1;
                CarSet_0_counter = 6;

                pMenuItem = &Menu_0_loggingin;
                pMenuItem->show();
            }
        }

        break;

    case KeyValueUP:
        if(Vin_Modify_Flag == 1)
        {
            if(VIN_SetFlag_temp > 2)
                VIN_SetFlag_temp--;
            VIN_Type_Sel(0, 1);
            //rt_kprintf("\r\nVIN_SetFlag_temp=%d",VIN_SetFlag_temp);
        }
        else
        {
            if((VIN_Type_flag == 1) || (Vin_Modify_Flag == 2)) //ѡ����0-9  A-M  N-Z
            {
                //
                if(Vin_Modify_Flag == 2)
                    Vin_Modify_Flag = 3;

                if(VIN_Type_Counter == 0)
                    VIN_Type_Counter = 2;
                else if(VIN_Type_Counter >= 1)
                    VIN_Type_Counter--;

                if(Vin_Modify_Flag == 3) //������ʾʱ���˲˵���
                {
                    VIN_Type_Sel(VIN_Type_Counter, 2);
                }
                else
                    VIN_Type_Sel(VIN_Type_Counter, 0);
                //rt_kprintf("\r\n(  up)VIN_Type_Counter=%d",VIN_Type_Counter);
            }
            else if((VIN_Type_flag == 2) || (Vin_Modify_Flag == 3)) //����ѡ��
            {
                if((VIN_SetFlag >= 1) && (VIN_SetFlag <= 17))
                {
                    if(VIN_Type_Counter == 0)
                    {
                        if(VIN_SetCounter_0 == 0)
                            VIN_SetCounter_0 = 9;
                        else
                            VIN_SetCounter_0--;

                        VIN_Set(VIN_SetCounter_0, 1);
                    }
                    else if(VIN_Type_Counter == 1)
                    {
                        if(VIN_SetCounter_1 == 0)
                            VIN_SetCounter_1 = 12;
                        else
                            VIN_SetCounter_1--;

                        VIN_Set(VIN_SetCounter_1, 2);
                    }
                    else if(VIN_Type_Counter == 2)
                    {
                        if(VIN_SetCounter_2 == 0)
                            VIN_SetCounter_2 = 12;
                        else
                            VIN_SetCounter_2--;

                        VIN_Set(VIN_SetCounter_2, 3);
                    }
                }
            }
        }
        break;

    case KeyValueDown:
        if(Vin_Modify_Flag == 1)
        {
            if(VIN_SetFlag_temp < VIN_SetFlag)
                VIN_SetFlag_temp++;
            VIN_Type_Sel(0, 1);
        }
        else
        {
            if((VIN_Type_flag == 1) || (Vin_Modify_Flag == 2)) //ѡ����0-9  A-M  N-Z//ѡ����0-9  A-M  N-Z
            {
                if(Vin_Modify_Flag == 2)
                    Vin_Modify_Flag = 3;

                VIN_Type_Counter++;
                if(VIN_Type_Counter > 2)
                    VIN_Type_Counter = 0;

                if(Vin_Modify_Flag == 3) //������ʾʱ���˲˵���
                {
                    VIN_Type_Sel(VIN_Type_Counter, 2);
                }
                else
                    VIN_Type_Sel(VIN_Type_Counter, 0);
                //rt_kprintf("\r\n(down)VIN_Type_Counter=%d",VIN_Type_Counter);
            }
            else if(VIN_Type_flag == 2) //����ѡ��
            {
                if((VIN_SetFlag >= 1) && (VIN_SetFlag <= 17))
                {
                    if(VIN_Type_Counter == 0)
                    {
                        VIN_SetCounter_0++;
                        if(VIN_SetCounter_0 > 9)
                            VIN_SetCounter_0 = 0;

                        VIN_Set(VIN_SetCounter_0, 1);
                    }
                    else if(VIN_Type_Counter == 1)
                    {
                        VIN_SetCounter_1++;
                        if(VIN_SetCounter_1 > 12)
                            VIN_SetCounter_1 = 0;

                        VIN_Set(VIN_SetCounter_1, 2);
                    }
                    else if(VIN_Type_Counter == 2)
                    {
                        VIN_SetCounter_2++;
                        if(VIN_SetCounter_2 > 12)
                            VIN_SetCounter_2 = 0;

                        VIN_Set(VIN_SetCounter_2, 3);
                    }
                }
            }
        }
        break;

    }
    KeyValue = 0;
}


static void timetick(unsigned int systick)
{

}


ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_0_4_vin =
{
    "VIN����",
    7,
    &show,
    &keypress,
    &timetick,
    &msg,
    (void *)0
};


