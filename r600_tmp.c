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
 * Temporary test
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


void tmp_test(adapter_t *adapt)
{
    uint32_t vs[100];

    static uint32_t ps[] = {
	// CF INST 0
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
    } ;

    typedef struct {
	float x, y;
	uint32_t argb;
    } vertex_t;
    static vertex_t vb[] = {
	{ 128,   0, 0xFFFF0000 },
	{   0, 196, 0xFF00FF00 },
	{ 256, 320, 0xFF0000FF },
	{ 192, -32, 0xFF00FFFF },
	{ 128, 128, 0xFFFF00FF },
	{ 320, 192, 0xFFFFFF00 },
	{   0,   0, 0xFFFFFFFF },
	{  64,   0, 0xFF000000 },
	{   0,  64, 0x00000000 },
    };

    int vs_len;

    int i = 0;
    // CF INST 0
    vs[i++] = CF_DWORD0(ADDR(4));
    vs[i++] = CF_DWORD1(POP_COUNT(0),
			CF_CONST(0),
			COND(SQ_CF_COND_ACTIVE),
			COUNT(2),
			CALL_COUNT(0),
			END_OF_PROGRAM(0),
			VALID_PIXEL_MODE(0),
			CF_INST(SQ_CF_INST_VTX),
			WHOLE_QUAD_MODE(0),
			BARRIER(1));
    // CF INST 1
    vs[i++] = CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(CF_POS0),
				      TYPE(SQ_EXPORT_POS),
				      RW_GPR(1),
				      RW_REL(ABSOLUTE),
				      INDEX_GPR(0),
				      ELEM_SIZE(0));
    vs[i++] = CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
					   SRC_SEL_Y(SQ_SEL_Y),
					   SRC_SEL_Z(SQ_SEL_Z),
					   SRC_SEL_W(SQ_SEL_W),
					   R6xx_ELEM_LOOP(0),
					   BURST_COUNT(0),
					   END_OF_PROGRAM(0),
					   VALID_PIXEL_MODE(0),
					   CF_INST(SQ_CF_INST_EXPORT_DONE),
					   WHOLE_QUAD_MODE(0),
					   BARRIER(1));
    // CF INST 2
    vs[i++] = CF_ALLOC_IMP_EXP_DWORD0(ARRAY_BASE(0),
				      TYPE(SQ_EXPORT_PARAM),
				      RW_GPR(2),
				      RW_REL(ABSOLUTE),
				      INDEX_GPR(0),
				      ELEM_SIZE(0));
    vs[i++] = CF_ALLOC_IMP_EXP_DWORD1_SWIZ(SRC_SEL_X(SQ_SEL_X),
					   SRC_SEL_Y(SQ_SEL_Y),
					   SRC_SEL_Z(SQ_SEL_Z),
					   SRC_SEL_W(SQ_SEL_W),
					   R6xx_ELEM_LOOP(0),
					   BURST_COUNT(0),
					   END_OF_PROGRAM(1),
					   VALID_PIXEL_MODE(0),
					   CF_INST(SQ_CF_INST_EXPORT_DONE),
					   WHOLE_QUAD_MODE(0),
					   BARRIER(1));
    // CF INST 3
    vs[i++] = 0;
    vs[i++] = 0;
    // VTX clause INST 0
    vs[i++] = VTX_DWORD0(VTX_INST(SQ_VTX_INST_FETCH),
			 FETCH_TYPE(SQ_VTX_FETCH_VERTEX_DATA),
			 FETCH_WHOLE_QUAD(0),
			 BUFFER_ID(0),
			 SRC_GPR(0),
			 SRC_REL(ABSOLUTE),
			 SRC_SEL_X(SQ_SEL_X),
			 MEGA_FETCH_COUNT(12));
    vs[i++] = VTX_DWORD1_GPR(DST_GPR(1), DST_REL(0),
			     DST_SEL_X(SQ_SEL_X),
			     DST_SEL_Y(SQ_SEL_Y),
			     DST_SEL_Z(SQ_SEL_0),
			     DST_SEL_W(SQ_SEL_1),
			     USE_CONST_FIELDS(0),
			     DATA_FORMAT(FMT_32_32_FLOAT),
			     NUM_FORMAT_ALL(SQ_NUM_FORMAT_NORM),
			     FORMAT_COMP_ALL(SQ_FORMAT_COMP_SIGNED),
			     SRF_MODE_ALL(SRF_MODE_ZERO_CLAMP_MINUS_ONE));
    vs[i++] = VTX_DWORD2(OFFSET(0),
			 ENDIAN_SWAP(ENDIAN_NONE),
			 CONST_BUF_NO_STRIDE(0),
			 MEGA_FETCH(1));
    vs[i++] = VTX_DWORD_PAD,
	// VTX clause INST 1
	vs[i++] = VTX_DWORD0(VTX_INST(SQ_VTX_INST_FETCH),
			     FETCH_TYPE(SQ_VTX_FETCH_VERTEX_DATA),
			     FETCH_WHOLE_QUAD(0),
			     BUFFER_ID(0),
			     SRC_GPR(0),
			     SRC_REL(ABSOLUTE),
			     SRC_SEL_X(SQ_SEL_X),
			     MEGA_FETCH_COUNT(4));
    vs[i++] = VTX_DWORD1_GPR(DST_GPR(2), DST_REL(0),
			     DST_SEL_X(SQ_SEL_X),
			     DST_SEL_Y(SQ_SEL_Y),
			     DST_SEL_Z(SQ_SEL_Z),
			     DST_SEL_W(SQ_SEL_W),
			     USE_CONST_FIELDS(0),
			     DATA_FORMAT(FMT_8_8_8_8),
			     NUM_FORMAT_ALL(SQ_NUM_FORMAT_NORM),
			     FORMAT_COMP_ALL(SQ_FORMAT_COMP_UNSIGNED),
			     SRF_MODE_ALL(SRF_MODE_ZERO_CLAMP_MINUS_ONE));
    vs[i++] = VTX_DWORD2(OFFSET(8),
			 ENDIAN_SWAP(ENDIAN_NONE),
			 CONST_BUF_NO_STRIDE(0),
			 MEGA_FETCH(0));
    vs[i++] = VTX_DWORD_PAD;
    vs_len = i;
    
    draw_config_t   draw_conf;
    cb_config_t     cb_conf;
    vtx_resource_t  vtx_res;
    tex_resource_t  tex_res;
    tex_sampler_t   tex_samp;
    shader_config_t vs_conf, ps_conf;
    
    uint64_t vb_addr, vs_addr, ps_addr;

    CLEAR (draw_conf);
    CLEAR (cb_conf);
    CLEAR (vtx_res);
    CLEAR (tex_res);
    CLEAR (tex_samp);
    CLEAR (vs_conf);
    CLEAR (ps_conf);


    printf ("\n* Tri Test 3d\n\n");

    if (verbose) {
	dump_shader (adapt, vs, vs_len * 4, "vertex");
	dump_shader (adapt, ps, sizeof(ps), "pixel");
        printf ("\n");
    }


    /* Init */
    start_3d(adapt);
    set_default_state(adapt);


    /* Scissor / viewport */
    ereg  (PA_CL_VTE_CNTL,			VTX_XY_FMT_bit);
    ereg  (PA_CL_CLIP_CNTL,			CLIP_DISABLE_bit);

    
    /* Upload */
    vs_addr = upload (adapt, vs, vs_len * 4, 0);
    ps_addr = upload (adapt, ps, sizeof(ps), 4096);
    vb_addr = upload (adapt, vb, sizeof(vb), 8192);


    /* Shader */
    vs_conf.shader_addr         = vs_addr;
    vs_conf.num_gprs            = 4;
    vs_conf.stack_size          = 1;
    vs_setup                    (adapt, &vs_conf);

    ps_conf.shader_addr         = ps_addr;
    ps_conf.num_gprs            = 2;
    ps_conf.stack_size          = 0;
    ps_conf.uncached_first_inst = 1;
    ps_conf.clamp_consts        = 1;
    ps_conf.export_mode         = 2;
    ps_setup                    (adapt, &ps_conf);


    /* Render setup */
    ereg  (CB_SHADER_MASK,                      (0x0f << OUTPUT0_ENABLE_shift));
    ereg  (R7xx_CB_SHADER_CONTROL,              (RT0_ENABLE_bit));
    ereg  (CB_COLOR_CONTROL,                    (0xcc << ROP3_shift)); /* copy */

    cb_conf.id = 0;
    cb_conf.w = adapt->color_pitch;
    cb_conf.h = adapt->color_height;
    cb_conf.base = adapt->color_gpu;
    cb_conf.format = FMT_8_8_8_8;
    cb_conf.comp_swap = 1;
    cb_conf.source_format = 1;
    cb_conf.blend_clamp = 1;
    set_render_target(adapt, &cb_conf);

    ereg  (PA_SU_SC_MODE_CNTL,                  (FACE_bit			|
						 (POLYMODE_PTYPE__TRIANGLES << POLYMODE_FRONT_PTYPE_shift)	|
						 (POLYMODE_PTYPE__TRIANGLES << POLYMODE_BACK_PTYPE_shift)));
    ereg  (DB_SHADER_CONTROL,                   ((1 << Z_ORDER_shift)		| /* EARLY_Z_THEN_LATE_Z */
						 DUAL_EXPORT_ENABLE_bit)); /* Only useful if no depth export */


    /* Interpolator setup */
    ereg  (SPI_PS_IN_CONTROL_0,                 ((1 << NUM_INTERP_shift)));
