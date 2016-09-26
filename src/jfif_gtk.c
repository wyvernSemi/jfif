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
// $Id: jfif_gtk.c,v 1.1 2014-03-01 15:51:37 simon Exp $
// $Source: /home/simon/CVS/src/HDL/jfif/sw/jpeg_cpp/src/jfif_gtk.c,v $
//
//=============================================================

#include <gtk/gtk.h>
#include "jfif.h"
#include "bitmap.h"
#include "jfif_gtk.h"

#ifndef JPEG_NO_GRAPHICS

static int display_width  = MAX_DISPLAY_WIDTH;
static int display_height = MAX_DISPLAY_HEIGHT;

static int image_width;
static int image_height;

static GdkPixbuf *pxbuf;

//-------------------------------------------------------------
// jpeg_on_drawarea_expose()
//
// Description:
//
// Callback function invoked on drawable area exposed to paint
// the pixmap
//
static gboolean jpeg_on_drawarea_expose (GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{

    int dest_x=BORDER_WIDTH, dest_y = BORDER_WIDTH;
    int scale_width, scale_height;
    float scale;

    // Calculate the scaling of the image (if any)
    scale = (float)image_width / display_width;
    scale = (scale < ((float)image_height/display_height)) ? ((float)image_height/display_height) : scale;
    scale = (scale < 1.0) ? (float)1.0 : scale;

    // Calculate the scaled width/height
    scale_width  = (int)((float)image_width  / scale);
    scale_height = (int)((float)image_height / scale);


    // If scaled dimension less than window, centre in the window
    if (scale_width < display_width) 
        dest_x += (display_width-scale_width)/2;

    if (scale_height < display_height) 
        dest_y += (display_height-scale_height)/2;

    // Draw the pixbuf
    gdk_draw_pixbuf (widget->window,
                     widget->style->fg_gc[GTK_STATE_NORMAL],
                     pxbuf, 
                     0, 
                     0,
                     dest_x, 
                     dest_y, 
                     -1, 
                     -1, 
                     GDK_RGB_DITHER_MAX, 
                     0, 
                     0);

    return TRUE;
}

//-------------------------------------------------------------
// jpeg_delete_event()
//
// Description:
//
// Callback function called on a delte event
//
static gboolean jpeg_delete_event (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    // Tell engine we want a destroy event
    return FALSE;
}

//-------------------------------------------------------------
// jpeg_destroy()
//
// Description:
//
// Callback function called on a destroy event
//
static void jpeg_destroy (GtkWidget *widget, gpointer data)
{
    // Quit the event processing for the window and close
    gtk_main_quit();
}

//-------------------------------------------------------------
// jpeg_display_bmp_file()
//
// Description:
//
// Main routine called by application to display a bitmap file
// in a window. The bitmap will be scaled if either dimension
// is larger than given maximums. The routine does not decode
// the X and Y dimensions from the bitmap file, but expects
// them to be supplied as parameters from the calling application.
//
// Parameters:
//    argc:     number of arguments in argv parameter
//    argv:     passed on GTK/GDK parameters from command line (if any)
//    fname:    name of bitmap to display
//    X:        width dimension of bitmap image
//    Y:        height dimension of bitmap image
//
// Return Value:
//    NONE
//
void jpeg_display_bmp_file (int argc, char *argv[], unsigned char *fname, int X, int Y)
{
    GtkWidget *window, *drawarea;
    int draw_height, draw_width;

    GError *gtk_error=NULL;
    GdkColor colour;

    // Export the arguments for callback visibility
    image_width  = X;
    image_height = Y;

    // If image is smaller than the maximum window, resize the display around the image dimensions
    // (but only to a minimum)
    if ((image_width < MAX_DISPLAY_WIDTH) && (image_height < MAX_DISPLAY_HEIGHT)) {
        display_width  = (image_width  > MIN_DISPLAY_WIDTH)  ? image_width  : MIN_DISPLAY_WIDTH;
        display_height = (image_height > MIN_DISPLAY_HEIGHT) ? image_height : MIN_DISPLAY_HEIGHT;

    // If image is to be scaled, then adjust the display window to be the same aspect ratio
    } else {
        // Landscape
        if (((float)image_width/(float)image_height) < ((float)MAX_DISPLAY_WIDTH/(float)MAX_DISPLAY_HEIGHT)) 
            display_width = (int)((float)MAX_DISPLAY_HEIGHT*((float)image_width/(float)image_height));
        // Portrait
        else
            display_height = (int)((float)MAX_DISPLAY_WIDTH/((float)image_width/(float)image_height));
    }

    // If image dimensions are less than the display, use the image dimension for the pixmap scaling
    // (so as not to stretch)
    draw_height = (image_height < display_height) ? image_height :  display_height;
    draw_width  = (image_width  < display_width)  ? image_width  :  display_width;

    // Initialise GTK
    gtk_init (&argc, &argv);

    // Get a window
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    // Get an area to draw in, and set the size
    drawarea = gtk_drawing_area_new ();
    gtk_widget_set_size_request (drawarea, display_width+BORDER_WIDTH*2, display_height+BORDER_WIDTH*2);

    // Set drawable's background colour (to a light grey)
    gdk_color_parse ("#e0e0e0", &colour);
    gtk_widget_modify_bg (drawarea, GTK_STATE_NORMAL, &colour);

    // Create a pixmap from the bitmap file
    if ((pxbuf = gdk_pixbuf_new_from_file_at_size ((const char*)fname, draw_width, draw_height, &gtk_error)) == NULL) {
        // If NULL returned, deisplay error information and return
        fprintf(stderr, "ERROR: jpeg_display_bmp_file: %s : %d : %d : %s\n", fname, gtk_error->domain, 
                                                                                    gtk_error->code, 
                                                                                    gtk_error->message);
        return;
    }

    // Put drawing area in the window
    gtk_container_add (GTK_CONTAINER (window), drawarea);

    // Register callback to "jpeg_on_drawarea_expose" whe drawing area has an expose-event
    gtk_signal_connect (GTK_OBJECT(drawarea), "expose-event", GTK_SIGNAL_FUNC(jpeg_on_drawarea_expose), NULL);

    // Register callbacks for closing the window
    gtk_signal_connect (GTK_OBJECT(window),   "delete_event", GTK_SIGNAL_FUNC(jpeg_delete_event),    NULL);
    gtk_signal_connect (GTK_OBJECT(window),   "destroy",      GTK_SIGNAL_FUNC(jpeg_destroy),         NULL);

    // Display the window
    gtk_widget_show_all (window);

    // Process events
    gtk_main ();

}

#endif

