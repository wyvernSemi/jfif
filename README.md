# JFIF


JFIF is a JFIF and JPEG file decoder, converting these compressed file types to uncompressed 24 bit windows bitmaps. The resultant bitmap can also be optionally displayed in a popup window, scaled to a maximum display area of 800x600, for validating the conversion.

# INSTALL

## On Windows

The following is applicable to Windows 10, but is easily adapted for other versions of windows.

Clone the github repository to you chosen location. A pre-built executable (<tt>jfif.exe</tt>), built from MSVC, is in the top level folder. If you don't already have GTK+ 2.0 installed, then this needs to be done before the executable will run. Try the following link:

    https://download.gnome.org/binaries/win64/gtk+/2.22/gtk%2B-bundle_2.22.0-20101016_win64.zip

## On Linux

Clone the github repository to you chosen location. A pre-bult executable (<tt>jfif</tt>) is in the top level directory. If you don't already have GTK+ 2.0 installed, then this needs to be done before the executable will run:

    sudo apt-get install gtk2.0

## Running

Under windows, JFIF can be run from a windows command prompt window. E.g.

    jfif -d -i test.jpg
  
The full usage for the program is:

    Usage: jfif [-h] [-d] [-i ] [-o ]
        -h display help message
        -d display generated bitmap file's image in a window
        -i define input filename (default test.jpg)
        -o define output filename (default test.bmp)

JFIF is simple to use. Just typing "<tt>jfif</tt>" (or <tt>jfif.exe</tt>) will result in a file <tt>test.jpg</tt> being decoded (if exists), and a bitmap output test.bmp be written. The <tt>-i</tt> and <tt>-o</tt> options are used to alter the default input and output filenames. The resultant bitmap can also be optionally displayed in a popup window, scaled to a maximum display area of 800x600, for validating the conversion by eye, using the <tt>-d</tt> option. This is generated from the actual bitmap file rather than internal memory to guarantee no additional artifacts in bitmap generation are missed in the display. This delays the display of the file a fraction, but in the interests model integrity.

## Compiling

If you wish to recompile the source code under MinGW or Linux, using the makefile, then use the following command:

    make [SLOWIDCT=no]

Note that the option <tt>SLOWIDCT=no</tt> is only available if the <tt>jfif_idct.[ch]</tt> files are available. The generated executable will be output to the build folder.

If the bundle was unzipped into <tt>C:\Tools\gtk+</tt> then the <tt>makefile will</tt> automatically pick this up if wishing to compile. Otherwise you can use 

    make -DGTKBINDIR=<path to your gtk+ bin>
    
or update the variable in the makefile itself. Modify your PATH to include c:\Tools\gtk+\bin (or to wherever you installed the package).

For Microsoft Visual Studio (MSVC) 2019, a solution file (<tt>msvc/jpeg.sln</tt>) and associated files, are available for compilation, which works with the community version, as well as he subscriptions editions.

Note that, to compile under MSVC or MinGW, GTK+ (including headers) must be installed (at <tt>C:\Tools\gtk+</tt>). If this is not the case, then <tt>JPEG_NO_GRAPHICS</tt> must be defined as a Preprocessor definition, and the <tt>-d</tt> option will not be available. 

Any of the executables generated under MinGW will still run in a native Windows  environment (i.e. a command prompt window), so long as the runtime libraries are available and in the user's path.

## JPEG/JFIF Unsupported Features

The full JPEG standard is broad in it's scope, with alternatives and enhancing features optionally available. The jfif program is intended to be a starting point for a hardware implementation, and support of all the features of the standard is not practical and not really desirable. For instance, the compression technique, post-DCT, can be a Huffman table based algorithm, or an Arithmetic coding algorithm. Also there are extended and Progressive DCT formats, as well as a lossless compression mode. jfif has been limited to supporting a sub-set of these options—chosen, being a sub-set with the broadest overlap with real jpeg encoded data, whilst being most likely to be suitable in hardware decoding applications. The following list gives details of the known unsupported features.

* Extended and Progressive DCT
* All differential DCT
* All Arithmetic coding modes
* Lossless Compression
* Image components (Nf) greater than 4
* Define number of Lines (DNL)/multiple scans in a frame
* Horizontal sub-sampling factor (Hi) > 2
* Vertical sub-sampling factor (Vi) > 2

For more information see:
    http://www.anita-simulators.org.uk/wyvernsemi/jpeg/jfif_sw.htm

Simon Southwell
simon@anita-simulators.org.uk

