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

/*
 * Helper routines, to be split up
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <errno.h>
#include <sys/time.h>

#include "r600_reg.h"
#include "r600_emit.h"
#include "r600_lib.h"
#include "r600_shader.h"
#include "xf86drm.h"
#include "radeon_drm.h"

/* DRM / buffer related */
extern int       drmFD;
extern int       indirect_start, indirect_end, indirect_size, indirect_idx, ring_size;
extern drm_context_t context;
extern drmBufMapPtr BufMapPtr;
extern void     *ring;

extern int verbose;


void wait_reg (adapter_t *adapt, uint32_t reg, uint32_t v, const char *when)
{
    int i;
    for (i = 0; i < 1e6; i++)
	if (reg_read32 (reg) == v)
	    break;
    if (i == 1e6) {
	fprintf (stderr, "***** %s: still not set after %d loops: 0x%x, should be 0x%x\n",
		 when, i, reg_read32 (reg), v);
	show_state (adapt);
	fprintf (stderr, "***** FAILED\n\n");
	exit (1);
    } else if (verbose >= 2)
	fprintf (stderr, "%s: set correctly after %d loops: 0x%x\n", when, i, v);
}

float wait_reg_time (adapter_t *adapt, uint32_t reg, uint32_t v, const char *when, float maxtime)
{
    struct timeval start, end;
    int i;
    float diff;

    gettimeofday (&start, NULL);
    do {
	for (i = 0; i < 1e6; i++)
	    if (reg_read32 (reg) == v)
		break;
	gettimeofday (&end, NULL);
	diff = end.tv_sec - start.tv_sec + ((float)end.tv_usec - start.tv_usec) / 1e6;
    } while (i == 1e6 && diff < maxtime);

    if (i == 1e6) {
	fprintf (stderr, "***** %s: still not set after %f seconds: 0x%x, should be 0x%x\n",
		 when, diff, reg_read32 (reg), v);
	show_state (adapt);
	fprintf (stderr, "***** FAILED\n\n");
	exit (1);
    } else if (verbose >= 2)
	fprintf (stderr, "%s: set correctly after %f seconds: 0x%x\n", when, diff, v);

    return diff;
}

void wait_3d_idle_clean()
{
    //flush caches, don't generate timestamp
    pack3 (IT_EVENT_WRITE, 1);
    e32   (CACHE_FLUSH_AND_INV_EVENT);
    // wait for 3D idle clean
    ereg  (WAIT_UNTIL,                          WAIT_3D_IDLECLEAN_bit);
}


void wait_3d_idle()
{
    ereg  (WAIT_UNTIL,                          WAIT_3D_IDLE_bit);
}


void wait_3d_full_idle_clean ()
{
    CMD_BUFFER_PREAMBLE (2 + 7);
    
    //flush caches, don't generate timestamp
    PACK3 (IT_EVENT_WRITE, 1);
    E32   (CACHE_FLUSH_AND_INV_EVENT);

    // wait_3d_idle_clean doesn't actually wait for the engine to be idle
    // but stalls later memory writes by the CP
    PACK3 (IT_WAIT_REG_MEM, 6);
    E32   (0x00000003);						// ME, Register, EqualTo
    E32   (GRBM_STATUS >> 2);
    E32   (0);
    E32   (CB03_CLEAN_bit | DB03_CLEAN_bit);			// Ref value
    E32   (CB03_BUSY_bit | CR_BUSY_bit | DB03_BUSY_bit | PA_BUSY_bit |
	   SC_BUSY_bit | SMX_BUSY_bit | SPI03_BUSY_bit | SH_BUSY_bit |
	   SX_BUSY_bit | GRBM_STATUS__TC_BUSY_bit | TA03_BUSY_bit |
	   VGT_BUSY_NO_DMA_bit | GRBM_STATUS__VC_BUSY_bit |
	   CB03_CLEAN_bit | DB03_CLEAN_bit);			// Ref mask
    E32   (10);							// Wait interval
}


static void flush_indirect (void)
{
    drm_radeon_indirect_t  ind;
    int r, discard = 0;
    drmDMAReq  dma;

    // If buffer is more than half filled, fetch a new one
    if (indirect_end > indirect_size / 2) {
	fprintf (stderr, "  Buffer more than half filled, fetching new one after committing.\n");
	discard = 1;
    }

    ind.idx     = indirect_idx;
    ind.start   = indirect_start;
    ind.end     = indirect_end;
    ind.discard = discard;

    drmGetLock(drmFD,context,DRM_LOCK_READY);
    r=drmCommandWriteRead(drmFD, DRM_RADEON_INDIRECT,
			  &ind, sizeof(drm_radeon_indirect_t));
    drmUnlock(drmFD,context);

    if(r<0){
    	drmError(r, __func__);
    }

    if (discard) {
	// Fetch a new one. Yes, this should be a separate function.
	dma.context         = context;
	dma.send_count      = 0;
	dma.request_count   = 1;
	dma.request_size    = 64000;
	dma.request_list    = &indirect_idx;
	dma.request_sizes   = &indirect_size;
	dma.flags           = DRM_DMA_WAIT;
	do {
	    drmGetLock(drmFD, context, DRM_LOCK_READY);
	    r = drmDMA (drmFD, &dma);
	    drmUnlock(drmFD,context);
	} while (r == EBUSY);
	if (r && r != EBUSY) {
	    drmError(r, __func__);
	    exit(-1);
	}
	indirect = BufMapPtr->list[indirect_idx].address;
	indirect_start = indirect_end = BufMapPtr->list[indirect_idx].used;
	if (verbose >= 1)
	    fprintf (stderr, "  New indirect buffer: #%d, size %d, @0x%p\n",
		     indirect_idx, indirect_size, indirect);

	indirect_start=indirect_end = 0;

    } else
	indirect_start=indirect_end;
}


static void flush_ib_to_ring (void)
{
    int i;
    int rstart = reg_read32 (CP_RB_WPTR);
    int r = rstart;

    for (i = indirect_start>>2; i < indirect_end>>2; i++) {
	((uint32_t *) ring) [r] = indirect[i];
	if (++r >= ring_size>>2)
	    r = 0;
	while (r == reg_read32 (CP_RB_RPTR))		/* should never happen */
	    fprintf (stderr, "Waiting for ring buffer to become free...\n");
    }

    if (r > rstart)
	flush_cache ((uint32_t *) ring + rstart, (uint32_t *) ring + r);
    else {
	flush_cache ((uint32_t *) ring + rstart, (char *) ring + ring_size);
	flush_cache (ring, (uint32_t *) ring + r);
    }

    reg_write32 (CP_RB_WPTR, r);

    indirect_start=indirect_end;
}


