//=============================================================
//                                                               
// Copyright (c) 2010-2014 Simon Southwell
// All rights reserved.
//                                                               
// Date: 2nd February 2010
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
// $Id: bitmap.h,v 1.1 2014-03-01 15:51:37 simon Exp $
// $Source: /home/simon/CVS/src/HDL/jfif/sw/jpeg_cpp/src/bitmap.h,v $
//
//=============================================================

#ifdef _WIN32
#define __BYTE_ORDER __LITTLE_ENDIAN
#else
#include <endian.h>
#endif

#ifndef _BITMAP_H_
#define _BITMAP_H_

// General bitmap definitions
#define BMP_FORMATHDRSIZE    0x0E
#define BMP_INFOHDRSIZE      0x28
#define BMP_HDRSIZE          (BMP_FORMATHDRSIZE+BMP_INFOHDRSIZE)

// TransformBmp monochrome unary flags
#define BMP_MONOALL          0
#define BMP_MONORED          1
#define BMP_MONOGREEN        2
#define BMP_MONOBLUE         4

#define TBMP_ERR_BADPARAM    1
#define TBMP_ERR_CONVERROR   2

// GetBitmap Error codes
#define GBMP_ERR_MEM         1
#define GBMP_ERR_EOF         2
#define GBMP_ERR_NOTBMP      3
#define GBMP_ERR_BADPLANES   4
#define GBMP_ERR_BADPIXELS   5
#define GBMP_ERR_BADCOMPRESS 6

// ConvertBmpTo24bit error codes
#define CBMP_ERR_MEM         1
#define CBMP_ERR_CONVERROR   2

#define BMP_WIDTH_TO_PADDED_BYTES(_x) ((((_x)*3)/4 + (((_x)*3)%4 ? 1 : 0))*4)

#if __BYTE_ORDER == __LITTLE_ENDIAN

// No bitmap endian conversion needed for little endian architectures
#define BMP_SWPEND32(_x) (_x)
#define BMP_SWPEND16(_x) (_x)
#define BMP_HDRENDIAN

#else

// Endian conversion macros for big endian machines
#define BMP_SWPEND32(_x) ((((_x) & 0xff)<<24) | (((_x) & 0xff00)<<8) | (((_x) & 0xff0000)>>8) | (((_x) & 0xff000000)>>24))
#define BMP_SWPEND16(_x) ((((_x) & 0xff)<<8) | (((_x) & 0xff00)>>8))

// Endian conversion for header
#define BMP_HDRENDIAN(_hdr) { \
    (_hdr)->f.bfSize           = BMP_SWPEND32((_hdr)->f.bfSize);          \
    (_hdr)->f.bfOffBits        = BMP_SWPEND32((_hdr)->f.bfOffBits);       \
    (_hdr)->i.biSize           = BMP_SWPEND32((_hdr)->i.biSize);          \
    (_hdr)->i.biWidth          = BMP_SWPEND32((_hdr)->i.biWidth);         \
    (_hdr)->i.biHeight         = BMP_SWPEND32((_hdr)->i.biHeight);        \
    (_hdr)->i.biPlanes         = BMP_SWPEND16((_hdr)->i.biPlanes);        \
    (_hdr)->i.biBitCount       = BMP_SWPEND16((_hdr)->i.biBitCount);      \
    (_hdr)->i.biCompression    = BMP_SWPEND32((_hdr)->i.biCompression);   \
    (_hdr)->i.biSizeImage      = BMP_SWPEND32((_hdr)->i.biSizeImage);     \
    (_hdr)->i.biXPxlsPerMeter  = BMP_SWPEND32((_hdr)->i.biXPxlsPerMeter); \
    (_hdr)->i.biYPxlsPerMeter  = BMP_SWPEND32((_hdr)->i.biYPxlsPerMeter); \
    (_hdr)->i.biClrUsed        = BMP_SWPEND32((_hdr)->i.biClrUsed);       \
    (_hdr)->i.biClrImportant   = BMP_SWPEND32((_hdr)->i.biClrImportant);  \
 }
#endif

// Set packing of structures to 1 byte boundaries so that headers
// are aligned correctly
#ifdef _WIN32
#pragma pack (push, 1)                      
#else
#pragma pack (push)
#pragma pack (1)                      
#endif

// Bitmap format header structure
typedef struct {
    char     bfType[2];
    uint32   bfSize;
    char     bfRes[4];
    uint32   bfOffBits;

} bmfh_t, *pbmfh_t;

// Bitmap information table structure
typedef struct {
    uint32 biSize;
    uint32 biWidth;
    uint32 biHeight;
    uint16 biPlanes;
    uint16 biBitCount;
    uint32 biCompression;
    uint32 biSizeImage;
    uint32 biXPxlsPerMeter;
    uint32 biYPxlsPerMeter;
    uint32 biClrUsed;
    uint32 biClrImportant;
} bmih_t, *pbmih_t;

// Combined format header/information table header
typedef struct {
    bmfh_t f;
    bmih_t i;
} bmhdr_t, *pbmhdr_t;

// Bitmap RGB Quad structure
typedef struct {
    uint8 Blue;
    uint8 Green;
    uint8 Red;
    uint8 rgbReserved;
} rgbquad_t, *prgbquad_t;

// Revert to default packing
#pragma pack (pop)                      

#endif
