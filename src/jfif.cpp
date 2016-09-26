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
// $Id: jfif.cpp,v 1.5 2016-09-26 06:36:59 simon Exp $
// $Source: /home/simon/CVS/src/HDL/jfif/sw/jpeg_cpp/src/jfif.cpp,v $
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
// Basic structure and function call hierarchy
//
// jpeg_process_jfif()             -- Converts input baseline DCT JFIF buffer to 24 bit bitmap
//     jpeg_extract_header()       -- Extracts jpeg header information (scan, frame, quantisation/huffman tables, DRI)
//         jpeg_dht()              -- Constructs a Huffman decode structure from huffman table data
//     jpeg_bitmap_init()          -- Creates space for appropriately sized bitmap and initialises header data
// 
//   LOOP for each MCU:            -- jpeg_process_jfif() process an MCU at a time until EOI (or error)
//     jpeg_huff_decode()          -- Gets an adjusted Huffman/RLE decoded amplitude value and ZRLs, or marker or end-of-block
//         jpeg_dht_lookup()       -- Does huffman lookup on code and extracts amplitude data, or flags a marker
//             jpeg_get_bits()     -- Gets top bits from barrel shifter and (optionally) removes. Pulls in extra input if needed.
//             jpeg_amp_adjust()   -- Adjusts decoded huffman decoded amplitude to +/- amplitude value
//         <dequantise>            -- De-quantisation done in jpeg_huff_decode directly from selected table
//     jpeg_idct()                 -- Inverse discrete cosine transform (define in jfif_idct base class)
//     jpeg_ycc_to_rgb()           -- Converts YCbCr to RGB on an MCU
//     jpeg_bitmap_update()        -- Updates bitmap data buffer with 8x8 RGB values
//   ENDLOOP
//   return bitmap pointer
//
//=============================================================

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <cmath>

#include "jfif_class.h"
#include "bitmap.h"

// Set the constant values for the internal jfif class tables
const int jfif::aanscales[DCTSIZE*DCTSIZE]  = JPEG_SCALING_INIT;
const int jfif::jpeg_inv_zigzag[]           = JPEG_INV_ZIGZAG_MAP;
const int jfif::jpeg_zigzag[]               = JPEG_ZIGZAG_MAP;

const int jfif::jpeg_adj_neg[]              = JPEG_MAG_ADJUST_NEG;
const int jfif::jpeg_adj_pos[]              = JPEG_MAG_ADJUST_POS;

#ifdef JPEG_DEBUG_MODE

//-------------------------------------------------------------
// int2binstr()
//
// Description:
//
// Construct a binary string from 'num' of length 'len'  into
// a buffer pointed to by 'buf, and return a pointer to the 
// string.
//
// Parameters:
//      buf:    pointer to buffer for returning binary string
//      num:    number to be converted
//      len:    Length of bits for conversion (1 - 31)
//
// Return value:
//      Pointer to a buffer containing converted string
//

char* jfif::int2binstr(char* buf, int num, int len)
{
    int idx, jdx, bit; 

    for (jdx = idx = 0; idx < len; idx++) {
        bit = (num >> (len-1-idx)) & 1;
        buf[jdx++] = bit ? '1' : '0';
    }

    buf[jdx] = 0;

    return buf;
}

//-------------------------------------------------------------
// print_frame_hdr()
//
// Description:
//
// Prints to stdout a formatted frame header for debug purposes.
//
// Parameters:
//    fptr:     A pointer to a frame header
//
// Return value:
//    None
//

void jfif::print_frame_hdr(const frame_header_t* fptr)
{

    using std::cout;
    using std::endl;

    cout << "  Lf   : " << JPEG_REORDER16(fptr->length) << endl;
    cout << "  P    : " << (int)(fptr->P)               << endl;
    cout << "  Y    : " << JPEG_REORDER16(fptr->Y)      << endl;
    cout << "  X    : " << JPEG_REORDER16(fptr->X)      << endl;
    cout << "  Nf   : " << (int)fptr->Nf                << endl;

    for (int idx = 0; idx < fptr->Nf; idx++) {
        cout << "  C" << idx+1  << "   : " << (int)fptr->Ci[idx].Cid           << endl;
        cout << "  H" << idx+1  << "   : " << ((fptr->Ci[idx].HVi >> 4 & 0xf)) << endl;
        cout << "  V" << idx+1  << "   : " << (fptr->Ci[idx].HVi & 0xf)        << endl;
        cout << "  Tq" << idx+1 <<  "  : " << (int)fptr->Ci[idx].Tq            << endl;
    }
}

//-------------------------------------------------------------
// print_scan_hdr()
//
// Description:
//
// Prints to stdout a formatted scan header for debug purposes.
//
// Parameters:
//    sptr:     A pointer to a SOS header
//
// Return value:
//    None
//

void jfif::print_scan_hdr(const scan_header_t* sptr)
{
    using std::cout;
    using std::endl;

    cout << "  Ls   : " << sptr->length  << endl;
    cout << "  Ns   : " << (int)sptr->Ns << endl;

    for (int idx = 0; idx < sptr->Ns; idx++) {
        cout << "  Cs" << idx+1 << "  : " << (int)(sptr->p_Ci+idx)->Cs            << endl;
        cout << "  Td" << idx+1 << "  : " << (((sptr->p_Ci+idx)->Tda >> 4) & 0xf) << endl;
        cout << "  Ta" << idx+1 << "  : " << (((sptr->p_Ci+idx)->Tda >> 0) & 0xf) << endl;
    }

    cout << "  Ss   : " << (int)sptr->p_tail->Ss                 << endl;
    cout << "  Se   : " << (int)sptr->p_tail->Se                 << endl;
    cout << "  Ah   : " << ((sptr->p_tail->Ahl >> 4) & 0xf) << endl;
    cout << "  Al   : " << ((sptr->p_tail->Ahl >> 0) & 0xf) << endl;
}

//-------------------------------------------------------------
// print_DQT()
//
// Description:
//
// Prints to stdout a formatted quantisation table for debug 
// purposes.
//
// Parameters:
//    qtr:      A pointer to a quantisation table
//
// Return value:
//    None
//

void jfif::print_DQT(const DQT_raw_t* qptr)
{
    using std::cout;
    using std::endl;
    using std::hex;
    using std::setfill;
    using std::setw;

    int jdx = 0, byte_count = 0;

    while (byte_count < JPEG_DQT_TABLE_SIZE) {

        cout << "  Pq" << jdx << "  : " << ((qptr->PTq >> 4) & 0xf) << endl;
        cout << "  Tq" << jdx << "  : " << ((qptr->PTq >> 0) & 0xf) << endl;
        
        byte_count++;

        cout << "    ";

        for (int idx = 0; idx < JPEG_DQT_ELEMENTS; idx++) {

            if ((qptr->PTq >> 4) == 0) {

                cout <<  setfill('0');
                cout << hex << setw(2) << (int)qptr->Qn[jpeg_zigzag[idx]] << " ";
                byte_count += 1;

            } else {

                cout << hex << setw(4) << (qptr->Qn[idx*2] << 8) << qptr->Qn[idx*2+1] << " ";
                byte_count += 2;
            }
        
            if ((idx%8) == 7)
                cout << endl << "    ";
        }
        jdx++;
        cout << endl;
    }
}

//-------------------------------------------------------------
// print_MCU()
//
// Description:
//
// Prints to stdout formatted MCU data for debug purposes.
//
// Parameters:
//    sptr:     A pointer to an array of MCU values arranged 
//              in an 8x8 array of integers
//    N:        Number of components in MCU
//
// Return value:
//    None
//

void jfif::print_MCU(int (*sptr)[JPEG_MCU_ELEMENTS], int N)
{
    using std::cout;
    using std::endl;
    using std::setw;

    for (int scans = 0; scans < N; scans++) {

        if (scans < (N-2))
            cout << "Y";
        else if (scans == (N-2))
            cout << "Cb";
        else
            cout << "Cr";

        cout << endl << "    ";

        for (int idx = 0; idx < JPEG_MCU_ELEMENTS; idx++) {
            
            cout << setw(4) << sptr[scans][idx] << " ";

            if ((idx%8) == 7)
                cout << endl << "    ";
        }
        cout << endl;
    }
}
#endif

