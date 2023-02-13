#ifndef EGL_DEVICE_ST7735_H
#define EGL_DEVICE_ST7735_H
//----------------------------------------------------------------------------

//============================================================================
// interface
//============================================================================
// external
#include "../egl_device.h"
#include "lcd.h"

#define lcd_data_write(x)               LCD_0IN96_SendData_16Bit(x)
#define lcd_windows_set(x1,y1,x2,y2)    LCD_0IN96_SetWindows(x1,y1,x2,y2)
#define lcd_write_data(x)               LCD_0IN96_SendData_8Bit(x)
#define lcd_write_cmd(x)                LCD_0IN96_SendCommand(x)
#define write_data(x)                   lcd_write_data(x)
#define write_cmd(x)                    lcd_write_cmd(x)

// rotation

#define ST7735_MADCTL_MY  0x80
#define ST7735_MADCTL_MX  0x40
#define ST7735_MADCTL_MV  0x20
#define ST7735_MADCTL_ML  0x10
#define ST7735_MADCTL_RGB 0x00
#define ST7735_MADCTL_BGR 0x08
#define ST7735_MADCTL_MH  0x04

// lcd config

#define ST7735_IS_160X80 1
#define ST7735_XSTART    0  // offset
#define ST7735_YSTART    24
#define ST7735_WIDTH     160  // size
#define ST7735_HEIGHT    80
#define ST7735_ROTATION  (ST7735_MADCTL_MY | ST7735_MADCTL_MV | ST7735_MADCTL_BGR)

// registers

#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_SLPIN  0x10
#define ST7735_SLPOUT 0x11
#define ST7735_PTLON  0x12
#define ST7735_NORON  0x13

#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E

#define ST7735_PTLAR  0x30
#define ST7735_COLMOD 0x3A
#define ST7735_MADCTL 0x36

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1 0xC0
#define ST7735_PWCTR2 0xC1
#define ST7735_PWCTR3 0xC2
#define ST7735_PWCTR4 0xC3
#define ST7735_PWCTR5 0xC4
#define ST7735_VMCTR1 0xC5

#define ST7735_RDID1 0xDA
#define ST7735_RDID2 0xDB
#define ST7735_RDID3 0xDC
#define ST7735_RDID4 0xDD

#define ST7735_PWCTR6 0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

EGL_NAMESPACE_BEGIN

// new
class graphics_device_st7735;
//----------------------------------------------------------------------------

//============================================================================
// graphics_device_st7735
//============================================================================

// stm32
//typedef GPIO_TypeDef*      port_t;
//typedef uint16_t           pin_t;
//typedef SPI_HandleTypeDef* spi_t;

class graphics_device_st7735 : public graphics_device<graphics_device_st7735> {
public:
    // device properties
    enum { fb_width  = ST7735_WIDTH,
           fb_height = ST7735_HEIGHT };
    enum { fb_format = pixfmt_b5g6r5 };
    typedef pixel<e_pixel_format(fb_format)> fb_format_t;

    // construction
    graphics_device_st7735();
    graphics_device_st7735(uint8_t pin_cs_, uint8_t pin_rs_, uint8_t pin_wr_, uint8_t pin_rd_, uint8_t pin_reset_=0xff);
    ~graphics_device_st7735();
    void init(uint8_t pin_cs_, uint8_t pin_rs_, uint8_t pin_wr_, uint8_t pin_rd_, uint8_t pin_reset_=0xff);
    void init_rasterizer(const rasterizer_cfg&, const rasterizer_tiling_cfg&, const rasterizer_vertex_cache_cfg&);
    void init_dma(rasterizer_data_transfer*, uint8_t num_transfers_, fb_format_t *dma_buffer_, size_t dma_buffer_size_);

    // immediate rendering interface
    template<class IPShader> EGL_INLINE void fast_draw_hline(uint16_t x_, uint16_t y_, uint16_t width_, const IPShader&);
    template<class IPShader> EGL_INLINE void fast_draw_vline(uint16_t x_, uint16_t y_, uint16_t height_, const IPShader&);
    template<class IPShader> EGL_INLINE void fast_draw_rect(uint16_t x_, uint16_t y_, uint16_t width_, uint16_t height_, const IPShader&);
private:
    graphics_device_st7735(const graphics_device_st7735&); // not implemented
    void operator=(const graphics_device_st7735&); // not implemented
    virtual void submit_tile(uint8_t tx_, uint8_t ty_, const vec2u16 &reg_min_, const vec2u16 &reg_end_, uint16_t thread_idx_);
    EGL_INLINE void set_data_pins(uint8_t);
    EGL_INLINE void write_cmd(uint8_t);
    EGL_INLINE void start_data();
    EGL_INLINE void write_data(uint8_t);
    EGL_INLINE void wait_twrl();
    EGL_INLINE void wait_twrh();
    EGL_INLINE void set_data_window(uint16_t x0_, uint16_t y0_, uint16_t x1_, uint16_t y1_);

