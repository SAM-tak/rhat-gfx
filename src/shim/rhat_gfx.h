/* The gfx and audio layers as R^ calls them: scalars and (pointer, length)
   pairs only. The layers' own API passes {data, len} slices and packed structs
   by value, which an externC^ declaration cannot express. Handles are u32. */

#ifndef RHAT_GFX_H
#define RHAT_GFX_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Device, window, frames. Text is bytes with a length; no NUL is needed. */
uint8_t rg_open(const uint8_t *title, int64_t title_len, int64_t width, int64_t height, int64_t flags);
void rg_close(void);
uint8_t rg_frame(void);
/* The last error of the gfx layer or of a shader compile, copied to out. */
int64_t rg_error(uint8_t *out, int64_t cap);
int64_t rg_misuse_count(void);
int64_t rg_screen_width(void);
int64_t rg_screen_height(void);
double rg_delta_time(void);
double rg_time(void);

/* Input. A key is named as SDL names it ("W", "Left Shift", "Escape"). */
uint8_t rg_key_down(const uint8_t *name, int64_t len);
uint8_t rg_key_pressed(const uint8_t *name, int64_t len);
uint8_t rg_mouse_down(int64_t button);
uint8_t rg_mouse_pressed(int64_t button);
float rg_mouse_dx(void);
float rg_mouse_dy(void);
uint8_t rg_focused(void);
uint8_t rg_set_mouse_relative(uint8_t on);
uint8_t rg_mouse_relative(void);
/* Whether this frame had a focus-lost event. */
uint8_t rg_focus_lost(void);
uint8_t rg_key_released(const uint8_t *name, int64_t len);
float rg_mouse_x(void);
float rg_mouse_y(void);
/* Queue input as if it came from the devices, seen at the next frame. */
uint8_t rg_inject_key(const uint8_t *name, int64_t len, uint8_t down);
void rg_inject_mouse(float x, float y, int64_t button, uint8_t down);
uint8_t rg_inject_focus(uint8_t focused);

/* Buffers and textures. */
uint32_t rg_create_buffer(int64_t usage, int64_t size, const uint8_t *data, int64_t len);
uint8_t rg_update_buffer(uint32_t buffer, int64_t offset, const uint8_t *data, int64_t len);
int64_t rg_buffer_size(uint32_t buffer);
void rg_release_buffer(uint32_t buffer);
uint32_t rg_create_texture(int64_t format, int64_t usage, int64_t width, int64_t height);
uint8_t rg_update_texture(uint32_t texture, const uint8_t *data, int64_t len);
void rg_release_texture(uint32_t texture);
uint32_t rg_create_sampler(int64_t filter, int64_t wrap);
uint32_t rg_screen_depth(void);

/* A pipeline from GLSL 450 source (compiled here, see gfx.rh); attributes
   are the vertex formats by location, 16 bytes, 0 where unused. */
uint32_t rg_create_pipeline(const uint8_t *vs, int64_t vs_len, const uint8_t *fs, int64_t fs_len,
                            int64_t cull, int64_t blend, uint8_t depth_test, uint8_t depth_write,
                            const uint8_t *formats);
void rg_release_pipeline(uint32_t pipeline);

/* Passes and drawing: a pass draws to the screen, cleared to the color. */
uint8_t rg_begin_screen(float r, float g, float b, float a, uint8_t with_depth);
void rg_end_pass(void);
void rg_bind_pipeline(uint32_t pipeline);
void rg_bind_vertex_buffer(int64_t slot, uint32_t buffer, int64_t offset);
void rg_bind_texture(int64_t stage, int64_t slot, uint32_t texture, uint32_t sampler);
void rg_push_uniforms(int64_t stage, int64_t slot, const uint8_t *data, int64_t len);
void rg_viewport(float x, float y, float w, float h);
void rg_draw(int64_t vertices);
uint8_t rg_screenshot(const uint8_t *path, int64_t len);

/* Audio: finite f32 PCM, mono or interleaved stereo. Sounds and voices are
   the audio layer's u64 handles. */
uint8_t rg_audio_open(int64_t sample_rate);
void rg_audio_close(void);
uint64_t rg_audio_create_sound(const float *samples, int64_t count, int64_t sample_rate, int64_t channels);
int64_t rg_audio_error(uint8_t *out, int64_t cap);
uint8_t rg_audio_release_sound(uint64_t sound);
uint64_t rg_audio_play(uint64_t sound, float volume, float pan, float pitch, uint8_t loop);
uint8_t rg_audio_playing(uint64_t voice);
void rg_audio_stop(uint64_t voice);
void rg_audio_stop_all(void);

#ifdef __cplusplus
}
#endif

#endif