void flush_cmds (void)
{
    int i;
    
    while(indirect_end & 0x3c){
	e32(RADEON_CP_PACKET2); /* fill up to multiple of 16 dwords */
    }

//    flush_cache_indirect ();				/* Apparently not necessary */
    
    if(indirect_end & 0x3) {
	fprintf(stderr, "*** indirect_end & 0x3\n");
	return; /* does not end on dword boundary */
    }
    if(indirect_start==indirect_end) {
	if (verbose >= 2)
	    fprintf(stderr, "indirect empty\n");
	return;
    }
    if(indirect_end>=indirect_size) {
	fprintf(stderr, "*** indirect out of bounds\n");
	return;
    }
    if (verbose) {
	printf ("flushing indirect buffer %d with %d dwords%s:\n",
		indirect_idx, (indirect_end-indirect_start)>>2,
		use_ring_directly ? " to ring":"");
	for (i = indirect_start>>2; i < indirect_end>>2; i += 8) {
	    printf ("  %08x %08x %08x %08x %08x %08x %08x %08x\n",
		    indirect[i],   indirect[i+1], indirect[i+2], indirect[i+3],
		    indirect[i+4], indirect[i+5], indirect[i+6], indirect[i+7]);
	}
    }

    if (do_not_flush)
	return;
    
    if (use_ring_directly)
	flush_ib_to_ring ();		/* Copy IB directly to ring w/o DRM intervention */
    else
	flush_indirect ();		/* Flush IB through DRM */
}


void flush_gpu_source_cache (adapter_t *adapt, uint32_t type, uint64_t lower, uint64_t upper)
{
    /* To be used after texture uploads etc. */
#if 0
    CMD_BUFFER_PREAMBLE (3*2 + 7);
    lower = lower & ~0xffULL;
    upper = (upper + 0xff) & ~0xffULL;

    EREG  (CP_COHER_CNTL,			type);
    EREG  (CP_COHER_SIZE,			(upper - lower) >> 8);
    EREG  (CP_COHER_BASE,			lower >> 8);
    PACK3 (IT_WAIT_REG_MEM, 6);
    E32   (0x00000003);						// ME, Register, EqualTo
    E32   (CP_COHER_STATUS >> 2);
    E32   (0);
    E32   (0);							// Ref value
    E32   (STATUS_bit);						// Ref mask
    E32   (10);							// Wait interval
#endif
    CMD_BUFFER_PREAMBLE (5);
    lower = lower & ~0xffULL;
    upper = (upper + 0xff) & ~0xffULL;
    if (verbose >= 1)
	printf ("  GPU source cache clear %08x00 - %08x00\n",
		(uint32_t) (lower >> 8), (uint32_t)(upper >> 8));

    PACK3 (IT_SURFACE_SYNC, 4);
    E32   (type);
    E32   ((upper-lower) >> 8);		/* SIZE */
    E32   (lower >> 8);			/* BASE */
    E32   (10);				/* POLL_INTERVAL */
}


void flush_gpu_dest_cache (adapter_t *adapt, uint32_t type, uint64_t lower, uint64_t upper)
{
    // TODO: not correct at all yet
    /* To be used before readpixels, copy-to-texture etc. */
    pack3 (IT_SURFACE_SYNC, 4);
    e32   (type);
    e32   (0xffffffff);			/* SIZE */
    e32   (0);				/* BASE */
    e32   (1);				/* POLL_INTERVAL useful value? */
/*    Submit a time stamp write back packet.
      Driver polls the write back scratch register. When write back value presents in the register, driver
      knows synchronization process has completed.
      To synchronize all surfaces (flush all caches) by enabling all corresponding bits in CP_COHER_CNTL and setting
      CP_COHER_SIZE = 0xFFFFFFFF, CP_COHER_BASE = 0x00000000.
*/
}

/* Write shader/buffer to agreed upon location (with offset) and return address */
static uint64_t do_upload (adapter_t *adapt, uint64_t addr_gpu, void *addr_cpu, void *input, int size, int offset)
{
    memcpy (addr_cpu, input, size);
    if (verbose >= 2) {
	int i;
	printf ("Upload %d dwords to offset 0x%x -> 0x"PRINTF_UINT64_HEX"\n", size/4, offset, addr_gpu);
	for (i = 0; i < size/4; i++)
	    printf ("  %08x%s", ((uint32_t *)input)[i], (i & 7) == 7 ? "\n":"");
	if ((i & 7) != 0)
	    printf ("\n");
    }
    flush_gpu_source_cache (adapt, FLUSH_GPU_INPUT_TYPE_ALL, addr_gpu, addr_gpu + size);
    return addr_gpu;
}

uint64_t upload_gart (adapter_t *adapt, void *input, int size, int offset)
{
    return do_upload (adapt, vtx_gpu + offset, (char *)vtx + offset,
		      input, size, offset);
}

uint64_t upload_gpu (adapter_t *adapt, void *input, int size, int offset)
{
    int off = (adapt->color_pitch * adapt->color_height * 4) + offset;
    return do_upload (adapt, adapt->framebuffer_gpu + off, (char *)adapt->framebuffer + off,
		      input, size, offset);
}

/* Dump shader to screen */
void dump_shader (adapter_t *adapt, uint32_t *shader, int size, char *what)
{
    uint32_t *p32 = shader;
    int plen = size/8, i;
    printf ("  %s shader (len %d):\n", what, plen);
    for (i = 0; i < plen; i++, p32 += 2)
	printf ("    %08x  %08x%s", p32[0], p32[1], (i & 3) == 3 ? "\n":"");
    if ((i & 3) != 0)
	printf ("\n");
}


