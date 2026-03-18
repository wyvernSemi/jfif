##############################################################
#
# Copyright (c) 2010-2014 Simon Southwell
#
# Date: 18th January 2010
#
# This file is part of JFIF.
#
# JFIF is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# JFIF is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with JFIF. If not, see <http://www.gnu.org/licenses/>.
#
##############################################################
# Makefile for jfif, for both linux and MinGW/Cygwin (Windows)
#
# Options:
#
# SLOWIDCT=yes|no     Compile with slow integer algorithm (default no)
# FLOATIDCT=yes|no    Compile slow algorithm with floating point (SLOWIDCT=yes only, default no)
# DEBUGMODE=yes|no    Include debug features (default no)
#
##############################################################


USRFLAGS           =

# By default, compiling for fast iDCT. Use "make SLOWIDCT=yes"
# if slow idct (optionally floating point) required

SLOWIDCT           = no

# Set FLOATIDCT=yes for floating point iDCT (ignored when SLOWIDCT=no)

FLOATIDCT          = no

# Set DEBUGMODE=yes for compilation with debug featured (adds -D option)

DEBUGMODE          = no

BUILDDIR           = ./build
SRCDIR             = ./src

# Targets to build

EXETARGET          = $(BUILDDIR)/jfif
LIBTARGET          = $(BUILDDIR)/libjfif.a

# All the include files

INCLFILES          = $(SRCDIR)/jfif.h            \
                     $(SRCDIR)/jfif_idct.h       \
                     $(SRCDIR)/jfif_class.h      \
                     $(SRCDIR)/jfif_local.h      \
                     $(SRCDIR)/jfif_gtk.h        \
                     $(SRCDIR)/bitmap.h          \
                     $(SRCDIR)/jpeg_dct_cos.h

# Define some compile variables based on the SLOWIDCT and FLOATIDCT settings

ifeq ($(SLOWIDCT), no)
  ifeq ($(FLOATIDCT), no)
    IDCTCFLAG      = -DJPEG_FAST_INT_IDCT
  else
    IDCTCFLAG      =
  endif
else
  IDCTCFLAG        = -DJPEG_DCT_INTEGER
endif

# All the object files
OBJMAIN            = obj/jfif_main.o
OBJFILES           = obj/jfif.o                  \
                     obj/jfif_gtk.o              \
                     obj/jfif_idct.o

# Select if to compile with verbose debug output, based on DEBUGMODE,
# which adds the "-D<debug mask> option"

ifeq ($(DEBUGMODE), no)
  DEFDEBUG         =
else
  DEFDEBUG         = -DJPEG_DEBUG_MODE
endif

# Swap over (or override on the cmd line) for debug symbol compilation
#COMMOPTS    = -g
COMMOPTS           = -ffast-math -finline-functions -funroll-loops -O4

# Use the GNU compiler
CC                 = gcc
CPP                = g++

# Default pre-processor definitions

DEFINES            = $(IDCTCFLAG)      \
                     $(DEFDEBUG)

GTKFLAGS           = $(shell pkg-config --cflags gtk+-3.0)

# C compilation flags
CFLAGS             = $(COMMOPTS) $(USRFLAGS) -I$(SRCDIR) $(DEFINES) $(GTKFLAGS)


# In MinGW/Cygwin, the pkg-config command *must* be the last on the line,
# (more specifically, the ` characters) so the link rule must reflect this.

# Just required library references
LDFLAGS            = 

#LDLIBS             = $(shell pkg-config --libs gtk+-3.0)
LDLIBS             = -lgtk-3           \
                     -lgdk-3           \
                     -lcairo           \
                     -lgdk_pixbuf-2.0  \
                     -lgobject-2.0

##########################################################
# Dependency definitions
##########################################################

all: $(EXETARGET) $(LIBTARGET)

##########################################################
# Linking rules
##########################################################
#
# Remember: don't move $(LDFLAGS) from being last, for
# MinGW/Cygwin
#

$(EXETARGET): $(OBJMAIN) $(OBJFILES) makefile
	@$(CPP) $(OBJMAIN) $(OBJFILES) -o $@ $(LDLIBS) $(LDFLAGS)

$(LIBTARGET): $(OBJFILES) makefile
	@ar -c -r $@ $(OBJFILES)

##########################################################
# Compilation rules
##########################################################

#
# Dependencies not optimal for includes, but
# a generic and concise rule instead.
#
obj/%.o : $(SRCDIR)/%.c $(INCLFILES) makefile
	@$(CC)  -c $< -o $@ $(CFLAGS)

obj/%.o : $(SRCDIR)/%.cpp $(INCLFILES) makefile
	@$(CPP) -c $< -o $@ $(CFLAGS)

##########################################################
# Clean up rules
##########################################################

.PHONY : clean
clean:
	@rm -f $(EXETARGET) $(EXETARGET).exe $(LIBTARGET) obj/*.o