//-------------------------------------------------------------
// jpeg_ycc_to_rgb()
//
// Description:
//
// Converts MCU with Y*n, Cb, Cr 8x8 data blocks to RGB
// set of nxn data blocks. 
//
// Non-subsampled data is just YCbCr, but can be YYCbCr
// for vertical or horizontal sub-sampling, or YYYYCbCr if
// sub-sampled in both directions. Sub-sampling of up to 
// 2 in each direction is supported (not 4). Returned RGB
// data is 8x8, 16x8 or 16x16 triplet of red, green and blue
// values, depending on sub-sampling.
//
// Parameters:
//    ptr:      pointer to buffer with YCbCr 8x8 block triplet
//    optr:     pointer to buffer space for RGB values
//    Ns:       Number of components in scan
//    Hi:       Horizontal sub sampling
//    Vi:       Vertical sub sampling
//    is_RGB:   3 component data is JPEG RGB data
//
// Return Value:
//    None
//

int jfif::jpeg_ycc_to_rgb(jpeg_nx8x8_block_t ptr, jpeg_rgb_block_t optr, int Ns, int Hi, int Vi, bool is_RGB)
{
    using std::cerr;
    using std::endl;

    int ny = Hi * Vi;                   // Number of Y components to process

    if (Ns != JPEG_NUM_COLOUR_SCANS) {
        cerr << "ERROR: jpeg_ycc_to_rgb() called with no chroma data" << endl;
        return JPEG_FORMAT_ERROR;
    }

    // For each luminance (Y) value (sub-sampling)...
    for (int lum_idx = 0; lum_idx < ny; lum_idx++) {

        // Local calculations on lum_idx for optimisation
        int lum_div_2 = lum_idx >> 1;
        int lum_mod_2 = lum_idx & 0x1;

        for (int row = 0; row < JPEG_BLOCK_DIMENSION; row++) {

            for (int col = 0; col < JPEG_BLOCK_DIMENSION; col++) {

                int r, g, b;

                // Get Y value for this array
                int Y = ptr[lum_idx][row][col];

                // Get chroma values (with sub-sampling indexing)
                int Cb = ptr[ny]  [row/Vi + (lum_div_2<<2)][col/Hi + (lum_mod_2<<2)];
                int Cr = ptr[ny+1][row/Vi + (lum_div_2<<2)][col/Hi + (lum_mod_2<<2)];

                // If data is already RGB, simply copy from array blocks normally used for YCC data
                if (is_RGB) {

                    r = Y;
                    g = Cb;
                    b = Cr;

                } else {

#ifdef JPEG_DCT_INTEGER
                    // Do conversion, and clip
                    r = Y*JPEG_RGB_SCALE + JPEG_RGB_Kr1 * (Cr-128);
                    g = Y*JPEG_RGB_SCALE - JPEG_RGB_Kg1 * (Cb-128) - JPEG_RGB_Kg2 * (Cr-128);
                    b = Y*JPEG_RGB_SCALE + JPEG_RGB_Kb1 * (Cb-128);

                    r = JPEG_CLIP((r >> JPEG_RGB_BITS) + ((r & JPEG_RGB_ROUND_MASK) ? 1 : 0));
                    g = JPEG_CLIP((g >> JPEG_RGB_BITS) + ((g & JPEG_RGB_ROUND_MASK) ? 1 : 0));
                    b = JPEG_CLIP((b >> JPEG_RGB_BITS) + ((b & JPEG_RGB_ROUND_MASK) ? 1 : 0));
#else
                    r = JPEG_CLIP(JPEG_ROUND((double)Y + JPEG_RGB_Kr1 * (double)(Cr-128)));
                    g = JPEG_CLIP(JPEG_ROUND((double)Y - JPEG_RGB_Kg1 * (double)(Cb-128) - JPEG_RGB_Kg2 * (double)(Cr-128)));
                    b = JPEG_CLIP(JPEG_ROUND((double)Y + JPEG_RGB_Kb1 * (double)(Cb-128)));
#endif
                }

                // Calculate destination row and column indexes, with special case
                // of Vertical, but no horizontal sub-sampling
                int col_idx = (Vi == 2 && Hi == 1) ? col+(lum_div_2<<3) : col+(lum_mod_2<<3);
                int row_idx = (Vi == 2 && Hi == 1) ? row+(lum_mod_2<<3) : row+(lum_div_2<<3);

                // Update buffer with RGB values
                optr[0][row_idx][col_idx] = r;
                optr[1][row_idx][col_idx] = g;
                optr[2][row_idx][col_idx] = b;
            }
        }
    }

    return JPEG_NO_ERROR;
}

//-------------------------------------------------------------
// jpeg_amp_adjust()
//
// Description:
//
// Takes a Huffman decode amplitude value and returns the +/-
// adjusted value (as per ITU.T81 Table F.1 and Table F.2)
//
// Parameters:
//    value:    Unadjusted decoded amplitude value (0 - 2047)
//    size:     Size of bit width for decoded value (0 - 11)
//
// Return value:
//    Adjusted value (range -2047 to 2047)
//

int jfif::jpeg_amp_adjust (int value, int size)
{

    if (size == 0)
        return 0;

    // Top bit selects positive or negative table
    int top_bit         = value & (1 << (size-1));

    // Rest of bits are offset from table
    int bottom_bit_mask = (1 << (size-1))-1;

    // Return value
    return (top_bit ? jpeg_adj_pos[size] : jpeg_adj_neg[size]) + (value & bottom_bit_mask);
}

//-------------------------------------------------------------
// jpeg_get_bits()
//
// Description:
//
// Returns top 'n' bits from a barrel shifter. If insufficient
// bits available, data is pulled from input buffer onto barrel
// until enough available. The barrel state and barrel bit count
// is maintained by function. In addition, it manages padded 
// special byte (0xFF00), stripping the 0x00 byte, and also
// detects (but not decode) markers (0xFFnn - where nn is 1 to 255).
//
// Parameters:
//      n:              number of bits required from top of barrel
//      buf:            pointer to input buffer
//      idx:            pointer to current input byte in buf (updated)
//      barrel:         pointer to barrel shifter (updated)
//      bit_count:      Pointer to bit count of bits on barrel (updated)
//      remove_bits:    Control to return bits without updating bit_count 
//                      for removal of n bits (still updated for any input 
//                      byte added).
//
// Return value:
//      0xFFFFmmmm      - if top bits set, bottom mmmm bits contain marker value.
//      0x0000nnnn      - else returned n bits of barrel top
//

int jfif::jpeg_get_bits(int n, uint8 *buf, int *idx, uint32 *barrel, int *bit_count, bool remove_bits)
{
    int rtn_value;

    // Update barrel to have enough bits for requested width
    while (*bit_count < n) {

        // Hit a marker
        if (buf[*idx] == JPEG_MARKER_BYTE && buf[*idx+1] != 0x00) {

            // Flush the barrel shifter
            *bit_count = 0;

            // Return the marker in lieu of a codeword (tagged in upper bits to make negative)
            rtn_value = (int)((buf[*idx] << 8) | buf[*idx+1] | JPEG_MARKER_FLAG);

            // Skip over the marker
            *idx += 2;

            return rtn_value;
        }

        // Add a byte to bottom of the barrel
        *barrel = (*barrel << 8) | (uint32)buf[*idx];
        *bit_count += 8;

        // Detect padded byte and remove
        if (buf[*idx] == 0xff && buf[*idx+1] == 0x00)
            *idx += 1;

        // Update inout buffer index for added byte
        *idx += 1;
    }

    // Get top n bits from barrel
    rtn_value = (*barrel >> (*bit_count - n)) & ((1U << n) - 1U) ;

    // Only remove bits from barrel if enabled
    if (remove_bits)
        *bit_count -= n;

    return rtn_value;
}

