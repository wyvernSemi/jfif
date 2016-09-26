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
// $Id: jfif_local.h,v 1.4 2014-03-14 16:04:33 simon Exp $
// $Source: /home/simon/CVS/src/HDL/jfif/sw/jpeg_cpp/src/jfif_local.h,v $
//
//=============================================================
//
// The code implements decoding of JFIF/JPEG data based on the
// following standards (links supplied)
//
// JPEG Standard (JPEG ISO/IEC 10918-1, ITU-T Recommendation T.81): 
//     http://www.w3.org/Graphics/JPEG/itu-t81.pdf
//
// JPEG File Interchange Format version 1.02
//     http://www.w3.org/Graphics/JPEG/jfif3.pdf
//
//=============================================================
//
// This header contains all the definitions used internally
// to the JFIF/JPEG decode routines.
//
// The following definitions alter the behaviour of the code,
// and can be defined in this file, or via the compile command
// line.
//
// JPEG_DEBUG_MODE:             Enables verbose debug output to 
//                              stdout during execution.
//
// JPEG_DCT_INTEGER:            Makes the iDCT one of two integer 
//                              based algorithms (see below). When 
//                              undefined, iDCT is floating point based.
//
// JPEG_FAST_INT_IDCT:          Compiles the fast (less accurate) integer 
//                              iDCT. JPEG_DCT_INTEGER is forced to be 
//                              defined if this is defined.
//
// JPEG_IGNORE_SOS_TAIL_ERRORS: Suppresses errors associated with 
//                              missing EOI markers. 
//
// JPEG_LIMITED_SUB_SAMPLING:   Turns off support for vertical 
//                              sub-sampling.
//
// JPEG_NO_WARNINGS:            Suppresses output of warnings
//
//=============================================================

#ifndef _JFIF_LOCAL_H_
#define _JFIF_LOCAL_H_

#ifdef _WIN32
#define __BYTE_ORDER __LITTLE_ENDIAN
#else
#include <endian.h>
#endif

#include "jfif.h"

// Undefine before including jpeg_dct_cos.h to have 
// integer based DCT (or define in makefile)
//#define JPEG_DCT_INTEGER

// Use fast integer iDCT. 
//#define JPEG_FAST_INT_IDCT

//#include "jpeg_dct_cos.h"

// Uncomment (or add to makefile) for compiling test code
//#define JPEG_TEST_MODE

// Uncomment (or add to makefile) for debug output
//#define JPEG_DEBUG_MODE

// Uncomment (or add to makefile) to skip checking of Ss, Se, Ah 
// and Al fields in SOS header, which aren't used as they should 
// be constant for baseline DCT
//#define JPEG_IGNORE_SOS_TAIL_ERRORS

// Uncomment (or add to makefile) if only 4:4:4 sampling to be supported
//#define JPEG_LIMITED_SUB_SAMPLING

// Uncomment (or add to makefile) to suppress warnings (not recommended)
//#define JPEG_NO_WARNINGS

// If JPEG_FAST_INT_IDCT defined, then ensure that JPEG_DCT_INTEGER 
// is also defined
#ifdef  JPEG_FAST_INT_IDCT
#ifndef JPEG_DCT_INTEGER
#define JPEG_DCT_INTEGER
#endif
#endif

#define JPEG_ZIGZAG_MAP   {         \
     0,  1,  5,  6, 14, 15, 27, 28, \
     2,  4,  7, 13, 16, 26, 29, 42, \
     3,  8, 12, 17, 25, 30, 41, 43, \
     9, 11, 18, 24, 31, 40, 44, 53, \
    10, 19, 23, 32, 39, 45, 52, 54, \
    20, 22, 33, 38, 46, 51, 55, 60, \
    21, 34, 37, 47, 50, 56, 59, 61, \
    35, 36, 48, 49, 57, 58, 62, 63  \
    }

