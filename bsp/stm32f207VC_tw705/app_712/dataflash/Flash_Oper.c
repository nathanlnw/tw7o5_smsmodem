#include "App_moduleConfig.h"




//--------   ˳���ȡ�������  ------------
u8    ReadCycle_status = RdCycle_Idle;


u32     cycle_write = 0, cycle_read = 0, delta_0704_rd = 0, mangQu_read_reg = 0; // ѭ���洢��¼
#ifdef SPD_WARN_SAVE
u32    ExpSpdRec_write = 0, ExpSpdRec_read = 0; // ���ٱ����洢��¼
#endif
u32    pic_current_page = 0, pic_PageIn_offset = 0, pic_size = 0;; // ͼƬ�洢��¼
u32   	Distance_m_u32 = 0;	 // �г���¼�����о���	  ��λ��
u32     DayStartDistance_32 = 0; //ÿ����ʼ�����Ŀ


//-----------------------------------------------------------------------------------------------------------------------------
u8 SaveCycleGPS(u32 cyclewr, u8 *content , u16 saveLen)
{
    /*
           //old  NOTE : Flash  1 page = 512 Bytes  ; 1 Record = 32 Bytes ;  1page= 16 Records   1Sector=8Page=128Records
           NOTE : Flash  1 page = 512 Bytes  ; 1 Record = 128 Bytes ;  1page= 4Records   1Sector=8Page=32Records
    */
    u32  pageoffset = 0; //Page ƫ��
    u32  InPageoffset;   //ҳ��Recordƫ��
    u16  InPageAddr = 0; //ҳ�� ��ַƫ��
    //	u8   reg[1]={0};
    u8   rd_back[128];
    u16  i = 0, retry = 0;

    //----------------------------------------------------------------------------------------------
    //   1. Judge  Whether  needs to Erase page
    pageoffset = (u32)(cycle_write >> 2);            // ����� Page ƫ��  ����4 (ÿ��page�ܷ�4����¼)
    InPageoffset = cycle_write - (u32)(pageoffset << 2); // ����� ҳ��ƫ�Ƶ�ַ
    InPageAddr = (u16)(InPageoffset << 7);       // �����ҳ�� �ֽ�   ���� 128 (ÿ����¼128���ֽ�)
    if(((pageoffset % 8) == 0) && (InPageoffset == 0)) // �ж��Ƿ���Ҫ����Sector  ���Ƴ�����һ��Sector  1Sector=8Page
    {
        WatchDog_Feed();
        SST25V_SectorErase_4KByte((pageoffset + CycleStart_offset)*PageSIZE);    // erase Sector
        DF_delay_ms(60);
        if(GB19056.workstate == 0)
            rt_kprintf("\r\n Erase Cycle Sector : %d\r\n", (pageoffset >> 3));
    }
    //	   2. write  and read back
SV_RTRY:
    if(retry >= 2)
    {
        return false;
    }
    delay_ms(5);
    WatchDog_Feed();
    DF_WriteFlashDirect(pageoffset + CycleStart_offset, InPageAddr, content, saveLen); //   д����Ϣ
    DF_delay_us(500);
    DF_ReadFlash(pageoffset + CycleStart_offset, InPageAddr, rd_back, saveLen); //   ��ȡ��Ϣ
    //  compare
    for(i = 0; i < saveLen; i++)
    {
        if(content[i] != rd_back[i])
        {
            cycle_write++;
            if(cycle_write >= Max_CycleNum)
                cycle_write = 0;
            if(retry == 0)
            {
                retry++;
                goto SV_RTRY;
            }
            else
            {
                //---------------------------
                PositionSD_Enable();
                Current_State = 1; // ʹ�ܼ�ʱ�ϱ�
                Current_UDP_sd = 1;
                if(GB19056.workstate == 0)
                    rt_kprintf("\r\n wrte error-> current\r\n");
                return false;
            }
        }
    }
    return true;
    //--------------------------------------------------------------------------------------------
}


