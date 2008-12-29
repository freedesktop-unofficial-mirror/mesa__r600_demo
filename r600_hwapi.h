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

#ifndef __R600_HWAPI_H__
#define __R600_HWAPI_H__

/*
 * This defines hardware api related stuff
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <endian.h>


#if BYTE_ORDER == BIG_ENDIAN
#  define SWAP16(A)  ((((unsigned short)(A) & 0xff00) >> 8) |	\
		      (((unsigned short)(A) & 0x00ff) << 8))
#  define SWAP32(A)  ((((CARD32)(A) & 0xff000000) >> 24) |	\
		      (((CARD32)(A) & 0x00ff0000) >> 8)  |	\
		      (((CARD32)(A) & 0x0000ff00) << 8)  |	\
		      (((CARD32)(A) & 0x000000ff) << 24))
#else
#  define SWAP16(A)  (A)
#  define SWAP32(A)  (A)
#endif


#define DEBUG(x) do { x } while (0)			/* Always, on r600_demo */

/*
 * Internal stuff needed for API
 */

extern volatile void *registers;
extern int indirect_end, indirect_size;
extern uint32_t *indirect;


void flush_cmds (void);


/*
 * Register access
 */
static inline uint32_t reg_read32 (uint32_t reg)
{
    uint32_t v;
    v = * (volatile uint32_t *) (registers+reg);
    return SWAP32 (v);
}

static inline void reg_write32 (uint32_t reg, uint32_t v)
{
    * (volatile uint32_t *) (registers+reg) = SWAP32 (v);
}


/*
 * Packet Macros + Routines
 */

/* Preamble. Allocate space for dwords uint32_t values. If not enough space
 * is available, bail out.
 * In a production driver this can even be a NOP. Real buffer allocation has
 * to be done beforehand. */
/* For r600demo this is trivial. */
#define CMD_BUFFER_PREAMBLE(dwords)		\
    checkCmdBufferSpace (dwords)


/* Preamble, without space checking. Use with care. */
/* For r600demo this is trivial. */
#define CMD_BUFFER_NOCHECK_PREAMBLE		\
    ((void) 0)


/* Check for additional space for dwords uint32_t values. If not enough space
 * is available, bail out. */
/* For r600demo this is trivial. */
#define CMD_BUFFER_ALLOC(dwords)		\
    checkCmdBufferSpace (dwords)


/* Space checker for debugging purposes. Bail out if not enough space. */
/* Always enabled in r600demo. */
#define CMD_BUFFER_DEBUG_AVAIL(dwords)		\
    checkCmdBufferSpace (dwords)


/* Emit a single uint32_t value */
#define E32(dword)				\
    do {					\
	CMD_BUFFER_DEBUG_AVAIL (1);		\
	indirect[indirect_end>>2] = (dword);	\
	indirect_end += 4;			\
    } while (0)


static inline void checkCmdBufferSpace (int dwords)
{
    if (indirect_end + 4*dwords >= indirect_size) {
	fprintf (stderr, "Insufficient indirect buffer size (have %d, need %d) - aborting\n",
		 dwords, (indirect_size-indirect_end)/4);
	exit (1);
    }
}


#endif
