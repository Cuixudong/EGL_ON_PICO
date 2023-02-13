#include "egl_device_lib.h"
#include "egl_mesh.h"
#include "egl_vertex.h"

EGL_USING_NAMESPACE

typedef graphics_device_st7735 gfx_device_t;

static gfx_device_t s_gfx_device;

// This monkey geometry has two segments, where the eyes and head are separated
// to different segments. You can open 03_MultiDispatch/monkey_2mat.dae in
// Blender to see the two segments.
static const EGL_ALIGN(4) uint32_t s_p3g_monkey_2mat_data[] = {
#include "p3g_monkey_2mat.h"
};
static p3g_mesh       s_p3g_monkey_2mat;
typedef vertex_p48n32 vtx_monkey_t;
// This is the torus geometry. Unlike in the previous example, this is generated
// with vertex format "pn"
static const EGL_ALIGN(4) uint32_t s_p3g_torus_data[] = {
#include "p3g_torus_2.h"
};
static p3g_mesh   s_p3g_torus;
typedef vertex_pn vtx_torus_t;

struct rasterizer_memory_cfg : rasterizer_memory_cfg_base {
    enum { rt0_fmt = gfx_device_t::fb_format };
    enum { pso_store_size = 2048 };
    enum { vcache_size = 2048 };
    enum { max_dma_transfers = 0 };
    enum { dma_buffer_px = 0 };

};

static rasterizer_memory<gfx_device_t, rasterizer_memory_cfg> s_gfx_device_mem;
static cameraf                                                s_camera;
static vec3f                                                  s_light_dir;

// Simple vertex shader that transforms vertex position and calculates n.l
struct vs_ndotl {
    struct vsout {
        float ndotl;
    };

    template <class PSIn, class PSOState, class VIn>
    EGL_INLINE void exec(PSIn& psin_, const PSOState& pso_state_, const typename VIn::transform_state& tstate_, const VIn& vin_, uint8_t) const
    {
        psin_.pos = vec4f(get_pos(vin_, tstate_), 1.0f) * pso_state_.obj_to_proj;
        vsout vo;
        vo.ndotl = dot(get_normal(vin_, tstate_), light_obj_dir);
        psin_.set_attribs(vo);
    }

    vec3f light_obj_dir;
};

// Simple pixel shader which interpolates ndotl and multiplies the abs() of the
// result with given color.
struct ps_ndotl {
    struct psin {
        vec4f pos;
        float ndotl;
        template <class VSOut>
        EGL_INLINE void set_attribs(const VSOut& vo_) { ndotl = vo_.ndotl; }
    };

    template <class PSC>
    EGL_INLINE void exec(rasterizer_pixel_out<PSC>& psout_, const typename PSC::pso_state&, const psin& v0_, const psin& v1_, const psin& v2_, const vec3f& bc_, uint8_t) const
    {
        float ndotl = v0_.ndotl * bc_.x + v1_.ndotl * bc_.y + v2_.ndotl * bc_.z;
        psout_.export_rt0(color * abs(ndotl));
    }

    color_rgbaf color;
};

// Simple pixel shader which outputs a constant color.
struct ps_color {
    struct psin {
        vec4f pos;

        template <class VSOut>
        EGL_INLINE void set_attribs(const VSOut&) {}
    };

    template <class PSC>
    EGL_INLINE void exec(rasterizer_pixel_out<PSC>& psout_, const typename PSC::pso_state&, const psin&, const psin&, const psin&, const vec3f&, uint8_t) const
    {
        psout_.export_rt0(color);
    }

    color_rgbaf color;
};

// rstate_opaque

struct rstate_opaque : rasterizer_psc_base {
    enum { rt0_fmt = rasterizer_memory_cfg::rt0_fmt };
    enum { depth_format = rasterizer_memory_cfg::depth_format };
};

#ifdef __cplusplus
extern "C" {
#endif
void app_entry(void* arg)
{
    s_gfx_device_mem.init(s_gfx_device);
    s_gfx_device.clear_depth(cleardepth_max);

    s_p3g_monkey_2mat.init(s_p3g_monkey_2mat_data);
    s_p3g_torus.init(s_p3g_torus_data);

    float  cam_fov          = 60.0f;
    float  cam_aspect_ratio = float(gfx_device_t::fb_width) / float(gfx_device_t::fb_height);
    float  cam_near_plane   = 0.1f;
    float  cam_far_plane    = 100.0f;
    mat44f view2proj        = perspective_matrix<float>(cam_fov * mathf::deg_to_rad, cam_aspect_ratio, cam_near_plane, cam_far_plane);
    s_camera.set_view_to_proj(view2proj, cam_near_plane, cam_far_plane);
    s_camera.set_view_to_world(tform3f::identity());

    s_light_dir = unit(vec3f(1.0f, 1.0f, -1.0f));

    while (1) {
        vec3f  monkey_pos(0.0f, 0.0f, 4.0f);
        mat33f monkey_rot(1.0f, 0.0f, 0.0f,
                          0.0f, 0.0f, -1.0f,
                          0.0f, 1.0f, 0.0f);

        psc_p3g_mesh<rstate_opaque, vtx_monkey_t, vs_ndotl, ps_ndotl> pso_monkey_0;
        pso_monkey_0.set_geometry(s_p3g_monkey_2mat, 0);
        pso_monkey_0.set_transform(s_camera, tform3f(monkey_rot, monkey_pos));
        pso_monkey_0.vshader().light_obj_dir = unit(monkey_rot * s_light_dir);
        pso_monkey_0.pshader().color.set(0.0f, 1.0f, 0.0f);
        s_gfx_device.dispatch_pso(pso_monkey_0);  

        psc_p3g_mesh<rstate_opaque, vtx_monkey_t, vs_ndotl, ps_color> pso_monkey_1;
        pso_monkey_1.set_geometry(s_p3g_monkey_2mat, 1);
        pso_monkey_1.set_transform(s_camera, tform3f(monkey_rot, monkey_pos));
        pso_monkey_1.pshader().color.set(1.0f, 0.0f, 0.0f);  
        s_gfx_device.dispatch_pso(pso_monkey_1);

        psc_p3g_mesh<rstate_opaque, vtx_torus_t, vs_ndotl, ps_ndotl> pso_torus;
        static const float                                           s_rad         = 2.0f;  
        static const float                                           s_torus_scale = 0.5f;  
        static float                                                 s_t           = 0.2f;
        s_t += 0.15f;
        mat33f torus_rot;
        set_rotation_xyz(torus_rot, s_t * 2.2f, s_t * 3.7f, 0.0f);
        pso_torus.set_geometry(s_p3g_torus, 0);
        pso_torus.set_transform(s_camera, tform3f(torus_rot * s_torus_scale, monkey_pos + vec3f(cos(s_t) * s_rad, 0.0f, sin(s_t) * s_rad)));
        pso_torus.vshader().light_obj_dir = unit(torus_rot * s_light_dir);
        pso_torus.pshader().color.set(0.0f, 0.0f, 1.0f);  
        s_gfx_device.dispatch_pso(pso_torus);

        s_gfx_device.commit();
    }
}

#ifdef __cplusplus
}
#endif