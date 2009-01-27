/*
 * r600_demo
 *
 * Copyright (C) 2008-2009  Matthias Hopf
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

/*
 * Performance tests
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "r600_reg.h"
#include "r600_emit.h"
#include "r600_lib.h"
#include "r600_state.h"
#include "r600_init.h"
#include "r600_shader.h"


#define MAX_NUM_QUADS		32768
#define RENDER_QUAD_WIDTH	480		// Not 1:1 by intention
#define RENDER_QUAD_HEIGHT	600

/*
 * Test rasterization performance with one texture
 */

void test_tex_quad_perf(adapter_t *adapt)
{
    static uint32_t vs[] = {
	// CF INST 0
	CF_DWORD0(ADDR(4)),
	CF_DWORD1(POP_COUNT(0),
		  CF_CONST(0),
		  COND(SQ_CF_COND_ACTIVE),
		  COUNT(2),
		  CALL_COUNT(0),
		  END_OF_PROGRAM(0),
		  VALID_PIXEL_MODE(0),
		  CF_INST(SQ_CF_INST_VTX),
		  WHOLE_QUAD_MODE(0),
		  BARRIER(1)),
	// CF INST 1
	CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(CF_POS0),
				TYPE(SQ_EXPORT_POS),
				RW_GPR(1),
				RW_REL(ABSOLUTE),
				INDEX_GPR(0),
				ELEM_SIZE(0)),
	CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
				     SRC_SEL_Y(SQ_SEL_Y),
				     SRC_SEL_Z(SQ_SEL_Z),
				     SRC_SEL_W(SQ_SEL_W),
				     R6xx_ELEM_LOOP(0),
				     BURST_COUNT(0),
				     END_OF_PROGRAM(0),
				     VALID_PIXEL_MODE(0),
				     CF_INST(SQ_CF_INST_EXPORT_DONE),
				     WHOLE_QUAD_MODE(0),
				     BARRIER(1)),
	// CF INST 2
	CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(0),
				TYPE(SQ_EXPORT_PARAM),
				RW_GPR(0),
				RW_REL(ABSOLUTE),
				INDEX_GPR(0),
				ELEM_SIZE(0)),
	CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
				     SRC_SEL_Y(SQ_SEL_Y),
				     SRC_SEL_Z(SQ_SEL_Z),
				     SRC_SEL_W(SQ_SEL_W),
				     R6xx_ELEM_LOOP(0),
				     BURST_COUNT(0),
				     END_OF_PROGRAM(1),
				     VALID_PIXEL_MODE(0),
				     CF_INST(SQ_CF_INST_EXPORT_DONE),
				     WHOLE_QUAD_MODE(0),
				     BARRIER(0)),
	// padding vtx/tex inst are 128 bit aligned
	0x00000000,
	0x00000000,
	// VTX INST 0
	VTX_DWORD0(VTX_INST(SQ_VTX_INST_FETCH),
		   FETCH_TYPE(SQ_VTX_FETCH_VERTEX_DATA),
		   FETCH_WHOLE_QUAD(0),
		   BUFFER_ID(0),
		   SRC_GPR(0),
		   SRC_REL(ABSOLUTE),
		   SRC_SEL_X(SQ_SEL_X),
		   MEGA_FETCH_COUNT(16)),
	VTX_DWORD1_GPR(DST_GPR(1), DST_REL(0),
		       DST_SEL_X(SQ_SEL_X),
		       DST_SEL_Y(SQ_SEL_Y),
		       DST_SEL_Z(SQ_SEL_Z),
		       DST_SEL_W(SQ_SEL_1),
		       USE_CONST_FIELDS(0),
		       DATA_FORMAT(FMT_32_32_FLOAT),
		       NUM_FORMAT_ALL(SQ_NUM_FORMAT_NORM),
		       FORMAT_COMP_ALL(SQ_FORMAT_COMP_UNSIGNED),
		       SRF_MODE_ALL(SRF_MODE_ZERO_CLAMP_MINUS_ONE)),
	VTX_DWORD2(OFFSET(0),
		   ENDIAN_SWAP(ENDIAN_NONE),
		   CONST_BUF_NO_STRIDE(0),
		   MEGA_FETCH(1)),
	VTX_DWORD_PAD,
	// VTX INST 1
	VTX_DWORD0(VTX_INST(SQ_VTX_INST_FETCH),
		   FETCH_TYPE(SQ_VTX_FETCH_VERTEX_DATA),
		   FETCH_WHOLE_QUAD(0),
		   BUFFER_ID(0),
		   SRC_GPR(0),
		   SRC_REL(ABSOLUTE),
		   SRC_SEL_X(SQ_SEL_X),
		   MEGA_FETCH_COUNT(8)),
	VTX_DWORD1_GPR(DST_GPR(0), DST_REL(0),
		       DST_SEL_X(SQ_SEL_X),
		       DST_SEL_Y(SQ_SEL_Y),
		       DST_SEL_Z(SQ_SEL_0),
		       DST_SEL_W(SQ_SEL_1),
		       USE_CONST_FIELDS(0),
		       DATA_FORMAT(FMT_32_32_FLOAT),
		       NUM_FORMAT_ALL(SQ_NUM_FORMAT_NORM),
		       FORMAT_COMP_ALL(SQ_FORMAT_COMP_UNSIGNED),
		       SRF_MODE_ALL(SRF_MODE_ZERO_CLAMP_MINUS_ONE)),
	VTX_DWORD2(OFFSET(8),
		   ENDIAN_SWAP(ENDIAN_NONE),
		   CONST_BUF_NO_STRIDE(0),
		   MEGA_FETCH(0)),
	VTX_DWORD_PAD,
    } ;

    static uint32_t ps[] = {
	// CF INST 0
	CF_DWORD0(ADDR(2)),
	CF_DWORD1(POP_COUNT(0),
		  CF_CONST(0),
		  COND(SQ_CF_COND_ACTIVE),
		  COUNT(1),
		  CALL_COUNT(0),
		  END_OF_PROGRAM(0),
		  VALID_PIXEL_MODE(0),
		  CF_INST(SQ_CF_INST_TEX),
		  WHOLE_QUAD_MODE(0),
		  BARRIER(1)),
	// CF INST 1
	CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(CF_PIXEL_MRT0),
				TYPE(SQ_EXPORT_PIXEL),
				RW_GPR(0),
				RW_REL(ABSOLUTE),
				INDEX_GPR(0),
				ELEM_SIZE(1)),
	CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
				     SRC_SEL_Y(SQ_SEL_Y),
				     SRC_SEL_Z(SQ_SEL_Z),
				     SRC_SEL_W(SQ_SEL_W),
				     R6xx_ELEM_LOOP(0),
				     BURST_COUNT(1),
				     END_OF_PROGRAM(1),
				     VALID_PIXEL_MODE(0),
				     CF_INST(SQ_CF_INST_EXPORT_DONE),
				     WHOLE_QUAD_MODE(0),
				     BARRIER(1)),
	// TEX INST 0
	TEX_DWORD0(TEX_INST(SQ_TEX_INST_SAMPLE),
		   BC_FRAC_MODE(0),
		   FETCH_WHOLE_QUAD(0),
		   RESOURCE_ID(0),
		   SRC_GPR(0),
		   SRC_REL(ABSOLUTE),
		   R7xx_ALT_CONST(0)),
	TEX_DWORD1(DST_GPR(0),
		   DST_REL(ABSOLUTE),
		   DST_SEL_X(SQ_SEL_X),
		   DST_SEL_Y(SQ_SEL_Y),
		   DST_SEL_Z(SQ_SEL_Z),
		   DST_SEL_W(SQ_SEL_W),
		   LOD_BIAS(0),
		   COORD_TYPE_X(TEX_NORMALIZED),
		   COORD_TYPE_Y(TEX_NORMALIZED),
		   COORD_TYPE_Z(TEX_NORMALIZED),
		   COORD_TYPE_W(TEX_NORMALIZED)),
	TEX_DWORD2(OFFSET_X(0),
		   OFFSET_Y(0),
		   OFFSET_Z(0),
		   SAMPLER_ID(0),
		   SRC_SEL_X(SQ_SEL_X),
		   SRC_SEL_Y(SQ_SEL_Y),
		   SRC_SEL_Z(SQ_SEL_0),
		   SRC_SEL_W(SQ_SEL_1)),
    } ;

    float *vb, *v;

    draw_config_t   draw_conf;
    cb_config_t     cb_conf;
    vtx_resource_t  vtx_res;
    tex_resource_t  tex_res;
    tex_sampler_t   tex_samp;
    shader_config_t vs_conf, ps_conf;

    uint64_t vb_addr, vs_addr, ps_addr;

    int i, render_num;
    float render_time;

    CLEAR (draw_conf);
    CLEAR (cb_conf);
    CLEAR (vtx_res);
    CLEAR (tex_res);
    CLEAR (tex_samp);
    CLEAR (vs_conf);
    CLEAR (ps_conf);


    printf ("\n* Textured Quad Performance Test\n\n");

    if (verbose) {
	dump_shader (adapt, vs, sizeof(vs), "vertex");
	dump_shader (adapt, ps, sizeof(ps), "pixel");
        printf ("\n");
    }


    /* Init */
    start_3d(adapt);
    set_default_state(adapt);


    /* Scissor / viewport */
    ereg  (PA_CL_VTE_CNTL,                      VTX_XY_FMT_bit);
    /* Not necessary due to PA_CL_VTE_CNTL */