//-------------------------------------------------------------
// jpeg_dht()
//
// Description:
//
// Construct a Huffman data decode structure from JPEG DHT
// segment on a buffer. Space is dynamically created for the
// decode structure, and a pointer returned (or NULL on error)
//
// The information required for huffman decode is created here.
// For each bit-width length Ln a, value (current_prefix, reset to 0)
// is shifted right and Ln added. This number is then stored for
// the bit-width. The stored numbers are then the upper bound (+1) for
// that bit-width's values, and the previous value shited right 1 
// is the lower bound. To look up a value, the code is checked to be
// less than the upper bound for the bit width, and the offset into
// that widths values is the code minus lower bound. The offsets
// into the Vmn data are stored with each bit width as well.
//
// Parameters:
//    dht:              pointer to a byte buffer containing JPEG DHT segment
//    decode_ptr:       pointer to a DHT table pointer, for returning decode table data
//                      if NULL, or as preallocated space to use otherwise.
//
// Return value:
//    DHT_offsets_t structure pointer containing decode data or
//    NULL if an error occurred.
//

DHT_offsets_t* jfif::jpeg_dht(uint8 *dht, DHT_offsets_t* decode_ptr)
{
    using std::cout;
    using std::cerr;
    using std::dec;
    using std::hex;
    using std::setw;
    using std::setfill;
    using std::endl;

    DHT_offsets_t* rtnptr;
    DHT_offsets_t* ptr;

#ifdef JPEG_DEBUG_MODE    
    char strbuf[JPEG_DEFAULT_STR_SIZE];
    int  map_array     [JPEG_DHT_MAX_TABLES][JPEG_DHT_MAX_VALUES], *map;
    int  codelen_array [JPEG_DHT_MAX_TABLES][JPEG_DHT_MAX_VALUES], *codelen;
#endif


    // Allocate some initialised space for the DHT pointers, if none already
    if (decode_ptr == NULL) {

        try {
            rtnptr = new DHT_offsets_t[JPEG_DHT_MAX_TABLES]();
        }
        catch(std::bad_alloc &ba) {
            cerr << "ERROR: jpeg_dht(): memory allocation failed: " << ba.what() << endl;
            return (DHT_offsets_t*) NULL;
        }

    } else {

        rtnptr = decode_ptr;

    }

    // Extract DHT segment length
    int length = (dht[0] << 8) | dht[1];

    // Point to start of DHT payload
    int offset = JPEG_DHT_PAYLOAD_OFFSET;

#ifdef JPEG_DEBUG_MODE
    if (debug_enable & JPEG_DEBUG_DHT_EN)
        cout << "  length: " << length << endl;

    // Initialise maps to mark as invalid (not all may be present)
    for (int idx = 0; idx < JPEG_DHT_MAX_TABLES; idx++)
        map_array[idx][0] = JPEG_INVALID;
#endif
    
    while (offset < length) {

        int current_prefix = 0;

        // Get combined Tc/Th index 
        int Tch = (((dht[offset] >> 4) & JPEG_NIBBLE_MASK) ? 2 : 0) | ((dht[offset] & 0xf) ? 1 : 0) ;

        // Point to the appropriate Tc/Th table
        ptr = rtnptr + Tch;

        ptr->Tc = (dht[offset] >> 4) & JPEG_NIBBLE_MASK;
        ptr->Th = (dht[offset] >> 0) & JPEG_NIBBLE_MASK;

#ifdef JPEG_DEBUG_MODE
        if (debug_enable & JPEG_DEBUG_DHT_EN) {
            cout << "  Tc    : " << (int)ptr->Tc << endl;
            cout << "  Th    : " << (int)ptr->Th << endl;
        }
#endif

        // Skip past Tc/Th byte
        offset++;

        // Point to (Tc,Th) tables indicated
        ptr->Ln = &dht[offset];

#ifdef JPEG_DEBUG_MODE
        map     = map_array[Tch];
        codelen = codelen_array[Tch];
        for (int idx = 0; idx < JPEG_DHT_MAX_VALUES; idx++)
            codelen[idx] = map[idx] = 0;
#endif

#ifdef JPEG_DEBUG_MODE
        // Print the length values for each bit length for this table
        if (debug_enable & JPEG_DEBUG_DHT_EN) 
            for (int bit_length = 1; bit_length <= JPEG_DHT_MAX_BITS; bit_length++) 
                cout << "  L" << setfill('0') << dec << setw(2) << bit_length << "  : " << (int)ptr->Ln[bit_length-1] << endl;
#endif

        // Skip past lengths
        offset += JPEG_DHT_MAX_BITS;

        // Construct the code/byte mapping table for each bit length
        for (int bit_length = 1; bit_length <= JPEG_DHT_MAX_BITS; bit_length++) {

            // Process any valid codes for this bit length
            if (ptr->Ln[bit_length-1]) {

                // Store locations of the Vmn codes for each bit length with values
                // (this can't be zero---the initialised memory value)
                ptr->vmn_offset[bit_length-1] = &dht[offset];

#ifdef JPEG_DEBUG_MODE
                // Print the Vm,n values
                if (debug_enable & JPEG_DEBUG_DHT_EN) 
                    for (int jdx = 0; jdx < ptr->Ln[bit_length-1]; jdx++) 
                        cout << "    V" << dec << bit_length << "," << jdx << " = " << hex << setw(2) << (int)dht[offset+jdx] << endl;
#endif

                // Add code to map table
#ifdef JPEG_DEBUG_MODE
                for (int jdx = 0; jdx < ptr->Ln[bit_length-1]; jdx++) {
                    
                    map[dht[offset+jdx]] = current_prefix;
                    codelen[dht[offset+jdx]] = bit_length;
                    current_prefix++;

                }
#else
                current_prefix += ptr->Ln[bit_length-1];
#endif
                // Skip over values for this bit width
                offset += ptr->Ln[bit_length-1];
            } 

            // Remember the first unused code for this bit width
            ptr->row_break_codes[bit_length-1] = current_prefix;

            // Tail code becomes current prefix shifted left for next bit width row
            current_prefix <<= 1;
        }

#ifdef JPEG_DEBUG_MODE
        // Print out the mapping table
        if (debug_enable & JPEG_DEBUG_DHT_EN) {

            cout << "Table " << ((Tch >> 1) & 1) << "," << (Tch & 1) << endl;

            for (int idx = 0; idx < JPEG_DHT_MAX_VALUES; idx++)
                if (codelen[idx]) {
                    setfill('0');
                    cout << "  "  << dec << setw(3) << idx << " (0x" << hex << setw(2) << idx << ")";
                    cout << " = (" << hex << setw(4) << (int)map[idx] << ") " << int2binstr(strbuf, map[idx], codelen[idx]) << endl;
                }
        }
#endif

    }

    return rtnptr;
}

//-------------------------------------------------------------
// jpeg_dht_lookup()
//
// Description:
//
// Gets an adjusted Huffman/RLE decoded amplitude value from 
// the input buffer bit stream (via a barrel shifter) for 
// either DC or AC data. Flags encountering a marker, an EOB 
// or ZRL.
//
// Parameters:
//    dht_ptr:          pointer to Huffman decode data
//    is_DC:            flags if required code is for DC (true) or not (false)
//    T:                defines huffman table ID (0 or 1 for baseline DCT)
//    buf:              input data buffer pointer
//    idx:              pointer to current offset index into input buffer (updated)
//    barrel:           pointer to barrel shifter (updated)
//    bit_count:        Pointer to bit count of bits on barrel (updated)
//
// Return value:
//    Returns a structure with either a decoded amplitude value and
//    zero run length, or flags for marker/boundary state. The structure
//    has the following fields:
//        marker:       Returning a marker if > 0 (is 0xmmmm, where mmmm is marker value)
//        is_EOB:       Hit EOB (if marker 0)
//        is_ZRL:       Hit ZRL (if marker 0)
//        ZRL:          Zero run length (if marker == 0 and is_EOB false)
//        amplitude:    amplitude value if ZRL < 16 and if marker == 0 and is_EOB is false
//