void soft_reset (adapter_t *adapt)
{
    uint32_t cpptr, cpmecntl, cprbcntl;

    printf ("\n* Resetting\n");
    
    cpptr = reg_read32 (CP_RB_WPTR);			/* propagated by drm, so we mustn't mess it up */
    cpmecntl = reg_read32 (CP_ME_CNTL);
    reg_write32 (CP_ME_CNTL, 0x10000000);
    
    reg_write32 (GRBM_SOFT_RESET, 0x7fff);
    reg_read32  (GRBM_SOFT_RESET);
    usleep (50);
    reg_write32 (GRBM_SOFT_RESET, 0);
    reg_read32  (GRBM_SOFT_RESET);

    reg_write32 (CP_RB_WPTR_DELAY, 0);
    cprbcntl = reg_read32 (CP_RB_CNTL);
    reg_write32 (CP_RB_CNTL, 0x80000000);
    
    reg_write32 (CP_RB_RPTR_WR, cpptr);			/* Reset CP position to last known */
    reg_write32 (CP_RB_WPTR, cpptr);
    reg_write32 (CP_RB_CNTL, cprbcntl);
    reg_write32 (CP_ME_CNTL, cpmecntl);
    
    show_state (adapt);
}


void show_state (adapter_t *adapt)
{
    uint32_t reg;
    
#define STATE_TEST(n,x,t) do { if (n (reg & x ## _bit)) printf ("    %-20s: %s\n", #n #x, t); } while (0)
#define STATE_TESTi(n,x,t) do { if (n (reg & x ## _bit)) printf ("    %-20s: %s #%d\n", #n #x, t, i); } while (0)

    printf ("\n* Current GPU state\n\n");

    if (verbose >= 1)
	printf ("    CP      Read Ptr: 0x%04x   CP      Write Ptr: 0x%04x\n"
		"    MEQ     Read Ptr: 0x%04x   MEQ     Write Ptr: 0x%04x\n"
		"    ROQ     Read Ptr: 0x%04x   ROQ     Write Ptr: 0x%04x\n"
		"    ROQ IB1 Read Ptr: 0x%04x   ROQ IB1 Write Ptr: 0x%04x\n"
		"    CMDFIFO_AVAIL (0-16): %d\n"
		"    ME STATUS: 0x%02x\n"
		,
		reg_read32 (CP_RB_RPTR), reg_read32 (CP_RB_WPTR),
		reg_read32 (CP_MEQ_STAT) & MEQ_RPTR_mask, (reg_read32 (CP_MEQ_STAT) & MEQ_WPTR_mask) >> MEQ_WPTR_shift,
		reg_read32 (CP_ROQ_RB_STAT) & ROQ_RPTR_PRIMARY_mask, (reg_read32 (CP_ROQ_RB_STAT) & ROQ_WPTR_PRIMARY_mask) >> ROQ_WPTR_PRIMARY_shift,
		reg_read32 (CP_ROQ_IB1_STAT) & ROQ_RPTR_INDIRECT1_mask, (reg_read32 (CP_ROQ_IB1_STAT) & ROQ_WPTR_INDIRECT1_mask) >> ROQ_WPTR_INDIRECT1_shift,
		reg_read32 (GRBM_STATUS) & CMDFIFO_AVAIL_mask, reg_read32 (CP_ME_STATUS));

    if (verbose >= 1)
	printf ("  GRBM_STATUS*:\n");
    reg  = reg_read32 (GRBM_STATUS);
// Always on due to GRBM_STATUS request from CPU
//    STATE_TEST ( , SRBM_RQ_PENDING, "System Register Backbone request pending in GRBM");
///    STATE_TEST ( , CP_RQ_PENDING, "Non-queued request from the CP pending in GRBM");
    STATE_TEST ( , CF_RQ_PENDING, "Queued request from the CP pending in GRBM");
    STATE_TEST ( , PF_RQ_PENDING, "Request from the CP`s Pre-Fetch Parser pending in GRBM");
    STATE_TEST ( , GRBM_EE_BUSY, "GRBM Event Engine is busy");
///    STATE_TEST ( , GRBM_STATUS__VC_BUSY, "Vertex Cache is busy");
    STATE_TEST (!, DB03_CLEAN, "At least one of Depth Blocks (DB<0-3>) is not clean");
    STATE_TEST (!, CB03_CLEAN, "At least one of Color Blocks (CB<0-3>) is not clean");
    STATE_TEST ( , VGT_BUSY_NO_DMA, "VGT (excluding DMA) is busy");
    STATE_TEST ( , GRBM_STATUS__VGT_BUSY, "Vertex Grouper Tessellator is busy");
    if (adapt->chipset <= CHIPSET_RV670) {
	STATE_TEST ( , TA03_BUSY, "At least one of Texture Pipes is busy");
	STATE_TEST ( , GRBM_STATUS__TC_BUSY, "Texture Cache is busy");
    } else {
	STATE_TEST ( , R7XX_TA_BUSY, "At least one of Texture Pipes is busy");
    }
    STATE_TEST ( , SX_BUSY, "Shader Export is busy");
    STATE_TEST ( , SH_BUSY, "Sequencer is busy");
    STATE_TEST ( , SPI03_BUSY, "Any of the SPI`s are busy");
    if (adapt->chipset <= CHIPSET_RV670) {
	STATE_TEST ( , SMX_BUSY, "Shader Memory Export is busy");
    }
    STATE_TEST ( , SC_BUSY, "Scan Converter is busy");
    STATE_TEST ( , PA_BUSY, "Primitive Assembly is busy");
    STATE_TEST ( , DB03_BUSY, "Any of the Depth Blocks (DB<0-3>) are busy");
    if (adapt->chipset <= CHIPSET_RV670) {
	STATE_TEST ( , CR_BUSY, "Compse Rect block is busy");
    }
    STATE_TEST ( , CP_COHERENCY_BUSY, "Surface Coherency Logic is busy");
    STATE_TEST ( , GRBM_STATUS__CP_BUSY, "Command Processor is busy");
    STATE_TEST ( , CB03_BUSY, "Any of the Color Blocks (CB<0-3>) are busy");
    STATE_TEST ( , GUI_ACTIVE, "Graphics Pipe is busy. This includes whether the CP and GRBM also have transactions");

    reg = reg_read32 (GRBM_STATUS2);
    if (adapt->chipset <= CHIPSET_RV670) {
	STATE_TEST (!, CR_CLEAN, "Compose Rect block is not clean");
    }
    STATE_TEST (!, SMX_CLEAN, "Shader Memory Export is not clean");
    STATE_TEST ( , SPI0_BUSY, "Shader Pipe Interpolator (SPI0) is busy");
    STATE_TEST ( , SPI1_BUSY, "Shader Pipe Interpolator (SPI1) is busy");
    STATE_TEST ( , SPI2_BUSY, "Shader Pipe Interpolator (SPI2) is busy");
    STATE_TEST ( , SPI3_BUSY, "Shader Pipe Interpolator (SPI3) is busy");
    STATE_TEST ( , TA0_BUSY, "Texture Pipe (TA0) is busy");
    STATE_TEST ( , TA1_BUSY, "Texture Pipe (TA1) is busy");
    STATE_TEST ( , TA2_BUSY, "Texture Pipe (TA2) is busy");
    STATE_TEST ( , TA3_BUSY, "Texture Pipe (TA3) is busy");
    STATE_TEST ( , DB0_BUSY, "Depth Block (DB0) is busy");
    STATE_TEST ( , DB1_BUSY, "Depth Block (DB1) is busy");
    STATE_TEST ( , DB2_BUSY, "Depth Block (DB2) is busy");
    STATE_TEST ( , DB3_BUSY, "Depth Block (DB3) is busy");
    STATE_TEST ( , CB0_BUSY, "Color Block (CB0) is busy");
    STATE_TEST ( , CB1_BUSY, "Color Block (CB1) is busy");
    STATE_TEST ( , CB2_BUSY, "Color Block (CB2) is busy");
    STATE_TEST ( , CB3_BUSY, "Color Block (CB3) is busy");

    if (verbose >= 1)
	printf ("  SRBM_STATUS:\n");
    reg = reg_read32 (SRBM_STATUS);
    STATE_TEST ( , RLC_RQ_PENDING       , "There is an RLC request pending in the SRBM");
    STATE_TEST ( , RCU_RQ_PENDING       , "There is a RCU request pending in the SRBM");
    STATE_TEST ( , GRBM_RQ_PENDING      , "There is a GRBM request pending in the SRBM");
// Always on due to SRBM_STATUS request from CPU?
//    STATE_TEST ( , HI_RQ_PENDING        , "There is a HI/BIF request pending in the SRBM");
// Always on due to SRBM_STATUS request from CPU?
//    STATE_TEST ( , IO_EXTERN_SIGNAL     , "External IO Signal unconditioned");
    STATE_TEST ( , VMC_BUSY             , "Memory Controller Block VMC is Busy");
    STATE_TEST ( , MCB_BUSY             , "Memory Controller Block MCB is Busy");
    STATE_TEST ( , MCDZ_BUSY            , "Memory Controller Block MCDZ is Busy");
    STATE_TEST ( , MCDY_BUSY            , "Memory Controller Block MCDY is Busy");
// Always on due to SRBM_STATUS request from CPU?
//    STATE_TEST ( , MCDX_BUSY            , "Memory Controller Block MCDX is Busy");
    STATE_TEST ( , MCDW_BUSY            , "Memory Controller Block MCDW is Busy");
    STATE_TEST ( , SEM_BUSY             , "Semaphore Block is Busy");
    STATE_TEST ( , SRBM_STATUS__RLC_BUSY, "Run List Controller is Busy");
    STATE_TEST ( , PDMA_BUSY            , "Paging DMA Block is Busy");
    STATE_TEST ( , IH_BUSY              , "Interrupt Handler is Busy");
    STATE_TEST ( , CSC_BUSY             , "Context Switch Controller is Busy");
    STATE_TEST ( , CMC7_BUSY            , "Context Memory Client7 is Busy");
    STATE_TEST ( , CMC6_BUSY            , "Context Memory Client6 is Busy");
    STATE_TEST ( , CMC5_BUSY            , "Context Memory Client5 is Busy");
    STATE_TEST ( , CMC4_BUSY            , "Context Memory Client4 is Busy");
    STATE_TEST ( , CMC3_BUSY            , "Context Memory Client3 is Busy");
    STATE_TEST ( , CMC2_BUSY            , "Context Memory Client2 is Busy");
    STATE_TEST ( , CMC1_BUSY            , "Context Memory Client1 is Busy");
    STATE_TEST ( , CMC0_BUSY            , "Context Memory Client0 is Busy");
// Always on due to SRBM_STATUS request from CPU?
//    STATE_TEST ( , BIF_BUSY             , "Bus Interface is Busy");
    STATE_TEST ( , IDCT_BUSY            , "IDCT is Busy");

    if (verbose >= 1)
	printf ("  CP_STAT:\n");
    reg = reg_read32 (CP_STAT);
    STATE_TEST ( , CSF_RING_BUSY     , "The Ring fetcher still has command buffer data to fetch or the PFP still has data left to process from the reorder queue");
    STATE_TEST ( , CSF_WPTR_POLL_BUSY, "The Write Pointer polling logic is still polling for an updated write pointer");
    STATE_TEST ( , CSF_INDIRECT1_BUSY, "The Indirect1 fetcher still has command buffer data to fetch or the PFP still has data left to process from the reorder queue");
    STATE_TEST ( , CSF_INDIRECT2_BUSY, "The Indirect2 fetcher still has command buffer data to fetch or the PFP still has data left to process from the reorder queue");
    STATE_TEST ( , CSF_STATE_BUSY    , "The State fetcher still has command buffer data to fetch or the PFP still has data left to process from the reorder queue");
    STATE_TEST ( , CSF_PREDICATE_BUSY, "The Predicate fetcher still has command buffer data to fetch or the PFP still has data left to process from the reorder queue");
    STATE_TEST ( , CSF_BUSY          , "The input FIFOs have command buffers to be fetched, or one or more of the fetchers are busy, or the arbiter has a request to send to the MIU");
    STATE_TEST ( , MIU_RDREQ_BUSY    , "The read path logic inside the MIU is busy");
    STATE_TEST ( , MIU_WRREQ_BUSY    , "The write path logic inside the MIU is busy");
    STATE_TEST ( , ROQ_RING_BUSY     , "The Ring portion of the reorder queue has valid data in it");
    STATE_TEST ( , ROQ_INDIRECT1_BUSY, "The Indirect1 portion of the reorder queue has valid data in it");
    STATE_TEST ( , ROQ_INDIRECT2_BUSY, "The RiIndirect2 portion of the reorder queue has valid data in it");
    STATE_TEST ( , ROQ_STATE_BUSY    , "The State data portion of the reorder queue has valid data in it");
    STATE_TEST ( , ROQ_PREDICATE_BUSY, "The Predicate data portion of the reorder queue has valid data in it");
    STATE_TEST ( , ROQ_ALIGN_BUSY    , "The alignment logic inside the reoder queue is busy");
    STATE_TEST ( , PFP_BUSY          , "The Prefetch parser is busy processing PM4 packets");
    STATE_TEST ( , MEQ_BUSY          , "The PFP-to-ME queue has valid data in it");
    STATE_TEST ( , ME_BUSY           , "The MicroEngine is busy processing PM4 packets");
    STATE_TEST ( , QUERY_BUSY        , "The Query unit is busy");
    STATE_TEST ( , SEMAPHORE_BUSY    , "The Semaphore unit is busy");
    STATE_TEST ( , INTERRUPT_BUSY    , "The Interrupt unit is busy");
    STATE_TEST ( , SURFACE_SYNC_BUSY , "The Surface Sync unit is busy");
    STATE_TEST ( , DMA_BUSY          , "The DMA is busy");
    STATE_TEST ( , RCIU_BUSY         , "The RCIU has pending read or write transactions to the GRBM");
    STATE_TEST ( , CP_STAT__CP_BUSY  , "Any block in the CP is busy");
    
    if (verbose >= 1)
	printf ("  CP_BUSY_STAT:\n");
    reg = reg_read32 (CP_BUSY_STAT);
    STATE_TEST ( , REG_BUS_FIFO_BUSY     , "There is a register bus transaction for the CP in the input FIFO");
    STATE_TEST ( , RING_FETCHING_DATA    , "There is still data to be fetched from the Ring Buffer");
    STATE_TEST ( , INDR1_FETCHING_DATA   , "There is still data to be fetched from the Indirect1 Command Buffer");
    STATE_TEST ( , INDR2_FETCHING_DATA   , "There is still data to be fetched from the Indirect2 Command Buffer");
    STATE_TEST ( , STATE_FETCHING_DATA   , "There is still Renderstate or Constant data to be fetched");
    STATE_TEST ( , PRED_FETCHING_DATA    , "There is still Predicate data to be fetched");
    STATE_TEST ( , COHER_CNTR_NEQ_ZERO   , "The Coherency Counter for managing Load & Set packet interaction is not zero");
    STATE_TEST ( , PFP_PARSING_PACKETS   , "The PFP`s packet count is not zero and it is therefore parsing a packet");
    STATE_TEST ( , ME_PARSING_PACKETS    , "The ME`s packet count is not zero and it is therefore parsing a packet");
    STATE_TEST ( , RCIU_PFP_BUSY         , "The PFP path in the RCIU has data to send");
    STATE_TEST ( , RCIU_ME_BUSY          , "The ME path in the RCIU has data to send");
    STATE_TEST ( , OUTSTANDING_READ_TAGS , "The MIU is waiting for read request data to be returned from the MC");
    STATE_TEST ( , SEM_CMDFIFO_NOT_EMPTY , "There is either Wait or Signal Semaphore commands waiting to be sent");
    STATE_TEST ( , SEM_FAILED_AND_HOLDING, "The Wait Semaphore failed to pass within the programmed clock cycles and is holding for a context switch");
    STATE_TEST ( , SEM_POLLING_FOR_PASS  , "The Semaphore Unit is currently polling, waiting for its semaphore to get a Pass response");
    if (adapt->chipset > CHIPSET_RV670) 	/* Seems to always be set on R6xx with activated 3D context */
	STATE_TEST ( , _3D_BUSY          , "There is at least one context allocated for 3D processing");
    STATE_TEST ( , _2D_BUSY              , "There is at least one context allocated for 2D processing");

    if (verbose >= 1)
	printf ("  CP_INT_STATUS:\n");
    reg = reg_read32 (CP_INT_STATUS);
    STATE_TEST ( , DISABLE_CNTX_SWITCH_INT_STAT, "Interrupt Status for disabling context switching in the RLC. Write: 0 - No affect. 1 - Set Interrupt Status Bit. Default = 0");
    STATE_TEST ( , ENABLE_CNTX_SWITCH_INT_STAT , "Interrupt Status for enabling context switching in the RLC (see bit 0 for definition)");
    STATE_TEST ( , SEM_SIGNAL_INT_STAT         , "Interrupt Status for Memory Semaphore Signal Interrupt (See bit 31 for definition). Set when CP performs a Signal that increments the memory semaphore from 0 to 1. This informs the scheduler that a consumer process can be scheduled and its WAIT is guaranteed to Pass. Only Valid for RV670");
    STATE_TEST ( , CNTX_BUSY_INT_STAT          , "Interrupt Status for Context Busy (!GUI Idle) Interrupt (See bit 31 for definition)");
    STATE_TEST ( , CNTX_EMPTY_INT_STAT         , "Interrupt Status for Context Empty (GUI Idle) Interrupt (See bit 31 for definition)");
    STATE_TEST ( , WAITMEM_SEM_INT_STAT        , "Interrupt Status for Wait Memory Semaphore Fault (See bit 31 for definition)");
    STATE_TEST ( , PRIV_INSTR_INT_STAT         , "Interrupt Status for Privileged Instruction Fault (See bit 31 for definition)");
    STATE_TEST ( , PRIV_REG_INT_STAT           , "Interrupt Status for Privileged Register Fault (See bit 31 for definition)");
    STATE_TEST ( , OPCODE_ERROR_INT_STAT       , "Interrupt Status for the Opcode Error (See bit 31 for definition)");
    STATE_TEST ( , SCRATCH_INT_STAT            , "Interrupt Status for the Scratch Register Compare Interrupt (See bit 31 for definition)");
    STATE_TEST ( , TIME_STAMP_INT_STAT         , "Interrupt Status for the Time Stamp Interrupt (See bit 31 for definition)");
    STATE_TEST ( , RESERVED_BIT_ERROR_INT_STAT , "Interrupt Status for Reserved bits != 0 Error (non- functional mode) (See bit 31 for definition)");
    STATE_TEST ( , DMA_INT_STAT                , "Interrupt Status for the DMA Engine (See bit 31 for definition)");
    STATE_TEST ( , IB2_INT_STAT                , "Interrupt Status for CP_INTERRUPT packet in IB2 Stream (See bit 31 for definition)");
    STATE_TEST ( , IB1_INT_STAT                , "Interrupt Status for CP_INTERRUPT packet in IB1 Stream (See bit 31 for definition)");
    STATE_TEST ( , RB_INT_STAT                 , "Interrupt Status for CP_INTERRUPT packet in Ring Buffer. Write: 0 - No affect. 1 - Set Interrupt Status Bit. Default = 0");

    if (verbose >= 1)
	printf ("  CP_STALLED_STAT*:\n");
    reg = reg_read32 (CP_STALLED_STAT1);
    STATE_TEST ( , RBIU_TO_DMA_NOT_RDY_TO_RCV   , "The RBIU cannot write to the DTAQ or SRC, DST, CMD registers");
    STATE_TEST ( , RBIU_TO_IBS_NOT_RDY_TO_RCV   , "The RBIU cannot write to the Indirect Buffer Base & Size registers");
    STATE_TEST ( , RBIU_TO_SEM_NOT_RDY_TO_RCV   , "The RBIU cannot write to the Semaphore units address FIFO");
    STATE_TEST ( , RBIU_TO_2DREGS_NOT_RDY_TO_RCV, "The RBIU cannot write to the 2D registers in the Scratch RAM");
    STATE_TEST ( , RBIU_TO_MEMWR_NOT_RDY_TO_RCV , "The RBIU cannot write to the Address & Data FIFOs to initiate a write to memory");
    STATE_TEST ( , RBIU_TO_MEMRD_NOT_RDY_TO_RCV , "The RBIU cannot write to the Address & Data registers to initiate a read from memory");
    STATE_TEST ( , RBIU_TO_EOPD_NOT_RDY_TO_RCV  , "The RBIU cannot write to the EOP Done FIFO");
    STATE_TEST ( , RBIU_TO_RECT_NOT_RDY_TO_RCV  , "The RBIU cannot write to the Rect Done FIFO");
    STATE_TEST ( , RBIU_TO_STRMO_NOT_RDY_TO_RCV , "The RBIU cannot write to the Stream Out address FIFO");
    STATE_TEST ( , RBIU_TO_PSTAT_NOT_RDY_TO_RCV , "The RBIU cannot write to the Pipeline Statistics address FIFO");
    STATE_TEST ( , MIU_WAITING_ON_RDREQ_FREE    , "The MIU read request logic is stalled waiting for Free signals to come back from the MC");
    STATE_TEST ( , MIU_WAITING_ON_WRREQ_FREE    , "The MIU write request logic is stalled waiting for Free signals to come back from the MC");
    STATE_TEST ( , MIU_NEEDS_AVAIL_WRREQ_PHASE  , "The MIU write request logic is stalled waiting for an available phase for write confirmations");
    STATE_TEST ( , RCIU_WAITING_ON_GRBM_FREE    , "The RCIU is stalled waiting for Free signals to come back from the GRBM");
    STATE_TEST ( , RCIU_WAITING_ON_VGT_FREE     , "The RCIU is stalled waiting for VGT Free signals to come back from the GRBM");
    STATE_TEST ( , RCIU_STALLED_ON_ME_READ      , "The RCIU is stalled waiting for register read data to come back for the ME");
    STATE_TEST ( , RCIU_STALLED_ON_DMA_READ     , "The RCIU is stalled waiting for register read data to come back for the DMA");
    STATE_TEST ( , RCIU_HALTED_BY_REG_VIOLATION , "The RCIU has been halted because of a register violation (a write to a privileged register from a non- privileged command buffer");

    reg = reg_read32 (CP_STALLED_STAT2);
    STATE_TEST ( , PFP_TO_CSF_NOT_RDY_TO_RCV    , "The PFP is stalled trying to write to the I1, I2, State or Predicate FIFOs");
    STATE_TEST ( , PFP_TO_MEQ_NOT_RDY_TO_RCV    , "The PFP is stalled trying to write to the MEQ");
    STATE_TEST ( , PFP_TO_VGT_NOT_RDY_TO_RCV    , "The PFP is stalled trying to write to the VGT");
    STATE_TEST ( , PFP_HALTED_BY_INSTR_VIOLATION, "The PFP has been halted because of an instruction violation (Priv IB called from a non-priv IB)");
    STATE_TEST ( , MULTIPASS_IB_PENDING_IN_PFP  , "There is a multipass IB pending in the PFP and it is therefore waiting for the Loop or Continue command from the SC");
    STATE_TEST ( , ME_BRUSH_WC_NOT_RDY_TO_RCV   , "There are 32 pending Brush writes with write confirms pending and the ME is stalled trying to write another Brush write confirm");
    STATE_TEST ( , ME_STALLED_ON_BRUSH_LOGIC    , "The ME is either stalled trying to initiate the Brush logic which is currently active or it is stalled waiting for the Brush logic to finish generating writes before sending another write to the RCIU");
    STATE_TEST ( , CR_CNTX_NOT_AVAIL_TO_ME      , "All CR contexts are active and the ME is stalled waiting for a free CR context");
    STATE_TEST ( , GFX_CNTX_NOT_AVAIL_TO_ME     , "All GFX contexts are active and the ME is stalled waiting for a free GFX contexts");
    STATE_TEST ( , ME_RCIU_NOT_RDY_TO_RCV       , "The ME is stalled trying to write to the RCIU");
    STATE_TEST ( , ME_TO_CONST_NOT_RDY_TO_RCV   , "The ME is stalled trying to write down its Constant path");
    if (adapt->chipset <= CHIPSET_RV670) {
	// On if CP is idle?
	//    STATE_TEST ( , ME_WAITING_DATA_FROM_PFP     , "The ME is expecting data from the PFP, but there is none in the PFP-to-ME queue");
	STATE_TEST (!, ME_WAITING_DATA_FROM_PFP     , "The ME is expecting data from the PFP, and there is some in the PFP-to-ME queue. Indicates CP is not idle.");
    }
    STATE_TEST ( , ME_WAITING_ON_PARTIAL_FLUSH  , "The ME sent out a Partial Flush event and is waiting for a response from the SPI (SPI_CP_partial_flush)");
    STATE_TEST ( , RECT_FIFO_NEEDS_CR_RECT_DONE , "The Rect Done FIFO has data to send, but it is waiting for a CR_RECT_DONE pulse");
    STATE_TEST ( , RECT_FIFO_NEEDS_WR_CONFIRM   , "The Rect Done FIFO is waiting for a write confirm before popping the FIFO");
    STATE_TEST ( , EOPD_FIFO_NEEDS_SC_EOP_DONE  , "The Eop Done FIFO has data to send, but it is waiting for a SC_EOP_DONE pulse");
    STATE_TEST ( , EOPD_FIFO_NEEDS_SMX_EOP_DONE , "The Eop Done FIFO has data to send, but it is waiting for a SMX_EOP_DONE pulse");
    STATE_TEST ( , EOPD_FIFO_NEEDS_WR_CONFIRM   , "The Eop Done FIFO is waiting for a write confirm before popping the FIFO");
    STATE_TEST ( , EOPD_FIFO_NEEDS_SIGNAL_SEM   , "The Eop Done FIFO is waiting for Signal Sem results before popping the FIFO");
    STATE_TEST ( , SO_NUMPRIM_FIFO_NEEDS_SOADDR , "The Stream Out FIFOs have the Num_Prim data and are waiting for an address");
    STATE_TEST ( , SO_NUMPRIM_FIFO_NEEDS_NUMPRIM, "The Stream Out FIFOs have the address are are waiting for Num_Prim data");
    STATE_TEST ( , PIPE_STATS_FIFO_NEEDS_SAMPLE , "The Pipeline Statistics FIFOs are waiting for a Sample signal from one of the reporting blocks");
    STATE_TEST ( , SURF_SYNC_NEEDS_IDLE_CNTXS   , "The Surface Sync logic is waiting for all active contexts to go idle");
    STATE_TEST ( , SURF_SYNC_NEEDS_ALL_CLEAN    , "The Surface Sync logic is waiting for all of the Clean signals to be asserted");
    
    if (verbose >= 1)
	printf ("  VGT_CNTL_STATUS:\n");
    reg = reg_read32 (VGT_CNTL_STATUS);
    STATE_TEST ( , VGT_OUT_INDX_BUSY        , "If set, the Output Index block within the VGT is busy");
    STATE_TEST ( , VGT_OUT_BUSY             , "If set, the Output block within the VGT is busy");
    STATE_TEST ( , VGT_PT_BUSY              , "If set, the Pass-thru block within the VGT is busy");
    STATE_TEST ( , VGT_TE_BUSY              , "If set, the Tessellation Engine block within the VGT is busy");
    STATE_TEST ( , VGT_VR_BUSY              , "If set, the Vertex Reuse Block within the VGT is busy");
    STATE_TEST ( , VGT_GRP_BUSY             , "If set, the Grouper Block within the VGT is busy");
    STATE_TEST ( , VGT_DMA_REQ_BUSY         , "If set, the VGT DMA is busy requesting");
    STATE_TEST ( , VGT_DMA_BUSY             , "If set, the VGT DMA is busy");
    STATE_TEST ( , VGT_GS_BUSY              , "If set, VGT GS is actively processing");
    STATE_TEST ( , VGT_BUSY                 , "If set, VGT is Busy");

    if (verbose >= 1)
	printf ("  Vertex + Texture:\n");
    if (adapt->chipset >= CHIPSET_RV630) {
	reg = reg_read32 (VC_CNTL_STATUS);
	STATE_TEST ( , RP_BUSY, "Vertex Cache Request Processor is busy");
	STATE_TEST ( , RG_BUSY, "Vertex Cache Request Generator is busy");
	STATE_TEST ( , VC_BUSY, "Vertex Cache is busy");
	reg = reg_read32 (TC_STATUS);
	STATE_TEST ( , TC_BUSY, "Texture Cache is busy");
    }
    
    if (adapt->chipset == CHIPSET_R600) {
	/* Registers apparently not available on RV630 already */
	int i;
	for (i = 0; i < TD0_STATUS_num; i++) {
	    reg = reg_read32 (TD0_STATUS + i * 4);
	    STATE_TESTi ( , BUSY, "Texture Data is busy");
	}
	for (i = 0; i < 4; i++) {
	    reg = reg_read32 (TA0_STATUS + i*4);
	    STATE_TESTi ( , FG_PFIFO_EMPTYB , "Gradient FIFO state, pipeline fifo not empty");
	    STATE_TESTi ( , FG_LFIFO_EMPTYB , "Gradient FIFO state, latency fifo not empty");
	    STATE_TESTi ( , FG_SFIFO_EMPTYB , "Gradient FIFO state, state fifo not empty");
	    STATE_TESTi ( , FL_PFIFO_EMPTYB , "LOD FIFO state, pipeline fifo not empty");
	    STATE_TESTi ( , FL_LFIFO_EMPTYB , "LOD FIFO state, latency fifo not empty");
	    STATE_TESTi ( , FL_SFIFO_EMPTYB , "LOD FIFO state, state fifo not empty");
	    STATE_TESTi ( , FA_PFIFO_EMPTYB , "Addresser FIFO state, pipeline fifo not empty");
	    STATE_TESTi ( , FA_LFIFO_EMPTYB , "Addresser FIFO state, latency fifo not empty");
	    STATE_TESTi ( , FA_SFIFO_EMPTYB , "Addresser FIFO state, state fifo not empty");
	    STATE_TESTi ( , IN_BUSY         , "Input/LOD(Deriv) busy");
	    STATE_TESTi ( , FG_BUSY         , "Gradient FIFO busy");
	    STATE_TESTi ( , FL_BUSY         , "LOD FIFO busy");
	    STATE_TESTi ( , TA_BUSY         , "Addresser busy");
	    STATE_TESTi ( , FA_BUSY         , "Addresser FIFO busy");
	    STATE_TESTi ( , AL_BUSY         , "Aligner busy");
	    STATE_TESTi ( , BUSY            , "Global TA busy");
	}
    }
 
    if (verbose >= 1)
	printf ("  Various:\n");
    reg = reg_read32 (PA_CL_CNTL_STATUS);
    STATE_TEST ( , CL_BUSY, "PA Clipping Unit is busy");
    reg = reg_read32 (PA_SU_CNTL_STATUS);
    STATE_TEST ( , SU_BUSY, "PA SU (Rasterizer?) Unit is busy");

    if (verbose >= 1)
	printf ("  Read Errors:\n");
    reg = reg_read32 (SRBM_READ_ERROR);
    if (reg & READ_ERROR_bit) {
	printf ("    * SRBM READ ERROR indicated:\n      Target Adr 0x%04x by %s%s%s%s\n",
		(reg & READ_ADDRESS_mask) >> READ_ADDRESS_shift << 2,
		reg & READ_REQUESTER_HI_bit ? "HI/BIF ":"",
		reg & READ_REQUESTER_GRBM_bit ? "GRBM ":"",
		reg & READ_REQUESTER_RCU_bit ? "RCU ":"",
		reg & READ_REQUESTER_RLC_bit ? "RLC ":"");
	reg_write32 (SRBM_INT_ACK, 1);
	reg_write32 (SRBM_INT_STATUS, 1);
    }
    reg = reg_read32 (GRBM_READ_ERROR);
    if (reg & READ_ERROR_bit) {
	printf ("    * GRBM READ ERROR indicated:\n      Target Adr 0x%04x by %s%s%s\n",
		(reg & READ_ADDRESS_mask) >> READ_ADDRESS_shift << 2,
		reg & READ_REQUESTER_SRBM_bit ? "SRBM ":"",
		reg & READ_REQUESTER_CP_bit ? "CP ":"",
		reg & READ_REQUESTER_WU_POLL_bit ? "WU_POLL ":"");
	reg_write32 (GRBM_READ_ERROR, 0);
    }
    
    if (verbose >= 1)
	printf ("\n");
}


