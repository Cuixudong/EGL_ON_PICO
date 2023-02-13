#include "egl_device_lib.h"
#include "egl_mesh.h"
#include "egl_vertex.h"
#include "egl_rasterizer.h"
EGL_USING_NAMESPACE
typedef graphics_device_st7735 gfx_device_t;
static gfx_device_t            s_gfx_device;
static const EGL_ALIGN(4) uint32_t s_p3g_monkey_data[] = {
#include "p3g_monkey.h"

};
static p3g_mesh       s_p3g_monkey;
typedef vertex_p48n32 vtx_monkey_t;
struct rasterizer_memory_cfg : rasterizer_memory_cfg_base {
    // The value below defines the used depth buffer format. For this example we
    // use 16bpp depth buffer, which is enough depth precision in this case. You
    // can also use 8bpp format or 32bpp format depending on the depth precision
    // requirements.
    enum { depth_format = depthfmt_uint16 }; 

    // Hi-Z is a low-resolution representation of the depth buffer to enable fast
    // region depth testing. This can be used for example for conservative cluster
    // occlusion culling to skip processing clusters if they are completely
    // occluded by geometry rasterized earlier in the frame. Because of this it's
    // good to rasterize opaque geometry in the order of increasing distance from
    // camera to increase the occlusion opportunities. There is a cost associated
    // in both updating and testing against Hi-Z (and some minor memory overhead),
    // but usually it's good to have Hi-Z enabled.
    enum { depth_hiz = true };

    // Here we specify the intermediate tile pixel format to be the same as the
    // native display pixel format. Everything is first rendered to the tile
    // buffer of this format that resides in RAM and after the tile has been
    // completed, the pixels are converted and transferred to the native display
    // frame buffer. Why would you use anything but the native format you ask?
    // You could do frame buffer blending (e.g. alpha or additive) which might
    // require higher precision than the native format, or want to perform some
    // operations with the rendered result (e.g. deferred lighting) using tile
    // shaders once the tile rendering is complete. Also for plain grayscale
    // rendering 8bpp pixel format could be used to really squeeze the RAM usage
    // and expand the final result to the native format upon pixel transfer.
    // You can also use up to 4 multiple render targets (MRTs) by defining the
    // format rtile*_fmt for each. By default the other render tile formats are
    // set to none and thus don't allocate memory.   
    enum { rt0_fmt = gfx_device_t::fb_format };

    // The value below defines how many PSO dispatches can be done between in a
    // frame (between commit() calls). In this example we render only a single
    // object with single material per frame so this is all we need. For better
    // explanation about PSO's check out README.md. This value must be at least
    // "Max dispatches" reported by the rasterizer stats.   
    enum { max_dispatches = 1 };

    // This defines how much memory is allocated for all the dispatched PSO's
    // between commits. The size depends how big PSO's are used to render the
    // frame. This value must be at least "Max PSO store" reported by the
    // rasterizer stats.  
    enum { pso_store_size = 128 };

    // This value defines the maximum memory required by a PSO state. This buffer
    // is allocated temporarily from the stack and setting this value to the
    // smallest required value reduces the stack usage. This value must be at
    // least "Max PSO state" reported by the rasterizer stats.   
    enum { max_pso_state_size = 128 };  

    // The frame is rendered in tiles and here We can define the tile size.
    // Larger tiles require larger memory buffer for intermediate results, while
    // for smaller tiles a single cluster needs to be binned to more tiles and
    // thus use more memory for the binning. From the performance point of view
    // larger tiles require processing more clusters per tile, while with smaller
    // tiles there are more tiles to process in total. It's really a balancing
    // act which tile size is the best fit, but this is a good starting point.   
    enum { tile_width  = 64,    
    tile_height = 64 };

    // This defines the order in which the tiles are rendered. Below we specify
    // Morton order which walks through the tiles in more local manner. The main
    // benefit is that the post-transform vertex cache is likely better utilized
    // because the same cluster is likely processed for consecutively rendered
    // tiles, thus reducing the vertex transform cost.          
    enum { tile_order = tileorder_morton };  

    // This is the maximum number of total clusters binned to tiles between
    // commits. Note that if a cluster is culled before binning (e.g. with
    // v-cones, or because it's out of screen) it doesn't count towards the
    // total cluster count. This value must be at least "Max clusters" reported
    // by the rasterizer stats.   
    enum { max_clusters = 256 };

    // For binning clusters to tiles we build a linked list of cluster index
    // strips for each tile that holds indices to the clusters binned to the
    // tile. Each strip can contain some number of cluster indices. Depending
    // how the clusters are binned we need different amount of strips and for
    // the same object this varies depending how the object is rendered.
    // It's not clear yet what's the best way to adjust this value but it's
    // better to be quite conservative of the reported value. This value must
    // be at least "Max cluster strips" reported by the rasterizer stats.   
    enum { max_cluster_strips = 256 };

    // This is the size of a temporal post-transform vertex buffer (PTV buffer)
    // to hold the vertex transform result of a single cluster for the PSC
    // tform_cluster() function. The required size depends on how many vertices
    // are in the clusters and what's the size of the PTVs (ptv_fmt) of the used
    // PSCs. Because for P3G generation we use <=64 vertices in a cluster and the
    // sizeof(ps_simple_ndotl::psin) is 20, we use value 64*20. This value must
    // be at least "Max PTV buffer size" reported by the rasterizer stats.   
    enum { max_vout_size = 64 * 20 };

