/*
 * r600_demo
 *
 * Copyright (C) 2008-2009  Matthias Hopf
 * Copyright (C) 2008-2009  Alexander Deucher
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
 * r600_demo
 * Bringup tool for R6xx, R7xx
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <getopt.h>

#include "xf86drm.h"
#include "radeon_drm.h"
#include "r600_reg.h"
#include "r600_lib.h"
#include "r600_hwapi.h"


/* Options */
int verbose = 0;
int use_ring_directly = 0;
int vertex_format = 0;
int vertex_load_as_int = 0;
int vertex_unsigned = 0;
int vertex_alu_scale_override = 0;
int do_not_flush = 0;

/* DRM context & file descriptor & dma structure */
int drmFD=-1;
drm_context_t context;
drmDMAReq  dma;

drmBufMapPtr BufMapPtr=NULL;

volatile void *registers=NULL;
void *framebuffer=NULL;
int framebuffer_size=0;

int display_width, display_pitch, display_height;
uint32_t display_gpu;		// Should be 64bit, but is currently 32bit address in R6xx, R7xx chips

/* DMA buffers */
#define DMA_COUNT 3
#define DMA_SIZE 64000
enum { INDIRECT = 0, VERTICES, TEXTURE } ;

int indices[DMA_COUNT];
int sizes[DMA_COUNT];

int       indirect_start=0, indirect_end=0, indirect_idx=-1, indirect_size=0, ring_size=0;
int       vtx_idx=-1, tex_idx=-1;
uint32_t *indirect, *vtx, *tex;
void     *gart, *gartbuf, *garttex, *ring;
uint64_t  gart_gpu, gartbuf_gpu, garttex_gpu, vtx_gpu, tex_gpu, ring_gpu;
uint32_t  garttex_handle;

adapter_t adapter;


/* This sets some specifics card pipe functionalities according
   to card model. */


void xf86InstallSIGIOHandler(void)
{
    fprintf(stderr,"I should not be called !!!\n");
    exit(-1);
}


void xf86RemoveSIGIOHandler(void)
{
    fprintf(stderr,"I should not be called !!!\n");
    exit(-1);
}


void print_versions(void)
{
    drmVersionPtr v;
    v=drmGetVersion(drmFD);
    fprintf(stderr,"driver \"%s\" version %d.%d.%d\n",v->name,v->version_major,v->version_minor,v->version_patchlevel);
    fprintf(stderr,"driver description \"%s\"\n",v->desc);
    fprintf(stderr,"AGPVendorID: %04x  AGPDeviceId: %04x\n", 
	    drmAgpVendorId(drmFD),
	    drmAgpDeviceId(drmFD));
    fprintf(stderr,"BusID: \"%s\"\n", drmGetBusid(drmFD));	
}