rle_amplitude_t jfif::jpeg_dht_lookup(DHT_offsets_t* dht_ptr, bool is_DC, int T, uint8 *buf, int *idx, uint32 *barrel, int *bit_count)
{
    using std::cout;
    using std::cerr;
    using std::hex;
    using std::setw;
    using std::endl;

    int             value;
    rle_amplitude_t rval;
    DHT_offsets_t*  dht;

    // Lookup which DHT table to use (don't assume any ordering)
    for (int table = 0; table < JPEG_DHT_MAX_TABLES; table++) {
        dht = dht_ptr + table;

        // Once the table is found, stop looking
        if ((is_DC && dht->Tc == JPEG_DHT_DC_CLASS && T == dht->Th) ||
           (!is_DC && dht->Tc == JPEG_DHT_AC_CLASS && T == dht->Th))
            break;
    }

    // Check for smallest matching code.
    // (In hardware, could match all available bit widths simultaneously, and 
    // use a priority encoder to match to the smallest width, in case of multi-match 
    // [is that possible?])
    for (int bit_width = 1; bit_width <= JPEG_DHT_MAX_BITS; bit_width++) {

        int code;

        // Get next code of 'bit_width' width from input stream
        if ((code = jpeg_get_bits(bit_width, buf, idx, barrel, bit_count, false)) < 0) {

            // Returned code was a marker, so strip top marker indicator bits and return
            rval.marker = code & 0xffff;
            return rval;

        // Not a marker, so check if there is a match for this bit width.
        // This is just code being less than row_break_code (and Ln not 0)
        } else if (dht->Ln[bit_width-1] && (code < dht->row_break_codes[bit_width-1])) {

            // Code to retrieve is "extracted code - previous width's break code<<1" 
            // from beginning of vmn bytes for this width
            value = dht->vmn_offset[bit_width-1][code - ((bit_width == 1) ? 0 : (dht->row_break_codes[bit_width-2] << 1))];
            
            // Remove these extracted bits from the barrel shifter and finish loop
            *bit_count -= bit_width;
            
#ifdef JPEG_DEBUG_MODE
            if (debug_enable & JPEG_DEBUG_HUFF_DECODE)
                cout << "code=0x" << hex << setw(4) << (int)code << " : value=0x" << hex << setw(4) << (int)value << endl;
#endif

            break;
        // Didn't get a match on maximum bits, so this is an error
        } else if (bit_width == JPEG_DHT_MAX_BITS) {

            cerr << "ERROR: jpeg_dht_lookup(): lookup failure" << endl;
            exit(JPEG_FORMAT_ERROR);
        }
    }

    // Decode lookup value and get additional bits if applicable...

    // By default, let's assume this isn't EOB, ZRL or a marker
    rval.marker = 0;
    rval.is_EOB = false;
    rval.is_ZRL = false;

    // Get the zeros run-length
    rval.ZRL    = (value >> 4) & 0xf;

    // Check for the special cases
    if (value == JPEG_ZRL) {

        rval.is_ZRL = true;

    } else if (value == JPEG_EOB) {

        rval.is_EOB = true;

    } else {

        // For DC, value is just an additional bit length (no ZRL nibble)
        if (is_DC)
            rval.ZRL       = 0;

        // Fetch additional bits (and remove from barrel)
        rval.amplitude = jpeg_amp_adjust( jpeg_get_bits((value & 0xf), buf, idx, barrel, bit_count, true), (value & 0xf));
    }

    return rval;
}

//-------------------------------------------------------------
// jpeg_bitmap_update()
//
// Description:
//
// Takes an RGB Ns x [8|16]x[8|16] block at JPEG position mcu_col,mcu_row
// and positions its pixels within a bitmap buffer.
//
// Parameters:
//    ptr:      pointer to block of RGB data generated from an MCU
//    mcu_row:  the block's MCU row position
//    mcu_col:  the block's MCU column position
//    Ns:       The number of components in the scan (1 or 3)
//    Hi:       The number of horizontal Y components in data (sub-sampling)
//    Vi:       The number of vertical Y components in data (sub-sampling)
//    bmp_data_ptr: pointer to the start of the bitmap's data buffer
//    X:        Size of the image's width
//    Y:        Size of the image's height
//
// Return value:
//    NONE
//

void jfif::jpeg_bitmap_update (jpeg_rgb_block_t ptr, int mcu_row, int mcu_col, int Ns, int Hi, int Vi, uint8 *bmp_data_ptr, int X, int Y)
{

    // Each row extended to align to 32 bits;
    int ext_X = BMP_WIDTH_TO_PADDED_BYTES(X);

    for (int mrow = 0; mrow < (JPEG_BLOCK_DIMENSION*Vi); mrow++) {

        for (int mcol = 0; mcol < (JPEG_BLOCK_DIMENSION*Hi); mcol++) {

            uint8 r, g, b;

            // Extract RGB values
            if (Ns == 1) {

                // When monochrome, all values the same 
                r = g = b = ptr[0][mrow>>1][mcol+((mrow&1)<<3)];

            } else {

                // Extract RGB values
                r = ptr[0][mrow][mcol];
                g = ptr[1][mrow][mcol];
                b = ptr[2][mrow][mcol];
            }

            // Calculate picture position (not accounting for padding)
            int x_pos = 8 * Hi * mcu_col + mcol;
            int y_pos = 8 * Vi * mcu_row + mrow;

            // If x_pos and y_pos not off the scale, then not an MCU padding byte
            if (x_pos < X && y_pos < Y) {

                // Flip for bitmap (which starts at the bottom)
                y_pos = Y - y_pos - 1;

                // Write to bitmap buffer
                bmp_data_ptr[y_pos*ext_X + x_pos*3 + 2] = r;
                bmp_data_ptr[y_pos*ext_X + x_pos*3 + 1] = g;
                bmp_data_ptr[y_pos*ext_X + x_pos*3 + 0] = b;
            }
        }
    }
}

//-------------------------------------------------------------
// jpeg_extract_header()
//
// Description:
//
// Parses the JFIF/JPEG header, extracting the relevant header data
// needed for processing the scan data correctly. Checks are done on
// the header for format errors or unsupported configurations.
//
// Parameters:
//    buf:      pointer to input buffer containing JFIF data
//    sptr:     pointer to SOS header structure pointer, updated to point to
//              allocated and updated scan data.
//    fptr:     pointer to frame header pointer, updated to point to frame data in buffer
//    qptr:     Pointer to quantisation table pointers
//    hptr:     pointer to huffman decode data structure pointer, updated to point to
//              allocated and update decode data.
//    dri:      pointer to an integer where DRI value will be returned
//    is_RGB:   pointer to flag to indicate RGB colour space
//
// Return value:
//    JPEG_NO_ERROR:          on successful completion
//    JPEG_MEMORY_ERROR:      memory allocation failure
//    JPEG_FORMAT_ERROR:      format violation (message on stderr)
//    JPEG_UNSUPPORTED_ERROR: valid, but not-supported (message on stderr)
//

