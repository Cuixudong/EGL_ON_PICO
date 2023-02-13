#include "pico/stdlib.h"
#include "perf_counter.h"
#include "hardware/spi.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include "lcd.h"

uint slice_num;
LCD_0IN96_ATTRIBUTES LCD_0IN96;

/**
 * delay x ms
**/
void DEV_Delay_ms(UDOUBLE xms)
{
    sleep_ms(xms);
}

void DEV_Delay_us(UDOUBLE xus)
{
    sleep_us(xus);
}

/**
 * GPIO read and write
**/
void DEV_Digital_Write(UWORD Pin, UBYTE Value)
{
    gpio_put(Pin, Value);
}

UBYTE DEV_Digital_Read(UWORD Pin)
{
    return gpio_get(Pin);
}

/**
 * SPI
**/
void DEV_SPI_WriteByte(uint8_t Value)
{
    spi_write_blocking(SPI_PORT, &Value, 1);
}

void DEV_SPI_Write_nByte(uint8_t pData[], uint32_t Len)
{
    spi_write_blocking(SPI_PORT, pData, Len);
}

/**
 * GPIO Mode
**/
void DEV_GPIO_Mode(UWORD Pin, UWORD Mode)
{
    gpio_init(Pin);
    if(Mode == 0) {
        gpio_set_dir(Pin, GPIO_IN);
    } else {
        gpio_set_dir(Pin, GPIO_OUT);
    }
}

void DEV_SET_PWM(uint8_t Value)
{
    if((Value > 0) && (Value < 100))
    {
        pwm_set_chan_level(slice_num, PWM_CHAN_B, Value);
    }
}

void LCD_0IN96_Reset(void)
{
    DEV_Digital_Write(LCD_RST_PIN, 1);
    DEV_Delay_ms(200);
    DEV_Digital_Write(LCD_RST_PIN, 0);
    DEV_Delay_ms(200);
    DEV_Digital_Write(LCD_RST_PIN, 1);
    DEV_Delay_ms(200);
}

void LCD_0IN96_SendCommand(UBYTE Reg)
{
    DEV_Digital_Write(LCD_DC_PIN, 0);
    DEV_Digital_Write(LCD_CS_PIN, 0);
    DEV_SPI_WriteByte(Reg);
    DEV_Digital_Write(LCD_CS_PIN, 1);
}

void LCD_0IN96_SendData_8Bit(UBYTE Data)
{
    DEV_Digital_Write(LCD_DC_PIN, 1);
    DEV_Digital_Write(LCD_CS_PIN, 0);
    DEV_SPI_WriteByte(Data);
    DEV_Digital_Write(LCD_CS_PIN, 1);
}

void LCD_0IN96_SendData_16Bit(UWORD Data)
{
    DEV_Digital_Write(LCD_DC_PIN, 1);
    DEV_Digital_Write(LCD_CS_PIN, 0);
    DEV_SPI_WriteByte((Data >> 8) & 0xFF);
    DEV_SPI_WriteByte(Data);
    DEV_Digital_Write(LCD_CS_PIN, 1);
}

