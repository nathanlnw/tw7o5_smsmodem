/*****************************************************************
 MSP430 AT45DB041B ����
*****************************************************************/
/*
#include <string.h>
*/
#include "App_moduleConfig.h"
#include "DF_Oper.h"


u8 SectorBuf_save[8][DFBakSize];//512bytes һ����λ              ֻ�����洢������Ϣ
u8   DF_initOver = 0;  //Dataflash  Lock



void DF_init(void)
{
    u32 device_id = 0;

    SST25V_Init();

    SST25V_CS_LOW();
    SPI_Flash_SendByte(WriteDisable);
    SST25V_CS_HIGH();

    SST25V_CS_LOW();

    //-----erase chip-------------------
    //	DF_delay_ms(350);
    //--------------------------------

    SPI_Flash_SendByte( ReadJedec_ID  );
    device_id  = SPI_Flash_SendByte(0xFF) << 16;
    device_id = device_id | SPI_Flash_SendByte(0xFF) << 8;
    device_id = device_id | SPI_Flash_SendByte(0xFF);
    SST25V_CS_HIGH();

    //if(device_id == 0xBF254A)//SST25VF032B = 0xBF254A,
    {
        SST25V_CS_LOW();
        SPI_Flash_SendByte( DBSY );
        SST25V_CS_HIGH();

        SST25V_CS_LOW();
        SPI_Flash_SendByte( EnableWriteStatusRegister );
        SST25V_CS_HIGH();

        SST25V_CS_LOW();
        SPI_Flash_SendByte( WriteStatusRegister );
        SPI_Flash_SendByte( 0 );
        SST25V_CS_HIGH();
    }

    delay_ms(700);

}
void DF_delay_us(u16 j)
{
    u8 i;
    while(j--)
    {
        i = 3;
        while(i--);
    }
}

void DF_delay_ms(u16 j)
{
    while(j--)
    {
        DF_delay_us(2000);// 1000
    }

}

u8 DF_Spi_Tranbyte(u8 BYTE)
{
    //USART_TxStr(USART1,"\r\nSpi_Tranbyte\r\n");
    /*
    BSPI1->TXR=(u16)(BYTE<<8);
    while(((BSPI1->CSR2)&0x10)!=0x10);
    return (BSPI1->RXR)>>8;
    */
    //Set_AT45_CLK_1;

    while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) == RESET);
    //USART_TxStr(USART1,"\r\nSpi_Tranbyte111111111\r\n");
    //GPIO_SetBits(GPIOB,GPIO_Pin_3);
    SPI_I2S_SendData(SPI3, (u16)BYTE);
    //USART_TxStr(USART1,"\r\nSpi_Tranbyte2222222222\r\n");
    //   DF_delay_us(400);

    //Set_AT45_CLK_0;
    //GPIO_ResetBits(GPIOB,GPIO_Pin_3);
    while ( (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET ) );
    return SPI_I2S_ReceiveData(SPI3);
    //   DF_delay_us(400);
    //USART_TxStr(USART1,"\r\nSpi_Tranbyte_over\r\n");
}



void DF_ReadFlash(u16 page_counter, u16 page_offset, u8 *p, u16 length)
{
    u16 i = 0;
    u32 ReadAddr = ((u32)page_counter * PageSIZE) + (u32)page_offset;

    //------ Byte Read-------
#if 0
    for(i = 0; i < length; i++)
    {
        *p = SST25V_ByteRead(((u32)page_counter * PageSIZE) + (u32)(page_offset + i)); //512bytes һ����λ
        p++;
    }
    //DF_delay_us(200);
#endif
    //----  Addr +  content ���ݶԱ�-----
    SST25V_BufferRead(p, ReadAddr, length);
    DF_delay_ms(5);

}

void DF_WriteFlash(u16 page_counter, u16 page_offset, u8 *p, u16 length) //512bytes һ����λ
{
    u16 i = 0, j = 0, k = 0; //д����

    for(i = 0; i < 8; i++)
    {
        DF_ReadFlash((8 * (page_counter / 8)) + i, 0, SectorBuf_save[i], DFBakSize); //PageSIZE
    }

    SST25V_SectorErase_4KByte((8 * ((u32)page_counter / 8))*PageSIZE);
    DF_delay_ms(100);
    for(j = 0; j < 8; j++)
    {
        if(j == (page_counter % 8))
        {
            for(i = 0; i < length; i++)
            {
                SectorBuf_save[j][page_offset + i] = *p;
                p++;
            }
        }
        for(k = 0; k < DFBakSize; k++)
        {
            SST25V_ByteWrite(SectorBuf_save[j][k], ((8 * (page_counter / 8)) + j)*PageSIZE + k);
        }
    }
    DF_delay_ms(20);
}
void DF_WriteFlashSector(u16 page_counter, u16 page_offset, u8 *p, u16 length) //512bytes ֱ�Ӵ洢
{
    u16 i = 0;

    // Ҫ�������ڵ�sector(Ҫ��pagecounter Ϊ����sector�ĵ�һ��page) ��Ȼ���sector�ĵ�һ��pageд
    SST25V_SectorErase_4KByte((8 * ((u32)page_counter / 8))*PageSIZE);
    DF_delay_ms(200);

    for(i = 0; i < length; i++)
    {
        SST25V_ByteWrite(*p, page_counter * 512 + i + page_offset);
        p++;
    }
    DF_delay_ms(30);

}
void DF_Erase(void)
{
    u16 i = 0;
    /*
         1. ��0x1000    4K  ��ʼ����28K     7 sector
         2.����56������   7 ��32k block
      */

    DF_TAKE;
    rt_kprintf("\r\n  ISP erase start\r\n");
    //  -----erase        28 K   area  -------------
    for(i = 0; i < 7; i++) // 7  secotor
    {
        WatchDog_Feed();
        SST25V_SectorErase_4KByte(ISP_StartArea + i * 0x1000);
        DF_delay_ms(220);
        WatchDog_Feed();
    }
    //----------- erase  32k
    for(i = 0; i < 7; i++) // 56sector
    {
        WatchDog_Feed();
        SST25V_BlockErase_32KByte(0x8000 + i * 0x8000);
        DF_delay_ms(800);
        WatchDog_Feed();
    }
    DF_RELEASE;
    rt_kprintf("\r\n  ISP erase OK DFReady\r\n");
}
//FINSH_FUNCTION_EXPORT(DF_Erase, DF_Erase);

void DF_WriteFlashDirect(u16 page_counter, u16 page_offset, u8 *p, u16 length) //512bytes ֱ�Ӵ洢
{
    u16 i = 0;

    for(i = 0; i < length; i++)
    {
        SST25V_ByteWrite(*p, (u32)page_counter * PageSIZE + (u32)(page_offset + i));
        p++;
    }
    DF_delay_ms(5);
}



