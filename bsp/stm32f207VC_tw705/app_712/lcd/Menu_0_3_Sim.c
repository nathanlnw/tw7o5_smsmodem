#include  <string.h>
#include "Menu_Include.h"

#define  Sim_width1  6


u8 Sim_SetFlag = 1, Sim_SetFlag_temp = 1, Sim_SetCounter = 0;
u8 Sim_Modify_Flag = 0; //���޸���

unsigned char select_Sim[] = {0x0C, 0x06, 0xFF, 0x06, 0x0C};

DECL_BMP(8, 5, select_Sim);



void Sim_Set(u8 par, u8 invert)
{
    lcd_fill(0);

    lcd_text12(0, 3, (char *)Menu_sim_Code, Sim_SetFlag - 1, LCD_MODE_SET);
    if(invert == 1)
    {
        if(Sim_SetFlag_temp >= 2)
            lcd_text12((Sim_SetFlag_temp - 2) * 6, 3, (char *)&Menu_sim_Code[Sim_SetFlag_temp - 2], 1, LCD_MODE_INVERT);
    }
    else
    {
        if(invert == 2)
        {
            if(Sim_SetFlag_temp >= 2)
                lcd_text12((Sim_SetFlag_temp - 2) * 6, 3, (char *)&Menu_sim_Code[Sim_SetFlag_temp - 2], 1, LCD_MODE_INVERT);
        }

        lcd_bitmap(par * Sim_width1, 14, &BMP_select_Sim, LCD_MODE_SET);
        lcd_text12(0, 19, "0123456789", 10, LCD_MODE_SET);
    }

    lcd_update_all();
}


static void show(void)
{
    MenuIdle_working = 0; //clear

    Sim_Set(Sim_SetCounter * Sim_width1, 0);


}