#define JPEG_INV_ZIGZAG_MAP {       \
     0,  1,  8, 16,  9,  2,  3, 10, \
    17, 24, 32, 25, 18, 11,  4,  5, \
    12, 19, 26, 33, 40, 48, 41, 34, \
    27, 20, 13,  6,  7, 14, 21, 28, \
    35, 42, 49, 56, 57, 50, 43, 36, \
    29, 22, 15, 23, 30, 37, 44, 51, \
    58, 59, 52, 45, 38, 31, 39, 46, \
    53, 60, 61, 54, 47, 55, 62, 63  \
    }

#define JPEG_MAG_ADJUST_POS {0,  1,  2,  4,   8,  16,  32,   64,  128,  256,   512,  1024}
#define JPEG_MAG_ADJUST_NEG {0, -1, -3, -7, -15, -31, -63, -127, -255, -511, -1023, -2047}

// Precomputed values scaled up by 14 bits, and inverse zigzagged
#define JPEG_SCALING_INIT { \
  16384, 22725, 22725, 21407, 31521, 21407, 19266, 29692, \
  29692, 19266, 16384, 26722, 27969, 26722, 16384, 12873, \
  22725, 25172, 25172, 22725, 12873,  8867, 17855, 21407, \
  22654, 21407, 17855,  8867,  4520, 12299, 16819, 19266, \
  19266, 16819, 12299,  4520,  6270, 11585, 15137, 16384, \
  15137, 11585,  6270,  5906, 10426, 12873, 12873, 10426, \
   5906,  5315,  8867, 10114,  8867,  5315,  4520,  6967, \
   6967,  4520,  3552,  4799,  3552,  2446,  2446,  1247 }


// General definitions

#define INPUT_FILENAME                  "test.jpg"
#define OUTPUT_FILENAME                 "test.bmp"
#define DEFAULT_BUFSIZE                 4096

// JPEG segment definitions
#define JPEG_MARKER_MASK                0xff00

#define JPEG_MKR_SOI                    0xffd8
#define JPEG_MKR_SOF0                   0xffc0
#define JPEG_MKR_SOF1                   0xffc1
#define JPEG_MKR_SOF2                   0xffc2
#define JPEG_MKR_SOF3                   0xffc3
#define JPEG_MKR_DHT                    0xffc4
#define JPEG_MKR_SOF5                   0xffc5
#define JPEG_MKR_SOF6                   0xffc6
#define JPEG_MKR_SOF7                   0xffc7
#define JPEG_MKR_SOF9                   0xffc9
#define JPEG_MKR_SOFa                   0xffca
#define JPEG_MKR_SOFb                   0xffcb
#define JPEG_MKR_SOFd                   0xffcd
#define JPEG_MKR_SOFe                   0xffce
#define JPEG_MKR_SOFf                   0xffcf
#define JPEG_MKR_DQT                    0xffdb
#define JPEG_MKR_DRI                    0xffdd
#define JPEG_MKR_SOS                    0xffda
#define JPEG_MKR_RST0                   0xffd0
#define JPEG_MKR_RST1                   0xffd1
#define JPEG_MKR_RST2                   0xffd2
#define JPEG_MKR_RST3                   0xffd3
#define JPEG_MKR_RST4                   0xffd4
#define JPEG_MKR_RST5                   0xffd5
#define JPEG_MKR_RST6                   0xffd6
#define JPEG_MKR_RST7                   0xffd7
#define JPEG_MKR_APP0                   0xffe0
#define JPEG_MKR_APP1                   0xffe1
#define JPEG_MKR_APP2                   0xffe2
#define JPEG_MKR_APP3                   0xffe3
#define JPEG_MKR_APP4                   0xffe4
#define JPEG_MKR_APP5                   0xffe5
#define JPEG_MKR_APP6                   0xffe6
#define JPEG_MKR_APP7                   0xffe7
#define JPEG_MKR_APP8                   0xffe8
#define JPEG_MKR_APP9                   0xffe9
#define JPEG_MKR_APPa                   0xffea
#define JPEG_MKR_APPb                   0xffeb
#define JPEG_MKR_APPc                   0xffec
#define JPEG_MKR_APPd                   0xffed
#define JPEG_MKR_APPe                   0xffee
#define JPEG_MKR_APPf                   0xffef
#define JPEG_MKR_COM                    0xfffe
#define JPEG_MKR_EOI                    0xffd9

