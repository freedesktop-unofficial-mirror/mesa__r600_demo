#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include "r600_reg.h"
#include "r600_emit.h"
#include "r600_lib.h"
#include "r600_shader.h"


/*
 * Old tests - disfunctional
 * to be removed after analysis
 */


#if 0

/* Trivial vertex shader: fetch indexed vertex, put it straight to SX (position buffer)
 * Data layout float x,y,z,w
 */
static uint64_t trivial_vs[] = {
    CF_WORD         (CF_INST_VTX, 2 /* vtx */, 1, 0),
    CF_EXPORT_WORD  (CF_INST_EXPORT_DONE, CF_EXPORT_POS, 1, 1, 60, CF_FLAG_BARRIER | CF_FLAG_END_OF_PROGRAM),
    /* vtx: */
    VTX_2WORDS_MEGA (0, 0, 0, 1, FORMAT_32_32_32_32_FLOAT, NUM_FORMAT_NORM, 16, 0)
} ;
/* Trivial pixel shader: load constant color from cfile(0), put straight to FB */
static uint64_t trivial_ps[] = {
    CF_ALU_WORD    (CF_INST_ALU, 2 /* alu */, 4, 0),
    CF_EXPORT_WORD (CF_INST_EXPORT_DONE, CF_EXPORT_PIXEL, 0, 1, 0, CF_FLAG_BARRIER | CF_FLAG_END_OF_PROGRAM),
    /* alu: */ 
    ALU_OP2_WORD   (ALU_OP2_MOV, ALU_GPR(0)|ALU_DEST_X, ALU_CFILE(0)|ALU_SRC_X, 0, ALU_VEC_012, 0),
    ALU_OP2_WORD   (ALU_OP2_MOV, ALU_GPR(0)|ALU_DEST_Y, ALU_CFILE(0)|ALU_SRC_Y, 0, ALU_VEC_012, 0),
    ALU_OP2_WORD   (ALU_OP2_MOV, ALU_GPR(0)|ALU_DEST_Z, ALU_CFILE(0)|ALU_SRC_Z, 0, ALU_VEC_012, 0),
    ALU_OP2_WORD   (ALU_OP2_MOV, ALU_GPR(0)|ALU_DEST_W, ALU_CFILE(0)|ALU_SRC_W, 0, ALU_VEC_012, ALU_FLAG_LAST)
} ;

#else

/* Trivial vertex shader: fetch indexed vertex + color, put it straight to SX (position buffer)
 * Data layout float x,y,z,w,r,g,b,a
 */
static uint64_t trivial_vs[] = {
    CF_WORD         (CF_INST_VTX, 2 /* vtx */, 2, 0),
    CF_EXPORT_WORD  (CF_INST_EXPORT_DONE, CF_EXPORT_POS,   1, 1, 60, CF_FLAG_BARRIER),
    CF_EXPORT_WORD  (CF_INST_EXPORT_DONE, CF_EXPORT_PARAM, 2, 1,  0, CF_FLAG_BARRIER | CF_FLAG_END_OF_PROGRAM),
    /* vtx: */
    VTX_2WORDS_MEGA (0,  0, 0, 1, FORMAT_32_32_32_32_FLOAT, NUM_FORMAT_NORM_SIGNED, 32, 0),
    VTX_2WORDS      (0, 16, 0, 2, FORMAT_32_32_32_32_FLOAT, NUM_FORMAT_NORM_SIGNED, 16, 0),
} ;
/* Trivial pixel shader: put param(0) straight to FB */
static uint64_t trivial_ps[] = {
    CF_EXPORT_WORD (CF_INST_EXPORT_DONE, CF_EXPORT_PIXEL, 0, 1, 0, CF_FLAG_END_OF_PROGRAM),
} ;

#endif


