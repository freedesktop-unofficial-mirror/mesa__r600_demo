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
 * Basic tests
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "r600_reg.h"
#include "r600_emit.h"
#include "r600_lib.h"


/* Test for working CP */
void test_cp (adapter_t *adapt)
{
    printf      ("\n* CP test: PACK0 scratch reg\n\n");

    /* Test for CP working by simple register writes (CP_PACKET0) */
    reg_write32 (SCRATCH_REG7, 0xdeadbeef);
    wait_reg    (adapt, SCRATCH_REG7, 0xdeadbeef, "PACK0: init");
    
    pack0       (SCRATCH_REG7, 1);
    e32         (0xcafebabe);
    flush_cmds  ();
    wait_reg    (adapt, SCRATCH_REG7, 0xcafebabe, "CP: scratch");
}


/* Test CP + DMA */
void test_packets (adapter_t *adapt)
{
    printf      ("\n* PACK0 scratch reg test\n\n");

    /* Test for CP working by simple register writes (CP_PACKET0) */
    reg_write32 (SCRATCH_REG7, 0xdeadbeef);
    wait_reg    (adapt, SCRATCH_REG7, 0xdeadbeef, "PACK0: init");

    pack0       (SCRATCH_REG7, 1);
    e32         (0xcafebabe);
    flush_cmds  ();
    wait_reg    (adapt, SCRATCH_REG7, 0xcafebabe, "PACK0: scratch");
    pack0       (SCRATCH_REG7, 1);
    e32         (0xfeedface);
    flush_cmds  ();
    wait_reg    (adapt, SCRATCH_REG7, 0xfeedface, "PACK0: scratch2");

}

void simple_clear (adapter_t* adapt)
{
    uint32_t *dst = adapt->display;
    int i, j;
    uint32_t col = 0x00008000;

    for (i = 0; i < adapt->display_height; i++) {
	for (j = 0; j < adapt->display_pitch; j++) {
	    *dst++ = col;
	    if (j % 32 == 31)
		col ^= 0x00008080;
	}
	if (i % 32 == 31)
	    col ^= 0x00008080;
    }
}

