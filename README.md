## bchunk v1.2.1 - BinChunker for Windows / Unix / Linux

binchunker converts a CD image in a ".bin / .cue" format (sometimes ".raw / .cue") to a set of .iso and .cdr tracks. The bin/cue format is used by some popular non-Unix cd-writing software, but is not supported on most other CD burning programs. A lot of CD/VCD images distributed on the Internet are in BIN/CUE format, I've been told.

This is a Windows/Unix/C rewrite of the fine BinChunker software. The non-Unix version of BinChunker has been obsoleted by the CD-R/W burning software FireBurner, which can be found at [www.fireburner.com](http://www.fireburner.com/). FireBurner does a lot more than BinChunker did, but can be used to do exactly the same even without a burner device installed. Thanks go to Bob Marietta <marietrg ( atz ] slu.edu>, the author of BinChunker, for the extensive help, documentation and letting me look at his Pascal/Delphi source code!</marietrg ( atz ] slu.edu>

Enhancements provided by:

- Colas Nahaboo <Colas ( atz ] Nahaboo.com>, 1999
- Godmar Back <gback ( atz ] cs.utah.edu>, 2001
- Matthew Green <mrg ( atz ] eterna.com.au>, 2003
		
[http://he.fi/bchunk/](http://he.fi/bchunk/)

[https://mzex.wordpress.com/2015/07/29/bchunk%E3%82%92windows%E3%81%A7%E4%BD%BF%E3%81%88%E3%82%8B%E3%82%88%E3%81%86%E3%81%AB%E3%81%99%E3%82%8B/](https://mzex.wordpress.com/2015/07/29/bchunk%E3%82%92windows%E3%81%A7%E4%BD%BF%E3%81%88%E3%82%8B%E3%82%88%E3%81%86%E3%81%AB%E3%81%99%E3%82%8B/)

[http://blog.extramaster.net/2016/08/bchunk-v121-binchunker-for-windows.html](http://blog.extramaster.net/2016/08/bchunk-v121-binchunker-for-windows.html)

### What on earth is this stuff?

The .iso track contains an ISO file system, which can be mounted through a loop device on Linux systems, or written on a CD-R using cdrecord. The .cdr tracks are in the native CD audio format. They can be either written on a CD-R using cdrecord -audio, or converted to WAV (or any other sound format for that matter) using sox. bchunk 1.1.0 (and later versions) can also output audio tracks in WAV format.

bchunk compiles and runs on any host with an ANSI C compiler, and somewhat POSIX.1 compliant environment. It has been tested on the following platforms:

- Linux 2.0, i686, glibc 2.0.7, gcc 2.7.2.3 (RedHat 5.1)  
- Linux 2.2, i686, glibc 2.1.1, egcs 1.1.2 (RedHat 6.0)  
- Linux 2.4, i686, glibc 2.2, gcc 2.96 (RedHat 7.0)  
- Linux 2.6, i686, glibc 2.3, gcc 3.2.2 (RedHat 9.0)  
- Linux 2.6, i686, glibc 2.3.5, gcc 4.0.2 (Debian sarge, etch)  
- Solaris 2.5.1, SPARC, gcc 2.7.2  
- Solaris 2.6, SPARC, gcc 2.8.1  
- Digital Unix 4.0c, Alpha, DEC C V5.2-033  
- SGI IRIX 6.5.7m, MIPS R4600 IP22, gcc 2.95.2  
- Mac OS X 10.1.4, gcc 2.95.2
- Windows 10, MSVC 14.0

OpenBSD, FreeBSD and NetBSD all have a port of bchunk available in their respective archives for a very good set of hardware platforms.

Debian GNU/Linux users can install bchunk 1.2.0 using 'apt-get install bchunk', as bchunk is maintained as a [Debian package](http://packages.debian.org/bchunk).

bchunk has apparently also been successfully compiled on BeOS without source modifications and the i386 binary can be [downloaded here](http://bezip.de/app/1340/).

### What's new?

##### 1.2.1 (08 August 2016):

*   Added Windows Support

##### 1.2.0 (29 Jun 2004):

*   Man page patch from the openbsd port
*   -r option for MODE2/2352 MPEG/VCD support

##### 1.1.1 (7 Aug 2001):

*   Error checking while reading the bin file
*   psxtruncate was always enabled by default - made -p work
*   Fixed .spec file to compile on other distributions

##### 1.1.0 (30 Jun 2001):

*   Please note that 1.1.0 has NOT been extensively tested. Feedback and bug reports welcome.
*   MODE2/2352, PSX support
*   WAV audio file output (byte order fixing code might break compiling on some platforms)
*   Audio file byte order swapping
*   man page, make install
*   More error checking

##### 1.0.0 (Oct 11 1998):

*	Initial release.
    
### Tips and tricks:

**To record an MP3 image with a CUE sheet to an audio CD** (for example the mixes at [sicktracks.com](http://sicktracks.com/)):

1.  convert the .mp3 to a raw PCM audio file:  
    `$ mpg123 -sv sicktracks8.mp3 > sicktracks8.pcm`
2.  split into tracks according to the CUE file:  
    `$ bchunk sicktracks8.pcm sicktracks8.txt tracks`
3.  burn it:  
    `$ cdrecord -v -dao -audio tracks??.cdr`

If you only get loud noise on the tracks, try either -swab on cdrecord or -s on bchunk to swap the byte order.

### License:

`This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.`

`This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.`

`You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.`