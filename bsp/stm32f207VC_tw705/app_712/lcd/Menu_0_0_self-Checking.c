#include  <string.h>
#include "Menu_Include.h"

u8 self_checking_screen = 0;
u16 self_checking_counter = 0; //�Լ�ʱ�����
u16 self_checking_pro = 0; //�Լ���������쳣��+1

u16 Auto_exit = 0;
static void msg( void *p)
{

}
static void show(void)
{
    MenuIdle_working = 0; //clear
    if(self_checking_screen == 0)
    {
        self_checking_screen = 1;
        self_checking_counter = 0;
        CounterBack = 0;
        //rt_kprintf("\r\n------------------��ʼ�Լ�-----------");
        lcd_fill(0);
		lcd_text12(20, 10, "��ϢModem ", 10, LCD_MODE_SET);  
        lcd_update_all(); 
    }

}


static void keypress(unsigned int key)
{
    switch(KeyValue)
    {
    case KeyValueMenu:
        //pMenuItem=&Menu_0_loggingin;
        //pMenuItem->show();
        break;
    case KeyValueOk:
        if(self_checking_result == 2)
        {
            self_checking_result = 30;

            self_checking_screen = 0;
            self_checking_counter = 0; //�Լ�ʱ�����
            self_checking_pro = 0; //�Լ���������쳣��+1
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
 /*   if(Auto_exit == 0)
    {
        self_checking_counter++;//250ms
        if(self_checking_counter >= 30)
        {
            self_checking_counter = 0;
            //rt_kprintf("\r\n-------***********------�鿴�Լ���-------*************----");
            if(self_checking_result == 1)
            {
                self_checking_result = 10;
                //rt_kprintf("\r\n------�Լ�����-----------");
                lcd_fill(0);
                lcd_text12_local(36, 10, "�Լ�����", 8, LCD_MODE_SET);
                lcd_update_all();
            }
            else if(self_checking_result == 2)
            {
                Auto_exit = 1;
                //rt_kprintf("\r\n------�Լ��쳣-----------");
                lcd_fill(0);
                if(self_checking_Antenna == 1) //���߿�·
                {
                    // rt_kprintf("\r\n  ���߿�· \r\n");
                    lcd_text12_local(36, 3, "���߿�·", 8, LCD_MODE_SET);
                }
                else if(self_checking_Antenna == 2) //���߶�·
                {
                    // rt_kprintf("\r\n  ���߶�· \r\n");
                    lcd_text12_local(36, 3, "���߶�·", 8, LCD_MODE_SET);
                }
                if(self_checking_PowerCut == 1)
                {
                    // rt_kprintf("\r\n  �ڲ���ع��� \r\n");
                    lcd_text12_local(24, 18, "�ڲ���ع���", 12, LCD_MODE_SET);
                }
                lcd_update_all();
            }
        }
    }
    else if(Auto_exit == 1)
    {
        self_checking_counter++;
        if(self_checking_counter >= 20)
        {
            if(self_checking_result == 2)
            {
                self_checking_result = 30;

                self_checking_screen = 0;
                self_checking_counter = 0; //�Լ�ʱ�����
                self_checking_pro = 0; //�Լ���������쳣��+1

            }
        }
    }*/
}


MENUITEM	Menu_0_0_self_Checking =
{
    "�豸�Լ�",
    8,
    &show,
    &keypress,
    &timetick,
    &msg,
    (void *)0
};


