/* The shim over the gfx, audio and shaderc layers. See rhat_gfx.h. */

#include "rhat_gfx.h"

#include "audio_api.h"
#include "gfx_api.h"
#include "shaderc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char shader_error[1024];

static gs_gfx_bytes bytes(const uint8_t *data, int64_t len)
{
    return (gs_gfx_bytes){(uint8_t *)data, len};
}

static int64_t copy_text(const char *text, uint8_t *out, int64_t cap)
{
    int64_t n = (int64_t)strlen(text);
    n = n < cap ? n : cap;
    memcpy(out, text, (size_t)n);
    return n;
}

uint8_t rg_open(const uint8_t *title, int64_t title_len, int64_t width, int64_t height, int64_t flags)
{
    return gs_gfx_open(bytes(title, title_len), width, height, flags);
}

void rg_close(void)
{
    gs_gfx_close();
}

uint8_t rg_frame(void)
{
    return gs_gfx_frame();
}

int64_t rg_error(uint8_t *out, int64_t cap)
{
    if (shader_error[0]) {
        int64_t n = copy_text(shader_error, out, cap);
        shader_error[0] = 0;
        return n;
    }
    return gs_gfx_error(bytes(out, cap));
}

int64_t rg_misuse_count(void)
{
    return gs_gfx_misuse_count();
}

int64_t rg_screen_width(void)
{
    gs_gfx_int2 size;
    gs_gfx_screen_size(&size);
    return size.x;
}

int64_t rg_screen_height(void)
{
    gs_gfx_int2 size;
    gs_gfx_screen_size(&size);
    return size.y;
}

double rg_delta_time(void)
{
    return gs_gfx_delta_time();
}

double rg_time(void)
{
    return gs_gfx_time();
}

uint8_t rg_key_down(const uint8_t *name, int64_t len)
{
    return gs_gfx_key_down(bytes(name, len));
}

uint8_t rg_key_pressed(const uint8_t *name, int64_t len)
{
    return gs_gfx_key_pressed(bytes(name, len));
}

uint8_t rg_mouse_down(int64_t button)
{
    return gs_gfx_mouse_down(button);
}

uint8_t rg_mouse_pressed(int64_t button)
{
    return gs_gfx_mouse_pressed(button);
}

float rg_mouse_dx(void)
{
    gs_gfx_float2 d;
    gs_gfx_mouse_delta(&d);
    return d.x;
}

float rg_mouse_dy(void)
{
    gs_gfx_float2 d;
    gs_gfx_mouse_delta(&d);
    return d.y;
}

uint8_t rg_focused(void)
{
    return gs_gfx_focused();
}

uint8_t rg_set_mouse_relative(uint8_t on)
{
    return gs_gfx_set_mouse_relative(on);
}

uint8_t rg_mouse_relative(void)
{
    return gs_gfx_mouse_relative();
}

uint8_t rg_focus_lost(void)
{
    gs_gfx_event events[64];
    int64_t n = gs_gfx_events((gs_gfx_event_slice){events, 64});
    for (int64_t i = 0; i < n && i < 64; i++) {
        if (events[i].kind == GS_GFX_EVENT_FOCUS_LOST) {
            return 1;
        }
    }
    return 0;
}

uint32_t rg_create_buffer(int64_t usage, int64_t size, const uint8_t *data, int64_t len)
{
    return gs_gfx_create_buffer(usage, size, bytes(data, len)).id;
}

uint8_t rg_update_buffer(uint32_t buffer, int64_t offset, const uint8_t *data, int64_t len)
{
    return gs_gfx_update_buffer((gs_gfx_buffer){buffer}, offset, bytes(data, len));
}

int64_t rg_buffer_size(uint32_t buffer)
{
    return gs_gfx_buffer_size((gs_gfx_buffer){buffer});
}

void rg_release_buffer(uint32_t buffer)
{
    gs_gfx_release_buffer((gs_gfx_buffer){buffer});
}

uint32_t rg_create_texture(int64_t format, int64_t usage, int64_t width, int64_t height)
{
    gs_gfx_texture_desc desc = {.kind = GS_GFX_TEXTURE_2D,
                                .format = (int32_t)format,
                                .usage = (int32_t)usage,
                                .width = (int32_t)width,
                                .height = (int32_t)height,
                                .depth = 1,
                                .mips = 1,
                                .samples = 1};
    return gs_gfx_create_texture(&desc).id;
}

uint8_t rg_update_texture(uint32_t texture, const uint8_t *data, int64_t len)
{
    gs_gfx_region whole = {0};
    return gs_gfx_update_texture((gs_gfx_texture){texture}, &whole, bytes(data, len));
}

void rg_release_texture(uint32_t texture)
{
    gs_gfx_release_texture((gs_gfx_texture){texture});
}

uint32_t rg_create_sampler(int64_t filter, int64_t wrap)
{
    gs_gfx_sampler_desc desc = {.min_filter = (int32_t)filter,
                                .mag_filter = (int32_t)filter,
                                .mip_filter = (int32_t)filter,
                                .wrap_u = (int32_t)wrap,
                                .wrap_v = (int32_t)wrap,
                                .wrap_w = (int32_t)wrap,
                                .max_anisotropy = 1.0f};
    return gs_gfx_create_sampler(&desc).id;
}