void init_3d(adapter_t *adapt)
{
    int i, num_qds, num_simds;
    uint32_t reg;
    printf      ("\n* 3D Init\n\n");

    /*
     * Mostly according to 15.1.4 Vertex Input Control, some regs according to TCore
     */

    if (adapt->chipset <= CHIPSET_RV670) {
	/* R6xx only (if at all - verify) */
	/* Get into 3D mode, so that 2D packets trigger reload of buffers */
	pack3 (IT_START_3D_CMDBUF, 1);
	e32   (0);
    }
#if 0
    pack3 (IT_ME_INITIALIZE, 6);
    e32   (0x01);
    e32   (0x00);					/* as in TCore; consider 0x03 instead */
    e32   (7);						/* consider 1 (0 currently in DRM) */
    e32   (0x00010000);					/* 0 currently in DRM */
    e32   (0);
    e32   (0);
    e32   (0x80000000);
    e32   (0x80000000);
    e32   (0x80000000);
    e32   (0x80000000);
    e32   (0x80000000);
    e32   (0x80000000);
#endif
    
    /* get number of available pipes, needed for a few register inits */
    reg       = reg_read32 (CC_GC_SHADER_PIPE_CONFIG);
    num_qds   = R6XX_MAX_QD_PIPES - count_bits (reg & INACTIVE_QD_PIPES_mask);
    num_simds = R6XX_MAX_SIMDS    - count_bits (reg & INACTIVE_SIMDS_mask);
    printf ("Number of QD pipes: %d  Number of SIMDs: %d\n", num_qds, num_simds);
    
    /*
     * VGT (Vertex Grouper Tessellator)
     */
    ereg  (VGT_VTX_VECT_EJECT_REG, PRIM_COUNT_mask);	/* Max number of prims allowed per vertex vector */
    ereg  (VGT_DMA_NUM_INSTANCES, 1);
    ereg  (VGT_PRIMITIVEID_EN, 0);			/* Primitive ID generation off */
    ereg  (VGT_MULTI_PRIM_IB_RESET_EN, 0);		/* Prim Reset Index off */
    ereg  (VGT_VTX_CNT_EN, 0);				/* Auto Index Generation off*/
    ereg  (VGT_GS_MODE, 0);				/* Geometry Shader off */
    ereg  (VGT_REUSE_OFF, REUSE_OFF_bit);		/* ? */
    ereg  (VGT_MAX_VTX_INDX, 0xffffffff);		/* vertex index max clamp - interesting for security */
    ereg  (VGT_MIN_VTX_INDX, 0);
    ereg  (VGT_INDX_OFFSET, 0);
    if (adapt->chipset <= CHIPSET_RV670) {
	ereg  (VGT_OUT_DEALLOC_CNTL, num_qds*4);
	ereg  (VGT_VERTEX_REUSE_BLOCK_CNTL, num_qds*4 - 2);
    } else {
	ereg  (VGT_OUT_DEALLOC_CNTL, 16);
	ereg  (VGT_VERTEX_REUSE_BLOCK_CNTL, 14);
    }
//    ereg  (VGT_GROUP_VECT_0_FMT_CNTL, 7);		/* Not needed for MODE 0 primitives */
    ereg  (VGT_INDEX_TYPE, 1);				/* 32 bit per index if not autoinc */
    

    /*
     * PA (Primitive Assembler)
     */
    pack0 (PA_SC_SCREEN_SCISSOR_TL, 2);			/* Screen scissor */
    e32   (0    | (0    << PA_SC_SCREEN_SCISSOR_TL__TL_Y_shift));
    e32   (1600 | (1200 << PA_SC_SCREEN_SCISSOR_BR__BR_Y_shift));
    pack0 (PA_SC_WINDOW_OFFSET, 3);			/* Window scissor */
    e32   (0    | (0    << WINDOW_Y_OFFSET_shift));	/* Window offset */
    e32   (0    | (0    << PA_SC_WINDOW_SCISSOR_TL__TL_Y_shift));
    e32   (640  | (480  << PA_SC_WINDOW_SCISSOR_BR__BR_Y_shift));
    ereg  (PA_CL_CLIP_CNTL, 0);				/* ZCLIP_NEAR_DISABLE_bit | ZCLIP_FAR_DISABLE_bit; DX_LINEAR_ATTR_CLIP_ENA? */
    ereg  (PA_SU_SC_MODE_CNTL,				/* Culling + Poly mode (0:tri 1:dual?) + etc */
	   (0 << POLY_MODE_shift) |
	   (2 << POLYMODE_FRONT_PTYPE_shift) | (2 << POLYMODE_BACK_PTYPE_shift) |
	   VTX_WINDOW_OFFSET_ENABLE_bit | PROVOKING_VTX_LAST_bit); /* TCore: | PERSP_CORR_DIS_bit */
    ereg  (PA_SC_MODE_CNTL, 0);
    ereg  (PA_SC_CLIPRECT_RULE, CLIP_RULE_mask);	/* Aux Cliprects (always pass, so ignore setup) */
// PA_SC_CLIPRECT_[0-3]_TL/BR
    pack0 (PA_SC_GENERIC_SCISSOR_TL, 2);
    e32   (0);
    e32   (0xffffffff);
    pack0 (PA_SC_VPORT_SCISSOR_0_TL, 2);
    e32   (0);
    e32   (0xffffffff);
    pack0 (PA_SC_VPORT_ZMIN_0, 2);
    efloat (0.0);
    efloat (1.0);
    pack0 (PA_SU_POINT_SIZE, 2);
    e32   (0x0008 | (0x0008 << PA_SU_POINT_SIZE__WIDTH_shift));	/* Default: half pixel radius */
    e32   (0      | (0xffff << MAX_SIZE_shift));	/* Min 0, max 4k radius */
    ereg  (PA_SC_AA_CONFIG, 0);
    ereg  (PA_SC_MPASS_PS_CNTL, 0);
    ereg  (PA_SC_AA_MASK, 0xffffffff);
    pack0 (PA_CL_GB_VERT_CLIP_ADJ, 4);
    efloat (1.0);					/* "should be set to 1.0 for no guard band"? */
    efloat (1.0);
    efloat (1.0);
    efloat (1.0);
    ereg  (PA_SU_VTX_CNTL, 1 |				/* Pixel Center 0: @0.0  1: @0.5 */
	   (0 << PA_SU_VTX_CNTL__ROUND_MODE_shift) |	/* Float->Fixed 0: trunc 1: Round 2: Round to even */
	   (0 << QUANT_MODE_shift));			/* 0: 1/16  1: 1/8  2: 1/4  3: 1/2  4: 1  5: 1/256 */
    pack0 (PA_CL_POINT_X_RAD, 4);
    e32   (0);
    e32   (0);
    e32   (0);						/* PA_CL_POINT_SIZE */
    e32   (0);						/* PA_CL_POINT_CULL_RAD */
    ereg  (PA_CL_VTE_CNTL,
	   VPORT_X_SCALE_ENA_bit | VPORT_X_OFFSET_ENA_bit |
	   VPORT_Y_SCALE_ENA_bit | VPORT_Y_OFFSET_ENA_bit |
	   VPORT_Z_SCALE_ENA_bit | VPORT_Z_OFFSET_ENA_bit |
	   VTX_XY_FMT_bit | VTX_Z_FMT_bit);		/* perfcounter */
    ereg  (PA_SC_LINE_CNTL, LAST_PIXEL_bit);		/* verify, also EXPAND_LINE_WIDTH_bit (AA lines) */
    pack0 (PA_CL_VPORT_XSCALE_0, 6);			/* Viewport trafo */
    efloat (1.0);					/* XScale */
    efloat (0.0);					/* XOffset */
    efloat (1.0);					/* YScale */
    efloat (0.0);					/* YOffset */
    efloat (1.0);					/* ZScale */
    efloat (0.0);					/* ZOffset */
    ereg  (PA_CL_VS_OUT_CNTL, 0);			/* vtx_point_size, user clip distance, vtx_kill_flag */
    pack0 (PA_SU_LINE_CNTL, 2);
    e32   (0x0008);					/* half pixel width */
    e32   (LINE_PATTERN_mask);				/* no stipple */


    /*
     * CB (Color Block)
     */
    ereg  (CB_COLOR0_BASE, adapt->color_gpu >> 8);
    ereg  (CB_COLOR0_SIZE, ((adapt->color_pitch >> 3) - 1) |
	   ((adapt->color_pitch*adapt->color_height/64-1) << SLICE_TILE_MAX_shift));
    ereg  (CB_COLOR0_VIEW, 0 | SLICE_MAX_mask);
    ereg  (CB_COLOR0_INFO,				/* clear_color, blend_clamp, blend_bypass */
	   (COLOR_8_8_8_8 << CB_COLOR0_INFO__FORMAT_shift) |
	   (ARRAY_LINEAR_ALIGNED << CB_COLOR0_INFO__ARRAY_MODE_shift) |
	   (NUMBER_SRGB /* ? */ << NUMBER_TYPE_shift) |
	   CB_COLOR0_INFO__READ_SIZE_bit | SIMPLE_FLOAT_bit);
    ereg  (CB_COLOR_CONTROL, DITHER_ENABLE_bit |
	   (0x01 << TARGET_BLEND_ENABLE_shift) |
	   (0xcc << ROP3_shift));			/* special modes, per-mrt-blend */
    ereg  (CB_CLRCMP_CONTROL, 0);
    ereg  (CB_SHADER_MASK, 0x0f);
    ereg  (CB_TARGET_MASK, 0x0f);
    ereg  (CB_BLEND_CONTROL, BLEND_ONE);		/* dest := src */
    pack0 (CB_CLEAR_RED, 4);				/* ? */
    efloat (1.0);
    efloat (0.0);
    efloat (0.0);
    efloat (1.0);
    
    
    /*
     * DB (Depth Block)
     */
    ereg  (DB_DEPTH_BASE, adapt->depth_gpu >> 8);
    ereg  (DB_DEPTH_SIZE, ((adapt->depth_pitch >> 3) - 1) |
	   ((adapt->depth_pitch*adapt->depth_height/64-1) << SLICE_TILE_MAX_shift));
    ereg  (DB_DEPTH_VIEW, 0 | SLICE_MAX_mask);		/* ??? "Maximum allowed Z slice index" */
    ereg  (DB_DEPTH_INFO, DEPTH_8_24 | (ARRAY_1D_TILED_THIN1 << DB_DEPTH_INFO__ARRAY_MODE_shift)); /* hiz */
//    ereg  (DB_HTILE_DATA_BASE, htilebase >> 8);
    ereg  (DB_PREFETCH_LIMIT, (adapt->depth_height >> 3)-1);
//DB_PRELOAD_CONTROL
    ereg  (DB_RENDER_CONTROL, 0);			/* hiz */
    ereg  (DB_RENDER_OVERRIDE, 0);			/* hiz, his */
//DB_HTILE_SURFACE
    ereg  (DB_SHADER_CONTROL,
	   (Z_ORDER__EARLY_Z_THEN_LATE_Z << Z_ORDER_shift));
//	   | KILL_ENABLE_bit | DUAL_EXPORT_ENABLE_bit);
    ereg  (DB_STENCILREFMASK,    0xff << STENCILWRITEMASK_shift);
    ereg  (DB_STENCILREFMASK_BF, 0xff << STENCILWRITEMASK_shift);
    pack0 (DB_DEPTH_CLEAR, 1);
    efloat (1.0);
    ereg  (DB_DEPTH_CONTROL, Z_WRITE_ENABLE_bit);	/* stencil, ztest */
    ereg  (DB_STENCIL_CLEAR, 0);
//    ereg  (DB_ALPHA_TO_MASK, 0);			/* ? */


    /*
     * SQ (Sequencer / Shader)
     */
    ereg  (SQ_CONFIG, VC_ENABLE_bit | DX9_CONSTS_bit |	/* DX9 for constant file, =0 for kcache */
//	   ALU_INST_PREFER_VECTOR_bit |
	   /* Priority of VS vs. PS has to be tested - see also SPI_CONFIG_CNTL */
//	   (3 << PS_PRIO_shift) | (2 << VS_PRIO_shift) |
	   (2 << PS_PRIO_shift) | (3 << VS_PRIO_shift) |
	   (1 << GS_PRIO_shift) | (0 << ES_PRIO_shift));
    ereg  (SQ_GPR_RESOURCE_MGMT_1, 191 | (63 << NUM_VS_GPRS_shift) |
	   (4 << NUM_CLAUSE_TEMP_GPRS_shift) |
	   (0 << CLAUSE_SEQ_PRIO_shift));		/* fixed number of total regs? */
    ereg  (SQ_THREAD_RESOURCE_MGMT, 95 | (31 << NUM_VS_THREADS_shift));
    ereg  (SQ_STACK_RESOURCE_MGMT_1, 2047 | (2047 << NUM_VS_STACK_ENTRIES_shift));
    ereg  (SQ_VTX_BASE_VTX_LOC, 0);			/* ? */
    ereg  (SQ_VTX_START_INST_LOC, 0);			/* ? */

    ereg  (SQ_PGM_EXPORTS_PS, 3);			/* Export 1 color + z */ /* ? */
    ereg  (SQ_VTX_SEMANTIC_CLEAR, 0xffffffff);
    ereg  (SQ_PGM_RESOURCES_FS, 0);
//    ereg  (SQ_ESGS_RING_ITEMSIZE, 0);
//    ereg  (SQ_GSVS_RING_ITEMSIZE, 0);
//    ereg  (SQ_ESTMP_RING_ITEMSIZE, 0);
//    ereg  (SQ_GSTMP_RING_ITEMSIZE, 0);
    ereg  (SQ_VSTMP_RING_ITEMSIZE, 0);			/* ? */
    ereg  (SQ_PSTMP_RING_ITEMSIZE, 0);
    ereg  (SQ_FBUF_RING_ITEMSIZE, 0);
    ereg  (SQ_REDUC_RING_ITEMSIZE, 0);
//    ereg  (SQ_GS_VERT_ITEMSIZE, 0);

    
    /*
     * SPI (Shader Processor Interpolator)
     */
    ereg  (SPI_CONFIG_CNTL, 0);				/* Prio VS, GS, ES, PS */
    ereg  (SPI_CONFIG_CNTL_1, 0);
    ereg  (SPI_INTERP_CONTROL_0, FLAT_SHADE_ENA_bit);	/* doc: turn off? point sprite params? */
    pack0 (SPI_FOG_CNTL, 3);
    e32   (0);
    e32   (0);
    e32   (0);
    ereg  (SPI_INPUT_Z, 0);				/* ? */
    pack0 (SPI_VS_OUT_ID_0, SPI_VS_OUT_ID_0_num);
    for (i = 0; i < SPI_VS_OUT_ID_0_num; i++)		/* VS output mapping */
	e32 (0x03020100 + i*0x04040404);
    pack0 (SPI_PS_INPUT_CNTL_0, SPI_PS_INPUT_CNTL_0_num);
    for (i = 0; i < SPI_PS_INPUT_CNTL_0_num; i++)	/* interpolation settings */
	e32 ((i /*<< SEMANTIC_shift*/) | (3 << DEFAULT_VAL_shift) | SEL_CENTROID_bit); /* SEL_CENTROID unknown? */
    ereg  (SPI_VS_OUT_CONFIG, 0 << VS_EXPORT_COUNT_shift); /* number of exported vectors -1 */
    ereg  (SPI_PS_IN_CONTROL_0, 1);			/* number of parameters to interpolate, generate */
    ereg  (SPI_PS_IN_CONTROL_1, 0);

    /*
     * SX (Shader Export)
     */
    ereg  (SX_EXPORT_BUFFER_SIZES, 0x1f |		/* ? */ /* err in doc: default 0x3 ?!? */
	   (0x1f << POSITION_BUFFER_SIZE_shift) | (0x1f << SMX_BUFFER_SIZE_shift));
//    ereg  (SX_MEMORY_EXPORT_BASE, (vtx_gpu + 512) >> 8);  /* ? */
    ereg  (SX_MEMORY_EXPORT_BASE, (adapt->display_gpu) >> 8);  /* ? */
    ereg  (SX_MEMORY_EXPORT_SIZE, 1024);
    ereg  (SX_ALPHA_TEST_CONTROL, ALPHA_TEST_BYPASS_bit); /* diff bypass vs. !enable ? */
    ereg  (SX_MISC, 0);					/* ? */

    /*
     * Misc
     */
    ereg  (GC_USER_SHADER_PIPE_CONFIG, 0);		/* ? */
}


