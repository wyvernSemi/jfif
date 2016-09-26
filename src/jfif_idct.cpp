//=============================================================
// 
// Copyright (c) 2010-2014 Simon Southwell
// All rights reserved.
//
// Date: 18th January 2010
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
// $Id: jfif_idct.cpp,v 1.3 2014-03-14 16:05:14 simon Exp $
// $Source: /home/simon/CVS/src/HDL/jfif/sw/jpeg_cpp/src/jfif_idct.cpp,v $
//
//=============================================================

#include <iostream>
#include <iomanip>

#include "jfif_idct.h"

const jpeg_dct_t jfif_idct::C[JPEG_BLOCK_DIMENSION][JPEG_BLOCK_DIMENSION] = JPEG_DCT_C_INIT;

//-------------------------------------------------------------
// jpeg_idct_1d()
//
// Description:
//
// One dimensional iDCT for use in jpeg_idct_ifast(). This
// code is "pipelined" for ease of implemenation in RTL. Each
// PHASE represents 1 cycle, with no more than two sequential
// additions, or a single multiplication depth in any PHASE.
// RTL will still need to align data used in a phase from an
// earlier phase, and provide flow control to hide the latency
// through the pipeline.
// 
// Parameters:
//      data0-data7:  pointers to 8 ints for transformation
//
// Return value:
//    None.
//

void jfif_idct::jpeg_idct_1d(int *data0, int *data1, int *data2, int *data3, 
                             int *data4, int *data5, int *data6, int *data7)

