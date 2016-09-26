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

INSTALL
-------

For Cygwin on Windows
+++++++++++++++++++++

The following is applicable to Windows 7, but is easily adapted for
previous versions of windows.

Run the setup_jfif.exe executable and extract the files to your chosen
location (the default is c:\Program Files (x86)\jfif). Then, if you
don't already have GTK+ 2.0 installed, modify your PATH environment variable as 
described below:

1. Right Clock on the 'My Computer' icon on your desktop
   and select 'Properties'

2. Click on 'Advanced System Settings'

3. Click on 'Environment Variables...'

4. In the 'System variables' window, locate and select the variable 'Path',
   and click on 'Edit...'

5. Append to the end of the existing value. e.g.:

   <EXISTING Path value>;C:\Program Files (x86)\jfif\build

   (substitute your install location if not C:\Program Files (x86)\jfif.) Note
   the semi-colon as the first character to add, to delineate from the
   previous value(s).

6. Click 'OK', and for the previous two opened windows, and then close
   the properties window.

RUNNING
-------

Under windows, JFIF can be run from the DOS cmd shell (cmd.exe), which is 
located in C:\Windows\system32, or run from the start menu (look in Accessories
for 'Command Prompt'). Under Linux/Un*x, the program is run from a terminal, 
such as xterm. E.g.:

  jfif -d -i test.jpg


COMPILING
---------

If you wish to recompile the source code under Cygwin, Linux or UN*X,
then use the following command:

  make [SLOWIDCT=no]

Note that the option "SLOWIDCT=no" is only available if the jfif_idct.[ch]
files are available.

For windows MSCV 2010, a solution file (msvc/jfif.sln) and associated files are 
available for compiling under Microsoft Visual Studio 2010, which works with 
the free Express version, as well as he subscriptions editions.

Note that to compile under MSVC GTK+ (including headers) must be installed 
(at C:\Tool\gtk+). If this is not the case, then JPEG_NO_GRAPHICS must be 
defined as a Preprocessor defintion, and the '-d' option will not be available. 

Any of the executables generated under Cygwin will still run in a native Windows 
environment (i.e. a command prompt window), so long as the runtime libraries are
available, and in the user's path.

For more information see http://www.anita-simulators.org.uk/wyvernsemi/jpeg/jfif_sw.htm.

Simon Southwell
simon@anita-simulators.org.uk

