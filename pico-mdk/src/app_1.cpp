
#include "egl_device_lib.h"
#include "egl_texture.h"

EGL_USING_NAMESPACE

typedef graphics_device_st7735 gfx_device_t;

static gfx_device_t s_gfx_device;

static const EGL_ALIGN(4) uint32_t s_ptx_smiley_data[] = {
#include "ptx_smiley.h"
};
static texture                                                    s_tex_smiley;
typedef sampler2d<pixfmt_r5g6b5, texfilter_linear, texaddr_wrap>  smp_r5g6b5_linear_wrap_t;
typedef sampler2d<pixfmt_r5g6b5, texfilter_linear, texaddr_clamp> smp_r5g6b5_linear_clamp_t;

template <class Sampler>
struct test_shader {
    // construction
    test_shader(const texture* tex_, int16_t x_, int16_t y_, float scale_x_, float scale_y_, const color_rgbaf& tint_)
        : tex(tex_), x(x_), y(y_), scale_x(scale_x_), scale_y(scale_y_), tint(tint_) {}
    //--------------------------------------------------------------------------

    // pixel shading
    template <e_pixel_format dst_fmt>
    EGL_INLINE void exec(pixel<dst_fmt>& res_, uint16_t x_, uint16_t y_) const
    {
        // Finally, lets sample the texture. First we calculate the texture
        // coordinates relatively to the coordinates we passed to the shader
        // constructor and scale it with the passed scaling values. Remember
        // that texture coordinates vary between [0, 1] across the texture.
        float       u = (x_ - x) * scale_x;
        float       v = (y_ - y) * scale_y;
        color_rgbaf col;
        sampler.sample(col, *tex, vec2f(u, v));
        res_ = col * tint;
    }

    const texture* tex;
    int16_t        x, y;
    float          scale_x, scale_y;
    color_rgbaf    tint;
    Sampler        sampler;
};
#ifdef __cplusplus
extern "C" {
#endif
void app_entry(void* arg)
{

    s_tex_smiley.init(s_ptx_smiley_data);

    int16_t x, y, w, h;

    uint8_t i = 0;
    while (1) {
        LCD_0IN96_Clear(BLACK);
        switch (i) {
            case 0: {
                // Draw 50x50px quad to the upper left corner. The original texture is 64x64px
                // so this downscales it a bit.
                x = 0, y = 0, w = 50, h = 50;
                test_shader<smp_r5g6b5_linear_clamp_t> sh(&s_tex_smiley, x, y, 1.0f / w, 1.0f / h, color_rgbaf(1.0f, 1.0f, 1.0f, 1.0f));
                s_gfx_device.draw_rect(x, y, w, h, sh);
                break;
            }
            case 1: {
                // Let's now draw 100x100 rectangle next to the previous rectangle. Because the
                // rectangle is larger, it upscales the texture a bit instead.
                x = 0, y = 0, w = 30, h = 30;
                test_shader<smp_r5g6b5_linear_clamp_t> sh(&s_tex_smiley, x, y, 1.0f / w, 1.0f / h, color_rgbaf(1.0f, 1.0f, 1.0f, 1.0f));
                s_gfx_device.draw_rect(x, y, w, h, sh);
                break;
            }
            case 2: {
                // Let's try the wrap address mode now! Let's draw 64x64 quad but tile the texture
                // twice horizontally and vertically.
                x = 0, y = 0, w = 64, h = 64;
                test_shader<smp_r5g6b5_linear_clamp_t> sh(&s_tex_smiley, x, y + 64, 1.0f / w, -1.0f / h, color_rgbaf(1.0f, 0.5f, 0.5f, 0.5f));
                s_gfx_device.draw_rect(x, y, w, h, sh);
                break;
            }
            case 3:
                // Let's try the wrap address mode now! Let's draw 64x64 quad but tile the texture
                // twice horizontally and vertically.
                {
                    x = 0, y = 0, w = 64, h = 64;
                    test_shader<smp_r5g6b5_linear_wrap_t> sh(&s_tex_smiley, x, y, 2.0f / w, 2.0f / h, color_rgbaf(1.0f, 1.0f, 1.0f, 1.0f));
                    s_gfx_device.draw_rect(x, y, w, h, sh);
                    break;
                }
            case 4:
                // We don't have to scale the rectangle uniformly but can squeeze it too!
                {
                    x = 0, y = 0, w = 100, h = 50;
                    test_shader<smp_r5g6b5_linear_clamp_t> sh(&s_tex_smiley, x, y, 1.0f / w, 1.0f / h, color_rgbaf(1.0f, 1.0f, 1.0f, 1.0f));
                    s_gfx_device.draw_rect(x, y, w, h, sh);
                    break;
                }
        }
        
        ++i;
        if (i > 4)
            i -= 4;
        //delay
    }
}

#ifdef __cplusplus
}
#endif