u8 ReadCycleGPS(u32 cycleread, u8 *content , u16 ReadLen)
{
    /*
       NOTE : Flash  1 page = 512 Bytes  ; 1 Record = 32 Bytes ;  1page= 16 Records   1Sector=8Page=128Records
    */
    u32  pageoffset = 0; //Page ƫ��
    u32  InPageoffset;   //ҳ��Recordƫ��
    u16  InPageAddr = 0; //ҳ�� ��ַƫ��
    u8  i = 0, FCS = 0;
    u8  Len_read = 0; // ��Ϣ����

    /*
        �ϱ���ÿһ������һ���ֽ�����Ч��Ϣ�ĳ��ȣ��ӵڶ����ֽ�����Ϣ���ݣ�
        ��Ϣ���ݵĺ����һ���ֽڶ�У��(У��ӳ��ȿ�ʼ���������һ���ֽ�)
    */
    //----------------------------------------------------------------------------------------------
    //   1. caculate address
    pageoffset = (u32)(cycle_read >> 2);             // ����� Page ƫ��  ����4 (ÿ��page�ܷ�4����¼)
    InPageoffset = cycle_read - (u32)(pageoffset << 2); // ����� ҳ��ƫ�Ƶ�ַ
    InPageAddr = (u16)(InPageoffset << 7);        // �����ҳ�� �ֽ�   ���� 128 (ÿ����¼128���ֽ�)
    //   2. Write Record Content
    DF_TAKE;
    DF_ReadFlash(pageoffset + CycleStart_offset, InPageAddr, content, ReadLen);
    DF_delay_ms(10);
    DF_RELEASE;
    //  ��ȡ��Ϣ����
    Len_read = content[0];

    if(DispContent == 2)
    {
        OutPrint_HEX("��ȡCycleGPS ����Ϊ ", content, Len_read + 1);
    }
    //  3. Judge FCS
    //--------------- �����Ѿ����͹�����Ϣ -------
    FCS = 0;
    for ( i = 0; i < Len_read; i++ )   //�����ȡ��Ϣ������
    {
        FCS ^= *( content + i );
    }
    if(((content[Len_read] != FCS) && (content[0] != 0xFF)) || (content[0] == 0xFF)) // �ж�����
    {
        if(content[0] == 0xFF)
        {
            // rt_kprintf("\r\n  content[0]==0xFF   read=%d,  write=%d  \r\n",cycle_read,cycle_write);

            cycle_read++;
            if(cycle_read >= Max_CycleNum)
                cycle_read = 0;
            ReadCycle_status = RdCycle_Idle;
            return false;
        }
        //------------------------------------------------
        cycle_read++;
        if(cycle_read >= Max_CycleNum)
            cycle_read = 0;
        ReadCycle_status = RdCycle_Idle;
        return false;
    }
    //------------------------------------------------------------------------------------------
    return true;
    //--------------------------------------------------------------------------------------------
}

#ifdef SPD_WARN_SAVE

//-----------------------------------------------------------------------------------------------------------
u8  Common_WriteContent(u32 In_write, u8 *content , u16 saveLen, u8 Type)
{
    //-----------------------------------------------------
    u8     reg[1];
    //u8   regStr[25];
    //-----------------------------------------------------
    u32  pageoffset = 0; //Page ƫ��
    u32  InPageoffset;   //ҳ��Recordƫ��
    u16  InPageAddr = 0; //ҳ�� ��ַƫ��
    u32  Start_offset = 0;

    //--------------------------------------------------
    //memset(regStr,0,sizeof(regStr));
    //  1.   Classify
    switch(Type)
    {

    case TYPE_ExpSpdAdd:
        Start_offset = ExpSpdStart_offset;
        // memcpy(regStr,"���ٱ���",25);
        break;
    default :
        return false;
    }
    //----------------------------------------------------------------------------------------------
    //	 2. caculate address

    pageoffset = (u32)(In_write >> 4);				 // ����� Page ƫ��  ����16
    InPageoffset = In_write - (u32)(pageoffset << 4);	 // ����� ҳ��ƫ�Ƶ�ַ
    InPageAddr = (u16)(InPageoffset << 5);			 // �����ҳ�� �ֽ�   ���� 32 (ÿ����¼32���ֽ�)
    if(((pageoffset % 8) == 0) && (InPageoffset == 0)) // �ж��Ƿ���Ҫ����Block  ���Ƴ�����һ��Block	1Block=8Page
    {
        SST25V_SectorErase_4KByte((pageoffset + Start_offset)*PageSIZE);    // erase Sector
        DF_delay_ms(70);
    }
    // 	  2. Filter write  area
    DF_ReadFlash(pageoffset + Start_offset, InPageAddr, reg, 1);
    if(reg[0] != 0xff)	// ���Ҫд������� dirty  �����ַ��Ȼ����¿�ʼѰ��֪���ҵ�Ϊֹ
    {
        In_write++;
        if(In_write >= Max_CommonNum)
            In_write = 0;
        return false;
    }
    //   3. Write Record Content
    DF_WriteFlashDirect(pageoffset + Start_offset, InPageAddr, content, saveLen); //   д����Ϣ
    DF_delay_ms(10);

    //   4. end
    switch(Type)
    {

    case TYPE_ExpSpdAdd:
        ExpSpdRec_write = In_write;
        break;
    default :
        return false;
    }

    return true;

}