uint32_t rg_screen_depth(void)
{
    return gs_gfx_screen_depth().id;
}

// Compiles one stage; on failure the message waits in shader_error.
static gs_shaderc_result compile(const uint8_t *source, int64_t len, int stage, const char *name)
{
    char *text = malloc((size_t)len + 1);
    memcpy(text, source, (size_t)len);
    text[len] = 0;
    gs_shaderc_result r = gs_shaderc_compile(text, name, stage);
    free(text);
    if (r.error) {
        snprintf(shader_error, sizeof shader_error, "%s", r.error);
    }
    return r;
}

uint32_t rg_create_pipeline(const uint8_t *vs, int64_t vs_len, const uint8_t *fs, int64_t fs_len,
                            int64_t cull, int64_t blend, uint8_t depth_test, uint8_t depth_write,
                            const uint8_t *formats)
{
    gs_shaderc_result v = compile(vs, vs_len, GS_GFX_VERTEX, "vertex shader");
    if (v.error) {
        gs_shaderc_free(&v);
        return 0;
    }
    gs_shaderc_result f = compile(fs, fs_len, GS_GFX_FRAGMENT, "fragment shader");
    uint32_t id = 0;
    if (!f.error) {
        gs_gfx_pipeline_desc desc = {.primitive = GS_GFX_TRIANGLES,
                                     .cull = (int32_t)cull,
                                     .depth_test = depth_test,
                                     .depth_write = depth_write,
                                     .depth_compare = GS_GFX_COMPARE_LESS,
                                     .blend = (int32_t)blend,
                                     .instance_location = -1};
        memcpy(desc.formats, formats, sizeof desc.formats);
        id = gs_gfx_create_pipeline(bytes(v.blob, (int64_t)v.size), bytes(f.blob, (int64_t)f.size), &desc).id;
    }
    gs_shaderc_free(&v);
    gs_shaderc_free(&f);
    return id;
}

void rg_release_pipeline(uint32_t pipeline)
{
    gs_gfx_release_pipeline((gs_gfx_pipeline){pipeline});
}

uint8_t rg_begin_screen(float r, float g, float b, float a, uint8_t with_depth)
{
    gs_gfx_pass_desc desc = {.clear_color = 1, .clear_depth = with_depth, .depth_value = 1.0f};
    desc.color[0] = gs_gfx_screen();
    desc.color_value = (gs_gfx_float4){r, g, b, a};
    if (with_depth) {
        desc.depth = gs_gfx_screen_depth();
    }
    return gs_gfx_begin_pass(&desc);
}

void rg_end_pass(void)
{
    gs_gfx_end_pass();
}

void rg_bind_pipeline(uint32_t pipeline)
{
    gs_gfx_bind_pipeline((gs_gfx_pipeline){pipeline});
}

void rg_bind_vertex_buffer(int64_t slot, uint32_t buffer, int64_t offset)
{
    gs_gfx_bind_vertex_buffer(slot, (gs_gfx_buffer){buffer}, offset);
}

void rg_bind_texture(int64_t stage, int64_t slot, uint32_t texture, uint32_t sampler)
{
    gs_gfx_bind_texture(stage, slot, (gs_gfx_texture){texture}, (gs_gfx_sampler){sampler});
}

void rg_push_uniforms(int64_t stage, int64_t slot, const uint8_t *data, int64_t len)
{
    gs_gfx_push_uniforms(stage, slot, bytes(data, len));
}

void rg_viewport(float x, float y, float w, float h)
{
    gs_gfx_viewport(x, y, w, h);
}

void rg_draw(int64_t vertices)
{
    gs_gfx_draw(vertices, 1, 0, 0);
}

uint8_t rg_screenshot(const uint8_t *path, int64_t len)
{
    return gs_gfx_save_png(gs_gfx_screen(), bytes(path, len));
}

uint8_t rg_audio_open(int64_t sample_rate)
{
    return gs_audio_open(sample_rate, 0);
}

void rg_audio_close(void)
{
    gs_audio_close();
}

uint64_t rg_audio_create_sound(const float *samples, int64_t count, int64_t sample_rate, int64_t channels)
{
    return gs_audio_create_sound((gs_audio_f32_slice){(float *)samples, count}, sample_rate, channels).id;
}

int64_t rg_audio_error(uint8_t *out, int64_t cap)
{
    return gs_audio_error((gs_audio_bytes){out, cap});
}

uint8_t rg_audio_release_sound(uint64_t sound)
{
    return gs_audio_release_sound((gs_audio_sound){sound});
}

uint64_t rg_audio_play(uint64_t sound, float volume, float pan, float pitch, uint8_t loop)
{
    return gs_audio_play((gs_audio_sound){sound}, volume, pan, pitch, loop).id;
}

uint8_t rg_audio_playing(uint64_t voice)
{
    return gs_audio_playing((gs_audio_voice){voice});
}

void rg_audio_stop(uint64_t voice)
{
    gs_audio_stop((gs_audio_voice){voice});
}

void rg_audio_stop_all(void)
{
    gs_audio_stop_all();
}
