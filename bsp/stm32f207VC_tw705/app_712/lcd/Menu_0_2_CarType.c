#include  <string.h>
#include "Menu_Include.h"


struct IMG_DEF test_scr_CarType = {12, 12, test_00};

unsigned char CarType_counter = 0;
unsigned char CarType_Type = 0;


void CarType(unsigned char type_Sle, unsigned char sel)
{
    switch(type_Sle)
    {
    case 1:
        lcd_fill(0);
        if(sel == 0)
        {
            lcd_text12(24, 3, "��������ѡ��", 12, LCD_MODE_SET);
            lcd_text12(0, 19, "1.  ���˳�", 10, LCD_MODE_SET);
        }
        else
            lcd_text12(12, 10, "��������:���˳�", 15, LCD_MODE_SET);
        lcd_update_all();
        break;
    case 2:
        lcd_fill(0);
        if(sel == 0)
        {
            lcd_text12(24, 3, "��������ѡ��", 12, LCD_MODE_SET);
            lcd_text12(0, 19, "2.  ���˳�", 10, LCD_MODE_SET);
        }
        else
            lcd_text12(12, 10, "��������:���˳�", 15, LCD_MODE_SET);
        lcd_update_all();
        break;
    case 3:
        lcd_fill(0);
        if(sel == 0)
        {
            lcd_text12(24, 3, "��������ѡ��", 12, LCD_MODE_SET);
            lcd_text12(0, 19, "3.  ΣƷ��", 10, LCD_MODE_SET);
        }
        else
            lcd_text12(12, 10, "��������:ΣƷ��", 15, LCD_MODE_SET);
        lcd_update_all();
        break;
    case 4:
        lcd_fill(0);
        if(sel == 0)
        {
            lcd_text12(24, 3, "��������ѡ��", 12, LCD_MODE_SET);
            lcd_text12(0, 19, "4.  ���ͳ�", 10, LCD_MODE_SET);
        }
        else
            lcd_text12(12, 10, "��������:���ͳ�", 15, LCD_MODE_SET);
        lcd_update_all();
        break;
    case 5:
        lcd_fill(0);
        if(sel == 0)
        {
            lcd_text12(24, 3, "��������ѡ��", 12, LCD_MODE_SET);
            lcd_text12(0, 19, "5.  ���ͳ�", 10, LCD_MODE_SET);
        }
        else
            lcd_text12(12, 10, "��������:���ͳ�", 15, LCD_MODE_SET);
        lcd_update_all();
        break;
    case 6:
        lcd_fill(0);
        if(sel == 0)
        {
            lcd_text12(24, 3, "��������ѡ��", 12, LCD_MODE_SET);
            lcd_text12(0, 19, "6.  С�ͳ�", 10, LCD_MODE_SET);
        }
        else
            lcd_text12(12, 10, "��������:С�ͳ�", 15, LCD_MODE_SET);
        lcd_update_all();
        break;
    case 7:
        lcd_fill(0);
        if(sel == 0)
        {
            lcd_text12(24, 3, "��������ѡ��", 12, LCD_MODE_SET);
            lcd_text12(0, 19, "7.  ΢�ͳ�", 10, LCD_MODE_SET);
        }
        else
            lcd_text12(12, 10, "��������:΢�ͳ�", 15, LCD_MODE_SET);
        lcd_update_all();
        break;
    case 8:
        lcd_fill(0);
        if(sel == 0)
        {
            lcd_text12(24, 3, "��������ѡ��", 12, LCD_MODE_SET);
            lcd_text12(0, 19, "8.  ���⳵", 10, LCD_MODE_SET);
        }
        else
            lcd_text12(12, 10, "��������:���⳵", 15, LCD_MODE_SET);
        lcd_update_all();
        break;
    }
}