#define JPEG_MKR_BYTE                   0xff

#define JPEG_STR_SOI                    "SOI"
#define JPEG_STR_SOF0                   "SOF0"
#define JPEG_STR_DHT                    "DHT"
#define JPEG_STR_DQT                    "DQT"
#define JPEG_STR_DRI                    "DRI"
#define JPEG_STR_SOS                    "SOS"
#define JPEG_STR_RST0                   "RST"
#define JPEG_STR_APP0                   "APP"
#define JPEG_STR_COM                    "COM"
#define JPEG_STR_EOI                    "EOI"

#define JPEG_DHT_PAYLOAD_OFFSET         2
#define JPEG_DHT_MAX_BITS               16
#define JPEG_DHT_MAX_TABLES             4
#define JPEG_DHT_MAX_VALUES             256
#define JPEG_DHT_DC_CLASS               0
#define JPEG_DHT_AC_CLASS               1

#define JPEG_SUB_SAMPLING_444           0x11
#define JPEG_SUB_SAMPLING_422           0x21

#define JPEG_NO_SUBSAMPLING             1
#define JPEG_SUBSAMPLING                2
#define JPEG_SUB_SAMPLE_444             ((JPEG_NO_SUBSAMPLING << 4) | JPEG_NO_SUBSAMPLING)
#define JPEG_SUB_SAMPLE_422             ((JPEG_SUBSAMPLING    << 4) | JPEG_NO_SUBSAMPLING)
#define JPEG_SUB_SAMPLE_420             ((JPEG_SUBSAMPLING    << 4) | JPEG_SUBSAMPLING)
#define JPEG_SUPPORTED_SUBSAMPLING      JPEG_SUB_SAMPLE_444

#define JPEG_DQT_ELEMENTS               64
#define JPEG_DQT_TABLE_SIZE             (JPEG_DQT_ELEMENTS+1)
#define JPEG_MCU_ELEMENTS               64
#define JPEG_BLOCK_DIMENSION            8 
#define JPEG_MAX_MCU_BLOCKS             6 
#define JPEG_MAX_QUANT_TABLES           4

#define JPEG_SOS_NS_OFFSET              2
#define JPEG_SOS_CI_OFFSET              3
#define JPEG_SOS_TAIL_SIZE              3
#define JPEG_SOS_MAX_NS                 4

#define JPEG_APP14_STR_OFFSET           2
#define JPEG_APP14_ADOBE_STR            "Adobe"
#define JPEG_APP14_ADOBE_STR_LEN        5 
#define JPEG_APP14_COLOUR_SPACE_OFFSET  13

#define JPEG_DQT_OFFSET                 2
#define JPEG_DRI_OFFSET                 2

#define JPEG_MAX_MCU_ARRAYS             6

#define JPEG_DEFAULT_STR_SIZE           33
#define JPEG_NIBBLE_MASK                0xf
#define JPEG_INVALID                    (-1)

#define JPEG_ZRL                        0xf0
#define JPEG_EOB                        0x00
#define JPEG_ZRL_BYTE_RUN_LEN           15

#define JPEG_MARKER_BYTE                0xff
#define JPEG_MARKER_FLAG                0xffff0000

#define JPEG_NUM_COLOUR_SCANS           3
#define JPEG_NUM_RGB_COLOURS            3

#define JPEG_JFIF_STR                   "JFIF"
#define JPEG_JFXX_STR                   "JFXX"

#ifdef JPEG_DCT_INTEGER

  #define JPEG_RGB_BITS                 10
  #define JPEG_RGB_SCALE                (1 << JPEG_RGB_BITS)
  #define JPEG_RGB_ROUND_MASK           (1 << (JPEG_RGB_BITS)-1)
  #define JPEG_RGB_Kr1                  1436
  #define JPEG_RGB_Kg1                  352
  #define JPEG_RGB_Kg2                  731
  #define JPEG_RGB_Kb1                  1815

