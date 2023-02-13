#include "egl_device_lib.h"

#define LCD_CS_Pin GPIO_PIN_4
#define LCD_CS_GPIO_Port GPIOA
#define LCD_SCL_Pin GPIO_PIN_5
#define LCD_SCL_GPIO_Port GPIOA
#define LCD_SDA_Pin GPIO_PIN_7
#define LCD_SDA_GPIO_Port GPIOA
#define LCD_DC_Pin GPIO_PIN_4
#define LCD_DC_GPIO_Port GPIOC
#define LCD_RES_Pin GPIO_PIN_5
#define LCD_RES_GPIO_Port GPIOC
#define LCD_BL_Pin GPIO_PIN_14
#define LCD_BL_GPIO_Port GPIOB

EGL_USING_NAMESPACE

typedef graphics_device_st7735 gfx_device_t;

static gfx_device_t s_gfx_device;

// test_shader

struct test_shader {
    test_shader(int16_t x_, int16_t y_, uint8_t frame_) : x(x_), y(y_), frame(frame_) {}

    // Pixel shader function executed for every rectangle pixel. This function
    // is called with the dst_fmt template argument matching the display native
    // pixel format. Different devices may use different native pixel format
    // (e.g. r5g6b5 or r8b8g8a8) but function doesn't need to be changed because
    // by calling res_.set_rgba8() the passed 8-bit RGBA values are converted to
    // the appropriate pixel format. We also use EGL_INLINE to ensure that this
    // function gets inlined and doesn't have function call overhead.
    template <e_pixel_format dst_fmt>
    EGL_INLINE void exec(pixel<dst_fmt>& res_, uint16_t x_, uint16_t y_) const
    {
        res_.set_rgba8(/*R*/ (uint8_t)(x_ - x) * 2, /*G*/ (uint8_t)(y_ - y) * 2, /*B*/ frame, 0);
    }

    int16_t x, y;   
    uint8_t frame;  
};

#ifdef __cplusplus
extern "C" {
#endif
void app_entry(void* arg)
{


    // Draw rectangle of width rect_width and height rect_height in the middle
    // of the display using the immediate mode draw_rect() function and the
    // shader we defined above. For the shader we pass the rectangle coordinates
    // and frame count as we defined it above.
    // Note that here we use draw_rect() but there's also fast_draw_rect(). The
    // difference is that draw_rect() clips the rectangle coordinates to the
    // screen bounds, while the fast-version doesn't, so it's a bit faster
    // (particularly if you draw a lot of small rectangles). If you know that
    // the rectangle is completely within the display (like in our case, unless if
    // you render to a tiny display), you could use the fast_draw_rect(). It's
    // "undefined behavior" to draw outside the screen bounds with a fast-function
    // (e.g. could assert, or have strange visual results depending on display).
    uint8_t s_frame = 0;  
    enum { rect_width  = 70,
           rect_height = 70 };

    int16_t x = gfx_device_t::fb_width / 2 - rect_width / 2;  
    int16_t y = gfx_device_t::fb_height / 2 - rect_height / 2;

    int16_t x1 = gfx_device_t::fb_width / 2;
    int16_t x2 = gfx_device_t::fb_width / 2 - rect_width;

    while (1) {
        s_gfx_device.draw_rect(x1, y, rect_width, rect_height, test_shader(x1, y, s_frame));
        s_gfx_device.draw_rect(x2, y, rect_width, rect_height, test_shader(x2, y, 255 - s_frame));
        s_frame += 1;
    }
}

#ifdef __cplusplus
}
#endif