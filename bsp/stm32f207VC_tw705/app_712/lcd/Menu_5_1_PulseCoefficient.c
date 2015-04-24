#include  <string.h>
#include "Menu_Include.h"

#define  pulse_width1  6

u8 pulse_sel_num = 0; // ѡ��鿴�����á���������ϵ��
u8 pulse_sel_screen = 0; //
u8 pulse_sel_flag = 0; //

u8 pulse_coefficient_num = 0; //�����ã�5λ
u8 pulse_coefficient_set = 0; //ÿһλ��������ݣ�0-9
u8 pulse_coefficient_arr[6];//����ϵ��

unsigned char select_pulse[] = {0x0C, 0x06, 0xFF, 0x06, 0x0C};

DECL_BMP(8, 5, select_pulse);

static void pulse_sel(u8 par)
{
    lcd_fill(0);
    if(par == 1)
    {
        pulse_sel_flag = 1; //
        lcd_text12(0, 3, "1.����ϵ���鿴", 14, LCD_MODE_INVERT);
        lcd_text12(0, 18, "2.����ϵ������", 14, LCD_MODE_SET);
    }
    else if(par == 2)
    {
        pulse_sel_flag = 2; //
        lcd_text12(0, 3, "1.����ϵ���鿴", 14, LCD_MODE_SET);
        lcd_text12(0, 18, "2.����ϵ������", 14, LCD_MODE_INVERT);
    }
    else if(par == 3)
    {
        pulse_sel_flag = 3; //
        lcd_text12(0, 3, "3.�������ٶȲ鿴", 16, LCD_MODE_INVERT);
    }
    lcd_update_all();
}

void pulse_Set(u8 par)
{
    lcd_fill(0);
    lcd_text12(0, 3, (char *)pulse_coefficient_arr, pulse_coefficient_num - 1, LCD_MODE_SET);
    lcd_bitmap(par * pulse_width1, 14, &BMP_select_pulse, LCD_MODE_SET);
    lcd_text12(0, 19, "0123456789", 10, LCD_MODE_SET);
    lcd_update_all();
}
static void msg( void *p)
{
}

static void show(void)
{
    MenuIdle_working = 0; //clear
    pulse_sel_screen = 0;
    pulse_sel_num = 1;
    pulse_sel(pulse_sel_num);
}