//						 (1 << BARYC_SAMPLE_CNTL_shift)	|
//						 PERSP_GRADIENT_ENA_bit));
    ereg  (SPI_PS_IN_CONTROL_1,                 0);
    ereg  (SPI_PS_INPUT_CNTL_0 + (0 <<2),       ((0    << SEMANTIC_shift)	|
						 (0x03 << DEFAULT_VAL_shift)	|
						 SEL_CENTROID_bit));
/*    ereg  (SPI_INTERP_CONTROL_0,                ((2 << PNT_SPRITE_OVRD_X_shift)	|
						 (3 << PNT_SPRITE_OVRD_Y_shift)	|
						 (0 << PNT_SPRITE_OVRD_Z_shift)	|
						 (1 << PNT_SPRITE_OVRD_W_shift)));*/
    ereg (SPI_INTERP_CONTROL_0,			0);


    /* Vertex buffer setup */
    vtx_res.id              = SQ_VTX_RESOURCE_vs;
    vtx_res.vtx_size_dw     = sizeof(vertex_t) / 4;
    vtx_res.vtx_num_entries = sizeof(vb) / 4;		/* Can overcommit if necessary */
    vtx_res.mem_req_size    = 1;
    vtx_res.vb_addr         = vb_addr;
    set_vtx_resource        (adapt, &vtx_res);

    ereg  (VGT_INSTANCE_STEP_RATE_0,            0);	/* ? */
    ereg  (VGT_INSTANCE_STEP_RATE_1,            0);

    ereg  (VGT_MAX_VTX_INDX,                    vtx_res.vtx_num_entries / vtx_res.vtx_size_dw);
    ereg  (VGT_MIN_VTX_INDX,                    0);
    ereg  (VGT_INDX_OFFSET,                     0);


    /* Draw */
    draw_conf.prim_type          = DI_PT_TRILIST;
    draw_conf.vgt_draw_initiator = DI_SRC_SEL_AUTO_INDEX;
    draw_conf.num_instances      = 1;
    draw_conf.num_indices        = 6;
    draw_conf.index_type         = DI_INDEX_SIZE_16_BIT;
    draw_auto (adapt, &draw_conf);

    /* Draw 2 */
    draw_conf.prim_type          = DI_PT_TRILIST;
    draw_conf.vgt_draw_initiator = DI_SRC_SEL_AUTO_INDEX;
    draw_conf.num_instances      = 1;
    draw_conf.num_indices        = 9;
    draw_conf.index_type         = DI_INDEX_SIZE_16_BIT;
    draw_auto (adapt, &draw_conf);
    
    wait_3d_idle_clean();

}