//    pack0  (PA_CL_VPORT_XSCALE_0, 4);
//    efloat (1.0);
//    efloat (0.0);
//    efloat (1.0);
//    efloat (0.0);
    ereg  (PA_CL_CLIP_CNTL,                     CLIP_DISABLE_bit);


    /* Create vertex buffer */
    vb = v = calloc (MAX_NUM_QUADS, 4 * 4 * sizeof (float));
    for (i = 0; i < MAX_NUM_QUADS; i++) {
	*v++ = 514;                             *v++ = 2;
	*v++ =       rand() / (6.0 * RAND_MAX); *v++ =       rand() / (6.0 * RAND_MAX);
	*v++ = 514 + RENDER_QUAD_WIDTH;         *v++ = 2;
	*v++ = 1.0 - rand() / (6.0 * RAND_MAX); *v++ =       rand() / (6.0 * RAND_MAX);
	*v++ = 514 + RENDER_QUAD_WIDTH;         *v++ = 2 + RENDER_QUAD_HEIGHT;
	*v++ = 1.0 - rand() / (6.0 * RAND_MAX); *v++ = 1.0 - rand() / (6.0 * RAND_MAX);
	*v++ = 514;                             *v++ = 2 + RENDER_QUAD_HEIGHT;
	*v++ =       rand() / (6.0 * RAND_MAX); *v++ = 1.0 - rand() / (6.0 * RAND_MAX);
    }

    /* Upload */
    vs_addr = upload (adapt, vs, sizeof(vs), 0);
    ps_addr = upload (adapt, ps, sizeof(ps), 4096);
    vb_addr = upload (adapt, vb, MAX_NUM_QUADS * 4 * 4 * sizeof(float), 8192);
    free (vb);


    /* Shader */
    vs_conf.shader_addr         = vs_addr;
    vs_conf.num_gprs            = 4;
    vs_conf.stack_size          = 1;
    vs_setup                    (adapt, &vs_conf);

    ps_conf.shader_addr         = ps_addr;
    ps_conf.num_gprs            = 1;
    ps_conf.stack_size          = 0;
    ps_conf.uncached_first_inst = 1;
    ps_conf.clamp_consts        = 1;
    ps_conf.export_mode         = 2;
    ps_setup                    (adapt, &ps_conf);


    /* Texture */
    tex_res.id                  = 0;
    tex_res.w                   = 512;
    tex_res.h                   = 512;
    tex_res.pitch               = adapt->display_pitch;
    tex_res.depth               = 0;			/* ? */
    tex_res.dim                 = 1;			//2D
    tex_res.base                = adapt->display_gpu;
    tex_res.mip_base            = adapt->display_gpu;
    tex_res.format              = FMT_8_8_8_8;
    tex_res.request_size        = 0;			/* 2 ? */
    tex_res.dst_sel_x           = 0;
    tex_res.dst_sel_y           = 1;
    tex_res.dst_sel_z           = 2;
    tex_res.dst_sel_w           = 3;
    tex_res.base_level          = 0;
    tex_res.last_level          = 0;			/* 1 test */
    tex_res.perf_modulation     = 0;			/* 1 ? */
    set_tex_resource            (adapt, &tex_res);

    tex_samp.id                 = 0;
    tex_samp.clamp_x            = 0;
    tex_samp.clamp_y            = 0;
    tex_samp.clamp_z            = 0;
    tex_samp.xy_mag_filter      = 1;			/* 0: point 1:bilinear 2:bicubic */
    tex_samp.xy_min_filter      = 1;			/* 0: point 1:bilinear 2:bicubic */
    tex_samp.z_filter           = 0;			/* 0: none 1: point 2: linear */
    tex_samp.mip_filter         = 0;			/* no mipmap */
    set_tex_sampler             (adapt, &tex_samp);


    /* Render setup */
    ereg  (CB_SHADER_MASK,                      (0x0f << OUTPUT0_ENABLE_shift));
    ereg  (R7xx_CB_SHADER_CONTROL,              (RT0_ENABLE_bit));
    ereg  (CB_COLOR_CONTROL,                    (0xcc << ROP3_shift)); /* copy */

    cb_conf.id = 0;
    cb_conf.w = adapt->color_pitch;
    cb_conf.h = adapt->color_height;
    cb_conf.base = adapt->color_gpu;
    cb_conf.format = FMT_8_8_8_8;
    cb_conf.comp_swap = 0;
    cb_conf.source_format = 1;
    cb_conf.blend_clamp = 1;
    set_render_target(adapt, &cb_conf);

    ereg  (PA_SU_SC_MODE_CNTL,                  (FACE_bit			|
						 (POLYMODE_PTYPE__TRIANGLES << POLYMODE_FRONT_PTYPE_shift)	|
						 (POLYMODE_PTYPE__TRIANGLES << POLYMODE_BACK_PTYPE_shift)));
    ereg  (DB_SHADER_CONTROL,                   ((1 << Z_ORDER_shift)		| /* EARLY_Z_THEN_LATE_Z */
						 DUAL_EXPORT_ENABLE_bit)); /* Only useful if no depth export */

    
    /* Interpolator setup */
    /* Enabling flat shading needs both FLAT_SHADE_bit in SPI_PS_INPUT_CNTL_x
     * *and* FLAT_SHADE_ENA_bit in SPI_INTERP_CONTROL_0 */
    ereg  (SPI_PS_IN_CONTROL_0,                 ((1 << NUM_INTERP_shift)));
    ereg  (SPI_PS_IN_CONTROL_1,                 0);
    ereg  (SPI_PS_INPUT_CNTL_0 + (0 <<2),       ((0    << SEMANTIC_shift)	|
						 (0x03 << DEFAULT_VAL_shift)	|
						 FLAT_SHADE_bit			|
						 SEL_CENTROID_bit));
    ereg  (SPI_INTERP_CONTROL_0,                /* FLAT_SHADE_ENA_bit | */ 0);


    /* Vertex buffer setup */
    vtx_res.id              = SQ_VTX_RESOURCE_vs;
    vtx_res.vtx_size_dw     = 4;
    vtx_res.vtx_num_entries = MAX_NUM_QUADS * 4 * 4;		/* Can overcommit if necessary */
    vtx_res.mem_req_size    = 1;
    vtx_res.vb_addr         = vb_addr;
    set_vtx_resource        (adapt, &vtx_res);


    ereg  (VGT_INSTANCE_STEP_RATE_0,            0);	/* ? */
    ereg  (VGT_INSTANCE_STEP_RATE_1,            0);

    ereg  (VGT_MAX_VTX_INDX,                    vtx_res.vtx_num_entries / vtx_res.vtx_size_dw);
    ereg  (VGT_MIN_VTX_INDX,                    0);
    ereg  (VGT_INDX_OFFSET,                     0);

    flush_cmds ();

    /* Loop: Start with few quads only, if rendering less than a second, increase */
    for (render_num = 32; render_num < MAX_NUM_QUADS; render_num *= 2)
    {
	/* Draw */
	draw_conf.prim_type          = DI_PT_QUADLIST;
	draw_conf.vgt_draw_initiator = DI_SRC_SEL_AUTO_INDEX;
	draw_conf.num_instances      = 1;
	draw_conf.num_indices        = render_num * 4;
	draw_conf.index_type         = DI_INDEX_SIZE_16_BIT;
	draw_auto (adapt, &draw_conf);

	render_time = time_flush_cmds (adapt, 5);
	printf ("  Rendering %d textured quads: %.3f ms\n", render_num, render_time * 1000);
	if (render_time >= 1)
	    break;
    }
    printf ("\n  Rendering speed: %.1f textured Megapixels per second\n\n",
	    (float) render_num * (RENDER_QUAD_WIDTH * RENDER_QUAD_HEIGHT / 1e6) / render_time);
}