int jfif::jpeg_extract_header(uint8 *buf, scan_header_t **sptr, frame_header_t **fptr, DQT_t *qptr, 
                              DHT_offsets_t **hptr, int *dri, bool *is_RGB)
{
    using std::cout;
    using std::cerr;
    using std::dec;
    using std::hex;
    using std::setw;
    using std::setfill;
    using std::endl;

    int buf_idx        = 0;       // Index into input data buffer
    int last_buf_idx   = 0;       // Index at end of last good marker/block (for error reporting location only)

    int  marker        = 0; 
    int  length        = 0;
    bool expecting_SOI = true;
    bool is_JFIF       = false;
    int  ptq;

    *is_RGB = false;

    do {

        // Remember starting buffer index
        last_buf_idx = buf_idx;

        // Construct the 16 bit marker
        marker       = buf[buf_idx++];
        marker       = (marker << 8) | buf[buf_idx++];

        // Decode the marker
        switch(marker) {
            
        // -----------------------
        // Start of Image
        // -----------------------

        case JPEG_MKR_SOI:

            expecting_SOI = false;
#ifdef JPEG_DEBUG_MODE
            if (debug_enable & JPEG_DEBUG_MKR_EN) 
                cout << JPEG_STR_SOI;
#endif
            break;

        // -----------------------
        // APP0 for JFIF
        // -----------------------

        case JPEG_MKR_APP0:

            length =  (buf[buf_idx] << 8) | buf[buf_idx+1];

#ifdef JPEG_DEBUG_MODE
            if (debug_enable & JPEG_DEBUG_MKR_EN) 
                cout << JPEG_STR_APP0 << (int)marker-JPEG_MKR_APP0 << " : " << dec << length << " : \"" << &buf[buf_idx+2] << "\"";
#endif

            // Check the header is JFIF. First time ID should be "JFIF", but can then be followed by an
            // extension segment "JFXX"
            if ( (is_JFIF && strncmp((const char *)&buf[buf_idx+2], JPEG_JFXX_STR, 4)) || 
                (!is_JFIF && strncmp((const char *)&buf[buf_idx+2], JPEG_JFIF_STR, 4))) {
                cerr << "ERROR: unrecognised APP0 encountered" << endl;
                return JPEG_FORMAT_ERROR;
            }

            // Flag that this is a JFIF format file
            is_JFIF = true;
            *is_RGB = false;

            buf_idx += length;

            break;

        // -----------------------
        // APP14 for Adobe
        // -----------------------

        case JPEG_MKR_APPe:

            length =  (buf[buf_idx] << 8) | buf[buf_idx+1];

#ifdef JPEG_DEBUG_MODE
            if (debug_enable & JPEG_DEBUG_MKR_EN) 
                cerr << JPEG_STR_APP0 << (int)marker-JPEG_MKR_APP0 << " : " << length << " : \"" << &buf[buf_idx+2] << "\"";
#endif

            // Check the header
            if (strncmp((const char *)&buf[buf_idx+2], JPEG_APP14_ADOBE_STR, JPEG_APP14_ADOBE_STR_LEN)) {
                cerr << "ERROR: unrecognised APP14 encountered" << endl;
                return JPEG_FORMAT_ERROR;
            }

            // Extract colour space info if not a JFIF format (which is YCbCr)
            if (!is_JFIF) {

                int colour_space;

                if ((colour_space = buf[buf_idx+JPEG_APP14_COLOUR_SPACE_OFFSET]) > 1) {
                    cerr << "ERROR: unsupported APP14 colour space encountered" << endl;
                    return JPEG_UNSUPPORTED_ERROR;
                }

                // Extracted colour space byte is 0 for RGB and 1 for YCbCr
                *is_RGB = colour_space == 0;
            }

            buf_idx += length;

            break;

        // -----------------------
        // Application Data marker
        // -----------------------

        case JPEG_MKR_APP1: case JPEG_MKR_APP2: case JPEG_MKR_APP3:
        case JPEG_MKR_APP4: case JPEG_MKR_APP5: case JPEG_MKR_APP6: case JPEG_MKR_APP7:
        case JPEG_MKR_APP8: case JPEG_MKR_APP9: case JPEG_MKR_APPa: case JPEG_MKR_APPb:
        case JPEG_MKR_APPc: case JPEG_MKR_APPd: case JPEG_MKR_APPf:

            length =  (buf[buf_idx] << 8) | buf[buf_idx+1];

#ifdef JPEG_DEBUG_MODE
            if (debug_enable & JPEG_DEBUG_MKR_EN) 
                cout << JPEG_STR_APP0 << (int)(marker-JPEG_MKR_APP0) << " : " << dec << length << " : \"" << &buf[buf_idx+2] << "\"";
#endif

            // Check that it's okay to receive this marker now
            if (expecting_SOI) {
                cerr << "ERROR: encountered unexpected marker when waiting for SOI" << endl;
                return JPEG_FORMAT_ERROR;
            }

            buf_idx += length;
            break;

        // -----------------------
        // Comment marker
        // -----------------------

        case JPEG_MKR_COM:

            length =  (buf[buf_idx] << 8) | buf[buf_idx+1];

#ifdef JPEG_DEBUG_MODE
            if (debug_enable & JPEG_DEBUG_MKR_EN) 
                cout << JPEG_STR_COM << " : " << dec << length << " : \"" << &buf[buf_idx+2] << "\"";
#endif

            // Check that it's okay to receive this marker now
            if (expecting_SOI) {
                cerr << "ERROR: encountered unexpected marker when waiting for SOI" << endl;
                return JPEG_FORMAT_ERROR;
            }

            buf_idx += length;
            break;

        // -----------------------
        // Quantisation marker
        // -----------------------

        case JPEG_MKR_DQT:

            length =  (buf[buf_idx] << 8) | buf[buf_idx+1];

#ifdef JPEG_DEBUG_MODE
            if (debug_enable & JPEG_DEBUG_MKR_EN) 
                cout << JPEG_STR_DQT << " : " << dec << length;
#endif

            // Check that it's okay to receive this marker now
            if (expecting_SOI) {
                cerr << "ERROR: encountered unexpected marker when waiting for SOI" << endl;
                return JPEG_FORMAT_ERROR;
            }

            for (int idx = 0; idx < (length-2); idx++) {

                int zdx = (idx%JPEG_DQT_TABLE_SIZE) - 1;

                // First byte of table data is table ID
                if ((idx%JPEG_DQT_TABLE_SIZE) == 0) {

                    ptq = buf[buf_idx+JPEG_DQT_OFFSET+idx] & 0xf;
                    qptr[ptq].PTq     = ptq;

                // Next 64 bytes are quantisation values. These are scaled with AAN prescale values now,
                // to avoid extra multiplication during iDCT.
                } else {

#ifdef JPEG_FAST_INT_IDCT
                    qptr[ptq].Qn[zdx] = jpeg_idescale(buf[buf_idx+JPEG_DQT_OFFSET+idx] * aanscales[zdx], PRE_DESCALE_BITS);
#else
                    qptr[ptq].Qn[zdx] = buf[buf_idx+JPEG_DQT_OFFSET+idx];
#endif

                }

#ifdef JPEG_DEBUG_MODE
                if ((debug_enable & JPEG_DEBUG_DQT_EN) && (idx%JPEG_DQT_TABLE_SIZE) == JPEG_DQT_TABLE_SIZE-1) {
                    cout << endl;
                    print_DQT((DQT_raw_pt)&buf[buf_idx+JPEG_DQT_OFFSET+idx-(JPEG_DQT_TABLE_SIZE-1)]);
                }
#endif
            }

            buf_idx += length;

            break;
 
        // -----------------------
        // Start of Frame 
        // -----------------------

        // SOF0 (for baseline DCT)---other frame types are unsupported

        case JPEG_MKR_SOF0:

            // Check that it's okay to receive this marker now
            if (expecting_SOI) {
                cerr << "ERROR: encountered unexpected marker when waiting for SOI" << endl;
                return JPEG_FORMAT_ERROR;
            }

            // Point to frame 
            *fptr = (frame_header_t*) &buf[buf_idx];

#ifdef JPEG_LIMITED_SUB_SAMPLING
            // Check sub-sampling parameters
            for (idx = 0; idx < (*fptr)->Nf; idx++) {
                if ((*fptr)->Ci[idx].HVi != JPEG_SUPPORTED_SUBSAMPLING) {
                    cerr << "ERROR: unsupported sub-sampling detected in file" < endl;
                    return JPEG_UNSUPPORTED_ERROR;
                }
            }
#else
            // Check sub-sampling parameters
            for (int idx = 0; idx < (*fptr)->Nf; idx++) {

                int Hi = (*fptr)->Ci[idx].HVi >> 4;
                int Vi = (*fptr)->Ci[idx].HVi & 0xf;

                // Check for unsupported vertical sub-sampling
                if ((idx && (Hi != JPEG_NO_SUBSAMPLING || Vi != JPEG_NO_SUBSAMPLING)) || 
                    (!idx && (Hi > JPEG_SUBSAMPLING || Hi < JPEG_NO_SUBSAMPLING || Vi > JPEG_SUBSAMPLING || Vi < JPEG_NO_SUBSAMPLING))) {

                    cerr << "ERROR: unsupported sub-sampling detected in file" << endl;
                    return JPEG_UNSUPPORTED_ERROR;

                }
            }
#endif

            // Extract block length
            length = JPEG_REORDER16((*fptr)->length);

#ifdef JPEG_DEBUG_MODE
            if (debug_enable & JPEG_DEBUG_MKR_EN) 
                cerr << JPEG_STR_SOF0 << " : " << length;

                // Print out frame contents
            if (debug_enable & JPEG_DEBUG_FRM_EN) {
                cout << endl;
                print_frame_hdr(*fptr);
            }
#endif
            // Skip passed frame
            buf_idx += length;

            break;

        // -----------------------
        // Huffman table marker
        // -----------------------

        case JPEG_MKR_DHT:

            length =  (buf[buf_idx] << 8) | buf[buf_idx+1];

#ifdef JPEG_DEBUG_MODE
            if (debug_enable & JPEG_DEBUG_MKR_EN) 
                cout << JPEG_STR_DHT << " : " << dec << length;
#endif

            // Check that it's okay to receive this marker now
            if (expecting_SOI) {
                cerr << "ERROR: encountered unexpected marker when waiting for SOI" << endl;
                return JPEG_FORMAT_ERROR;
            }

            buf_idx += length;

            // Construnct the Huffman table from the bytes
            if ((*hptr = jpeg_dht(&buf[buf_idx-length], *hptr)) == NULL) {

                cerr << "ERROR: failed call to jpeg_dht()" << endl;
                return(JPEG_MEMORY_ERROR);

            }

            break;

        // -----------------------
        // Define restart interval
        // -----------------------

        case JPEG_MKR_DRI:

            length =  (buf[buf_idx] << 8) | buf[buf_idx+1];
            *dri   =  (buf[buf_idx+JPEG_DRI_OFFSET] << 8) | buf[buf_idx+3];

#ifdef JPEG_DEBUG_MODE
            if (debug_enable & JPEG_DEBUG_MKR_EN) 
                cout << JPEG_STR_DRI << " : " << dec << length << " : " << (int)(*dri);
#endif

            // Check that it's okay to receive this marker now
            if (expecting_SOI) {
                cerr << "ERROR: encountered unexpected marker when waiting for SOI" << endl;
                return JPEG_FORMAT_ERROR;
            }

            buf_idx += length;

            break;

        // -----------------------
        // Start of scan marker
        // -----------------------

        case JPEG_MKR_SOS:

#ifdef JPEG_DEBUG_MODE
            if (debug_enable & JPEG_DEBUG_MKR_EN) 
                cout << JPEG_STR_SOS << endl;
#endif

            // Check that it's okay to receive this marker now
            if (expecting_SOI) {
                cout << "ERROR: encountered unexpected marker when waiting for SOI" << endl;
                return JPEG_FORMAT_ERROR;
            }

            // Create some space for the header (structure not overlayed on byte buffer,
            // since component specific parameter is not of fixed size, pushing
            // subsequent fields to non-predeterminate positions)
            try {
                *sptr = new scan_header_t;
            }
            catch(std::bad_alloc& ba) {
                cerr << "ERROR: memory allocation error: " << ba.what() << endl;
                return JPEG_MEMORY_ERROR;
            };

            // These fields extracted from byte buffer
            (*sptr)->length = (buf[buf_idx] << 8) | buf[buf_idx+1];
            (*sptr)->Ns     = buf[buf_idx+JPEG_SOS_NS_OFFSET];

            // Pointer to variable length parameters (determined by Ns)
            (*sptr)->p_Ci   = (scan_params_t*) &buf[buf_idx+JPEG_SOS_CI_OFFSET];
            
            // Pointer to last few parameters (known constants for baseline DCT)
            (*sptr)->p_tail = (scan_tail_t*)   &buf[buf_idx+JPEG_SOS_CI_OFFSET + ((*sptr)->Ns*2)];

#ifndef JPEG_IGNORE_SOS_TAIL_ERRORS
            // Check the values
            if ((*sptr)->p_tail->Ss != 0 || (*sptr)->p_tail->Se != JPEG_MCU_ELEMENTS-1 || (*sptr)->p_tail->Ahl != 0) {
                cerr << "ERROR: Unexpected values at end of SOS header" << endl;
                return JPEG_FORMAT_ERROR;
            }
#endif

            // Pointer to start of entropy coded data segment
            (*sptr)->p_ECS  = &buf[buf_idx+JPEG_SOS_CI_OFFSET + ((*sptr)->Ns*2) + JPEG_SOS_TAIL_SIZE];

#ifdef JPEG_DEBUG_MODE
            if (debug_enable & JPEG_DEBUG_SCAN_EN) {
                cout << endl;
                print_scan_hdr(*sptr);
            }
#endif
            // Return at start of scan, as all header data should now have been extracted
            return JPEG_NO_ERROR;

            break;

        // -----------------------
        // SOFx markers
        // -----------------------

        // Only baseline DCT supported, so whine if other SOF markers turn up
        case JPEG_MKR_SOF1: case JPEG_MKR_SOF2: case JPEG_MKR_SOF3:
        case JPEG_MKR_SOF5: case JPEG_MKR_SOF6: case JPEG_MKR_SOF7:
        case JPEG_MKR_SOF9: case JPEG_MKR_SOFa: case JPEG_MKR_SOFb:
        case JPEG_MKR_SOFd: case JPEG_MKR_SOFe: case JPEG_MKR_SOFf:
            
            cerr << "ERROR: encountered an unsupported SOF marker (0x" << hex << setw(4) << marker << "). Only baseline (SOF0) allowed." << endl;
            return JPEG_UNSUPPORTED_ERROR;

            break;

        // Reset interval markers (not expected in header)
        case JPEG_MKR_RST0: case JPEG_MKR_RST1: case JPEG_MKR_RST2: case JPEG_MKR_RST3:
        case JPEG_MKR_RST4: case JPEG_MKR_RST5: case JPEG_MKR_RST6: case JPEG_MKR_RST7:

#ifdef JPEG_DEBUG_MODE
            if (debug_enable & JPEG_DEBUG_MKR_EN) 
                cout << JPEG_STR_RST0 << (int)(marker-JPEG_MKR_RST0);
#endif

            cerr << "ERROR: encountered unexpected RSTn marker (0x" << hex << setw(4) << marker << ") in parsing header" << endl;
            return JPEG_FORMAT_ERROR;

            break;

        // -----------------------
        // End of image marker 
        // -----------------------

        // EOI not expected in header

        case JPEG_MKR_EOI:

            length =  (buf[buf_idx] << 8) | buf[buf_idx+1];

#ifdef JPEG_DEBUG_MODE
            if (debug_enable & JPEG_DEBUG_MKR_EN) 
                cout << JPEG_STR_EOI << " : " << dec << length;
#endif
            
            cerr << "ERROR: encountered unexpected EOI marker (0x" << hex << setw(4) << marker << ") in parsing header" << endl;
            return JPEG_FORMAT_ERROR;

            break;

        // -----------------------
        // Unkown marker
        // -----------------------

        default:

            if (expecting_SOI)
                cerr << "ERROR: expected SOI marker not found. Input not a JFIF/JPEG file" << endl;
            else
                cerr << "ERROR: Unrecognised or unsupported marker (" << hex << setw(4) << marker << ") at " << hex << setw(8) << last_buf_idx << endl;

            return JPEG_FORMAT_ERROR;
        }

#ifdef JPEG_DEBUG_MODE
        if (debug_enable & JPEG_DEBUG_MKR_EN) 
            cout << " at location 0x" << setfill('0') << hex << setw(8) << last_buf_idx << endl;
#endif


    } while (marker != JPEG_MKR_EOI);

    return JPEG_NO_ERROR;
}