#else

  #define JPEG_RGB_Kr1                  1.402
  #define JPEG_RGB_Kg1                  0.34414
  #define JPEG_RGB_Kg2                  0.71414
  #define JPEG_RGB_Kb1                  1.772

#endif

#define JPEG_CLIP(_a)                   (((_a) < 0) ? 0 : ((_a) > 255) ? 255 : (_a))
#define JPEG_ROUND(_a)                  ((int)floor((_a)+0.5))

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define JPEG_REORDER16(_x)              ((((_x) & 0xff)<<8) | (((_x) & 0xff00)>>8))
#else
#define JPEG_REORDER16(_x)              ((_x) & 0xffff)
#endif

#define JPEG_DEBUG_ALL_EN               0xFFFFFFFFU
#define JPEG_DEBUG_SCAN_EN              (1 << 0)
#define JPEG_DEBUG_FRM_EN               (1 << 1)
#define JPEG_DEBUG_DQT_EN               (1 << 2)
#define JPEG_DEBUG_DHT_EN               (1 << 3)
#define JPEG_DEBUG_MCU_EN               (1 << 4)
#define JPEG_DEBUG_MKR_EN               (1 << 5)
#define JPEG_DEBUG_AMP_EN               (1 << 6)
#define JPEG_DEBUG_QNT_EN               (1 << 7)
#define JPEG_DEBUG_IDCT_EN              (1 << 8)
#define JPEG_DEBUG_IDCT_EN_2            (1 << 9)
#define JPEG_DEBUG_IDCT_EN_3            (1 << 10)
#define JPEG_DEBUG_IDCT_EN_4            (1 << 11)
#define JPEG_DEBUG_IDCT_EN_5            (1 << 12)
#define JPEG_DEBUG_IDCT_EN_6            (1 << 13)
#define JPEG_DEBUG_IDCT_EN_7            (1 << 14)
#define JPEG_DEBUG_IDCT_EN_8            (1 << 15)
#define JPEG_DEBUG_IDCT_EN_9            (1 << 16)
#define JPEG_DEBUG_IDCT_EN_10           (1 << 17)
#define JPEG_DEBUG_IDCT_EN_11           (1 << 18)
#define JPEG_DEBUG_IDCT_EN_12           (1 << 19)
#define JPEG_DEBUG_IDCT_EN_13           (1 << 20)
#define JPEG_DEBUG_IDCT_EN_14           (1 << 21)
#define JPEG_DEBUG_HUFF_DECODE          (1 << 23)

#define JPEG_DEBUG_MAIN_EN              (1 << 24)

//--------------------------------------------------------------------------
// The following structure types are to be overlayed on top of byte memory, 
// and so must be byte aligned. NB. The fields > 8 bits are MSB first. 
// Byte reordering is required on LSB machines. The function bytereorder
// defaults to LSB machine operation, but can be rewritten for MSB machines
// (i.e. simply return input unaltered)

#ifdef _WIN32
#pragma pack (push, 1)                      
#else
#pragma pack (push)
#pragma pack (1)                      
#endif


// Frame segment (see ITU.T81 sec B.2.2)

// Frame header parameter type
typedef struct {
    uint8 Cid;                                  // Component ID (0-255)
    uint8 HVi;                                  // Combined horizontal/vertical sampling factors (1-4) 
                                                // (upper/lower nibbles)
    uint8 Tq;                                   // Quantisation destination table (0-3)
} frame_params_t, *frame_params_pt;

// Frame header type
typedef struct {
    uint16 length;                              // Length of frame header segment (including these 2 bytes) - LSB first
    uint8  P;                                   // Sample precision (8 for baseline, 8 or 12 for extended)
    uint16 Y;                                   // Number of lines (0 - 65535)
    uint16 X;                                   // Number of samples per line (1- 65535)
    uint8  Nf;                                  // Number of image components in frame (1-255)
    frame_params_t Ci[4];                       // Variable number of frame-component specification parameters
} frame_header_t, *frame_header_pt;

// Quantisation table (see ITU.T81 sec B.2.4.1)

