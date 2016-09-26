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
// $Id: jfif_gtk.h,v 1.1 2014-03-01 15:51:37 simon Exp $
// $Source: /home/simon/CVS/src/HDL/jfif/sw/jpeg_cpp/src/jfif_gtk.h,v $
//
//=============================================================

#include <gtk/gtk.h>

#ifndef _GTK_H_
#define _GTK_H_

#define MAX_DISPLAY_WIDTH  800
#define MAX_DISPLAY_HEIGHT 600

#define MIN_DISPLAY_WIDTH  200
#define MIN_DISPLAY_HEIGHT 200

// Default is for no border
#define BORDER_WIDTH 0

#ifndef JPEG_NO_GRAPHICS
extern void jpeg_display_bmp_file (int argc, char *argv[], unsigned char *ibuf, int X, int Y);
#endif

#endif
