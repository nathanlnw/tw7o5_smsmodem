#include "Menu_Include.h"
#include <string.h>


#define  DIS_Dur_width_inter 10


unsigned char noselect_recorder[] = {0x3C, 0x7E, 0xC3, 0xC3, 0xC3, 0xC3, 0x7E, 0x3C}; //����
unsigned char select_recorder[] = {0x3C, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x3C}; //ʵ��
DECL_BMP(8, 8, select_recorder);
DECL_BMP(8, 8, noselect_recorder);


static unsigned char menu_pos = 0;

static PMENUITEM psubmenu[8] =
{
    &Menu_4_1_pilao,     //��ʱ��ʻ��¼
    &Menu_5_5_CarInfor,  //������Ϣ
    &Menu_5_4_ICcard,    //��ʻԱ��Ϣ
    &Menu_5_6_speedlog,  //�ٶ���־��Ϣ
    &Menu_5_2_ExportData,//��¼�����ݵ���
    &Menu_5_1_PulseCoefficient,//��������
    &Menu_5_3_print,     //��ӡ���
    &Menu_2_6_Mileage,   //��̲鿴
};


static void menuswitch(void)
{
    unsigned char i = 0;

    lcd_fill(0);
    lcd_text12(0, 3, "��¼��", 6, LCD_MODE_SET);
    lcd_text12(6, 17, "��Ϣ", 4, LCD_MODE_SET);
    for(i = 0; i < 8; i++)
        lcd_bitmap(40 + i * DIS_Dur_width_inter, 5, &BMP_noselect_recorder, LCD_MODE_SET);
    lcd_bitmap(40 + menu_pos * DIS_Dur_width_inter, 5, &BMP_select_recorder, LCD_MODE_SET);
    lcd_text12(37, 19, (char *)(psubmenu[menu_pos]->caption), psubmenu[menu_pos]->len, LCD_MODE_SET);
    lcd_update_all();
}


static void msg( void *p)
{
}
static void show(void)
{
    MenuIdle_working = 0; //clear

    menu_pos = 0;
    menuswitch();
}



static void keypress(unsigned int key)
{
    switch(KeyValue)
    {
    case KeyValueMenu:
        Mileage_02_05_flag = 0;
        CounterBack = 0;
        pMenuItem = &Menu_1_menu; //
        pMenuItem->show();
        break;
    case KeyValueOk:
        if(menu_pos == 7)
            Mileage_02_05_flag = 1; //��ת�����
        else if(menu_pos)
            Mileage_02_05_flag = 2; //��ת����ʱ��ʻ
        pMenuItem = psubmenu[menu_pos]; //
        pMenuItem->show();
        break;
    case KeyValueUP:
        if(menu_pos == 0)
            menu_pos = 7;
        else
            menu_pos--;
        menuswitch();
        break;
    case KeyValueDown:
        menu_pos++;
        if(menu_pos > 7)
            menu_pos = 0;
        menuswitch();
        break;
    }
    KeyValue = 0;
}




static void timetick(unsigned int systick)
{

    CounterBack++;
    if(CounterBack != MaxBankIdleTime)
        return;
    pMenuItem = &Menu_1_Idle;
    pMenuItem->show();
    CounterBack = 0;

}

ALIGN(RT_ALIGN_SIZE)
MENUITEM	Menu_5_recorder =
{
    "��¼����Ϣ",
    10,
    &show,
    &keypress,
    &timetick,
    &msg,
    (void *)0
};