static void analyze_program (char *what, uint64_t *p, int plen, int *cf, int *alu, int *fetch)
{
    int i;
    uint32_t *p32 = (uint32_t *) p;
    
    *alu = *fetch = plen;

    /* searches for end of cf by first CF_FLAG_END_OF_PROGRAM - jumps are evil (read: won't work) */
    for (i = 0; i < plen; i++) {
	uint32_t inst = (p[i] & 0x3f80000000000000ULL) >> (23+32), t;
	if (inst & 0x40) {
	    t = p[i] & 0xffffffff;
	    if (t < *alu)
		*alu = t;
	}
	if (inst == CF_INST_TEX || inst == CF_INST_VTX || inst == CF_INST_VTX_TC) {
	    t = p[i] & 0x003fffff;
	    if (t < *fetch)
		*fetch = t;
	    if (t < *alu)
		*alu = t;
	}
	if (p[i] & CF_FLAG_END_OF_PROGRAM)
	    break;
    }
    assert (p[i] & CF_FLAG_END_OF_PROGRAM);
	
    *cf = i+1;
    printf ("  %s shader (len main cf %d, alu at %d, fetches at %d, len %d:\n",
	    what, *cf, *alu, *fetch, plen);
    for (i = 0; i < plen; i++, p32 += 2)
	printf ("    %08x  %08x%s", p32[0], p32[1], (i & 3) == 3 ? "\n":"");
    if ((i & 3) != 0)
	printf ("\n");
}


