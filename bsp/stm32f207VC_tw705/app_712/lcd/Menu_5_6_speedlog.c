#include "Menu_Include.h"
#include <string.h>

static u8 Dis_num = 0;
static u8 Dis_maxNum = 0;
static u8 Dis_status[16] = {"1.�ٶ�״̬:����"};
static u8 Dis_start_time[25] = {"14-06-02 10:20:15"};
static u8 Dis_end_time[25]  = {"14-06-02 15:18:32"};
static u8 get_indexnum = 0;
static u8 H15_reg[133];


static void  disp_record_15h(u8 record_num)
{
    u16  retnvlaue = 0;

    retnvlaue = get_15h(get_indexnum - record_num, H15_reg);
    if(H15_reg[0] == 0x01)
        memcpy(Dis_status + 11, "����", 4);
    else
        memcpy(Dis_status + 11, "�쳣", 4);
    sprintf(Dis_start_time, "st:%2X-%2X-%2X %2X:%2X:%2X", H15_reg[1], H15_reg[2], H15_reg[3], H15_reg[4], H15_reg[5], H15_reg[6]);
    sprintf(Dis_end_time, "ed:%2X-%2X-%2X %2X:%2X:%2X", H15_reg[7], H15_reg[8], H15_reg[9], H15_reg[10], H15_reg[11], H15_reg[12]);
    lcd_fill(0);
    if(retnvlaue)
    {
        lcd_text12(0, 0, Dis_status, strlen(Dis_status), LCD_MODE_SET);
        lcd_text12(0, 12, Dis_start_time, strlen(Dis_start_time), LCD_MODE_SET);
        lcd_text12(0, 22, Dis_end_time, strlen(Dis_end_time), LCD_MODE_SET);
    }
    else
        lcd_text12(0, 10, "���ٶ�״̬��־��¼", 18, LCD_MODE_SET);
    lcd_update_all();
}

static void msg( void *p)
{
}
static void show(void)
{
    u8  read_flag = 0;

    MenuIdle_working = 0; //clear
    //0.  --    get  usefull   record num
    if(Vdr_Wr_Rd_Offset.V_15H_Write >= 1)	//û����
        get_indexnum = Vdr_Wr_Rd_Offset.V_15H_Write - 1;
    else
        get_indexnum = 0;

    //  1.   flag  state
    if(Vdr_Wr_Rd_Offset.V_15H_Write >= 1) //û����
    {
        get_indexnum = Vdr_Wr_Rd_Offset.V_15H_Write - 1;
        read_flag = 1;
    }
    else if(Vdr_Wr_Rd_Offset.V_15H_Write == 1)
    {
        get_indexnum = 0;
        read_flag = 1;
    }
    else
        get_indexnum = 0;

    //   2. read
    if(read_flag)
    {
        Dis_num = 0;
        Dis_status[0] = Dis_num + 1 + '0';
        disp_record_15h(Dis_num);
    }
    else
    {
        Dis_num = 0;
        lcd_fill(0);
        lcd_text12(0, 10, "���ٶ�״̬��־��¼", 18, LCD_MODE_SET);
        lcd_update_all();
    }

}

//�ֽ�       ��������         ��������
// 1         2-7(bcd)         8-13(bcd)
//�ٶ�״̬   ��ʼʱ��         ����ʱ��
//01����
//02�쳣
static void keypress(unsigned int key)
{
    switch(KeyValue)
    {
    case KeyValueMenu:
        Dis_num = 0;
        pMenuItem = &Menu_5_recorder;
        pMenuItem->show();
        break;
    case KeyValueOk:
        break;
    case KeyValueUP:
        if(get_indexnum)
        {
            if(Dis_num == 0)
                Dis_num = Dis_maxNum;
            else
                Dis_num--;
            Dis_status[0] = Dis_num + 1 + '0';
            disp_record_15h(Dis_num);
        }
        break;
    case KeyValueDown:
        if(get_indexnum)
        {
            if(Dis_num > (Dis_maxNum - 1))
                Dis_num = 0;
            else
                Dis_num++;

            Dis_status[0] = Dis_num + 1 + '0';
            disp_record_15h(Dis_num);
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
MENUITEM	Menu_5_6_speedlog =
{
    "�ٶ�״̬��־",
    12,
    &show,
    &keypress,
    &timetick,
    &msg,
    (void *)0
};

