#include "Menu_Include.h"
#include "App_moduleConfig.h"

u8 Dis_screen_6_flag = 0;

static void msg( void *p)
{

}
static void show(void)
{
    MenuIdle_working = 0; //clear

    if(Dis_screen_6_flag == 0)
    {
        Dis_screen_6_flag = 1;
        lcd_fill(0);
        lcd_text12(0, 3, "����", 4, LCD_MODE_SET);
        lcd_text12(0, 17, "����", 4, LCD_MODE_SET);
        lcd_text12(42, 3, "������Ϣ����", 12, LCD_MODE_SET);
        lcd_text12(27, 17, "�밴ȷ����������", 16, LCD_MODE_SET);
        lcd_update_all();
    }
}


static void keypress(unsigned int key)
{
    switch(KeyValue)
    {
    case KeyValueMenu:
        //�����˳�
        Password_correctFlag = 1;
        Exit_to_Idle = 1;

        Dis_screen_6_flag = 0;

        pMenuItem = &Menu_1_menu;
        pMenuItem->show();
        break;
    case KeyValueOk:
        if(Dis_screen_6_flag == 1)
        {
            //���õ������Ϣ��־
            MENU_set_carinfor_flag = 1;

            Dis_screen_6_flag = 0;
            pMenuItem = &Menu_0_0_password;
            pMenuItem->show();
        }
        break;
    case KeyValueUP:

        break;
    case KeyValueDown:

        break;
    }
    KeyValue = 0;
}

static void timetick(unsigned int systick)
{
}

ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_6_SetInfor =
{
    "������Ϣ����",
    12,
    &show,
    &keypress,
    &timetick,
    &msg,
    (void *)0
};