void init_programs (adapter_t *adapt, uint64_t *vs, int vslen, int vsgpr, uint64_t *ps, int pslen, int psgpr)
{
    int cf, alu, fetch;

    memcpy ((char *)vtx, vs, vslen);
    flush_cache ((char *)vtx, (char *)vtx + vslen);
    analyze_program ("Vertex", vs, vslen/8, &cf, &alu, &fetch);

    ereg  (SQ_PGM_START_VS, vtx_gpu >> 8);		/* Base address vertex shader */
    ereg  (SQ_PGM_CF_OFFSET_VS, 0);			/* Base offset CF (control flow) */
    ereg  (SQ_PGM_END_CF_VS, alu);			/* End offset CF (ignored by HW?) */
    ereg  (SQ_PGM_END_ALU_VS, fetch);			/* End offset ALU (ignored by HW?) */
    ereg  (SQ_PGM_END_FETCH_VS, vslen);			/* End offset Fetches (ignored by HW?) */
    ereg  (SQ_PGM_RESOURCES_VS, (vsgpr /* << NUM_GS_GPRS_shift */) | (0 << STACK_SIZE_shift) |
	   PRIME_CACHE_ON_DRAW_bit | FETCH_CACHE_LINES_mask |
	   PRIME_CACHE_ENABLE_bit | PRIME_CACHE_ON_CONST_bit);

    memcpy ((char *)vtx + 4096, ps, pslen);
    flush_cache ((char *)vtx + 4096, (char *)vtx + 4096 + pslen);
    analyze_program ("Pixel", ps, pslen/8, &cf, &alu, &fetch);
    
    ereg  (SQ_PGM_START_PS, (vtx_gpu + 4096) >> 8);	/* All the same for pixel shader */
    ereg  (SQ_PGM_CF_OFFSET_PS, 0);
    ereg  (SQ_PGM_END_CF_PS, alu);
    ereg  (SQ_PGM_END_ALU_PS, fetch);
    ereg  (SQ_PGM_END_FETCH_PS, pslen);
    ereg  (SQ_PGM_RESOURCES_PS, (psgpr /* << NUM_GS_GPRS_shift */) | (0 << STACK_SIZE_shift) |
	   PRIME_CACHE_ON_DRAW_bit | FETCH_CACHE_LINES_mask |
	   PRIME_CACHE_ENABLE_bit | PRIME_CACHE_ON_CONST_bit |
	   (adapt->chipset == CHIPSET_R600 ? UNCACHED_FIRST_INST_bit : 0));
    
    printf ("\n");
}