void dump_register (adapter_t *adapt, uint32_t reg)
{
    uint32_t val;
    
#if 0
    if (reg < 65536) {
	val = reg_read32 (reg);
    } else {
	/* Let the GPU copy the register, because we will not be able to access it directly */
	reg_write32 (SCRATCH_REG6, 0xdeadbeef);
	pack3       (IT_COPY_DW, 5);
	e32         (0x00000000);			/* reg to reg copy */
	e32         (reg>>2);
	e32         (0);
	e32         (SCRATCH_REG7>>2);
	e32         (0);
	ereg        (SCRATCH_REG6, 0xfeedface);
	flush_cmds  ();
	wait_reg    (adapt, SCRATCH_REG6, 0xfeedface, "CP+DMA: PACK3: scratch");
	val = reg_read32 (SCRATCH_REG7);
    }
#else
    if (reg_read32 (SRBM_READ_ERROR) & READ_ERROR_bit) {
	printf ("    * uncleared SRBM READ ERROR by previous call\n");
	reg_write32 (SRBM_INT_ACK, 1);
	reg_write32 (SRBM_INT_STATUS, 1);
    }
    if (reg_read32 (GRBM_READ_ERROR) & READ_ERROR_bit) {
	printf ("    * uncleared GRBM READ ERROR by previous call\n");
	reg_write32 (GRBM_READ_ERROR, 0);
    }
    reg_write32 (MM_INDEX, reg);
    val = reg_read32 (MM_DATA);
    if (reg_read32 (SRBM_READ_ERROR) & READ_ERROR_bit) {
	printf  ("  %08x (# %04x) := not accessible (SRBM)\n", reg, reg >> 2);
	reg_write32 (SRBM_INT_ACK, 1);
	reg_write32 (SRBM_INT_STATUS, 1);
	return;
    }
    if (reg_read32 (GRBM_READ_ERROR) & READ_ERROR_bit) {
	printf  ("  %08x (# %04x) := not accessible (GRBM)\n", reg, reg >> 2);
	reg_write32 (GRBM_READ_ERROR, 0);
	return;
    }
#endif
    
    printf      ("  %08x (# %04x) := %08x\n", reg, reg >> 2, val);
}


