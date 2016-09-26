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
// $Id: jpeg_dct_cos.h,v 1.1 2014-03-01 15:51:38 simon Exp $
// $Source: /home/simon/CVS/src/HDL/jfif/sw/jpeg_cpp/src/jpeg_dct_cos.h,v $
//
//=============================================================

#ifndef _JPEG_DCT_COS_
#define _JPEG_DCT_COS_

//-------------------------------------------------------------
// The following definitions are for the floating point/slow 
// integer based iDCT functions

#ifndef JPEG_DCT_INTEGER

#define POSNUM0  0.3535533905932737     /* 1/sqrt(8) */
#define POSNUM1  0.4903926402016152     /* 0.5 * cos (1/8 * 90 deg) */
#define POSNUM2  0.4619397662556434     /* 0.5 * cos (2/8 * 90 deg) */
#define POSNUM3  0.4157348061512726     /* 0.5 * cos (3/8 * 90 deg) */ 
#define POSNUM4  0.3535533905932738     /* 0.5 * cos (4/8 * 90 deg) */ 
#define POSNUM5  0.2777851165098011     /* 0.5 * cos (5/8 * 90 deg) */ 
#define POSNUM6  0.1913417161825449     /* 0.5 * cos (6/8 * 90 deg) */ 
#define POSNUM7  0.0975451610080642     /* 0.5 * cos (7/8 * 90 deg) */ 

#else

// Fixed point u0.9 bits (i.e. round (512 * num))
#define JPEG_DCT_INT_SCALE      9

#define POSNUM0                 181     
#define POSNUM1                 251
#define POSNUM2                 237
#define POSNUM3                 213
#define POSNUM4                 181
#define POSNUM5                 142
#define POSNUM6                  98
#define POSNUM7                  50

#endif

#define NEGNUM0 (-1 * POSNUM0)          /* -1/sqrt(8) */
#define NEGNUM1 (-1 * POSNUM1)          /* 0.5 * cos (7/8 * 90 deg + 90 deg) */
#define NEGNUM2 (-1 * POSNUM2)          /* 0.5 * cos (6/8 * 90 deg + 90 deg) */
#define NEGNUM3 (-1 * POSNUM3)          /* 0.5 * cos (5/8 * 90 deg + 90 deg) */
#define NEGNUM4 (-1 * POSNUM4)          /* 0.5 * cos (4/8 * 90 deg + 90 deg) */
#define NEGNUM5 (-1 * POSNUM5)          /* 0.5 * cos (3/8 * 90 deg + 90 deg) */
#define NEGNUM6 (-1 * POSNUM6)          /* 0.5 * cos (2/8 * 90 deg + 90 deg) */
#define NEGNUM7 (-1 * POSNUM7)          /* 0.5 * cos (1/8 * 90 deg + 90 deg) */

/*
   0   0   0   0   0   0   0    0
   1   3   5   7  -7  -5  -3   -1
   2   6  -6  -2  -2  -6   6    2
   3  -7  -1  -5   5   1   7   -3
   4  -4  -4   4   4  -4  -4    4
   5  -1   7   3  -3  -7   1   -5
   6  -2   2  -6  -6   2  -2    6
   7  -5   3  -1   1  -3   5   -7
*/


#define JPEG_DCT_C_INIT {\
    {POSNUM0, POSNUM0, POSNUM0, POSNUM0, POSNUM0, POSNUM0, POSNUM0, POSNUM0},\
    {POSNUM1, POSNUM3, POSNUM5, POSNUM7, NEGNUM7, NEGNUM5, NEGNUM3, NEGNUM1},\
    {POSNUM2, POSNUM6, NEGNUM6, NEGNUM2, NEGNUM2, NEGNUM6, POSNUM6, POSNUM2},\
    {POSNUM3, NEGNUM7, NEGNUM1, NEGNUM5, POSNUM5, POSNUM1, POSNUM7, NEGNUM3},\
    {POSNUM4, NEGNUM4, NEGNUM4, POSNUM4, POSNUM4, NEGNUM4, NEGNUM4, POSNUM4},\
    {POSNUM5, NEGNUM1, POSNUM7, POSNUM3, NEGNUM3, NEGNUM7, POSNUM1, NEGNUM5},\
    {POSNUM6, NEGNUM2, POSNUM2, NEGNUM6, NEGNUM6, POSNUM2, NEGNUM2, POSNUM6},\
    {POSNUM7, NEGNUM5, POSNUM3, NEGNUM1, POSNUM1, NEGNUM3, POSNUM5, NEGNUM7},\
}

//-------------------------------------------------------------
// The following definitions are for the fast integer based 
// iDCT functions

#define DCTSIZE                 8
#define DCTSIZE2                64

#define SCALE_BITS              14
#define PRE_DESCALE_BITS        6
#define FINAL_SCALE_BITS        3

#define CONST_BITS              8

#define FIX_1_082392200         ((int)  277)            /* FIX(1.082392200) */
#define FIX_1_414213562         ((int)  362)            /* FIX(1.414213562) */
#define FIX_1_847759065         ((int)  473)            /* FIX(1.847759065) */
#define FIX_2_613125930         ((int)  669)            /* FIX(2.613125930) */
#define FIX_NEG_2_613125930     ((int)  -669)           /* FIX(-2.613125930) */

#endif