void clear_buffers (adapter_t* adapt, int colbuf, int zbuf)
{
    /* Choose random of 8 half saturated colors (blue may be saturated) */
    uint32_t color = random ();
    color = ((color & 8) << (31-3)) | ((color & 4) << (23-2)) | ((color & 2) << (15-1)) | ((color & 1) << 7) | 0x0000003f;
    color = 0x400000ff;

    /* Enable clearing of buffers selected, disable writing to others */
    if (colbuf) {
	ereg  (CB_COLOR0_INFO,
	       (COLOR_8_8_8_8 << CB_COLOR0_INFO__FORMAT_shift) |
	       (ARRAY_LINEAR_ALIGNED << CB_COLOR0_INFO__ARRAY_MODE_shift) |
	       (NUMBER_SRGB << NUMBER_TYPE_shift) |
	       CB_COLOR0_INFO__READ_SIZE_bit | CLEAR_COLOR_bit | SIMPLE_FLOAT_bit); /* clear_color: 0x3f800000 ?!? */
	ereg  (CB_COLOR_CONTROL, DITHER_ENABLE_bit |
	       (0x03 << SPECIAL_OP_shift) |
	       (0x01 << TARGET_BLEND_ENABLE_shift) |
	       (0xcc << ROP3_shift));
	/* State, shouldn't be changed after clear */
	pack0 (CB_CLEAR_RED, 4);
	efloat (1.0);					/* ? No freaking clue, set to red to see it happen */
	efloat (0.0);
	efloat (0.0);
	efloat (1.0);
    } else {
	ereg  (CB_TARGET_MASK, 0);
    }
    if (zbuf) {
	ereg  (DB_RENDER_CONTROL, DEPTH_CLEAR_ENABLE_bit | STENCIL_CLEAR_ENABLE_bit);
//	ereg  (DB_DEPTH_CONTROL, STENCIL_ENABLE_bit | Z_ENABLE_bit | Z_WRITE_ENABLE_bit | (1 << STENCILFAIL_shift));
	ereg  (DB_DEPTH_CONTROL, Z_WRITE_ENABLE_bit | (1 << STENCILFAIL_shift));
	/* State, shouldn't be changed after clear */
	pack0 (DB_DEPTH_CLEAR, 1);
	ereg  (DB_STENCIL_CLEAR, 0 | (((0-7) & 0xff) << MIN_shift));
	efloat (1.0);
    } else {
	ereg  (DB_DEPTH_CONTROL, 0);
    }
    
    /* TODO: draw quad for clearing */

    /* Cleanup: reset to sane values */
    ereg  (CB_COLOR0_INFO,				/* clear_color, blend_clamp, blend_bypass */
	   (COLOR_8_8_8_8 << CB_COLOR0_INFO__FORMAT_shift) |
	   (ARRAY_LINEAR_ALIGNED << CB_COLOR0_INFO__ARRAY_MODE_shift) |
	   (NUMBER_SRGB /* ? */ << NUMBER_TYPE_shift) |
	   CB_COLOR0_INFO__READ_SIZE_bit | SIMPLE_FLOAT_bit); /* clear_color: 0x3f800000 ?!? */
    ereg  (CB_COLOR_CONTROL, DITHER_ENABLE_bit |
	   (0x01 << TARGET_BLEND_ENABLE_shift) |
	   (0xcc << ROP3_shift));			/* special modes, per-mrt-blend */
    ereg  (DB_RENDER_CONTROL, 0);
    ereg  (DB_DEPTH_CONTROL, Z_WRITE_ENABLE_bit);
    ereg  (CB_TARGET_MASK, 0x0f);
}