static void msg( void *p)
{

}
static void show(void)
{

    MenuIdle_working = 0; //clear

    CounterBack = 0;
    lcd_fill(0);
    lcd_text12(24, 3, "��������ѡ��", 12, LCD_MODE_SET);
    lcd_text12(0, 19, "��ȷ�ϼ�ѡ��������", 20, LCD_MODE_SET);
    lcd_update_all();

    CarType_counter = 1;
    CarType_Type = 1;

    CarType(CarType_counter, 0);
    //--printf("\r\n��������ѡ�� = %d",CarType_counter);
}


static void keypress(unsigned int key)
{
    switch(KeyValue)
    {
    case KeyValueMenu:

        pMenuItem = &Menu_0_loggingin;
        pMenuItem->show();
        CarType_counter = 0;
        CarType_Type = 0;
        break;
    case KeyValueOk:
        if(CarType_Type == 1)
        {
            CarType_Type = 2;
            CarType(CarType_counter, 1);
            //printf("\r\nCarType_Type = %d",CarType_Type);
        }
        else if(CarType_Type == 2)
        {
            //д�복������
            if((CarType_counter >= 1) && (CarType_counter <= 8))
                memset(Menu_VechileType, 0, sizeof(Menu_VechileType));

            if(CarType_counter == 1)
                memcpy(Menu_VechileType, "���˳�", 6);
            else if(CarType_counter == 2)
                memcpy(Menu_VechileType, "���˳�", 6);
            else if(CarType_counter == 3)
                memcpy(Menu_VechileType, "ΣƷ��", 6);
            else if(CarType_counter == 4)
                memcpy(Menu_VechileType, "���ͳ�", 6);
            else if(CarType_counter == 5)
                memcpy(Menu_VechileType, "���ͳ�", 6);
            else if(CarType_counter == 6)
                memcpy(Menu_VechileType, "С�ͳ�", 6);
            else if(CarType_counter == 7)
                memcpy(Menu_VechileType, "΢�ͳ�", 6);
            else if(CarType_counter == 8)
                memcpy(Menu_VechileType, "���⳵", 6);

            CarType_Type = 3;
            // ��������
            if(MENU_set_carinfor_flag == 1)
            {
                //rt_kprintf("\r\n��������������ɣ����˵������أ�%s",Menu_VechileType);
                memset(Vechicle_Info.Vech_Type, 0, sizeof(Vechicle_Info.Vech_Type));
                memcpy(Vechicle_Info.Vech_Type, Menu_VechileType, 10);
                DF_WriteFlashSector(DF_Vehicle_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
                WatchDog_Feed();
                DF_WriteFlashSector(DF_VehicleBAK_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
                WatchDog_Feed();
                DF_WriteFlashSector(DF_VehicleBAK2_Struct_offset, 0, (u8 *)&Vechicle_Info, sizeof(Vechicle_Info));
            }
            lcd_fill(0);
            lcd_text12(12, 3, "��������ѡ�����", 16, LCD_MODE_SET);
            lcd_text12(6, 18, "��ȷ�ϼ�������һ��", 18, LCD_MODE_SET);
            lcd_update_all();
        }
        else if(CarType_Type == 3)
        {
            CarSet_0_counter = 4; //���õ�4��
            pMenuItem = &Menu_0_loggingin;
            pMenuItem->show();

            CarType_counter = 0;
            CarType_Type = 0;
        }
        break;
    case KeyValueUP:
        if(	CarType_Type == 1)
        {
            if(CarType_counter == 1)
                CarType_counter = 8;
            else
                CarType_counter--;
            //printf("\r\n  up  ��������ѡ�� = %d",CarType_counter);
            CarType(CarType_counter, 0);
        }
        break;
    case KeyValueDown:
        if(	CarType_Type == 1)
        {
            if(CarType_counter >= 8)
                CarType_counter = 1;
            else
                CarType_counter++;

            //printf("\r\n down ��������ѡ�� = %d",CarType_counter);
            CarType(CarType_counter, 0);
        }
        break;
    }
    KeyValue = 0;
}


static void timetick(unsigned int systick)
{

}


MENUITEM	Menu_0_2_CarType =
{
    "������������",
    12,
    &show,
    &keypress,
    &timetick,
    &msg,
    (void *)0
};

