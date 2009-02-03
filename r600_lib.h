/*
 * r600_demo
 *
 * Copyright (C) 2008-2009  Matthias Hopf
 *
 * Based on r300_demo,
 * Copyright (C) various authors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __R600_LIB_H__
#define __R600_LIB_H__

/*
 * Helper routines, to be split up
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>


#if __SIZEOF_LONG__ == 8
#  define PRINTF_INT64		"%ld"
#  define PRINTF_UINT64		"%lu"
#  define PRINTF_UINT64_HEX	"%010lx"		/* Yes, 64bit need 16 characters, but typically 10 are enough (mostly addresses) */
#else
#  define PRINTF_INT64		"%lld"
#  define PRINTF_UINT64		"%llu"
#  define PRINTF_UINT64_HEX	"%010llx"
#endif


#define CLEAR(x) memset (&x, 0, sizeof(x))


/* Options */
extern int verbose;
extern int use_ring_directly;
extern int vertex_format;
extern int vertex_load_as_int;
extern int vertex_unsigned;
extern int vertex_alu_scale_override;
extern int do_not_flush;


enum chipset_e {
    CHIPSET_NONE = 0,
    CHIPSET_R600,
    CHIPSET_RV610, CHIPSET_RV620,		// no VC
    CHIPSET_RS780, CHIPSET_M72,			// no VC
    CHIPSET_RV630, CHIPSET_RV635, CHIPSET_RV670,
    CHIPSET_RV770,
    CHIPSET_RV710,				// no VC
    CHIPSET_RV730
};

/* TODO: Should go to _hwapi.h, probably renamed context_t */
typedef struct {
    int chipID;
    enum chipset_e chipset;

    void    *framebuffer;
    uint64_t framebuffer_gpu;

    void    *display;
    uint64_t display_gpu;
    uint32_t display_pitch;
    uint32_t display_width;
    uint32_t display_height;

    uint64_t color_gpu;
    uint32_t color_pitch;
    uint32_t color_height;
    uint64_t depth_gpu;
    uint32_t depth_pitch;
    uint32_t depth_height;
} adapter_t;

/* TODO: Should go to _hwapi.h, probably moved to adapter_t / context_t */
/* Buffers of at least 64000 bytes, for vertices and textures */
/* should be allocated in garttex, but due to bugs in DRM currently in buffer space */
/* pointers are CPU space, uint64 values GPU space */
extern uint32_t  *vtx, *tex;
extern uint64_t  vtx_gpu, tex_gpu;


void flush_gpu_source_cache (adapter_t *adapt, uint64_t lower, uint64_t upper);
void flush_gpu_dest_cache   (adapter_t *adapt, uint64_t lower, uint64_t upper);
#define FLUSH_GPU_ALL_SOURCE_CACHE(adapt) flush_gpu_source_cache (adapt, 0, 0xffffffff00ULL)

uint64_t upload (adapter_t *adapt, void *shader, int size, int offset);
void dump_shader (adapter_t *adapt, uint32_t *shader, int size, char *what);

/* FIXME: naming: this waits on the CPU, while the others let the CP wait only */
void wait_reg (adapter_t *adapt, uint32_t reg, uint32_t v, const char *when);
float wait_reg_time (adapter_t *adapt, uint32_t reg, uint32_t v, const char *when, float maxtime);
float time_flush_cmds (adapter_t *adapt, float maxtime);

void wait_3d_idle_clean(void);
void wait_3d_idle(void);

void simple_clear (adapter_t* adapt);

void soft_reset (adapter_t *adapt);
void show_state (adapter_t *adapt);

void dump_register (adapter_t *adapt, uint32_t reg);
void write_register (adapter_t *adapt, uint32_t reg, uint32_t val);

uint32_t *create_sample_texture (int width, int height, int pitch);


/* Demos functions */
/* r600_basic.c : */
void test_cp            (adapter_t *);
void test_packets       (adapter_t *);
/* r600_triangles.c : */
void tri_test_2d        (adapter_t *);
void tri_test_3d        (adapter_t *);
void pm4play_tri_test   (adapter_t *);
void pm4play_clear_test (adapter_t *);
void pm4play_blit_test  (adapter_t *);
/* r600_texture.c : */
void quad_test_tex_scaled(adapter_t *);
void tmp_test           (adapter_t *);
/* r600_exa.c : */
void test_solid(adapter_t *adapt);
void test_copy(adapter_t *adapt);
/* r600_perf.c : */
void test_perf (adapter_t *, int textype, int alutype);


#endif