//-------------------------------------------------------------
// jpeg_huff_decode()
//
// Description:
//
// Uses the header information extracted by jpeg_extract_header()
// to return successive 8x8 arrays of integers of Huffman/RLE 
// decoded data, that's been amplitude adjusted and dequantised and
// reverse-serpentine positioned. The data is ready for inverse DCT
// conversion. Note the pointer points to Ns arrays---e.g if Ns == 3
// Y, Cb and Cr arrays are consecutively located in memory.
//
// Parameters:
//    sptr:     pointer to scan header data
//    hptr:     pointer to huffman decode data
//    qptr:     pointer to quantisation table pointers
//    fptr:     pointer to frame header data
//    ecs_ptr:  pointer to entropy coded segments pointer (updated)
//              When *ecs_ptr == NULL, starts from beginning. Is updated after
//              each returned MCU or marker to point to next data 
//              segment, for chaining.
//    marker:   pointer to int that's updated with a marker or error code,
//              if function returns NULL
//
// Return value:
//    NULL:     Indicates a marker or error is returned (to *marker) 
//              If marker < JPEG_MARKER_MASK, then an error code else
//              a marker, as defined in jfif_local.h
//
//    non-NULL: a pointer to 8x8 array of ints with decoded data
//

int (*jfif::jpeg_huff_decode(scan_header_t *sptr, DHT_offsets_t *hptr, DQT_t *qptr, frame_header_t *fptr, 
                             uint8 *ecs_ptr[],    int *marker)) [JPEG_MCU_ELEMENTS]
{
    using std::cout;
    using std::cerr;
    using std::setw;
    using std::endl;

    // Input stream (ECS) index
    int idx = 0;

    // Huffman/RLE  decode values
    rle_amplitude_t rle;

    // Default returned marker is 'none'
    *marker = 0;

    // If follow-on pointer null, point to beginning of ECS segment
    if (*ecs_ptr == NULL)
        *ecs_ptr = sptr->p_ECS;

    // Extract sampling factors locally (ignore if no chroma components)
    int horiz_sampling = (fptr->Nf == 1) ? 1 : (fptr->Ci[0].HVi >> 4) & 0xf;
    int vert_sampling  = (fptr->Nf == 1) ? 1 : (fptr->Ci[0].HVi)      & 0xf;

    int y_arrays       = horiz_sampling*vert_sampling;
    int total_arrays   = sptr->Ns + y_arrays - 1;

    // Clear MCU data (for as many 8x8 blocks as needed)
    for (int ydx = 0; ydx < total_arrays; ydx++)
        for (int mdx = 0; mdx < JPEG_MCU_ELEMENTS; mdx++)
            mcu[ydx][mdx] = 0;
        

    // The MCU contains up to 6 elements (e.g. Y alone, or Y Cb Cr, or Y..Y Cb Cr [sub-sampled])
    for (int array = 0; array < total_arrays; array++) {

        // Pick the table for Y until the last two arrays which are the chroma arrays
        int table = (array >= y_arrays) ? array - y_arrays + 1 : 0;

        // Get Td/Ta values for this table
        int Td = ((sptr->p_Ci+table)->Tda >> 4) & 0xf;
        int Ta = ((sptr->p_Ci+table)->Tda >> 0) & 0xf;

        // Pick the quantisation table for this segment
        int Tq = fptr->Ci[table].Tq;

        // Fetch DC codeword (= length of additional bits to follow), or marker
        rle = jpeg_dht_lookup (hptr, true, Td, *ecs_ptr, &idx, &jfif_barrel, &jfif_bit_count);

        // Got a marker
        if (rle.marker) {

            // Locally check for RSTn and reset DC diff value(s)
            if ((rle.marker & 0xfff0) == JPEG_MKR_RST0) {

#ifdef JPEG_DEBUG_MODE
                if (debug_enable & JPEG_DEBUG_MKR_EN)
                    cout << "jpeg_huff_decode: reset DC" << endl;
#endif
                // Reset all the DC values
                for (int jdx = 0; jdx < JPEG_SOS_MAX_NS; jdx++)
                    current_dc_value[jdx] = 0;
            }

            *ecs_ptr += idx;
            *marker   = rle.marker;

            return NULL;

        // If not EOB then get DC value
        } else {

            // Extract raw DC value and de-quantise
            if (!rle.is_EOB && rle.amplitude && qptr[Tq].Qn[0])
                current_dc_value[table] += rle.amplitude;

#ifdef JPEG_DEBUG_MODE
            if (debug_enable & JPEG_DEBUG_AMP_EN)
                cout << "AMP(dc):  " << 0 << ": " << current_dc_value[table] << " (" << rle.amplitude << ")" << endl;
#endif


#ifdef JPEG_FAST_INT_IDCT
            // The dequantised value is also descaled as Qn value also includes AAN iDCT prescaling, 
            // only partially descaled already.
            mcu[array][0] = jpeg_idescale(current_dc_value[table] * qptr[Tq].Qn[0], SCALE_BITS-PRE_DESCALE_BITS);
#else
            mcu[array][0] = current_dc_value[table] * qptr[Tq].Qn[0];
#endif

#ifdef JPEG_DEBUG_MODE
            if (debug_enable & JPEG_DEBUG_QNT_EN)  
                cout << setw(2) << 0 << ": " << setw(6) <<  current_dc_value[table] << " " << setw(6) << qptr[Tq].Qn[0] << " " << mcu[array][0] << endl;
#endif

        }

        // Fetch AC codewords
        for (int mdx = 1; mdx < JPEG_MCU_ELEMENTS; mdx++) {

            // Lookup the code in the Huffman table, or get marker
            rle = jpeg_dht_lookup (hptr, false, Ta, *ecs_ptr, &idx, &jfif_barrel, &jfif_bit_count);

            // Shouldn't get a marker in the middle of AC data except EOI near 
            // beginning (flushed to byte boundary)
            if (rle.marker) {

                if (rle.marker == JPEG_MKR_EOI) {

                    *ecs_ptr += idx;
                    *marker   = rle.marker;

                } else {

                    cerr << "ERROR: jpeg_huff_decode: got a marker in the middle of AC data" << endl;
                    *marker = JPEG_FORMAT_ERROR;
                } 

                return NULL;

            } else if (rle.is_EOB) {

                mdx = JPEG_MCU_ELEMENTS;

            } else {

                // If there are zero run length elements simply skip index
                if (rle.ZRL)
                    mdx += rle.ZRL;

                // If not a ZRL (0xF0) code update mcu (ZRL implies a 0 value at mdx)
                if (!rle.is_ZRL) {

                    // Update MCU matrix
                    if (!rle.is_EOB && rle.amplitude && qptr[Tq].Qn[mdx]) {

#ifdef JPEG_FAST_INT_IDCT
                        // Inverse zigzag MCU index and store dequantised amplitude value. The value is also descaled
                        // as Qn values also include AAN iDCT prescaling, only partially descaled already.
                        mcu[array][jpeg_inv_zigzag[mdx]] = jpeg_idescale(rle.amplitude * qptr[Tq].Qn[mdx], SCALE_BITS-PRE_DESCALE_BITS);
#else
                        mcu[array][jpeg_inv_zigzag[mdx]] = rle.amplitude * qptr[Tq].Qn[mdx];
#endif

#ifdef JPEG_DEBUG_MODE
                       if (debug_enable & JPEG_DEBUG_AMP_EN)  
                           cout << "AMP(ac): " << setw(2) << mdx << ": " << rle.amplitude << endl;

                       if (debug_enable & JPEG_DEBUG_QNT_EN) {
                           cout << setw(2) << jpeg_inv_zigzag[mdx] << ": ";
                           cout << setw(6) << rle.amplitude << " ";
                           cout << setw(6) << qptr[Tq].Qn[mdx] << " ";
                           cout << mcu[array][jpeg_inv_zigzag[mdx]] << endl;
                       }
#endif
                    }
                } 
#ifdef DEBUG_MODE
                else {
                    if (debug_enable & JPEG_DEBUG_MKR_EN)
                        cout << "ZRL" << endl;
                }
#endif
            }
        }
    }

    // Update data pointer to next segment
    *ecs_ptr += idx;

    // Return the array data
    return mcu;
}