void LCD_0IN96_InitReg(void)
{
    LCD_0IN96_SendCommand(0x11);//Sleep exit 
    DEV_Delay_ms (120);
    LCD_0IN96_SendCommand(0x21); 
    LCD_0IN96_SendCommand(0x21); 

    LCD_0IN96_SendCommand(0xB1); 
    LCD_0IN96_SendData_8Bit(0x05);
    LCD_0IN96_SendData_8Bit(0x3A);
    LCD_0IN96_SendData_8Bit(0x3A);

    LCD_0IN96_SendCommand(0xB2);
    LCD_0IN96_SendData_8Bit(0x05);
    LCD_0IN96_SendData_8Bit(0x3A);
    LCD_0IN96_SendData_8Bit(0x3A);

    LCD_0IN96_SendCommand(0xB3); 
    LCD_0IN96_SendData_8Bit(0x05);  
    LCD_0IN96_SendData_8Bit(0x3A);
    LCD_0IN96_SendData_8Bit(0x3A);
    LCD_0IN96_SendData_8Bit(0x05);
    LCD_0IN96_SendData_8Bit(0x3A);
    LCD_0IN96_SendData_8Bit(0x3A);

    LCD_0IN96_SendCommand(0xB4);
    LCD_0IN96_SendData_8Bit(0x03);

    LCD_0IN96_SendCommand(0xC0);
    LCD_0IN96_SendData_8Bit(0x62);
    LCD_0IN96_SendData_8Bit(0x02);
    LCD_0IN96_SendData_8Bit(0x04);

    LCD_0IN96_SendCommand(0xC1);
    LCD_0IN96_SendData_8Bit(0xC0);

    LCD_0IN96_SendCommand(0xC2);
    LCD_0IN96_SendData_8Bit(0x0D);
    LCD_0IN96_SendData_8Bit(0x00);

    LCD_0IN96_SendCommand(0xC3);
    LCD_0IN96_SendData_8Bit(0x8D);
    LCD_0IN96_SendData_8Bit(0x6A);   

    LCD_0IN96_SendCommand(0xC4);
    LCD_0IN96_SendData_8Bit(0x8D); 
    LCD_0IN96_SendData_8Bit(0xEE); 

    LCD_0IN96_SendCommand(0xC5);  /*VCOM*/
    LCD_0IN96_SendData_8Bit(0x0E);    

    LCD_0IN96_SendCommand(0xE0);
    LCD_0IN96_SendData_8Bit(0x10);
    LCD_0IN96_SendData_8Bit(0x0E);
    LCD_0IN96_SendData_8Bit(0x02);
    LCD_0IN96_SendData_8Bit(0x03);
    LCD_0IN96_SendData_8Bit(0x0E);
    LCD_0IN96_SendData_8Bit(0x07);
    LCD_0IN96_SendData_8Bit(0x02);
    LCD_0IN96_SendData_8Bit(0x07);
    LCD_0IN96_SendData_8Bit(0x0A);
    LCD_0IN96_SendData_8Bit(0x12);
    LCD_0IN96_SendData_8Bit(0x27);
    LCD_0IN96_SendData_8Bit(0x37);
    LCD_0IN96_SendData_8Bit(0x00);
    LCD_0IN96_SendData_8Bit(0x0D);
    LCD_0IN96_SendData_8Bit(0x0E);
    LCD_0IN96_SendData_8Bit(0x10);

    LCD_0IN96_SendCommand(0xE1);
    LCD_0IN96_SendData_8Bit(0x10);
    LCD_0IN96_SendData_8Bit(0x0E);
    LCD_0IN96_SendData_8Bit(0x03);
    LCD_0IN96_SendData_8Bit(0x03);
    LCD_0IN96_SendData_8Bit(0x0F);
    LCD_0IN96_SendData_8Bit(0x06);
    LCD_0IN96_SendData_8Bit(0x02);
    LCD_0IN96_SendData_8Bit(0x08);
    LCD_0IN96_SendData_8Bit(0x0A);
    LCD_0IN96_SendData_8Bit(0x13);
    LCD_0IN96_SendData_8Bit(0x26);
    LCD_0IN96_SendData_8Bit(0x36);
    LCD_0IN96_SendData_8Bit(0x00);
    LCD_0IN96_SendData_8Bit(0x0D);
    LCD_0IN96_SendData_8Bit(0x0E);
    LCD_0IN96_SendData_8Bit(0x10);

    LCD_0IN96_SendCommand(0x3A); 
    LCD_0IN96_SendData_8Bit(0x05);

    LCD_0IN96_SendCommand(0x36);
    LCD_0IN96_SendData_8Bit(0xA8);

    LCD_0IN96_SendCommand(0x29);
}