void GetMaps(void)
{
    int i, r;
    drm_handle_t offset;
    drmSize size;
    drmMapType type;
    drmMapFlags flags;
    drm_handle_t handle;
    int mtrr;
    char *typename;
    drm_radeon_getparam_t gp;
    uint32_t gpu;

    gp.param = RADEON_PARAM_GART_BASE;
    gp.value = (int *) &gpu;
    if ( (r = drmCommandWriteRead (drmFD, DRM_RADEON_GETPARAM, &gp, sizeof(gp))) < 0)
	drmError(r, "drm: RADEON_PARAM_GART_BASE");
    gart_gpu = gpu;
    if (verbose >= 2)
	fprintf (stderr, "gart_base: 0x" PRINTF_UINT64_HEX "\n", gart_gpu);
    gart = NULL;		// unknown how to get this - except for by calculation with gartbuf

    gp.param = RADEON_PARAM_GART_BUFFER_OFFSET;
    gp.value = (int *) &gpu;
    if ( (r = drmCommandWriteRead (drmFD, DRM_RADEON_GETPARAM, &gp, sizeof(gp))) < 0)
	drmError(r, "drm: RADEON_PARAM_GART_BUFFER_OFFSET");
    gartbuf_gpu = gpu;
    if (verbose >= 2)
	fprintf (stderr, "gart_buffer: 0x" PRINTF_UINT64_HEX "\n", gartbuf_gpu);

    gp.param = RADEON_PARAM_GART_TEX_HANDLE;
    gp.value = (int *) &garttex_handle;
    if ( (r = drmCommandWriteRead (drmFD, DRM_RADEON_GETPARAM, &gp, sizeof(gp))) < 0)
	drmError(r, "drm: RADEON_PARAM_GART_TEX_HANDLE");
    if (verbose >= 2)
	fprintf (stderr, "gart_tex_handle: 0x%08x  (ret %d)\n", garttex_handle, r);

//    agp_physical=drmAgpBase(drmFD);
//    fprintf(stderr,"AGP base: 0x%08x\n", (unsigned int)agp_physical);

    if (verbose >= 2)
	fprintf(stderr,"Map Offset         Size    Type Handle     Mtrr\n");

    for(i=0; !drmGetMap(drmFD, i, &offset, &size, &type, &flags, &handle, &mtrr); i++){
	switch(type){
	case DRM_FRAME_BUFFER:   
	    typename = "FB";  
	    if((r=drmMap(drmFD, offset, size, &framebuffer))<0)
		drmError(r, "drm: DRM_FRAME_BUFFER");
	    framebuffer_size = size;
	    break;
	case DRM_REGISTERS:      
	    typename = "REG"; 
	    if((r=drmMap(drmFD, offset, size, (drmAddressPtr)&registers))<0)
		drmError(r, "drm: DRM_REGISTERS");
	    break;
	case DRM_SHM:            
	    typename = "SHM"; 
	    break;
	case DRM_AGP:            
	    typename = "AGP";
#if 0
	    if(offset==agp_physical){
		if((r=drmMap(drmFD, offset, size, (drmAddressPtr)&agp_space))<0)
		    drmError(r, "drm: DRM_AGP");
	    }
#endif
	    break;
	case DRM_SCATTER_GATHER: 
	    typename = "SG";
	    break;
	default:                 
	    typename = "???"; 
	    break;
	}
	if (verbose >= 2)
	    fprintf(stderr,"%2d 0x%08x %8d KB  %3.3s 0x%08x ",
		    i, offset, size/1024, typename, handle);
	if(mtrr < 0 ) {
	    if (verbose >= 2)
		fprintf(stderr," none");
	} else {
	    if (verbose >= 2)
		fprintf(stderr,"%4d", mtrr);
	}
	if (handle == garttex_handle) {
	    if (verbose >= 2)
		fprintf (stderr, "  texture heap");
	    if( (r=drmMap (drmFD, offset, size, (drmAddressPtr)&garttex))<0) {
		drmError(r, "drm: texture heap");
		garttex_gpu = 0;
		garttex = NULL;
	    }
	    garttex_gpu = offset;
	}
	if (verbose >= 2)
	    fprintf (stderr, "\n");
    }
#if 0
/* On PPC ask the graphics card where it see AGP */
#if BYTE_ORDER == BIG_ENDIAN
    agp_physical=(((get_int(RADEON_MC_AGP_LOCATION))& 0x0ffffU) << 16);
#endif
#endif

    ring_size = 8 << (reg_read32 (CP_RB_CNTL) & RB_BUFSZ_mask);
    for(i=0; !drmGetMap(drmFD, i, &offset, &size, &type, &flags, &handle, &mtrr); i++) {
	/* !!! This is extremely fragile !!! */
	if (type == DRM_SCATTER_GATHER && size == ring_size + 4096)
	    if((r=drmMap(drmFD, offset, size, &ring))<0) {
		drmError(r, "drm: ring");
		ring = NULL;
	    }
    }
}