{
    using std::cout;
    using std::hex;
    using std::setfill;
    using std::setw;
    using std::endl;

    int phase1[DCTSIZE], phase2[DCTSIZE], phase3[DCTSIZE], phase4[DCTSIZE], phase5[DCTSIZE];
    int tmp11_minus_tmp13, tmp11_plus_tmp13, tmp11_plus_tmp7;

#ifdef JPEG_DEBUG_MODE
    if (debug_enable & JPEG_DEBUG_IDCT_EN_9) {
        cout << "jpeg_idct_1d: IN :" << setfill('0');
        cout << " data0=" << hex << setw(4) << (int)((*data0) & 0xffff);
        cout << " data1=" << hex << setw(4) << (int)((*data1) & 0xffff);
        cout << " data2=" << hex << setw(4) << (int)((*data2) & 0xffff);
        cout << " data3=" << hex << setw(4) << (int)((*data3) & 0xffff);
        cout << " data4=" << hex << setw(4) << (int)((*data4) & 0xffff);
        cout << " data5=" << hex << setw(4) << (int)((*data5) & 0xffff);
        cout << " data6=" << hex << setw(4) << (int)((*data6) & 0xffff);
        cout << " data7=" << hex << setw(4) << (int)((*data7) & 0xffff) << endl;
    }
#endif

// PHASE 1

    // Even part
    phase1[0] = *data0 + *data4;        // tmp10
    phase1[1] = *data0 - *data4;        // tmp11
    phase1[2] = *data2 + *data6;        // tmp13
    phase1[3] = *data2 - *data6;        // tmp1 - tmp3

    // Odd part
    phase1[4] = *data5 + *data3;        // z13 = tmp6 + tmp5
    phase1[5] = *data5 - *data3;        // z10 = tmp6 - tmp5
    phase1[6] = *data1 + *data7;        // z11 = tmp4 + tmp7
    phase1[7] = *data1 - *data7;        // z12 = tmp4 - tmp7

#ifdef JPEG_DEBUG_MODE
    if (debug_enable & JPEG_DEBUG_IDCT_EN_3) {
        cout << "jpeg_idct_1d: P1 :" << setfill('0');
        cout << " " << hex << setw(4) << (phase1[0] & 0xffff);
        cout << " " << hex << setw(4) << (phase1[1] & 0xffff);
        cout << " " << hex << setw(4) << (phase1[2] & 0xffff);
        cout << " " << hex << setw(4) << (phase1[3] & 0xffff);
        cout << " " << hex << setw(4) << (phase1[4] & 0xffff);
        cout << " " << hex << setw(4) << (phase1[5] & 0xffff);
        cout << " " << hex << setw(4) << (phase1[6] & 0xffff);
        cout << " " << hex << setw(4) << (phase1[7] & 0xffff) << endl;
    }

#endif

// PHASE 2

    // Even part
    phase2[3] = jpeg_idct_multiply(phase1[3], FIX_1_414213562);   // tmp12' = (tmp1 - tmp3) *  FIX_1_414213562 [- tmp13]

    // Odd part
    phase2[4] = phase1[6] + phase1[4];  // tmp7
    phase2[5] = phase1[6] - phase1[4];  // z11 - z13
    phase2[6] = phase1[5] + phase1[7];  // z10 + z12

    // Calculate in this phase, to alleviate chained additions in phase 3
    tmp11_minus_tmp13 = phase1[1] - phase1[2];
    tmp11_plus_tmp13  = phase1[1] + phase1[2];

#ifdef JPEG_DEBUG_MODE
    if (debug_enable & JPEG_DEBUG_IDCT_EN_4) {
        cout << "jpeg_idct_1d: P2 : xxxx xxxx xxxx" << setfill ('0');
        cout << " " << hex << setw(4) << (phase2[3] & 0xffff);
        cout << " " << hex << setw(4) << (phase2[4] & 0xffff);
        cout << " " << hex << setw(4) << (phase2[5] & 0xffff);
        cout << " " << hex << setw(4) << (phase2[6] & 0xffff);
        cout << " xxxx" << endl;
    }
#endif

// PHASE 3

    // Even part
    phase3[0] = phase1[0]         + phase1[2];          // tmp0 = tmp10 + tmp13
    phase3[1] = tmp11_minus_tmp13 + phase2[3];          // tmp1 = tmp11 + tmp12 => tmp11 - tmp13 + tmp12'
    phase3[2] = tmp11_plus_tmp13  - phase2[3];          // tmp2 = tmp11 - tmp12 => tmp11 + tmp13 - tmp12'
    phase3[3] = phase1[0]         - phase1[2];          // tmp3 = tmp10 - tmp13

    // Odd part
    phase3[4] = jpeg_idct_multiply(phase2[5], FIX_1_414213562);           // tmp11 = (z11 - z13) * `FIX_1_414213562
    phase3[5] = jpeg_idct_multiply(phase2[6], FIX_1_847759065);           // z5    = (z10 + z12) * `FIX_1_847759065
    phase3[6] = jpeg_idct_multiply(phase1[7], FIX_1_082392200);           // tmp10' = (z12 * `FIX_1_082392200)     [- z5]
    phase3[7] = jpeg_idct_multiply(phase1[5], FIX_NEG_2_613125930);       // tmp12' = (z10 * `FIX_NEG_2_613125930) [+ z5]

#ifdef JPEG_DEBUG_MODE
    if (debug_enable & JPEG_DEBUG_IDCT_EN_10) {
        cout << "jpeg_idct_1d: P3 :" << setfill ('0');
        cout << " " << hex << setw(4) << (phase3[0] & 0xffff);
        cout << " " << hex << setw(4) << (phase3[1] & 0xffff);
        cout << " " << hex << setw(4) << (phase3[2] & 0xffff);
        cout << " " << hex << setw(4) << (phase3[3] & 0xffff);
        cout << " " << hex << setw(4) << (phase3[4] & 0xffff);
        cout << " " << hex << setw(4) << (phase3[5] & 0xffff);
        cout << " " << hex << setw(4) << (phase3[6] & 0xffff);
        cout << " " << hex << setw(4) << (phase3[7] & 0xffff) << endl;
    }
#endif

// PHASE 4

    // Odd part
    phase4[6] = phase3[6] - phase3[5];                  // tmp10 = tmp10' - z5
    phase4[7] = phase3[7] + phase3[5];                  // tmp12 = tmp12' + z5

    // Calculate in this phase, to alleviate chained addition in phase 5
    tmp11_plus_tmp7 = phase3[4] + phase2[4];        

#ifdef JPEG_DEBUG_MODE
    if (debug_enable & JPEG_DEBUG_IDCT_EN_5) {
        cout << "jpeg_idct_1d: P4 : xxxx xxxx xxxx xxxx xxxx xxxx" << setfill ('0');
        cout << " " << hex << setw(4) << (phase4[6] & 0xffff);
        cout << " " << hex << setw(4) << (phase4[7] & 0xffff) << endl;
    }
#endif

// PHASE 5

    // Odd part
    phase5[6] = phase4[7]       - phase2[4];            // tmp6 = tmp12 - tmp7
    phase5[5] = tmp11_plus_tmp7 - phase4[7];            // tmp5 = tmp11 - tmp6 ==> tmp11 + tmp7 - tmp12

    // *NOTE* chained addition
    phase5[4] = phase4[6]       + phase5[5];            // tmp4 = tmp10 + tmp5

#ifdef JPEG_DEBUG_MODE
    if (debug_enable & JPEG_DEBUG_IDCT_EN_6) {
        cout << "jpeg_idct_1d: P5 : xxxx xxxx xxxx xxxx" << setfill ('0');
        cout << " " << hex << setw(4) << (phase5[4] & 0xffff);
        cout << " " << hex << setw(4) << (phase5[5] & 0xffff);
        cout << " " << hex << setw(4) << (phase5[6] & 0xffff);
        cout << " xxxx" << endl;
    }
#endif
    
// PHASE 6

    *data0 = phase3[0] + phase2[4];                     // *data0 = tmp0 + tmp7
    *data1 = phase3[1] + phase5[6];                     // *data1 = tmp1 + tmp6
    *data2 = phase3[2] + phase5[5];                     // *data2 = tmp2 + tmp5
    *data3 = phase3[3] - phase5[4];                     // *data3 = tmp3 - tmp4
    *data4 = phase3[3] + phase5[4];                     // *data4 = tmp3 + tmp4
    *data5 = phase3[2] - phase5[5];                     // *data5 = tmp2 - tmp5
    *data6 = phase3[1] - phase5[6];                     // *data6 = tmp1 - tmp6
    *data7 = phase3[0] - phase2[4];                     // *data7 = tmp0 - tmp7

#ifdef JPEG_DEBUG_MODE
    if (debug_enable & JPEG_DEBUG_IDCT_EN_7) {
        cout << "jpeg_idct_1d: OUT :" << setfill ('0');
        cout << " " << hex << setw(4) << (*data0 & 0xffff);
        cout << " " << hex << setw(4) << (*data1 & 0xffff);
        cout << " " << hex << setw(4) << (*data2 & 0xffff);
        cout << " " << hex << setw(4) << (*data3 & 0xffff);
        cout << " " << hex << setw(4) << (*data4 & 0xffff);
        cout << " " << hex << setw(4) << (*data5 & 0xffff);
        cout << " " << hex << setw(4) << (*data6 & 0xffff);
        cout << " " << hex << setw(4) << (*data7 & 0xffff) << endl;
    }
#endif
}

