//=============================================================
//
// Copyright (c) 2010-2026 Simon Southwell
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
//=============================================================

#include <gtk/gtk.h>

#ifndef _GTK_H_
#define _GTK_H_

#define MAX_DISPLAY_WIDTH  1200
#define MAX_DISPLAY_HEIGHT  900

#define MIN_DISPLAY_WIDTH   200
#define MIN_DISPLAY_HEIGHT  200

// Default is for no border
#define BORDER_WIDTH          0

#ifndef JPEG_NO_GRAPHICS
extern void jpeg_display_bmp_file (int argc, char *argv[], const unsigned char *ibuf, const int X, const int Y);
extern void jpeg_display_img_data (int argc, char *argv[], const uint8_t       *data, const int X, const int Y);
#endif

#endif