// Indivdual quantisation table (8 bits -- baseline DCT only)
typedef struct {
    uint8  PTq;                                 // Combined Precision/Table destination (upper/lower nibbles)
    uint8  Qn[JPEG_DQT_ELEMENTS];               // Quantisation table data bytes 
} DQT_raw_t, *DQT_raw_pt;

//--------------------------------------------------------------------------
// The rest of the structures are not directly overlaying byte memory, and 
// need not be packed. The wider fields need not be byte reordered (unless
// stated for a particular field)

// Internal DQT table, where Qn values are AAN prescaled values
typedef struct {
    uint8  PTq;                                 // Combined Precision/Table destination (upper/lower nibbles)
    int    Qn[JPEG_DQT_ELEMENTS];               // Quantisation table data, scaled with AAN iDCT prescaler values
} DQT_t, *DQT_pt;

#pragma pack(pop) // Restore original alignment from stack

// Scan header segment (see ITU.T81 sec B.2.3)

// Last parameters of SOS header (variable position). Is mapped over buffer memory,
// but constant for baseline DCT, so only used for format checking.
typedef struct {
    uint8 Ss;                                   // Start of spectral section (0 for baseline)
    uint8 Se;                                   // End of spectral section (63 for baseline)
    uint8 Ahl;                                  // Combined succesive approximation hi/lo bit positions 
                                                // (always 0 for baseline)
} scan_tail_t, *scan_tail_pt;

// Individual scan-component-specification parameter. Is mapped over 
// buffer memory.
typedef struct {
    uint8 Cs;                                   // Scan component selector (0-255)
    uint8 Tda;                                  // Combined Huffman DC/AC table selector (upper/lower nibble) 
                                                // (0-1 for baseline DCT)
} scan_params_t, *scan_params_pt;

// Start of scan header type
typedef struct {
    uint16 length;                              // Length of SOS segment (including the length bytes). 
                                                // Extracted from buffer
    uint8  Ns;                                  // Number of image components in scan. Extracted from buffer
    scan_params_t* p_Ci;                        // Pointer to component specific parameter list in buffer
    scan_tail_t* p_tail;                        // Pointer to tail scan parameters in buffer
    uint8 *p_ECS;
} scan_header_t, *scan_header_pt;

// Huffman table type (see ITU.T81 sec B.2.4.2)

typedef struct {
    int Tc;                                     // Extracted table class (0 == DC, 1 == AC)
    int Th;                                     // Extracted table destination (0-1 for baseline DCT)
    uint8 *Ln;                                  // Pointer to 16 element array of number of codes for (n+1)th 
                                                // bit width in buffer
    uint8 *vmn_offset     [JPEG_DHT_MAX_BITS];  // Array of pointers to first value for (n+1)th bit width (NULL if none)
    int   row_break_codes [JPEG_DHT_MAX_BITS];  // Value of Huffman code after last mapped for (n+1)th bit width
} DHT_offsets_t, *DHT_offsets_pt;

//--------------------------------------------------------------------------
// The following type definitions are internal to code, and not mapped to 
// the JPEG/JFIF standards
//

typedef struct {
    int  marker;                                // if non-zero, hit a marker
    bool is_EOB;                                // Flag if EOB code
    bool is_ZRL;                                // Flag if ZRL code
    int  ZRL;                                   // run length of zeros (ZRL code if 16)
    int  amplitude;                             // unadjusted amplitude (if not EOB or ZRL)
} rle_amplitude_t, *rle_amplitude_pt;

typedef int (* jpeg_8x8_block_t)   [JPEG_BLOCK_DIMENSION];
typedef int (* jpeg_nx8x8_block_t) [JPEG_BLOCK_DIMENSION]  [JPEG_BLOCK_DIMENSION];
typedef int (* jpeg_rgb_block_t)   [JPEG_BLOCK_DIMENSION*2][JPEG_BLOCK_DIMENSION*2];

#ifdef JPEG_DCT_INTEGER
typedef int    jpeg_dct_t;
#else
typedef double jpeg_dct_t;
#endif

#endif