//-------------------------------------------------------------
// jpeg_idct()
//
// Description:
//
// Perform inverse DCT on one block of coefficients. Based on
// fast algorithm in the independent JPEG group's library,
// after Arai, Agui and Nakajima (Trans. IEICE E-71(11):1095),
// but pipelined for RTL implementation. See jpeg_idct_ifast2()
// below for pre-pipelined code.
// 
// Parameters:
//      data:  pointer to 8x8 block of ints for transformation
//
// Return value:
//    None.
//


void jfif_idct::jpeg_idct (jpeg_8x8_block_t data) {

    using std::cout;
    using std::hex;
    using std::setfill;
    using std::setw;
    using std::endl;

    int row, col;

    // Eight 1d iDCTs across row
    for (row = 0; row < DCTSIZE; row++) {

#ifdef JPEG_DEBUG_MODE
        if (debug_enable & JPEG_DEBUG_IDCT_EN_11) {
            cout << "jpeg_idct_ifast: IN :" << setfill ('0');
            cout << " " << hex << setw(4) << (data[row][0] & 0xffff);
            cout << " " << hex << setw(4) << (data[row][1] & 0xffff);
            cout << " " << hex << setw(4) << (data[row][2] & 0xffff);
            cout << " " << hex << setw(4) << (data[row][3] & 0xffff);
            cout << " " << hex << setw(4) << (data[row][4] & 0xffff);
            cout << " " << hex << setw(4) << (data[row][5] & 0xffff);
            cout << " " << hex << setw(4) << (data[row][6] & 0xffff);
            cout << " " << hex << setw(4) << (data[row][7] & 0xffff) << endl;
        }
#endif

        // iDCT on row
                
        jpeg_idct_1d(&data[row][0], &data[row][1], &data[row][2], &data[row][3], 
                     &data[row][4], &data[row][5], &data[row][6], &data[row][7]);
    

#ifdef JPEG_DEBUG_MODE
        if (debug_enable & JPEG_DEBUG_IDCT_EN_12) {
            cout << "jpeg_idct_ifast: MID :" << setfill ('0');
            cout << " " << hex << setw(4) << (data[row][0] & 0xffff);
            cout << " " << hex << setw(4) << (data[row][1] & 0xffff);
            cout << " " << hex << setw(4) << (data[row][2] & 0xffff);
            cout << " " << hex << setw(4) << (data[row][3] & 0xffff);
            cout << " " << hex << setw(4) << (data[row][4] & 0xffff);
            cout << " " << hex << setw(4) << (data[row][5] & 0xffff);
            cout << " " << hex << setw(4) << (data[row][6] & 0xffff);
            cout << " " << hex << setw(4) << (data[row][7] & 0xffff) << endl;
        }
#endif

    }
  

    // Eight 1d iDCTs across columns
    for (col = 0; col < DCTSIZE; col++) {
    
#ifdef JPEG_DEBUG_MODE
        if (debug_enable & JPEG_DEBUG_IDCT_EN_13) {
            cout << "jpeg_idct_ifast: ROT :" << setfill ('0');
            cout << " " << hex << setw(4) << (data[0][col] & 0xffff);
            cout << " " << hex << setw(4) << (data[1][col] & 0xffff);
            cout << " " << hex << setw(4) << (data[2][col] & 0xffff);
            cout << " " << hex << setw(4) << (data[3][col] & 0xffff);
            cout << " " << hex << setw(4) << (data[4][col] & 0xffff);
            cout << " " << hex << setw(4) << (data[5][col] & 0xffff);
            cout << " " << hex << setw(4) << (data[6][col] & 0xffff);
            cout << " " << hex << setw(4) << (data[7][col] & 0xffff) << endl;
        }
#endif

        // iDCT on columns
    
        jpeg_idct_1d(&data[0][col], &data[1][col], &data[2][col], &data[3][col],
                     &data[4][col], &data[5][col], &data[6][col], &data[7][col]);
                
#ifdef JPEG_DEBUG_MODE
        if (debug_enable & JPEG_DEBUG_IDCT_EN_14) {
            cout << "jpeg_idct_ifast: OUT :" << setfill ('0');
            cout << " " << hex << setw(4) << (data[0][col] & 0xffff);
            cout << " " << hex << setw(4) << (data[1][col] & 0xffff);
            cout << " " << hex << setw(4) << (data[2][col] & 0xffff);
            cout << " " << hex << setw(4) << (data[3][col] & 0xffff);
            cout << " " << hex << setw(4) << (data[4][col] & 0xffff);
            cout << " " << hex << setw(4) << (data[5][col] & 0xffff);
            cout << " " << hex << setw(4) << (data[6][col] & 0xffff);
            cout << " " << hex << setw(4) << (data[7][col] & 0xffff) << endl;
        }
#endif

#ifdef JPEG_DEBUG_MODE
        if (debug_enable & JPEG_DEBUG_IDCT_EN_2) {
            cout << setfill ('0');
            cout << hex << setw(4) << (data[7][col] & 0xffff);
            cout << hex << setw(4) << (data[6][col] & 0xffff);
            cout << hex << setw(4) << (data[5][col] & 0xffff);
            cout << hex << setw(4) << (data[4][col] & 0xffff);
            cout << hex << setw(4) << (data[3][col] & 0xffff);
            cout << hex << setw(4) << (data[2][col] & 0xffff);
            cout << hex << setw(4) << (data[1][col] & 0xffff);
            cout << hex << setw(4) << (data[0][col] & 0xffff) << endl;
        }
#endif
        // Final output stage: scale down by a factor of 8 and range-limit 
        data[0][col] = JPEG_CLIP(128+jpeg_idescale(data[0][col], FINAL_SCALE_BITS));
        data[1][col] = JPEG_CLIP(128+jpeg_idescale(data[1][col], FINAL_SCALE_BITS));
        data[2][col] = JPEG_CLIP(128+jpeg_idescale(data[2][col], FINAL_SCALE_BITS));
        data[3][col] = JPEG_CLIP(128+jpeg_idescale(data[3][col], FINAL_SCALE_BITS));
        data[4][col] = JPEG_CLIP(128+jpeg_idescale(data[4][col], FINAL_SCALE_BITS));
        data[5][col] = JPEG_CLIP(128+jpeg_idescale(data[5][col], FINAL_SCALE_BITS));
        data[6][col] = JPEG_CLIP(128+jpeg_idescale(data[6][col], FINAL_SCALE_BITS));
        data[7][col] = JPEG_CLIP(128+jpeg_idescale(data[7][col], FINAL_SCALE_BITS));


#ifdef JPEG_DEBUG_MODE
        if (debug_enable & JPEG_DEBUG_IDCT_EN) {
            cout << "iDCT out: " << setfill ('0');
            cout << hex << setw(2) << (data[7][col] & 0xff);
            cout << hex << setw(2) << (data[6][col] & 0xff);
            cout << hex << setw(2) << (data[5][col] & 0xff);
            cout << hex << setw(2) << (data[4][col] & 0xff);
            cout << hex << setw(2) << (data[3][col] & 0xff);
            cout << hex << setw(2) << (data[2][col] & 0xff);
            cout << hex << setw(2) << (data[1][col] & 0xff);
            cout << hex << setw(2) << (data[0][col] & 0xff) << endl;
        }
#endif
    }

}