//-------------------------------------------------------------
// jpeg_bitmap_init()
//
// Description:
// 
// Creates a space big enough for the image being processed and
// a bitmap header, for a 24 bit bitmap, and initialises the 
// header. A pointer to this space is returned.
// 
// Parameters:
//    X:        Image width in pixels
//    Y:        Image height in pixels
//
// Return value:
//    a pointer to the bitmap space.
//

uint8* jfif::jpeg_bitmap_init(int X, int Y)
{
    // Create some space for a bitmap

    // Byte width flushed to 32 bit boundary
    int     bmp_width          = BMP_WIDTH_TO_PADDED_BYTES(X);
    int     bmp_height         = Y;

    // Allocate some space for a 24 bit bitmap (including header)
    int     bmp_size           = (bmp_width * bmp_height) + BMP_HDRSIZE;
    uint8*  bmp_ptr            = new uint8[bmp_size]();

    // Cast the bitmap start to a bitmap header
    bmhdr_t* bmp_hdr           = (bmhdr_t *)bmp_ptr;

    // Initialise bitmap file fields
    bmp_hdr->f.bfType[0]       = 'B';
    bmp_hdr->f.bfType[1]       = 'M';
    bmp_hdr->f.bfSize          = bmp_size;
    bmp_hdr->f.bfOffBits       = BMP_HDRSIZE;

    // Initialise bitmap info (unspecified fields are left as 0)
    bmp_hdr->i.biSize          = BMP_INFOHDRSIZE;
    bmp_hdr->i.biWidth         = X;
    bmp_hdr->i.biHeight        = Y;
    bmp_hdr->i.biPlanes        = 1;
    bmp_hdr->i.biBitCount      = 24;

    // Do endian swap if needed
    BMP_HDRENDIAN(bmp_hdr);

    return bmp_ptr;

}

