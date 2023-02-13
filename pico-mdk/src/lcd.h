#ifndef _DEV_LCD_H
#define _DEV_LCD_H

#ifdef __cplusplus
 extern "C" {
#endif

#define UBYTE   uint8_t
#define UWORD   uint16_t
#define UDOUBLE uint32_t

/**
 * GPIOI config
**/

#define LCD_RST_PIN  12
#define LCD_DC_PIN   8
#define LCD_BL_PIN   25
    
#define LCD_CS_PIN   9
#define LCD_CLK_PIN  10
#define LCD_MOSI_PIN 11
    
#define LCD_SCL_PIN  7
#define LCD_SDA_PIN  6

#define SPI_PORT spi1

typedef struct{
    UWORD WIDTH;
    UWORD HEIGHT;
    UBYTE SCAN_DIR;
}LCD_0IN96_ATTRIBUTES;
extern LCD_0IN96_ATTRIBUTES LCD_0IN96;

#define LCD_0IN96_HEIGHT 80
#define LCD_0IN96_WIDTH 160


#define HORIZONTAL 0
#define VERTICAL   1

#define LCD_0IN96_SetBacklight(Value) ;

/**
 * Display rotate
**/
#define ROTATE_0            0
#define ROTATE_90           90
#define ROTATE_180          180
#define ROTATE_270          270

/**
 * Display Flip
**/
typedef enum {
    MIRROR_NONE  = 0x00,
    MIRROR_HORIZONTAL = 0x01,
    MIRROR_VERTICAL = 0x02,
    MIRROR_ORIGIN = 0x03,
} MIRROR_IMAGE;
#define MIRROR_IMAGE_DFT MIRROR_NONE

/**
 * image color
**/
#define WHITE          0xFFFF
#define BLACK          0x0000
#define BLUE           0x001F
#define BRED           0XF81F
#define GRED           0XFFE0
#define GBLUE          0X07FF
#define RED            0xF800
#define MAGENTA        0xF81F
#define GREEN          0x07E0
#define CYAN           0x7FFF
#define YELLOW         0xFFE0
#define BROWN          0XBC40
#define BRRED          0XFC07
#define GRAY           0X8430


void DEV_Delay_ms(UDOUBLE xms);
void DEV_Delay_us(UDOUBLE xus);
void DEV_Digital_Write(UWORD Pin, UBYTE Value);
UBYTE DEV_Digital_Read(UWORD Pin);
void DEV_SPI_WriteByte(uint8_t Value);
void DEV_SPI_Write_nByte(uint8_t pData[], uint32_t Len);
void DEV_GPIO_Mode(UWORD Pin, UWORD Mode);
void DEV_SET_PWM(uint8_t Value);
void LCD_0IN96_Reset(void);
void LCD_0IN96_SendCommand(UBYTE Reg);
void LCD_0IN96_SendData_8Bit(UBYTE Data);
void LCD_0IN96_SendData_16Bit(UWORD Data);
void LCD_0IN96_InitReg(void);

void lcd_gpio_init(void);
void lcd_init(void);
void LCD_0IN96_SetWindows(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend);
void LCD_0IN96_Clear(UWORD Color);
void LCD_0IN96_Display(UWORD *Image);
void LCD_0IN96_DisplayWindows(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD *Image);
void LCD_0IN96_DisplayPoint(UWORD X, UWORD Y, UWORD Color);

#ifdef __cplusplus
}
#endif

#endif
