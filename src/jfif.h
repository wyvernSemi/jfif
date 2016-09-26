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
// $Id: jfif.h,v 1.1 2014-03-01 15:51:37 simon Exp $
// $Source: /home/simon/CVS/src/HDL/jfif/sw/jpeg_cpp/src/jfif.h,v $
//
//=============================================================
//
// The code implements decoding of JFIF/JPEG data based on the
// following standards (links supplied)
//
// JPEG Standard (JPEG ISO/IEC 10918-1 ITU-T Recommendation T.81): 
//     http://www.w3.org/Graphics/JPEG/itu-t81.pdf
//
// JPEG File Interchange Format version 1.02
//     http://www.w3.org/Graphics/JPEG/jfif3.pdf
//
// This file provides the definitions for external code to link
// to the main JFIF/JPEG decode routine jpeg_process_jfif().
//
//=============================================================

#ifndef _JFIF_H_
#define _JFIF_H_

//-------------------------------------------------------------
// General definitions

#define JPEG_NO_ERROR                0
#define JPEG_FILE_ERROR              1
#define JPEG_USER_INPUT_ERROR        2
#define JPEG_FORMAT_ERROR            3
#define JPEG_MEMORY_ERROR            4
#define JPEG_UNSUPPORTED_ERROR       5

#ifndef __cplusplus
#define true                         (1==1)
#define false                        (1==0)
#define bool                         int
#endif

//-------------------------------------------------------------
// Define the basic types

typedef unsigned char                uint8;
typedef unsigned short               uint16;
typedef unsigned int                 uint32;
typedef unsigned long long           uint64;

//-------------------------------------------------------------
// Exported function prototype(s) (see function main comments 
// for detailed description)

// Takes a byte buffer (ibuf) containing a JFIF/JPEG image, and 
// updates a pointer (obuf) to point to a 24 bit window bitmap. 
// Return value is one of the six values defined above. If other
// than JPEG_NO_ERROR, the obuf pointer is undefined.

#ifdef __cplusplus
extern "C" int jpeg_process_jfif_c (uint8 *ibuf, uint8 **obuf, int debug_enable);
#else
extern     int jpeg_process_jfif_c (uint8 *ibuf, uint8 **obuf, int debug_enable);
#endif

#endif