//-------------------------------------------------------------
// jpeg_process_jfif()
//
// Description:
// 
// Takes a buffer containing JFIF data and decodes to a series
// of 8x8 RGB triplets
//
// Parameters:
//    ibuf:     pointer to the input buffer containing the JFIF data
//    obuf:     pointer to a buffer pointer, updated to point to bitmap output
//
// Return value:
//    Returns JPEG_NO_ERROR on successful completion, or JPEG_FORMAT_ERROR on 
//    unexpected data or markers.
//
int jfif::jpeg_process_jfif(uint8 *ibuf, uint8 **obuf) 
{
    using std::cout;
    using std::cerr;
    using std::hex;
    using std::setw;
    using std::endl;

    // Pointers to JPEG segments and data
    scan_header_t*  scan_header  = NULL;
    frame_header_t* frame_header = NULL;
    DHT_offsets_t*  dht_table    = NULL;
    DQT_t           dqt_table[JPEG_MAX_QUANT_TABLES];

    // Data space for DCT data and RGB data
    int rgb_data[JPEG_NUM_RGB_COLOURS][JPEG_BLOCK_DIMENSION*2][JPEG_BLOCK_DIMENSION*2];

    // Pointer for decoded scan data with Y [Cb Cr] data
    int (*scan_data_ptr)[JPEG_MCU_ELEMENTS];

    // Pointer to buffer of next data for processing (used in chaining)
    uint8 *ecs_ptr = NULL;

    // Local state for marker and RSTn checking
    int  dri = 0, marker = 0, exp_rst_marker = JPEG_MKR_RST0;
    bool expecting_rstn = false;
    
    // Counter for tracking number of MCU's processed
    int mcu_count = 0; 

    // Local status holders
    int status;
    bool is_RGB;

    // Parse JFIF header 
    if (status = jpeg_extract_header(ibuf, &scan_header, &frame_header, dqt_table, &dht_table, &dri, &is_RGB))
        return status;

    // Extract the H and V subsampling parameters locally, from the frame header
    // (ignore if no chroma components)
    int Hi = (frame_header->Nf == 1) ? 1 : frame_header->Ci[0].HVi >> 4;
    int Vi = (frame_header->Nf == 1) ? 1 : frame_header->Ci[0].HVi & 0xf;

    // Extract the image dimensions (with any necessary byte swapping)
    int Y = JPEG_REORDER16(frame_header->Y);
    int X = JPEG_REORDER16(frame_header->X);

    // The number of arrays n MCU to process is number of scan components plus extra sub-samples
    int mcu_arrays = scan_header->Ns + Hi*Vi - 1 ;

    // MCU width in pixels is 8 x horizontal sub-sampling
    int mcu_width   = 8 * Hi;

    // MCU height in pixels is 8 x vertical sub-sampling
    int mcu_height  = 8 * Vi;

    // Calculate width in whole mcus
    int X_mcus = X/mcu_width + ((X%mcu_width) ? 1 : 0);

    // Calculate total number of MCU to cover image
    int total_mcus =  X_mcus * (Y/mcu_height + ((Y%mcu_height) ? 1 : 0));

    // Create some space for bitmap, and initialise header
    uint8* bmp_ptr = jpeg_bitmap_init(X, Y);
    uint8* bmp_data_ptr = bmp_ptr + BMP_HDRSIZE;

    // Process scan data until end-of-image marker
    while (marker != JPEG_MKR_EOI) {

        // Decode entropy data
        scan_data_ptr = jpeg_huff_decode(scan_header, dht_table, dqt_table, frame_header, &ecs_ptr, &marker);

        // NULL returned on encountering a marker or error
        if (scan_data_ptr == NULL) {

            // Encountered a marker
            if (marker & JPEG_MARKER_MASK) {

                // Only expecting RSTn or EOI
                if (marker >= JPEG_MKR_RST0 && marker <= JPEG_MKR_RST7) {

                    if (marker != exp_rst_marker) {
                        cerr << "ERROR: unexpected RSTn marker sequence (got " << hex << setw(4) << marker;
                        cerr << ", expected " << hex << setw(4) << exp_rst_marker << endl;
                        return JPEG_FORMAT_ERROR;
                    }

#ifdef JPEG_DEBUG_MODE
                    if (debug_enable & JPEG_DEBUG_MKR_EN)
                         cout << "RST" << (marker - JPEG_MKR_RST0) << endl;
#endif
                    exp_rst_marker = (exp_rst_marker & 0xfff0) + (exp_rst_marker+1)%8;
                    marker = 0;
                    expecting_rstn = false;

                } else if (marker == JPEG_MKR_EOI) {

#ifdef JPEG_DEBUG_MODE
                    if (debug_enable & JPEG_DEBUG_MKR_EN)
                        cout << "EOI" << endl;
#endif
                } else {
                    cerr << "ERROR: encountered unexpected marker (0x" << hex << setw(4) << marker << ") in scan data" << endl;
                    return JPEG_FORMAT_ERROR;
                }
            // An error occured, return error code (in marker)
            } else {
                return marker;
            }

        // Received a valid MCU
        } else {

            // Should have finished by now, but make this a warning only that EOI is missing
            if (mcu_count >= total_mcus) {
#ifndef JPEG_NO_WARNINGS
                cerr << "WARNING: receiving more data when EOI expected" << endl;
#endif
                // Break out of the while loop, as if EOI, as no more data needs processing
                break;
            }
            
#ifdef JPEG_DEBUG_MODE
            if (debug_enable & JPEG_DEBUG_MCU_EN) {
                cout << "MCU " << (mcu_count % X_mcus) << "," << (mcu_count / X_mcus) << endl;
                print_MCU(scan_data_ptr, mcu_arrays);
            }
#endif

            // Perform inverse DCT for each 8x8 element and return into same buffer
            for (int scans = 0; scans < mcu_arrays; scans++)

#ifdef JPEG_FAST_INT_IDCT
                jpeg_idct((jpeg_8x8_block_t)scan_data_ptr[scans]);
#else
                jpeg_idct_slow((jpeg_8x8_block_t)scan_data_ptr[scans]);
#endif

            // Convert from scan data to RGB
            if (scan_header->Ns != 1)
                if (status = jpeg_ycc_to_rgb((jpeg_nx8x8_block_t)scan_data_ptr, rgb_data, scan_header->Ns, Hi, Vi, is_RGB))
                    return status;

            // Update bitmap data buffer with converted block
            jpeg_bitmap_update (scan_header->Ns != 1 ? rgb_data : (jpeg_rgb_block_t)scan_data_ptr, 
                                mcu_count / X_mcus, 
                                mcu_count % X_mcus, 
                                scan_header->Ns, 
                                Hi, 
                                Vi, 
                                bmp_data_ptr, 
                                X, 
                                Y);

            // Keep count of MCUs processed
            mcu_count++;

            // Flag if DRI says next data should be an RSTn marker
            if (mcu_count && dri && !(mcu_count%dri)) 
                expecting_rstn = true;

        }
    }

    free(scan_header);
    free(dht_table);

    // Return bitmap data
    *obuf = bmp_ptr;

    return JPEG_NO_ERROR;
}

//-------------------------------------------------------------
// jpeg_process_jfif_c()
// 
// Description:
//
// C linkage for jpeg_process_jfif() member of jfif class
// 
// Parameters:
//    ibuf:         pointer to the input buffer containing the JFIF data
//    obuf:         pointer to a buffer pointer, updated to point to bitmap output
//    debug_enable: Debug control (whencompiled with JPEG_DEBUG_MODE)
//
// Return value:
//    Returns JPEG_NO_ERROR on successful completion, or JPEG_FORMAT_ERROR on 
//    unexpected data or markers.
//

extern "C" int jpeg_process_jfif_c (uint8 *ibuf, uint8 **obuf, int debug_enable)
{
    // JPEG decoder object
    jfif decoder(debug_enable);

    // Call decode method and return pointer to the bitmap and/or status 
    return decoder.jpeg_process_jfif(ibuf, obuf);
}

