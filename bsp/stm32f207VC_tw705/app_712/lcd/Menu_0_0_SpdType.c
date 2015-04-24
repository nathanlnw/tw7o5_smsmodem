#include  <string.h>
#include "Menu_Include.h"


struct IMG_DEF test_scr_spdType = {12, 12, test_00};

u8  Screen_SpdType = 0;
u8  SpdType_flag = 0; //   1:  gps  �ٶ�  2:   �������ٶ�
static void msg( void *p)
{

}
static void show(void)
{
    MenuIdle_working = 0; //clear

    CounterBack = 0;

    SpdType_flag = 1;
    JT808Conf_struct.Speed_GetType = 0; //gps �ٶ�
    lcd_fill(0);
    lcd_text12(24, 3, "�����ٶ�ѡ��", 12, LCD_MODE_SET);
    lcd_text12(0, 19, "GPS�ٶ�   �������ٶ�", 20, LCD_MODE_SET);
    lcd_text12(0, 19, "GPS�ٶ�", 7, LCD_MODE_INVERT);
    lcd_update_all();


}


static void keypress(unsigned int key)
{
    switch(KeyValue)
    {
    case KeyValueMenu:
        pMenuItem = &Menu_0_loggingin;
        pMenuItem->show();

        Screen_SpdType = 0;
        SpdType_flag = 0;
        break;
    case KeyValueOk:
        if(SpdType_flag == 1)
        {
            SpdType_flag = 3;
            Screen_SpdType = 1;

            lcd_fill(0);
            lcd_text12(10, 10, "�ٶ�ѡ��: GPS�ٶ�", 17, LCD_MODE_SET);
            lcd_update_all();
            JT808Conf_struct.Speed_GetType = 0;
            // ��������
            if(MENU_set_carinfor_flag == 1)
                spd_type(0);
        }
        else if(SpdType_flag == 2)
        {
            SpdType_flag = 3;
            Screen_SpdType = 1;

            lcd_fill(0);
            lcd_text12(0, 10, "�ٶ�ѡ��: �������ٶ�", 20, LCD_MODE_SET);
            lcd_update_all();
            JT808Conf_struct.Speed_GetType = 1;
            // ��������
            if(MENU_set_carinfor_flag == 1)
                spd_type(1);
        }
        else if(SpdType_flag == 3)
        {
            CarSet_0_counter = 2; //
            pMenuItem = &Menu_0_loggingin;
            pMenuItem->show();

            SpdType_flag = 0;
            Screen_SpdType = 0;
        }
        break;
    case KeyValueUP:
        if(Screen_SpdType == 0)
        {
            SpdType_flag = 1;
            lcd_fill(0);
            lcd_text12(24, 3, "�����ٶ�ѡ��", 12, LCD_MODE_SET);
            lcd_text12(0, 19, "GPS�ٶ�   �������ٶ�", 20, LCD_MODE_SET);
            lcd_text12(0, 19, "GPS�ٶ�", 7, LCD_MODE_INVERT);
            lcd_update_all();
        }
        break;
    case KeyValueDown:
        if(Screen_SpdType == 0)
        {
            SpdType_flag = 2;
            lcd_fill(0);
            lcd_text12(24, 3, "�����ٶ�ѡ��", 12, LCD_MODE_SET);
            lcd_text12( 0, 19, "GPS�ٶ�   �������ٶ�", 20, LCD_MODE_SET);
            lcd_text12(60, 19, "�������ٶ�", 10, LCD_MODE_INVERT);
            lcd_update_all();
        }
        break;
    }
    KeyValue = 0;
}


static void timetick(unsigned int systick)
{

}


MENUITEM	Menu_0_0_SpdType =
{
    "�豸�ٶ�ѡ��",
    12,
    &show,
    &keypress,
    &timetick,
    &msg,
    (void *)0
};