    // This is the size of the PTV cache (or v-cache for short). The cache is
    // used to reduce cluster vertex transform cost across tiles, i.e. if the
    // same cluster is binned to multiple tiles, the transform result could be
    // stored in the cache and reused when the other tiles are rendered. Here
    // we just disable the v-cache by setting the size to 0. There is no minimum
    // value but a larger v-cache reduces vertex transforms.    
    enum { vcache_size = 0 };  

    // The below value defines how many tiles worth of memory we want to allocate
    // for DMA transfers. The DMA buffers are allocated in native pixel format.
    // If tiles are rasterized faster than DMA has time to transfer them to the
    // display they are queued to the buffers for transfer. Note that some tiles
    // may be faster to rasterize than others so using more buffers can help to
    // utilize wait times of faster tiles to rasterize slower tiles. Note that if
    // the device doesn't support DMA no buffers are allocated regardless of this
    // value.    
    enum { max_dma_transfers = 0 };
};
static rasterizer_memory<gfx_device_t, rasterizer_memory_cfg> s_gfx_device_mem;
static cameraf                                                s_camera;
static vec3f                                                  s_light_dir;

// This defines a vertex shader used for rendering the geometry. This vertex
// shader implements the interface expected by psc_p3g_mesh and in theory you
// could have any kind of shader interface for custom PSCs. The rasterizer
// dictates only the PSC interface as defined by rasterizer_psc_base but how
// those requirements are "forwarded" is up to the PSC.
// That said, it's good to standardize the shader interface across PSCs so that
// shaders can be more freely shared across different PSCs.
struct vs_simple_ndotl {
    struct vsout {
        float ndotl;
    };
    template <class PSIn, class PSOState, class VIn>
    EGL_INLINE void exec(PSIn& psin_, const PSOState& pso_state_, const typename VIn::transform_state& tstate_, const VIn& vin_, uint8_t) const
    {
        vec3f pos    = get_pos(vin_, tstate_);
        psin_.pos    = vec4f(pos, 1.0f) * pso_state_.obj_to_proj;
        vec3f normal = get_normal(vin_, tstate_);
        vsout vo;
        vo.ndotl = dot(normal, light_obj_dir);
        psin_.set_attribs(vo);
    }
    vec3f light_obj_dir;
};

// ps_simple_ndotl
struct ps_simple_ndotl {
    struct psin {
        vec4f pos;
        float ndotl;
        template <class VSOut>
        EGL_INLINE void set_attribs(const VSOut& vo_)
        {
            ndotl = vo_.ndotl;
        }
    };
    template <class PSC>
    EGL_INLINE void exec(rasterizer_pixel_out<PSC>& psout_, const typename PSC::pso_state&, const psin& v0_, const psin& v1_, const psin& v2_, const vec3f& bc_, uint8_t) const
    {
        float ndotl = v0_.ndotl * bc_.x + v1_.ndotl * bc_.y + v2_.ndotl * bc_.z;
        psout_.export_rt0(color * abs(ndotl));  
    }
    color_rgbaf color;
};

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
    s_p3g_monkey.init(s_p3g_monkey_data);

    float  cam_fov          = 60.0f;  
    float  cam_aspect_ratio = float(gfx_device_t::fb_width) / float(gfx_device_t::fb_height);
    float  cam_near_plane   = 0.1f;
    float  cam_far_plane    = 100.0f;
    mat44f view2proj        = perspective_matrix<float>(cam_fov * mathf::deg_to_rad, cam_aspect_ratio, cam_near_plane, cam_far_plane);
    s_camera.set_view_to_proj(view2proj, cam_near_plane, cam_far_plane);


    vec3f  cam_pos(0.0f, 0.0f, -1.5f);
    mat33f cam_rot(1.0f, 0.0f, 0.0f,
                   0.0f, 1.0f, 0.0f,
                   0.0f, 0.0f, 1.0f);

    s_camera.set_view_to_world(tform3f(cam_rot, cam_pos));
    s_light_dir = unit(vec3f(1.0f, 1.0f, -1.0f));

    while (1) {

        static float s_rot_x = -1.2f, s_rot_y = -0.5f;  
        vec3f        obj_pos(0.0f, 0.0f, 3.0f);
        mat33f       obj_rot; 
        set_rotation_xyz(obj_rot, s_rot_x, s_rot_y, 0.0f);
        s_rot_x += 0.08f;
        s_rot_y += 0.07f;

        psc_p3g_mesh<rstate_opaque, vtx_monkey_t, vs_simple_ndotl, ps_simple_ndotl> pso;

        pso.set_transform(s_camera, tform3f(obj_rot, obj_pos));

        pso.vshader().light_obj_dir = unit(obj_rot * s_light_dir);


        pso.pshader().color.set(0.5f, 1.0f, 1.0f);  

        pso.set_geometry(s_p3g_monkey, 0);

        s_gfx_device.dispatch_pso(pso);

        s_gfx_device.commit();
        static unsigned s_log_counter = 0;
        if (s_log_counter == 0) {
            s_gfx_device.log_rasterizer_stats();
            s_log_counter = 100;
        }
    }
}
#ifdef __cplusplus
}
#endif