void write_register (adapter_t *adapt, uint32_t reg, uint32_t val)
{
    uint32_t oldval;
    
    if (reg_read32 (SRBM_READ_ERROR) & READ_ERROR_bit) {
	printf ("    * uncleared SRBM READ ERROR by previous call\n");
	reg_write32 (SRBM_INT_ACK, 1);
	reg_write32 (SRBM_INT_STATUS, 1);
    }
    if (reg_read32 (GRBM_READ_ERROR) & READ_ERROR_bit) {
	printf ("    * uncleared GRBM READ ERROR by previous call\n");
	reg_write32 (GRBM_READ_ERROR, 0);
    }
    reg_write32 (MM_INDEX, reg);
    oldval = reg_read32 (MM_DATA);
    reg_write32 (MM_DATA, val);
    if (reg_read32 (SRBM_READ_ERROR) & READ_ERROR_bit) {
	printf  ("  %08x (# %04x) := not accessible (SRBM)\n", reg, reg >> 2);
	reg_write32 (SRBM_INT_ACK, 1);
	reg_write32 (SRBM_INT_STATUS, 1);
	return;
    }
    if (reg_read32 (GRBM_READ_ERROR) & READ_ERROR_bit) {
	printf  ("  %08x (# %04x) := not accessible (GRBM)\n", reg, reg >> 2);
	reg_write32 (GRBM_READ_ERROR, 0);
	return;
    }
    
    printf      ("  %08x (# %04x) set to %08x  (old %08x)\n", reg, reg >> 2, val, oldval);
}


uint32_t *create_sample_texture (int width, int height, int pitch)
{
    int y, x;
    uint32_t *tex = malloc (pitch * height * sizeof(uint32_t));
    uint32_t *t = tex;
    for (y = 0; y < height; y++) {
	for (x = 0; x < width; x++)
	    t[x] = ((0x00000001 * (0x100 * x / width))					|
		    (0x00000100 * (0x100 * y / height))					|
		    (0x00010000 * (((0x200 * x / width) + (0x200 * y / height)) % 0xff))|
		    (0xff000000));
	t += pitch;
    }
    return tex;
}


float time_flush_cmds (adapter_t *adapt, float maxtime)
{
    reg_write32 (SCRATCH_REG5, 0xdeadbeef);
    wait_reg    (adapt, SCRATCH_REG5, 0xdeadbeef, "time_flush_cmds: init");

    wait_3d_full_idle_clean();
    pack0       (SCRATCH_REG5, 1);
    e32         (0xcafebabe);

    flush_cmds  ();

    return wait_reg_time (adapt, SCRATCH_REG5, 0xcafebabe, "time_flush_cmds: fence", maxtime);
}

