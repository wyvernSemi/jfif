//=============================================================
// 
// Copyright (c) 2014 Simon Southwell
// All rights reserved.
//
// Date: 7th February 2014
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
// $Id: jfif_main.c,v 1.3 2016-09-26 06:36:34 simon Exp $
// $Source: /home/simon/CVS/src/HDL/jfif/sw/jpeg_cpp/src/jfif_main.c,v $
//
//=============================================================

#include <stdio.h>
#include <stdlib.h>

#include "jfif.h"
#include "jfif_local.h"
#include "bitmap.h"

#ifndef JPEG_NO_GRAPHICS
#include "jfif_gtk.h"
#endif

#ifndef TRUE
#define TRUE (1==1)
#define FALSE (1=0)
#endif

// *************************************************************
// Test program to read in a JFIF file to a buffer, and call
// the JPEG routine to decode the data. Default input filename
// is "test.jpg", but a -i option can be used to override this.
// Output is to another file (test.bmp by default), configurable
// with -o option.
//

#ifdef WIN32
// MSVC doesn't have getopt, so declare hooks to bundled in version
extern int getopt(int nargc, char** nargv, char *ostr);
extern char *optarg;
#else
#include <getopt.h>
#endif


int main (int argc, char **argv)
{
    FILE     *ifp, *ofp;
    char*    ibuf;
    uint8*   obuf;
    int      c, current_bufsize = 4096, idx, status, fsize;
    char*    ifname = INPUT_FILENAME;
    char*    ofname = OUTPUT_FILENAME;
    bmhdr_t* bmp_hdr;
    int      debug_enable = 0;

#ifndef JPEG_NO_GRAPHICS
    int      display_RGB = FALSE;
#endif

    // Link to getopts

    int option;
    char option_str[20];

    // Process the command line options 
#ifdef JPEG_NO_GRAPHICS
    sprintf(option_str, "%s", "hi:o:D:");
#else
    sprintf(option_str, "%s", "hdi:o:D:");
#endif
    while ((option = getopt(argc, argv, option_str)) != EOF) {
        switch(option) {
        case 'i':
            ifname = optarg;
            break;

        case 'o':
            ofname = optarg;
            break;
#ifndef JPEG_NO_GRAPHICS
        case 'd':
            display_RGB = TRUE;
            break;
#endif

#ifdef JPEG_DEBUG_MODE
        case 'D':
            debug_enable = (int) strtol(optarg, NULL, 0);
            break;
#endif

        case 'h':
        case '?':
            fprintf(stderr, "Usage: jfif [-h] [-i <filename>] [-o <filename>]"
#ifndef JPEG_NO_GRAPHICS
                                             " [-d]"
#endif

#ifdef JPEG_DEBUG_MODE
                            " [-D <debug value>]"
#endif
                            "\n"
                            "    -h display help message\n"
#ifndef JPEG_NO_GRAPHICS
                            "    -d display generated bitmap file's image in a window\n"
#endif
                            "    -i define input filename (default test.jpg)\n"
                            "    -o define output filename (default test.bmp)\n"
#ifdef JPEG_DEBUG_MODE
                            "    -D specify debug enable value  (default off)\n"
#endif
                            );
            return JPEG_USER_INPUT_ERROR;
            break;

        }
    }

    // Check user input validity
    if ((status = strcmp(ifname, ofname)) == 0) {
        fprintf(stderr, "ERROR: input and output filenames identical\n");
        return JPEG_USER_INPUT_ERROR;
    }

    // Open file input file for reading
    if ((ifp = fopen(ifname, "rb")) == NULL) {
        fprintf(stderr, "ERROR: could not open %s for reading\n", ifname);
        return JPEG_FILE_ERROR;
    }

    // Create a buffer for input data
    if ((ibuf =(char *) malloc(current_bufsize)) == NULL) {
        fprintf(stderr, "ERROR: memory allocation failed\n");
        return JPEG_MEMORY_ERROR;
    }

    // Read data from file into a buffer
    idx = 0;
    while((c = getc(ifp)) != EOF) {
        ibuf[idx++] = c;
        if (idx == current_bufsize) {
            current_bufsize += DEFAULT_BUFSIZE;
            if ((ibuf = (char *)realloc(ibuf, current_bufsize)) == NULL) {
                fprintf(stderr, "ERROR: memory re-allocation failed\n");
                return JPEG_MEMORY_ERROR;
            }
        }
    }

    // Input file is finished with
    fclose(ifp);

#ifdef JPEG_DEBUG_MODE
    if (debug_enable & JPEG_DEBUG_MAIN_EN)
        printf("Read %d bytes\n", idx);
#endif

    // Decode jpeg input buffer, and return bitmap data location into obuf
    if (status = jpeg_process_jfif_c((uint8 *)ibuf, &obuf, debug_enable)) {
        return status;
    }

    // Map bitmap header over data buffer (to extract info)
    bmp_hdr = (bmhdr_t *)obuf;

    // Extract total number of bytes in bitmap
    fsize = BMP_SWPEND32(bmp_hdr->f.bfSize);

    // Open file for writing bitmap data
    if ((ofp = fopen(ofname, "wb")) == NULL) {
        fprintf(stderr, "ERROR: could not open %s for writing\n", ofname);
        return(JPEG_FILE_ERROR);
    }

#ifdef JPEG_DEBUG_MODE
    if (debug_enable & JPEG_DEBUG_MAIN_EN)
        printf("main(): writing data to output file\n");
#endif

    // write bitmap to file
    for (idx = 0; idx < fsize; idx++)
        fputc(obuf[idx], ofp);

    // Ensure data is written to disk before any display window
    fflush(ofp);
    fclose(ofp);

#ifndef JPEG_NO_GRAPHICS
    // Display written bitmap in a window, if requested
    if (display_RGB)
        jpeg_display_bmp_file (argc, argv, (unsigned char*)ofname, BMP_SWPEND32(bmp_hdr->i.biWidth), BMP_SWPEND32(bmp_hdr->i.biHeight));
#endif        

    // Completed successfully
    return JPEG_NO_ERROR;
}