u8  Common_ReadContent(u32 In_read, u8 *content , u16 ReadLen, u8 Type)
{
    //-----------------------------------------------------
    //u8    regStr[25];
    //-----------------------------------------------------
    u32  pageoffset = 0; //Page ƫ��
    u32  InPageoffset;   //ҳ��Recordƫ��
    u16  InPageAddr = 0; //ҳ�� ��ַƫ��
    u32  Start_offset = 0;
    u8   i = 0, FCS = 0;;
    //--------------------------------------------------

    // memset(regStr,0,sizeof(regStr));
    //  1.	Classify
    switch(Type)
    {

    case TYPE_ExpSpdAdd:
        Start_offset = ExpSpdStart_offset;
        //  memcpy(regStr,"���ٱ���",25);
        break;
    default :
        return false;
    }

    //----------------------------------------------------------------------------------------------
    //	 2. caculate address

    pageoffset = (u32)(In_read >> 4);				 // ����� Page ƫ��  ����64
    InPageoffset = In_read - (u32)(pageoffset << 4); // ����� ҳ��ƫ�Ƶ�ַ
    InPageAddr = (u16)(InPageoffset << 5);			 // �����ҳ�� �ֽ�   ���� 32 (ÿ����¼32���ֽ�)
    //	 2. Write Record Content
    DF_ReadFlash(pageoffset + Start_offset, InPageAddr, content, ReadLen);
    DF_delay_us(10);
    if(DispContent)
    {
        OutPrint_HEX("��ȡCommon ����Ϊ ", content, ReadLen);
    }
    //	3. Judge FCS
    //--------------- �����Ѿ����͹�����Ϣ -------
    FCS = 0;
    for ( i = 0; i < ReadLen - 1; i++ )	 //�����ȡ��Ϣ������
    {
        FCS ^= *( content + i );
    }
    if(((content[ReadLen - 1] != FCS) && (content[0] != 0xFF)) || (content[0] == 0xFF)) // �ж�����
    {

        if(content[0] == 0xFF)
        {
            //�����������0xFF ����ָ���дָ����ȣ����ٴ����ϱ���
            switch(Type)
            {

            case TYPE_ExpSpdAdd:
                ExpSpdRec_read = ExpSpdRec_write;
                break;
            default :
                return false;

            }
            return false;
        }
        In_read++;
        if(In_read >= Max_CommonNum)
            In_read = 0;
        //rt_kprintf("\r\n   record info error  \r\n");
        return false;
    }
    if(content[0] == 0xFF)
    {
        // rt_kprintf("\r\n  read  0xFF \r\n");
        switch(Type)
        {

        case TYPE_ExpSpdAdd:
            ExpSpdRec_read = ExpSpdRec_write;
            break;
        default :
            return false;

        }
        return false;
    }


    //   4. end
    switch(Type)
    {

    case TYPE_ExpSpdAdd:
        ExpSpdRec_read = In_read;
        break;
    default :
        return false;
    }

    return true;

}
#endif
//----------------------------------------------------------------------

