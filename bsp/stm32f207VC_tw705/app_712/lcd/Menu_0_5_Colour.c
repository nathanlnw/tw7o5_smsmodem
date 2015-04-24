#include  <string.h>
#include "Menu_Include.h"

u8 comfirmation_flag = 0;
u8 col_screen = 0;
u8 CarBrandCol_Cou = 1;

unsigned char car_col[13] = {"������ɫ:��ɫ"};

void car_col_fun(u8 par)
{
    //������ɫ�����
    if(par == 1)
        memcpy(Menu_VecLogoColor, "��ɫ", 4);   //   1
    else if(par == 2)
        memcpy(Menu_VecLogoColor, "��ɫ", 4);  //   2
    else if(par == 3)
        memcpy(Menu_VecLogoColor, "��ɫ", 4);   //   3
    else if(par == 4)
        memcpy(Menu_VecLogoColor, "��ɫ", 4);  //   4
    else if(par == 5)
    {
        memcpy(Menu_VecLogoColor, "����", 4);     //   9
        par = 9;
    }

    Menu_color_num = par;

    memcpy(car_col + 9, Menu_VecLogoColor, 4);
    lcd_fill(0);
    lcd_text12(20, 10, (char *)car_col, 13, LCD_MODE_SET);
    lcd_update_all();
}
static void msg( void *p)
{

}
static void show(void)
{
    CounterBack = 0;
    MenuIdle_working = 0; //clear


    col_screen = 1;
    car_col_fun(2);// old 1
}