static void keypress(unsigned int key)
{
    u16  value = 0;

    switch(KeyValue)
    {
    case KeyValueMenu:
        pulse_sel_num = 0; // ѡ��鿴�����á���������ϵ��
        pulse_sel_screen = 0; //
        pulse_sel_flag = 0; //

        pulse_coefficient_num = 0; //�����ã�5λ
        pulse_coefficient_set = 0; //ÿһλ��������ݣ�0-9
        memset(pulse_coefficient_arr, 0, sizeof(pulse_coefficient_arr));

        CounterBack = 0;
        pMenuItem = &Menu_5_recorder;
        pMenuItem->show();
        break;

    case KeyValueOk:
        if(pulse_sel_screen == 0)
        {
            if(pulse_sel_flag == 1) //����ϵ���鿴
            {
                convert_speed_pulse(2);
                Dis_speed_pulse[9] = '0' + JT808Conf_struct.Vech_Character_Value / 10000;
                Dis_speed_pulse[10] = '0' + (JT808Conf_struct.Vech_Character_Value % 10000) / 1000;
                Dis_speed_pulse[11] = '0' + (JT808Conf_struct.Vech_Character_Value % 1000) / 100;
                Dis_speed_pulse[12] = '0' + (JT808Conf_struct.Vech_Character_Value % 100) / 10;
                Dis_speed_pulse[13] = '0' + JT808Conf_struct.Vech_Character_Value % 10;
                lcd_fill(0);
                lcd_text12(0, 10, Dis_speed_pulse, strlen(Dis_speed_pulse), LCD_MODE_SET);
                lcd_update_all();
            }
            else if(pulse_sel_flag == 2) //����ϵ������
            {
                pulse_sel_screen = 1;
                pulse_coefficient_num = 0; //��ʼ����5λ����ϵ��
                lcd_fill(0);
                lcd_text12(10, 3, "������5λ����ϵ��", 17, LCD_MODE_SET);
                lcd_text12(19, 18, "��ȷ�Ͽ�ʼ����", 14, LCD_MODE_SET);
                lcd_update_all();
            }
            else if(pulse_sel_flag == 3) //�������ٶȲ鿴
            {
                convert_speed_pulse(3);
                lcd_fill(0);
                lcd_text12(0, 10, Dis_speed_sensor, strlen(Dis_speed_sensor), LCD_MODE_SET);
                lcd_update_all();
            }
        }
        else if(pulse_sel_screen == 1) //����Ҫ���õ�����ϵ��
        {
            //����ϵ������
            if((pulse_coefficient_num >= 0) && (pulse_coefficient_num <= 5))
            {
                if((pulse_coefficient_set <= 9) && (pulse_coefficient_num > 0))
                {
                    pulse_coefficient_arr[pulse_coefficient_num - 1] = pulse_coefficient_set + '0';
                }
                pulse_coefficient_num++;
                pulse_coefficient_set = 0;
                pulse_Set(0);
            }
            else if(pulse_coefficient_num == 6)
            {
                pulse_coefficient_num = 7;

                lcd_fill(0);
                lcd_text12(0, 3, (char *)pulse_coefficient_arr, 5, LCD_MODE_SET);
                lcd_text12(13, 19, "����ϵ���������", 16, LCD_MODE_SET);
                lcd_update_all();

                //��ӱ�����Ϣ
                value = (pulse_coefficient_arr[0] - '0') * 10000 + (pulse_coefficient_arr[1] - '0') * 1000 + \
                        (pulse_coefficient_arr[2] - '0') * 100 + (pulse_coefficient_arr[3] - '0') * 10 + (pulse_coefficient_arr[4] - '0');
                plus_num(value);

            }
            else if(pulse_coefficient_num == 7)
            {
                pulse_sel_num = 0; // ѡ��鿴�����á���������ϵ��
                pulse_sel_screen = 0; //
                pulse_sel_flag = 0; //

                pulse_coefficient_num = 0; //�����ã�5λ
                pulse_coefficient_set = 0; //ÿһλ��������ݣ�0-9
                memset(pulse_coefficient_arr, 0, sizeof(pulse_coefficient_arr));

                pMenuItem = &Menu_5_recorder;
                pMenuItem->show();
            }
        }
        break;

    case KeyValueUP:
        if(pulse_sel_screen == 0)
        {
            if(pulse_sel_num == 1)
                pulse_sel_num = 3;
            else
                pulse_sel_num--;
            pulse_sel(pulse_sel_num);
        }
        else if((pulse_sel_screen == 1) && (pulse_coefficient_num)) //��������ϵ��
        {
            if(pulse_coefficient_num <= 5)
            {
                if(pulse_coefficient_set < 1)
                    pulse_coefficient_set = 9;
                else
                    pulse_coefficient_set--;
                pulse_Set(pulse_coefficient_set);
            }
        }
        break;

    case KeyValueDown:
        if(pulse_sel_screen == 0)
        {
            pulse_sel_num++;
            if(pulse_sel_num > 3)
                pulse_sel_num = 1;
            pulse_sel(pulse_sel_num);
        }
        else if((pulse_sel_screen == 1) && (pulse_coefficient_num)) //��������ϵ��
        {
            if(pulse_coefficient_num <= 5)
            {
                if(pulse_coefficient_set == 9)
                    pulse_coefficient_set = 0;
                else
                    pulse_coefficient_set++;
                pulse_Set(pulse_coefficient_set);
            }
        }
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
MENUITEM	Menu_5_1_PulseCoefficient =
{
    "����ϵ�����",
    12,
    &show,
    &keypress,
    &timetick,
    &msg,
    (void *)0
};