void GetBufs(void)
{
    int i,r;
    uint32_t offset, addedoffset = 0;

    dma.context         = context;
    dma.send_count      = 0;
    dma.request_count   = DMA_COUNT;
    dma.request_size    = DMA_SIZE;
    dma.request_list    = indices;
    dma.request_sizes   = sizes;
    dma.flags           = DRM_DMA_WAIT;
    for (i=0; i<2000000; i++) {
	drmGetLock(drmFD, context, DRM_LOCK_READY);
	if((r=drmDMA(drmFD, &dma))!=EBUSY){
	    drmUnlock(drmFD,context);
	    if(!r)break;
			
	    drmError(r, __func__);
	    exit(-1);
	}
	fprintf(stderr,"drmDMA()=EBUSY, trying again\n");
	drmUnlock(drmFD,context);
    }
    if (verbose >= 2) {
	fprintf(stderr, "Buffers:\n");
	for(i=0;i<dma.granted_count;i++){
	    fprintf(stderr, "%3d: index = %d, size = %d\n",
		    i, dma.request_list[i], dma.request_sizes[i]);
	}
    }

#ifdef USE_RING_DIRECTLY
    indirect_size = ring_size;
    indirect      = calloc (indirect_size, 1);
#else
    indirect_idx=indices[INDIRECT];
    indirect_size=sizes[INDIRECT];
#endif
    vtx_idx=indices[VERTICES];
    tex_idx=indices[TEXTURE];

    if (verbose >= 2)
	fprintf (stderr, "Mapping buffers:\n");
    BufMapPtr = drmMapBufs(drmFD);
    if(BufMapPtr == NULL){
    	drmError (r, __func__);
	fprintf (stderr, "Could not map buffers, aborting\n");
    	exit (-1);
    }
    gartbuf = BufMapPtr->list[0].address;

    if (verbose >= 2)
	fprintf (stderr, "  idx size\tused\taddress\n");
    for (i = 0; i < BufMapPtr->count; i++) {
	if (verbose >= 2)
	    fprintf (stderr, "  %-3d %d\t%d\t%p",
		     BufMapPtr->list[i].idx,
		     BufMapPtr->list[i].total,
		     BufMapPtr->list[i].used,
		     BufMapPtr->list[i].address);
	if (BufMapPtr->list[i].idx == indirect_idx) {
	    if (verbose >= 2)
		fprintf (stderr, "\tINDIRECT");
	    indirect       = BufMapPtr->list[i].address;
	    indirect_start = BufMapPtr->list[i].used;
	    indirect_end   =indirect_start;
	}
	if (BufMapPtr->list[i].idx == vtx_idx) {
	    vtx     = BufMapPtr->list[i].address;
	    memset (vtx, 0, 32768);
	    vtx_gpu = gartbuf_gpu + addedoffset;
	    if (verbose >= 2)
		fprintf (stderr, "\tVERTICES cpu %p gpu 0x" PRINTF_UINT64_HEX, vtx, vtx_gpu);
	}
	if (BufMapPtr->list[i].idx == tex_idx) {
	    tex     = BufMapPtr->list[i].address;
	    memset (tex, 0, 32768);
	    tex_gpu = gartbuf_gpu + addedoffset;
	    if (verbose >= 2)
		fprintf (stderr, "\tTEXTURE cpu %p gpu 0x" PRINTF_UINT64_HEX, tex, tex_gpu);
	}
	if (verbose >= 2)
	    fprintf (stderr,"\n");
	addedoffset += BufMapPtr->list[i].total;
    }

    ring_gpu      = reg_read32 (CP_RB_BASE) << 8;
    if (verbose >= 2)
	fprintf (stderr, "command buffer: cpu %p size %x\nring buffer:    cpu %p gpu, 0x"
		 PRINTF_UINT64_HEX "\n",
		 indirect, indirect_size, ring, ring_gpu);
    if (! ring && use_ring_directly) {
	fprintf (stderr, "Cannot map ring\n");
	exit (1);
    }
    if (verbose >= 2)
	fprintf (stderr, "indirect buffer: cpu %p size %x\n", indirect, indirect_size);
    
    drm_radeon_mem_alloc_t alloc;
    alloc.region = RADEON_MEM_REGION_GART;
    alloc.alignment = 4096;
    alloc.size      = 4096;
    alloc.region_offset = (int *) &offset;

#if 0
    /* Apparently, all card internal offsets except gart_gpu (BASE) and
     * gartbuf_gpu (BUFFER_OFFSET) are wrong...
     * So use command buffers for all data as well. */
    if ( (r = drmCommandWriteRead (drmFD, DRM_RADEON_ALLOC, &alloc, sizeof(alloc))) < 0)
	drmError(r, __func__);
    vtx_gpu = garttex_gpu + offset;
//    vtx_gpu = gart_gpu + offset;
    vtx     = garttex + offset;
    fprintf (stderr, "alloced: offset 0x%08x, cpu %p, gpu 0x%08llx\n", offset, vtx, vtx_gpu);
#endif
}