static void keypress(unsigned int key)
{
    switch(KeyValue)
    {
    case KeyValueMenu:
        if(comfirmation_flag == 4)
        {
            pMenuItem = &Menu_1_Idle;
            pMenuItem->show();
        }
        else
        {
            pMenuItem = &Menu_0_loggingin;
            pMenuItem->show();
        }
        col_screen = 0;
        CarBrandCol_Cou = 1;
        comfirmation_flag = 0;
        break;
    case KeyValueOk:
        if(col_screen == 1)
        {
            if(MENU_set_carinfor_flag == 1)
            {
                comfirmation_flag = 0;
                lcd_fill(0);
                lcd_text12(12, 3, "������ɫ�������", 16, LCD_MODE_SET);
                lcd_text12(24, 18, "���˵�������", 12, LCD_MODE_SET);
                lcd_update_all();
                rt_kprintf("\r\n������ɫ������ɣ����˵������أ�%d", Vechicle_Info.Dev_Color);
                // ������ɫ
                if(License_Not_SetEnable == 1)
                    Vechicle_Info.Dev_Color = 0;
                else
                    Vechicle_Info.Dev_Color = Menu_color_num;
                DF_WriteFlashSector(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
            }
            else
            {
                col_screen = 2;
                CarSet_0_counter = 0; //
                menu_color_flag = 1; //������ɫ�������
                lcd_fill(0);
                lcd_text12(20, 3, (char *)car_col, 13, LCD_MODE_SET);
                lcd_text12(12, 18, "��ȷ�ϼ��鿴��Ϣ", 16, LCD_MODE_SET);
                lcd_update_all();
            }
        }
        else if(col_screen == 2)
        {
            menu_color_flag = 0;

            col_screen = 3;
            comfirmation_flag = 1; //����������Ϣ��־
            lcd_fill(0);
            if(License_Not_SetEnable == 1)
                lcd_text12(0, 0, "������", 6, LCD_MODE_SET);
            else
                lcd_text12(0, 0, (char *)Menu_Car_license, 8, LCD_MODE_SET);
            lcd_text12(54, 0, (char *)Menu_VechileType, 6, LCD_MODE_SET);
            //====  ���ƺ�δ����=====
            if(License_Not_SetEnable == 1)
                lcd_text12(96, 0, (char *)"0", 1, LCD_MODE_SET);
            else
                lcd_text12(96, 0, (char *)Menu_VecLogoColor, 4, LCD_MODE_SET);

            lcd_text12(0, 12, "SIM����", 7, LCD_MODE_SET);
            lcd_text12(43, 12, (char *)Menu_sim_Code, 11, LCD_MODE_SET);
            lcd_text12(24, 23, "ȷ��", 4, LCD_MODE_INVERT);
            lcd_text12(72, 23, "ȡ��", 4, LCD_MODE_SET);
            lcd_update_all();
        }
        else if(comfirmation_flag == 1)
        {
            col_screen = 0;
            comfirmation_flag = 4;
            //�������õ���Ϣ
            lcd_fill(0);
            lcd_text12(18, 3, "������������Ϣ", 14, LCD_MODE_SET);
            lcd_text12(0, 18, "���˵��������������", 20, LCD_MODE_SET);
            lcd_update_all();

            // �����ٶ�����
            // gps�ٶ�JT808Conf_struct.Speed_GetType= 0:   �������ٶ� JT808Conf_struct.Speed_GetType=1
            spd_type(JT808Conf_struct.Speed_GetType);

            //���ƺ�
            //rt_kprintf("\r\n(������Ϣ)Menu_Car_license=%s",Menu_Car_license);
            memset(Vechicle_Info.Vech_Num, 0, sizeof(Vechicle_Info.Vech_Num));
            memcpy(Vechicle_Info.Vech_Num, Menu_Car_license, strlen((const char *)Menu_Car_license));
            //rt_kprintf("\r\n(������Ϣ)Vechicle_Info.Vech_Num=%s",Vechicle_Info.Vech_Num);
            // ��������
            memset(Vechicle_Info.Vech_Type, 0, sizeof(Vechicle_Info.Vech_Type));
            memcpy(Vechicle_Info.Vech_Type, Menu_VechileType, 10);
            //rt_kprintf("\r\n(����1   )Vechicle_Info.Vech_Num=%s",Vechicle_Info.Vech_Num);

            ///////
            //����VIN
            memset(Vechicle_Info.Vech_VIN, 0, sizeof(Vechicle_Info.Vech_VIN));
            memcpy(Vechicle_Info.Vech_VIN, Menu_Vin_Code, 17);
            //rt_kprintf("\r\n(����2   )Vechicle_Info.Vech_Num=%s",Vechicle_Info.Vech_Num);

            // SIM������
            //memset(JT808Conf_struct.Vech_sim,0,sizeof(JT808Conf_struct.Vech_sim));
            //memcpy(JT808Conf_struct.Vech_sim,Menu_sim_Code,11);
            //-----------------------------------------------------------------------------
            memset(SimID_12D, 0, sizeof(SimID_12D));
            SimID_12D[0] = '0';
            memcpy(SimID_12D + 1, Menu_sim_Code, 11);
            DF_WriteFlashSector(DF_SIMID_12D, 0, SimID_12D, 13);
            delay_ms(80);
            //rt_kprintf("\r\n ��ʾ���豸SIM_ID����Ϊ : %s", SimID_12D);
            DF_ReadFlash(DF_SIMID_12D, 0, SimID_12D, 13);
            SIMID_Convert_SIMCODE();  // ת��
            //----------------------------------------------------------------------------
            //rt_kprintf("\r\n(����3   )Vechicle_Info.Vech_Num=%s",Vechicle_Info.Vech_Num);

            // ������ɫ
            if(License_Not_SetEnable == 1)
                Vechicle_Info.Dev_Color = 0;
            else
                Vechicle_Info.Dev_Color = Menu_color_num;
            //�����������
            Login_Menu_Flag = 1;   //  �������Ϊ0
            DF_WriteFlashSector(DF_LOGIIN_Flag_offset, 0, &Login_Menu_Flag, 1);
            //  �洢
            //rt_kprintf("\r\n(����4   )Vechicle_Info.Vech_Num=%s",Vechicle_Info.Vech_Num);
            DF_WriteFlashSector(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
            WatchDog_Feed();
            DF_WriteFlashSector(DF_VehicleBAK_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
            WatchDog_Feed();
            DF_WriteFlashSector(DF_VehicleBAK2_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
            delay_ms(3);
            DF_ReadFlash(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));

        }
        else if(comfirmation_flag == 2)
        {
            col_screen = 0;
            comfirmation_flag = 3;
            lcd_fill(0);
            lcd_text12(6, 3, "��ȷ���Ƿ���������", 18, LCD_MODE_SET);
            lcd_text12(12, 18, "��ȷ�ϼ���������", 16, LCD_MODE_SET);
            lcd_update_all();
        }
        else if(comfirmation_flag == 3)
        {
            col_screen = 0;
            comfirmation_flag = 0;
            //��������
            pMenuItem = &Menu_0_loggingin;
            pMenuItem->show();

            comfirmation_flag = 0;
            col_screen = 0;
            CarBrandCol_Cou = 1;
        }

        break;
    case KeyValueUP:
        if(col_screen == 1)
        {
            CarBrandCol_Cou--;
            if(CarBrandCol_Cou < 1)
                CarBrandCol_Cou = 5;
            car_col_fun(CarBrandCol_Cou);
        }
        else if(col_screen == 3)
        {
            comfirmation_flag = 1;

            lcd_fill(0);
            if(License_Not_SetEnable == 1)
                lcd_text12(0, 0, "������", 6, LCD_MODE_SET);
            else
                lcd_text12(0, 0, (char *)Menu_Car_license, 8, LCD_MODE_SET);
            lcd_text12(54, 0, (char *)Menu_VechileType, 6, LCD_MODE_SET);
            //====  ���ƺ�δ����=====
            if(License_Not_SetEnable == 1)
                lcd_text12(96, 0, (char *)"0", 1, LCD_MODE_SET);
            else
                lcd_text12(96, 0, (char *)Menu_VecLogoColor, 4, LCD_MODE_SET);

            lcd_text12(0, 12, "SIM����", 7, LCD_MODE_SET);
            lcd_text12(43, 12, (char *)Menu_sim_Code, 11, LCD_MODE_SET);
            lcd_text12(24, 23, "ȷ��", 4, LCD_MODE_INVERT);
            lcd_text12(72, 23, "ȡ��", 4, LCD_MODE_SET);
            lcd_update_all();
        }

        break;
    case KeyValueDown:
        if(col_screen == 1)
        {
            CarBrandCol_Cou++;
            if(CarBrandCol_Cou > 5)
                CarBrandCol_Cou = 1;
            car_col_fun(CarBrandCol_Cou);
        }
        else if(col_screen == 3)
        {
            comfirmation_flag = 2;
            lcd_fill(0);
            if(License_Not_SetEnable == 1)
                lcd_text12(0, 0, "������", 6, LCD_MODE_SET);
            else
                lcd_text12(0, 0, (char *)Menu_Car_license, 8, LCD_MODE_SET);
            lcd_text12(54, 0, (char *)Menu_VechileType, 6, LCD_MODE_SET);
            //====  ���ƺ�δ����=====
            if(License_Not_SetEnable == 1)
                lcd_text12(96, 0, (char *)"0", 1, LCD_MODE_SET);
            else
                lcd_text12(96, 0, (char *)Menu_VecLogoColor, 4, LCD_MODE_SET);

            lcd_text12(0, 12, "SIM����", 7, LCD_MODE_SET);
            lcd_text12(43, 12, (char *)Menu_sim_Code, 11, LCD_MODE_SET);
            lcd_text12(24, 23, "ȷ��", 4, LCD_MODE_SET);
            lcd_text12(72, 23, "ȡ��", 4, LCD_MODE_INVERT);
            lcd_update_all();
        }

        break;
    }
    KeyValue = 0;
}


static void timetick(unsigned int systick)
{

}

ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_0_5_Colour =
{
    "������ɫ����",
    12,
    &show,
    &keypress,
    &timetick,
    &msg,
    (void *)0
};


