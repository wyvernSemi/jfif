//=============================================================
// 
// Copyright (c) 2014 Simon Southwell
// All rights reserved.
//
// Date: 14th February 2014
//
// This file is part of JFIF.
//
// JFIF is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// JFIF is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with JFIF. If not, see <http://www.gnu.org/licenses/>.
//
// $Id: jfif_idct.h,v 1.1 2014-03-01 15:51:38 simon Exp $
// $Source: /home/simon/CVS/src/HDL/jfif/sw/jpeg_cpp/src/jfif_idct.h,v $
//
//=============================================================

#include "jfif_local.h"
#include "jpeg_dct_cos.h"

#ifndef _JFIF_IDCT_H_
#define _JFIF_IDCT_H_


// iDCT base class for use in JFIF/JPEG decoding
class jfif_idct {

protected:

    // Constructor
    jfif_idct(int debug_enable_in = 0) : debug_enable(debug_enable_in)
    {
    };

    // Pipelined fast integer iDCT implementation, reflecting h/w architecture
    void jpeg_idct (jpeg_8x8_block_t data);

    // Simple, but slow, iDCT
    void jpeg_idct_slow (jpeg_8x8_block_t data);

    // iDCT descale (truncate) an integer result
    inline int jpeg_idescale (int x, int n) {
        return x >> n;
    };

    // Multiply a int variable by an int constant, and immediately
    // descale to yield a int result.
    inline int jpeg_idct_multiply (int var1, int var2) {
        return (var1 * var2) >> CONST_BITS;
    };

    int debug_enable;

private:

    static const jpeg_dct_t C[JPEG_BLOCK_DIMENSION][JPEG_BLOCK_DIMENSION];

    void jpeg_idct_1d(int *data0, int *data1, int *data2, int *data3, 
                      int *data4, int *data5, int *data6, int *data7);

};

#endif