void read_registers(void)
{
    uint32_t vendor;
    
    if(registers==NULL){
	fprintf(stderr,"No register map found\n");
	exit(-1);
    }

    vendor = reg_read32 (VENDOR_DEVICE_ID);
    adapter.chipID = vendor >> 16;
    vendor &= 0xffff;
    fprintf (stderr,"VendorID: %04x DeviceID: %04x\n", vendor, adapter.chipID);
    if (vendor != 0x1002) {
	fprintf (stderr,"VendorID is not 0x1002, aborting\n");
	exit(-1);
    }

    switch (adapter.chipID) {
    case 0x9400:    case 0x9401:    case 0x9402:    case 0x9403:
    case 0x9405:    case 0x940A:    case 0x940B:    case 0x940F:
	adapter.chipset = CHIPSET_R600;
	break;
    case 0x94C0:    case 0x94C1:    case 0x94C3:    case 0x94C4:
    case 0x94C5:    case 0x94C6:    case 0x94C7:    case 0x94CC:
	adapter.chipset = CHIPSET_RV610;
	break;
    case 0x9500:    case 0x9501:    case 0x9505:    case 0x9507:
    case 0x9511:    case 0x9515:    case 0x9517:    case 0x9519:
	adapter.chipset = CHIPSET_RV670;
	break;
    case 0x9580:    case 0x9586:    case 0x9587:    case 0x9588:
    case 0x9589:    case 0x958A:    case 0x958C:    case 0x958D:
    case 0x958E:
	adapter.chipset = CHIPSET_RV630;
	break;
    case 0x9590:    case 0x9596:    case 0x9597:    case 0x9598:
    case 0x9599:
	adapter.chipset = CHIPSET_RV635;
	break;
    case 0x95C0:    case 0x95C5:    case 0x95C7:    case 0x95CC:
    case 0x95CD:    case 0x95CE:    case 0x95CF:
	adapter.chipset = CHIPSET_RV620;
	break;
    case 0x9610:    case 0x9611:    case 0x9612:    case 0x9613:
    case 0x9614:
	adapter.chipset = CHIPSET_RS780;
	break;
    case 0x94C9:    case 0x94CB:
	adapter.chipset = CHIPSET_M72;
	break;
    case 0x9440:    case 0x9442:    case 0x9444:    case 0x9446:
    case 0x944C:    case 0x944E:    case 0x9456:
	adapter.chipset = CHIPSET_RV770;
	break;
    case 0x9540:    case 0x9541:    case 0x9592:    case 0x954E:
    case 0x954F:    case 0x9552:    case 0x9553:
	adapter.chipset = CHIPSET_RV710;
	break;
    case 0x9487:    case 0x948F:    case 0x9490:    case 0x9498:
    case 0x9480:    case 0x9488:    case 0x949c:    case 0x949E:
    case 0x949F:
	adapter.chipset = CHIPSET_RV730;
	break;
    case 0x94C8: /* RHD_M74 */    case 0x9504: /* RHD_M88 */
    case 0x9506: /* RHD_M88 */    case 0x9508: /* RHD_M88 */
    case 0x9509: /* RHD_M88 */    case 0x9581: /* RHD_M76 */
    case 0x9583: /* RHD_M76 */    case 0x958B: /* RHD_M76 */
    case 0x958F: /* RHD_M76 */    case 0x950F: /* RHD_R680 */
    default:
	adapter.chipset = CHIPSET_NONE;
    }

    switch (adapter.chipset) {
    case CHIPSET_R600:
	fprintf (stderr, "Chipset: R600\n\n");
	break;
    case CHIPSET_RV610:
	fprintf (stderr, "Chipset: RV610\n\n");
	break;
    case CHIPSET_RV620:
	fprintf (stderr, "Chipset: RV620\n\n");
	break;
    case CHIPSET_RV630:
	fprintf (stderr, "Chipset: RV630\n\n");
	break;
    case CHIPSET_RV635:
	fprintf (stderr, "Chipset: RV635\n\n");
	break;
    case CHIPSET_RV670:
	fprintf (stderr, "Chipset: RV670\n\n");
	break;
    case CHIPSET_RS780:
	fprintf (stderr, "Chipset: RS780\n\n");
	break;
    case CHIPSET_M72:
	fprintf (stderr, "Chipset: M72\n\n");
	break;
    case CHIPSET_M74:
	fprintf (stderr, "Chipset: M74\n\n");
	break;
    case CHIPSET_M82:
	fprintf (stderr, "Chipset: M82\n\n");
	break;
    case CHIPSET_RV770:
	fprintf (stderr, "Chipset: RV770\n\n");
	break;
    case CHIPSET_RV710:
	fprintf (stderr, "Chipset: RV710\n\n");
	break;
    case CHIPSET_RV730:
	fprintf (stderr, "Chipset: RV730\n\n");
	break;
    default:
	fprintf (stderr, "Chipset: untested, #%d\n\n", adapter.chipset);
    }

    display_width  = reg_read32 (D1GRPH_X_END) - reg_read32 (D1GRPH_X_START);
    display_pitch  = reg_read32 (D1GRPH_PITCH);
    display_height = reg_read32 (D1GRPH_Y_END);
    display_gpu    = reg_read32 (D1GRPH_PRIMARY_SURFACE_ADDRESS);
    if (verbose >= 2) {
	fprintf(stderr,"display_width=%d\n", display_width);
	fprintf(stderr,"display_pitch=%d\n", display_pitch);
	fprintf(stderr,"display gpu: 0x%08x\n", display_gpu);
    }

    if (adapter.chipset < CHIPSET_RV770)
	adapter.framebuffer_gpu = ((uint64_t) (reg_read32 (R6XX_MC_VM_FB_LOCATION) & 0xffff)) << 24;
    else
	adapter.framebuffer_gpu = ((uint64_t) (reg_read32 (R7XX_MC_VM_FB_LOCATION) & 0xffff)) << 24;
}