    static graphics_device_st7735* s_active_dev;

#if defined(EGL_PLATFORM_TEENSY4X)
    typedef volatile uint32_t *port_reg_t;
#else
    typedef volatile uint8_t *port_reg_t;
#endif
    // pins
    port_reg_t m_port_cs;
    port_reg_t m_port_rs;
    port_reg_t m_port_wr;
    port_reg_t m_port_rd;
#ifdef EGL_PLATFORM_TEENSY4X
    port_reg_t m_port_data;
#endif
    uint32_t m_mask_cs;
    uint32_t m_mask_rs;
    uint32_t m_mask_wr;
    uint32_t m_mask_rd;
    // tile data
    fb_format_t *m_tile_rt0;
    rasterizer_tile_size_t m_tile_width;
    rasterizer_tile_size_t m_tile_height;
};

//============================================================================
// graphics_device_st7735
//============================================================================
template <class IPShader>
void graphics_device_st7735::fast_draw_hline(uint16_t x_, uint16_t y_, uint16_t width_, const IPShader& ips_)
{
    // draw horizontal line
    uint16_t x_end=x_+width_;
    *m_port_cs&=~m_mask_cs;
    set_data_window(x_, y_, x_end-1, y_);
    start_data();
    fb_format_t res;
    do
    {
        ips_.exec(res, x_, y_);
//        write_data(res.v>>8);
//        write_data(res.v);
        lcd_data_write(res.v);
        ++x_;
    } while(x_<x_end);
    *m_port_cs|=m_mask_cs;
}

template <class IPShader>
void graphics_device_st7735::fast_draw_vline(uint16_t x_, uint16_t y_, uint16_t height_, const IPShader& ips_)
{
    // draw vertical line
    uint16_t y_end=y_+height_;
    *m_port_cs&=~m_mask_cs;
    set_data_window(x_, y_, x_, y_end-1);
    start_data();
    fb_format_t res;
    do
    {
        ips_.exec(res, x_, y_);
//        write_data(res.v>>8);
//        write_data(res.v);
        lcd_data_write(res.v);
        ++y_;
    } while(y_<y_end);
    *m_port_cs|=m_mask_cs;
}

template <class IPShader>
void graphics_device_st7735::fast_draw_rect(uint16_t x_, uint16_t y_, uint16_t width_, uint16_t height_, const IPShader& ips_)
{
    // draw rectangle
    uint16_t x_end=x_+width_, y_end=y_+height_;
    set_data_window(x_, y_, x_end-1, y_end-1);
    fb_format_t res;
    do
    {
        uint16_t x=x_;
        do
        {
            ips_.exec(res, x, y_);
            lcd_data_write(res.v);
            ++x;
        } while(x<x_end);
        ++y_;
    } while(y_<y_end);
}

void graphics_device_st7735::set_data_window(uint16_t x0_, uint16_t y0_, uint16_t x1_, uint16_t y1_)
{
//    // setup data window

//    x0_ += ST7735_XSTART;
//    x1_ += ST7735_XSTART;
//    y0_ += ST7735_YSTART;
//    y1_ += ST7735_YSTART;

//    // column address set
//    write_cmd(ST7735_CASET);  // 0x2a
//    start_data();
//    write_data(x0_);
//    write_data(x1_);
//    // row address set
//    write_cmd(ST7735_RASET);  // 0x2b
//    start_data();
//    write_data(y0_);
//    write_data(y1_);
//    // write to RAM
//    write_cmd(ST7735_RAMWR);  // 0x2c
    lcd_windows_set(x0_, y0_, x1_, y1_);
}

void graphics_device_st7735::set_data_pins(uint8_t v_)
{
    //SET DC/RS Pin Level
}

void graphics_device_st7735::write_cmd(uint8_t cmd_)
{
//    *m_port_rs&=~m_mask_rs;
//    set_data_pins(cmd_);
//    *m_port_wr&=~m_mask_wr;
//    wait_twrl();
//    *m_port_wr|=m_mask_wr;
//    wait_twrh();
    lcd_write_cmd(cmd_);
}

void graphics_device_st7735::start_data()
{
    *m_port_rs|=m_mask_rs;
}

void graphics_device_st7735::write_data(uint8_t data_)
{
//    set_data_pins(data_);
//    *m_port_wr&=~m_mask_wr;
//    wait_twrl();
//    *m_port_wr|=m_mask_wr;
//    wait_twrh();
    lcd_write_data(data_);
}

void graphics_device_st7735::wait_twrl()
{
//    wait write control pulse low duration (min 20ns)
//    asm("nop; nop; nop; nop; nop; nop");
//    asm("nop; nop; nop; nop; nop; nop");
}

void graphics_device_st7735::wait_twrh()
{
//    wait write control pulse high duration (min 30ns)
//    asm("nop; nop; nop; nop; nop; nop");
//    asm("nop; nop; nop; nop; nop; nop");
//    asm("nop; nop; nop; nop; nop; nop");
}

EGL_NAMESPACE_END
#endif
