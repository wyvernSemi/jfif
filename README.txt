JFIF
----

JFIF is a JFIF and JPEG file decoder, converting these compressed file types to
uncompressed 24 bit windows bitmaps. The resultant bitmap can also be 
optionally displayed in a popup window, scaled to a maximum display area of
800x600, for validating the conversion.

USAGE
-----
Usage: jfif [-h] [-d] [-i <filename>] [-o <filename>]
    -h display help message
    -d display generated bitmap file's image in a window
    -i define input filename (default test.jpg)
    -o define output filename (default test.bmp)
    -D specify debug enable value (default off)

INSTALL
-------

On Windows
..........

The following is applicable to Windows 10, but is easily adapted for
other versions of windows.

Clone the github repository to you chosen location. A pre-built executable
(jfif.exe), built from MSVC, is in the top level folder. If you don't already
have GTK+ 2.0 installed, then this needs to be done before the executable will
run. Try the following link:

    https://download.gnome.org/binaries/win64/gtk+/2.22/gtk%2B-bundle_2.22.0-20101016_win64.zip


On Linux
........

Clone the github repository to you chosen location. A pre-bult executable
(jfif) is in the top level directory. If you don't already have GTK+ 2.0
installed, then this needs to be done before the executable will run:

    sudo apt-get install gtk2.0


RUNNING
-------

Under windows, JFIF can be run from a windows command prompt window. E.g.

  jfif -d -i test.jpg


COMPILING
---------

If you wish to recompile the source code under MinGW or Linux, using the
makefile, then use the following command:

  make [SLOWIDCT=no]

Note that the option "SLOWIDCT=no" is only available if the jfif_idct.[ch]
files are available. The generated executable will be output to the build
folder.

If the bundle was unzipped into C:\Tools\gtk+ then the makefile will auto-
matically pick this up if wishing to compile. Otherwise you can use 

    make -DGTKBINDIR=<path to your gtk+ bin>
    
or update the variable in the makefile itself. Modify your PATH to include 
c:\Tools\gtk+\bin (or to wherever you installed the package).

For Microsoft Visual Studio (MSVC) 2019, a solution file (msvc/jpeg.sln) and
associated files, are available for compilation, which works with the community
version, as well as he subscriptions editions.

Note that, to compile under MSVC or MinGW, GTK+ (including headers) must be
installed (at C:\Tools\gtk+). If this is not the case, then JPEG_NO_GRAPHICS
must be defined as a Preprocessor definition, and the '-d' option will not be
available. 

Any of the executables generated under MinGW will still run in a native Windows 
environment (i.e. a command prompt window), so long as the runtime libraries
are available and in the user's path.

For more information see:
    http://www.anita-simulators.org.uk/wyvernsemi/jpeg/jfif_sw.htm.

Simon Southwell
simon@anita-simulators.org.uk