void usage (char *argv[]) {
    printf ("\nUsage: %s [opts] <test(s)> [<reg>=<val>] [<reg>-[-<reg>]] [...]\n"
	    "Options:\n"
	    "-v\tVerbose (multiple to increase)\n"
	    "-r\tDirect ring programming\n"
	    "-f[0-4]\tVertex format 0: float\n"
	    "\t              1: int32 (FMT_32_32_FLOAT)  2: int16 (FMT_16_16_FLOAT)\n"
	    "\t              3: int32 (FMT_32_32)        4: int16 (FMT_16_16)\n"
	    "-i[0-2]\tLoad vertices 0: _SCALED by fetch engine\n"
	    "\t              1: _INT  (& scale by ALU if vertex format != float)\n"
	    "\t              2: _NORM (& scale by ALU if vertex format != float)\n"
	    "-u\tVertex format unsigned\n"
	    "-S[0-2]\tVertex alu scale override 0: default  1: alu scale  2: no alu scale\n"
	    "-n\tDon't flush command buffer on tests, just print (implies -v)\n"
	    "-p pci:<domain>:<bus>:<dev>.<func>\tManually specify PCI Id (e.g. pci:0000:02:00.1)\n"
	    "\n"
	    "Test is composed of:\n"
	    "\n"
	    ".\tAdditional CP test\n"
	    "p\tExtensive CP tests\n"
	    "\n"
	    "r\t'r'eset GPU + CP\n"
	    "-\tSleep for 100 millisecond\n"
	    "/\tWait for engine idle\n"
	    "c\tCPU based clear screen\n"
	    "\n"
	    "t\tedited tri test 2d (minimal)\n"
	    "T\tedited tri test 3d (clipping, modelview)\n"
	    "q\ttextured quad test (scaled)\n"
	    "\n"
	    "w\twin tri test\n"
	    "W\twin clear test\n"
	    "b\twin blit test\n"
	    "\n"
	    "x\ttemporary test (various)\n"
	    "\n"
	    "e\tEXA solid test\n"
	    "E\tEXA copy test\n"
	    "\n"
	    "P\tPerformance test\n"
	    "F\tContinous Full-Power test\n"
	    "\n"
	    "[reg]s are dumped (also ranges) or written to, register addresses in hex\n"
	    "\n",
	    argv[0]);
    exit (1);
}

