//=============================================================
//
// Copyright (c) 2014 - 2026 Simon Southwell
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
//=============================================================

#include "jfif_local.h"
#include "jfif_idct.h"

#ifndef _JFIF_CLASS_H_
#define _JFIF_CLASS_H_

class jfif : public jfif_idct
{

public:

    // Constructor. Initialise local state and base class
    jfif(int debug_enable_in = 0) : jfif_bit_count(0), jfif_barrel(0), debug_enable(debug_enable_in), jfif_idct(debug_enable_in)
    {

        for (int idx = 0; idx < JPEG_SOS_MAX_NS; idx++)
            current_dc_value[idx] = 0;
    };

    // Top level method, for external access
    int              jpeg_process_jfif   (uint8_t *ibuf, uint8_t **obuf, uint8_t **rawbuf = NULL) ;

    // Conversion functions for generating a 24bit bitmap
    uint8_t*           jpeg_bitmap_init    (int X, int Y);
    int              jpeg_ycc_to_rgb     (jpeg_nx8x8_block_t ptr, jpeg_rgb_block_t optr, int Ns, int Hi, int Vi,
                                          bool is_RGB);
    void             jpeg_bitmap_update  (jpeg_rgb_block_t ptr, int mcu_row, int mcu_col, int Ns, int Hi, int Vi,
                                          uint8_t *bmp_data_ptr, uint8_t *rawbuf, int X, int Y);

// Private state
private:

    // Constant tables
    static const int aanscales[DCTSIZE*DCTSIZE];
    static const int jpeg_inv_zigzag[];
    static const int jpeg_zigzag[];
    static const int jpeg_adj_neg[];
    static const int jpeg_adj_pos[];

    // Barrel shifter state
    int              jfif_bit_count;
    uint32_t           jfif_barrel;

    // Running DC value state
    int              current_dc_value[JPEG_SOS_MAX_NS];

    // MCU buffer
    int              mcu[JPEG_MAX_MCU_BLOCKS][JPEG_MCU_ELEMENTS];

    // Debug control
    int              debug_enable;


// Private methods
private:

#ifdef JPEG_DEBUG_MODE
    // Debug output methods (when compile option defined)
    char*            int2binstr          (char* buf, int num, int len);
    void             print_frame_hdr     (const frame_header_t* fptr);
    void             print_scan_hdr      (const scan_header_t* sptr);
    void             print_DQT           (const DQT_raw_t* qptr);
    void             print_MCU           (int (*sptr)[JPEG_MCU_ELEMENTS], int N);
#endif

    // Low level support methods
    int              jpeg_amp_adjust     (int value, int size);
    int              jpeg_get_bits       (int n, uint8_t *buf, int *idx, uint32_t *barrel, int *bit_count, bool remove_bits);
    DHT_offsets_t*   jpeg_dht            (uint8_t *dht, DHT_offsets_t* decode_ptr);
    rle_amplitude_t  jpeg_dht_lookup     (DHT_offsets_t* dht_ptr, bool is_DC, int T, uint8_t *buf, int *idx,
                                         uint32_t *barrel, int *bit_count);

    // Main decode methods for parsing header, and decoding scan data
    int              jpeg_extract_header (uint8_t *buf, scan_header_t **sptr, frame_header_t **fptr, DQT_t *qptr,
                                         DHT_offsets_t **hptr, int *dri, bool *is_RGB);

    int            (*jpeg_huff_decode    (scan_header_t *sptr, DHT_offsets_t *hptr, DQT_t *qptr, frame_header_t *fptr,
                                          uint8_t *ecs_ptr[],    int *marker)) [JPEG_MCU_ELEMENTS];
};

#endif