void lcd_gpio_init(void)
{
    // SPI Config
    spi_init(SPI_PORT, 10000 * 1000);
    gpio_set_function(LCD_CLK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(LCD_MOSI_PIN, GPIO_FUNC_SPI);
    
    DEV_GPIO_Mode(LCD_RST_PIN, 1);
    DEV_GPIO_Mode(LCD_DC_PIN, 1);
    DEV_GPIO_Mode(LCD_CS_PIN, 1);
    DEV_GPIO_Mode(LCD_BL_PIN, 1);
    
    
    DEV_GPIO_Mode(LCD_CS_PIN, 1);
    DEV_GPIO_Mode(LCD_BL_PIN, 1);

    DEV_Digital_Write(LCD_CS_PIN, 1);
    DEV_Digital_Write(LCD_DC_PIN, 0);
    DEV_Digital_Write(LCD_BL_PIN, 1);
    
    // PWM Config
    gpio_set_function(LCD_BL_PIN, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(LCD_BL_PIN);
    pwm_set_wrap(slice_num, 100);
    pwm_set_chan_level(slice_num, PWM_CHAN_B, 1);
    pwm_set_clkdiv(slice_num,50);
    pwm_set_enabled(slice_num, true);
}

void lcd_init(void)
{
    lcd_gpio_init();
    DEV_SET_PWM(90);
    LCD_0IN96_Reset();
    
    LCD_0IN96.HEIGHT = LCD_0IN96_HEIGHT;
    LCD_0IN96.WIDTH  = LCD_0IN96_WIDTH;
    
    //Set the initialization register
    LCD_0IN96_InitReg();
    
    LCD_0IN96_Clear(BLACK);
}

void LCD_0IN96_SetWindows(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend)
{
    Xstart = Xstart + 1;
    Xend = Xend + 1;
    Ystart = Ystart + 26;
    Yend = Yend+26;

    LCD_0IN96_SendCommand(0x2a);
    LCD_0IN96_SendData_8Bit(Xstart >> 8);
    LCD_0IN96_SendData_8Bit(Xstart);
    LCD_0IN96_SendData_8Bit(Xend >> 8);
    LCD_0IN96_SendData_8Bit(Xend );

    LCD_0IN96_SendCommand(0x2b);
    LCD_0IN96_SendData_8Bit(Ystart >> 8);
    LCD_0IN96_SendData_8Bit(Ystart);
    LCD_0IN96_SendData_8Bit(Yend >> 8);
    LCD_0IN96_SendData_8Bit(Yend);

    LCD_0IN96_SendCommand(0x2C);
}

void LCD_0IN96_Clear(UWORD Color)
{
    UWORD j;
    UWORD Image[LCD_0IN96.WIDTH*LCD_0IN96.HEIGHT];

    for (j = 0; j < LCD_0IN96.HEIGHT*LCD_0IN96.WIDTH; j++) {
        Image[j] = Color;
    }
    
    LCD_0IN96_SetWindows(0, 0, LCD_0IN96.WIDTH-1, LCD_0IN96.HEIGHT-1);
    DEV_Digital_Write(LCD_DC_PIN, 1);
    DEV_Digital_Write(LCD_CS_PIN, 0);

    for(j = 0; j < LCD_0IN96.WIDTH*LCD_0IN96.HEIGHT; j++)
    {
        LCD_0IN96_SendData_16Bit(Color);
    }
    DEV_Digital_Write(LCD_CS_PIN, 1);
}

void LCD_0IN96_Display(UWORD *Image)
{
    UWORD j;
    LCD_0IN96_SetWindows(0, 0, LCD_0IN96.WIDTH-1, LCD_0IN96.HEIGHT-1);
    DEV_Digital_Write(LCD_DC_PIN, 1);
    DEV_Digital_Write(LCD_CS_PIN, 0);

    for (j = 0; j < LCD_0IN96.HEIGHT; j++) {
        DEV_SPI_Write_nByte((uint8_t *)&Image[j*LCD_0IN96.WIDTH], LCD_0IN96.WIDTH*2);
    }
    DEV_Digital_Write(LCD_CS_PIN, 1);
    //LCD_0IN96_SendCommand(0x29);
}

void LCD_0IN96_DisplayWindows(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD *Image)
{
    // display
    UDOUBLE Addr = 0;

    UWORD j;
    LCD_0IN96_SetWindows(Xstart, Ystart, Xend , Yend);
    DEV_Digital_Write(LCD_DC_PIN, 1);
    DEV_Digital_Write(LCD_CS_PIN, 0);
    for (j = Ystart; j < Yend - 1; j++) {
        Addr = Xstart + j * LCD_0IN96.WIDTH ;
        DEV_SPI_Write_nByte((uint8_t *)&Image[Addr], (Xend-Xstart)*2);
    }
    DEV_Digital_Write(LCD_CS_PIN, 1);
}

void LCD_0IN96_DisplayPoint(UWORD X, UWORD Y, UWORD Color)
{
    LCD_0IN96_SetWindows(X,Y,X,Y);
    LCD_0IN96_SendData_16Bit(Color);
}