int main(int argc, char *argv[])
{
    int i, r;
    char *pciid = NULL;

    fprintf (stderr, "\n*** %s, version %s\n\n", argv[0], VERSION);
    while ((i = getopt(argc, argv, "vrf:i:uS:np:")) != -1) {
	switch (i) {
	case 'v':
	    verbose++;
	    break;
	case 'r':
	    use_ring_directly = 1;
	    break;
	case 'f':
	    vertex_format = atoi (optarg);
	    if (vertex_format < 0 || vertex_format > 4)
		usage(argv);
	    break;
	case 'i':
	    vertex_load_as_int = atoi (optarg);
	    if (vertex_load_as_int < 0 || vertex_load_as_int > 2)
		usage(argv);
	    break;
	case 'u':
	    vertex_unsigned = 1;
	    break;
	case 'S':
	    vertex_alu_scale_override = atoi (optarg);
	    if (vertex_alu_scale_override < 0 || vertex_alu_scale_override > 2)
		usage(argv);
	    break;
	case 'n':
	    verbose++;
	    do_not_flush = 1;
	    break;
	case 'p':
	    pciid = optarg;
	    break;
	default:
	    usage (argv);
	}
    }
    
    if (optind >= argc)
	usage (argv);

    if (pciid)
	drmFD=drmOpen(NULL, pciid);
    else
	drmFD=drmOpen("radeon", NULL);
    if(drmFD<0){
	drmError(drmFD, __func__ );
	fprintf(stderr, "Check that BusId is correct. You can find the correct BusId in /var/log/Xorg.0.log\n");
	fprintf(stderr, "You can also try setting the environment variable LIBGL_DEBUG to \"verbose\" to see what libdrm is trying to do.\n");
	exit(-1);
    }
    if (verbose >= 1)
	print_versions();
    if((r=drmCreateContext(drmFD, &context))){
    	drmError(r, __func__);
	fprintf(stderr, "Could not create context, aborting\n");	
    	exit(-1);
    }
    if (verbose >= 2)
	fprintf(stderr,"Context %d\n",context);

    GetMaps();
    GetBufs();

    /* setup */

    srandom (time (NULL));
    read_registers();
    reg_write32 (SCRATCH_REG7, 0);

    adapter.framebuffer    = framebuffer;
    adapter.framebuffer_size = framebuffer_size;
    if (verbose >= 2)
	fprintf(stderr, "framebuffer cpu %p, gpu 0x" PRINTF_UINT64_HEX "\n",
		framebuffer, adapter.framebuffer_gpu);
    adapter.display_gpu    = display_gpu;
    adapter.display        = framebuffer + display_gpu - adapter.framebuffer_gpu;
    adapter.display_pitch  = display_pitch;
    adapter.display_width  = display_width;
    adapter.display_height = display_height;
    
    adapter.color_gpu      = display_gpu;
    adapter.color_pitch    = display_pitch;
    adapter.color_height   = display_height;

    adapter.depth_gpu      = display_gpu + display_pitch*4*500;
    adapter.depth_pitch    = display_pitch;
    adapter.depth_height   = 480;

    if (verbose >= 1) {
	fprintf (stderr, "\nfb:       gpu 0x" PRINTF_UINT64_HEX ", cpu %p, size 0x" PRINTF_UINT64_HEX "\n",
		 adapter.framebuffer_gpu, adapter.framebuffer, adapter.framebuffer_size);
	fprintf (stderr, "display:  gpu 0x" PRINTF_UINT64_HEX ", cpu %p  (%dx%d) @%d\n",
		 adapter.display_gpu, adapter.display, adapter.display_width, adapter.display_height, adapter.display_pitch);
	fprintf (stderr, "color RT: gpu 0x" PRINTF_UINT64_HEX "  (%dx%d)\n",
		 adapter.color_gpu, adapter.color_pitch, adapter.color_height);
	fprintf (stderr, "depth:    gpu 0x" PRINTF_UINT64_HEX "  (%dx%d)\n",
		 adapter.depth_gpu, adapter.depth_pitch, adapter.depth_height);
	fprintf (stderr, "gart:     gpu 0x" PRINTF_UINT64_HEX ", cpu %p\n",
		 gart_gpu, gart);
	fprintf (stderr, "gart bufs:gpu 0x" PRINTF_UINT64_HEX ", cpu %p\n",
		 gartbuf_gpu, gartbuf);
	fprintf (stderr, "buf vtx:  gpu 0x" PRINTF_UINT64_HEX ", cpu %p\n",
		 vtx_gpu, vtx);
	fprintf (stderr, "buf tex:  gpu 0x" PRINTF_UINT64_HEX ", cpu %p\n",
		 tex_gpu, tex);
	fprintf (stderr, "ring:     gpu 0x" PRINTF_UINT64_HEX ", cpu %p\n",
		 ring_gpu, ring);
    }
    if (verbose >= 1)
	fprintf (stderr, "garttex:  gpu 0x" PRINTF_UINT64_HEX ", cpu %p\n", garttex_gpu, garttex);

    if (verbose >= 1)
	show_state (&adapter);

    if (verbose >= 1)
	fprintf(stderr,"\nTests:\n\n");
    system("sync");

    /* play around with indirect buffer here */

    switch (argv[optind][0]) {
    case 'r':
    case '\0':
	break;
    default:
	/* CP test only if not resetting or dumping */
	if (! do_not_flush) {
	    test_cp    (&adapter);
	    flush_cmds ();
	}
    }

    for (i=0; argv[optind][i]; i++) {
	switch (argv[optind][i]) {
	case '.':
	    test_cp (&adapter);
	    break;
	case 'p':
	    test_packets (&adapter);
	    break;
	case 'r':
	    soft_reset (&adapter);
	    break;
	case '-':
	    usleep (100000);
	    break;
	case '/':
	    time_flush_cmds (&adapter, 5.0);
	    break;
	case 'c':
	    simple_clear (&adapter);
	    break;
	case 't':
	    tri_test_2d (&adapter);
	    break;
	case 'T':
	    tri_test_3d (&adapter);
	    break;
	case 'q':
	    quad_test_tex_scaled (&adapter);
	    break;
	case 'w':
	    pm4play_tri_test (&adapter);
	    break;
	case 'W':
	    pm4play_clear_test (&adapter);
	    break;
	case 'b':
	    pm4play_blit_test (&adapter);
	    break;
	case 'x':
	    tmp_test (&adapter);
	    break;
	case 'e':
	    test_solid (&adapter);
	    break;
	case 'E':
	    test_copy (&adapter);
	    break;
	case 'P':
	    test_perf (&adapter, 1, 1);
	    break;
	case 'F':
	    test_perf (&adapter, 0, 2);
	    break;
	default:
	    fprintf (stderr, "***** Don't know '%c' test\n\n", argv[optind][i]);
	    exit (1);
	}
	flush_cmds ();
	if (strcmp (argv[optind], "r") != 0 && ! do_not_flush) {
	    /* Verify that CP is not locked up */
	    test_cp    (&adapter);
	    flush_cmds ();
	}
    }

    if (argv[optind][0] != '\0')
	usleep (100000);
    
    /* If registers are to be dumped or written to, do it */
    if (argv[optind+1]) {
	printf ("\n");
	for (i = optind+1; argv[i]; i++) {
	    unsigned int start, end=0, val;
	    if (sscanf (argv[i], "%x=%x", &start, &val) == 2)
		write_register (&adapter, start, val);	/* reread after writing. */
	    if (sscanf (argv[i], "%x-%x", &start, &end)) {
		do {
		    dump_register (&adapter, start);
		    start += 4;
		} while (start <= end);
	    }
	}
    }
    
    show_state (&adapter);

    /* free stuff */

    drmFreeBufs(drmFD,dma.granted_count,indices);

    drmClose(drmFD);
    return 0;
}