static void keypress(unsigned int key)
{
    switch(KeyValue)
    {
    case KeyValueMenu:
        if(Sim_Modify_Flag == 1) //���ַ���Ҫ�޸�
        {
            if(Sim_SetFlag == 13)
            {
                pMenuItem = &Menu_0_loggingin;
                pMenuItem->show();
                memset(Menu_sim_Code, 0, sizeof(Menu_sim_Code));

                Sim_SetFlag = 1;
                Sim_SetFlag_temp = 1;
                Sim_SetCounter = 0;
                Sim_Modify_Flag = 0; //���޸���
            }
            else
            {
                Sim_Modify_Flag = 2; //ѡ����Ҫ�޸ĵ��ַ�
                Sim_SetFlag_temp = Sim_SetFlag;

                Sim_Set(Sim_SetCounter, 1);
            }
            //rt_kprintf("\r\n��  �˵���  �޸�");
        }
        else
        {
            pMenuItem = &Menu_0_loggingin;
            pMenuItem->show();
            memset(Menu_sim_Code, 0, sizeof(Menu_sim_Code));

            Sim_SetFlag = 1;
            Sim_SetFlag_temp = 1;
            Sim_SetCounter = 0;
            Sim_Modify_Flag = 0; //���޸���
        }
        break;

    case KeyValueOk:
        if(Sim_Modify_Flag == 2)
        {
            Sim_Modify_Flag = 3;
            Sim_Set(0, 2); ///////
            //rt_kprintf("\r\n ȷ���޸�λ Sim_SetFlag_temp = %d ",Sim_SetFlag_temp);
        }
        else if(Sim_Modify_Flag == 3)
        {
            if(Sim_SetCounter <= 9)
                Menu_sim_Code[Sim_SetFlag_temp - 2] = Sim_SetCounter + '0';
            Sim_Set(0, 0);
            //rt_kprintf("\r\n ���ַ��޸���� Sim_SetFlag_temp = %d ");
            Sim_Modify_Flag = 1;
            Sim_SetCounter = 0;
        }
        else
        {
            if((Sim_SetFlag >= 1) && (Sim_SetFlag <= 11))
            {
                Sim_Modify_Flag = 1;
                //rt_kprintf("\r\n���޸��Sim_SetFlag=%d",Sim_SetFlag);

                if(Sim_SetCounter <= 9)
                    Menu_sim_Code[Sim_SetFlag - 1] = Sim_SetCounter + '0';

                Sim_SetFlag++;
                //rt_kprintf("\r\n  Sim_SetFlag = %d ,�������ַ���=%s",Sim_SetFlag,Menu_sim_Code);

                Sim_SetCounter = 0;
                Sim_Set(0, 0);
            }
            if(Sim_SetFlag == 12)
            {
                Sim_SetFlag = 13;
                if(MENU_set_carinfor_flag == 1)
                {
                    //rt_kprintf("\r\nSIM ������������ɣ����˵������أ�%s",Menu_sim_Code);

                    memset(SimID_12D, 0, sizeof(SimID_12D));
                    SimID_12D[0] = '0';
                    memcpy(SimID_12D + 1, Menu_sim_Code, 11);
                    DF_WriteFlashSector(DF_SIMID_12D, 0, SimID_12D, 13);
                    delay_ms(80);
                    //rt_kprintf("\r\n ��ʾ���豸SIM_ID����Ϊ : %s", SimID_12D);
                    DF_ReadFlash(DF_SIMID_12D, 0, SimID_12D, 13);
                    SIMID_Convert_SIMCODE();  // ת��

                    //rt_kprintf("\r\n �洢:%s",SimID_12D);
                }
                lcd_fill(0);
                lcd_text12(0, 5, (char *)Menu_sim_Code, 11, LCD_MODE_SET);
                lcd_text12(13, 19, "SIM�����������", 15, LCD_MODE_SET);
                lcd_update_all();
            }
            else if(Sim_SetFlag == 13)
            {
                CarSet_0_counter = 5;
                Sim_SetFlag = 1;

                Sim_SetFlag = 1,
                Sim_SetFlag_temp = 1,
                Sim_SetCounter = 0;
                Sim_Modify_Flag = 0; //���޸���

                pMenuItem = &Menu_0_loggingin;
                pMenuItem->show();
            }
        }

        break;

    case KeyValueUP:
        //ѡ����Ҫ�޸ĵ��ַ�
        if(Sim_Modify_Flag == 2)
        {
            if(Sim_SetFlag_temp > 2)
                Sim_SetFlag_temp--;
            //rt_kprintf("\r\n Sim_SetFlag_temp=%d",Sim_SetFlag_temp);

            Sim_Set(Sim_SetCounter, 1);
        }
        else
        {
            if((Sim_SetFlag >= 1) && (Sim_SetFlag <= 12))
            {
                if(Sim_SetCounter == 0)
                    Sim_SetCounter = 9;
                else if(Sim_SetCounter >= 1)
                    Sim_SetCounter--;
                if(Sim_Modify_Flag == 3)
                    Sim_Set(Sim_SetCounter, 2);
                else
                    Sim_Set(Sim_SetCounter, 0);
            }
        }
        break;

    case KeyValueDown:
        //ѡ����Ҫ�޸ĵ��ַ�
        if(Sim_Modify_Flag == 2)
        {
            if(Sim_SetFlag_temp < Sim_SetFlag)
                Sim_SetFlag_temp++;
            //rt_kprintf("\r\n Sim_SetFlag_temp=%d",Sim_SetFlag_temp);

            Sim_Set(Sim_SetCounter, 1);
        }
        else
        {
            if((Sim_SetFlag >= 1) && (Sim_SetFlag <= 12))
            {
                Sim_SetCounter++;
                if(Sim_SetCounter > 9)
                    Sim_SetCounter = 0;

                if(Sim_Modify_Flag == 3)
                    Sim_Set(Sim_SetCounter, 2);
                else
                    Sim_Set(Sim_SetCounter, 0);
            }
        }
        break;

    }
    KeyValue = 0;
}


static void timetick(unsigned int systick)
{

}


MENUITEM	Menu_0_3_Sim =
{
    "�����ֻ�����",
    12,
    &show,
    &keypress,
    &timetick,
    (void *)0
};