void test_triangles(adapter_t *adapt)
{
    int i;
    static float vertices[] = {				/* format x,y,z,w,r,g,b,a */
	0, 0, 0, 1,
	1.0, 1.0, 0.0, 1.0,
	200, 0, 0, 1,
	1.0, 0.0, 1.0, 1.0,
	0, 200, 0, 1,					/* eol */
	0.0, 1.0, 1.0, 1.0,
	100, 100, 0, 1,
	1.0, 0.0, 0.0, 1.0,
	110, 100, 0, 1,
	1.0, 0.0, 0.0, 1.0,
	100, 110, 0, 1,
	1.0, 0.0, 0.0, 1.0,
	200, 200, 0, 1,
	1.0, 0.0, 0.0, 1.0,
	210, 200, 0, 1,
	1.0, 0.0, 0.0, 1.0,
	200, 210, 0, 1,
	1.0, 0.0, 0.0, 1.0,
	100, 200, 0, 1,
	1.0, 0.0, 0.0, 1.0,
	110, 200, 0, 1,
	1.0, 0.0, 0.0, 1.0,
	100, 210, 0, 1,
	1.0, 0.0, 0.0, 1.0
    } ;

    printf("\nTriangles:\n\n");

    /* Shader upload */
    init_programs (adapt, trivial_vs, sizeof (trivial_vs), 2, trivial_ps, sizeof (trivial_ps), 1);

    /* Shader constants (cfile) */
    pack0 (SQ_ALU_CONSTANT0_0, 4);			/* consts 0-255.xyzw for PS, 256-511.xyzw for VS */
    efloat (1.0);					/* red ARGB 1/1/0/0 */
    efloat (1.0);
    efloat (0.0);
    efloat (0.0);

    /* Vertex buffer upload */
    memcpy (((char *)vtx) + 8192, vertices, 3*8*4 * 4);	/* overcomitting, in case too much is drawn */
    flush_cache ((char *)vtx + 8192, (char *)vtx + 8192 + 3*8*4 *4);
    flush_gpu_input_cache ();				/* needed? */

    /* Vertex buffer setup */
//    pack0 (SQ_VTX_CONSTANT_WORD0_0 + 160*7*4, 7);
    /* FIXME: set all resources until everything works */
    for (i = SQ_VTX_CONSTANT_WORD0_0; i < 0x3B640; i+=7*4) {
	pack0 (i, 7);
	e32   ((vtx_gpu + 8192) & 0xffffffff);			/* WORD0 */
	e32   (3 *8*4);						/* WORD1:  bytes ? */
	e32   ((((vtx_gpu + 8192) >> 32) & BASE_ADDRESS_HI_mask) |
	       ((8*4) << SQ_VTX_CONSTANT_WORD2_0__STRIDE_shift) |
	       (FORMAT_32_32_32_32_FLOAT << SQ_VTX_CONSTANT_WORD2_0__DATA_FORMAT_shift) |
	       (2 << SQ_VTX_CONSTANT_WORD2_0__NUM_FORMAT_ALL_shift) |
	       SQ_VTX_CONSTANT_WORD2_0__FORMAT_COMP_ALL_bit);	/* WORD2 */
	e32   (1 << MEM_REQUEST_SIZE_shift);			/* WORD3 */
	e32   (0);						/* WORD4 */
	e32   (0);						/* WORD5 */
	e32   (3 << SQ_VTX_CONSTANT_WORD6_0__TYPE_shift);	/* WORD6 */
    }

    ereg  (VGT_PRIMITIVE_TYPE, 4);			/* draw (unconnected) triangles */
    pack3 (IT_DRAW_INDEX_AUTO, 2);
    e32   (3);						/* see VGT_NUM_INDICES */
    e32   ((2 /*<< SOURCE_SELECT_shift*/));		/* see VGT_DRAW_INITIATOR */
}