//-------------------------------------------------------------
// jpeg_idct_slow()
//
// Description:
// 
// Inverse discrete cosine transform for an 8x8 block.
// Adapted from "The Data Compression Book", 2nd ed., Nelson et al., 1995
//
// Parameters:
//      data:  pointer to 8x8 block of ints for transformation
//
// Return value:
//    None.
//

void jfif_idct::jpeg_idct_slow(jpeg_8x8_block_t data)
{

#ifndef JPEG_FAST_INT_IDCT

    jpeg_dct_t temp[JPEG_BLOCK_DIMENSION][JPEG_BLOCK_DIMENSION];
    jpeg_dct_t temp1;

    int idx, jdx, kdx;

    // temp = C * data
    for (idx = 0; idx < JPEG_BLOCK_DIMENSION; idx++) {

        for (jdx = 0; jdx < JPEG_BLOCK_DIMENSION; jdx++ ) {

            temp[idx][jdx] = 0.0;

            for (kdx = 0; kdx < JPEG_BLOCK_DIMENSION; kdx++)
                // Save a multiply and add if possible. Many coeff should be 0.
                if (data[idx][kdx])
                    temp[idx][jdx] += data[idx][kdx] * C[kdx][jdx];

#ifdef JPEG_DCT_INTEGER
            // Renormalize scaled integer values
            temp[idx][jdx] = temp[idx][jdx] >> JPEG_DCT_INT_SCALE;
#endif
        }
    }

    // output = temp * Ct
    for (idx = 0 ; idx < JPEG_BLOCK_DIMENSION; idx++ ) {

        for (jdx = 0; jdx < JPEG_BLOCK_DIMENSION; jdx++) {

            temp1 = 0.0;

            for (kdx = 0; kdx < JPEG_BLOCK_DIMENSION; kdx++ )
                // Save a multiply and add if possible
                if (temp[kdx][jdx])
                    temp1 += C[kdx][idx] * temp[kdx][jdx];

#ifdef JPEG_DCT_INTEGER
            // Renormalize scaled integer values
            temp1 = (temp1 >> JPEG_DCT_INT_SCALE);
#endif

            // Renormalize to 0 to 255
            temp1 += 128.0;

            // Perform clipping and store in output buffer
            data[idx][jdx] = (uint8)JPEG_CLIP(temp1);
        }
    }

#endif
}

