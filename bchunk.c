
 /*
  *	binchunker for Unix
  *	Copyright (C) 1998-2004  Heikki Hannikainen <hessu@hes.iki.fi>
  *
  *  This program is free software; you can redistribute it and/or modify
  *  it under the terms of the GNU General Public License as published by
  *  the Free Software Foundation; either version 2 of the License, or
  *  (at your option) any later version.
  *
  *  This program is distributed in the hope that it will be useful,
  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *  GNU General Public License for more details.
  *
  *  You should have received a copy of the GNU General Public License
  *  along with this program; if not, write to the Free Software
  *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#if defined(_WIN32)
#include "unistd.h"
#else
#include <unistd.h>
#endif

#define VERSION "1.2.1"
#define USAGE "Usage: bchunk [-v] [-r] [-p (PSX)] [-w (wav)] [-s (swabaudio)]\n" \
        "         <image.bin> <image.cue> <basename>\n" \
	"Example: bchunk foo.bin foo.cue foo\n" \
	"  -v  Verbose mode\n" \
	"  -r  Raw mode for MODE2/2352: write all 2352 bytes from offset 0 (VCD/MPEG)\n" \
	"  -p  PSX mode for MODE2/2352: write 2336 bytes from offset 24\n" \
	"      (default MODE2/2352 mode writes 2048 bytes from offset 24)\n"\
	"  -w  Output audio files in WAV format\n" \
	"  -s  swabaudio: swap byte order in audio tracks\n"
	
#define VERSTR	"binchunker for Windows, version " VERSION " by ...\n" \
		"\tBased upon work by Heikki Hannikainen <hessu@hes.iki.fi>\n" \
		"\tReleased under the GNU GPL, version 2 or later (at your option).\n\n"

#define CUELLEN 1024
#define SECTLEN 2352

#define WAV_RIFF_HLEN 12
#define WAV_FORMAT_HLEN 24
#define WAV_DATA_HLEN 8
#define WAV_HEADER_LEN WAV_RIFF_HLEN + WAV_FORMAT_HLEN + WAV_DATA_HLEN

/*
 *	Ugly way to convert integers to little-endian format.
 *	First let netinet's hton() functions decide if swapping should
 *	be done, then convert back.
 */

#include <inttypes.h>

#if defined(_WIN32)
    #include "getopt.h"
    #define snprintf _snprintf 
    #define vsnprintf _vsnprintf 
    #define strcasecmp _stricmp 
    #define strncasecmp _strnicmp 
    #define WIN32_LEAN_AND_MEAN 1
    #pragma comment(lib, "Ws2_32.lib")
    #include <winsock2.h>
    #include <windows.h>
#else
    #include <getopt.h> 
    #include <netinet/in.h>
#endif

#define bswap_16(x) \
     ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))
#define bswap_32(x) \
     ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >>  8) |  \
      (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

#define htoles(x) bswap_16(htons(x))
#define htolel(x) bswap_32(htonl(x))



struct track_t {
	int num;
	int mode;
	int audio;
	char *modes;
	char *extension;
	int bstart;
	int bsize;
	long startsect;
	long stopsect;
	long start;
	long stop;
	struct track_t *next;
};

char *basefile = NULL;
char *binfile = NULL;
char *cuefile = NULL;
int verbose = 0;
int psxtruncate = 0;
int raw = 0;
int swabaudio = 0;
int towav = 0;

/*
 *	Parse arguments
 */

void parse_args(int argc, char *argv[])
{
	int s;
	
	while ((s = getopt(argc, argv, "swvp?hr")) != -1) {
		switch (s) {
			case 'r':
				raw = 1;
				break;
			case 'v':
				verbose = 1;
				break;
			case 'w':
				towav = 1;
				break;
			case 'p':
				psxtruncate = 1;
				break;
			case 's':
				swabaudio = 1;
				break;
			case '?':
			case 'h':
				fprintf(stderr, "%s", USAGE);
				exit(0);
		}
	}

	if (argc - optind != 3) {
		fprintf(stderr, "%s", USAGE);
		exit(1);
	}
	
	while (optind < argc) {
		switch (argc - optind) {
			case 3:
				binfile = strdup(argv[optind]);
				break;
			case 2:
				cuefile = strdup(argv[optind]);
				break;
			case 1:
				basefile = strdup(argv[optind]);
				break;
			default:
				fprintf(stderr, "%s", USAGE);
				exit(1);
		}
		optind++;
	}
}

/*
 *	Convert a mins:secs:frames format to plain frames
 */

long time2frames(char *s)
{
	int mins = 0, secs = 0, frames = 0;
	char *p, *t;
	
	if (!(p = strchr(s, ':')))
		return -1;
	*p = '\0';
	mins = atoi(s);
	
	p++;
	if (!(t = strchr(p, ':')))
		return -1;
	*t = '\0';
	secs = atoi(p);
	
	t++;
	frames = atoi(t);
	
	return 75 * (mins * 60 + secs) + frames;
}

/*
 *	Parse the mode string
 */

void gettrackmode(struct track_t *track, char *modes)
{
	static char ext_iso[] = "iso";
	static char ext_cdr[] = "cdr";
	static char ext_wav[] = "wav";
	static char ext_ugh[] = "ugh";
	
	track->audio = 0;
	
	if (!strcasecmp(modes, "MODE1/2352")) {
		track->bstart = 16;
		track->bsize = 2048;
		track->extension = ext_iso;
		
	} else if (!strcasecmp(modes, "MODE2/2352")) {
		track->extension = ext_iso;
		if (raw) {
			/* Raw MODE2/2352 */
			track->bstart = 0;
			track->bsize = 2352;
		} else if (psxtruncate) {
			/* PSX: truncate from 2352 to 2336 byte tracks */
			track->bstart = 0;
			track->bsize = 2336;
		} else {
			/* Normal MODE2/2352 */
			track->bstart = 24;
			track->bsize = 2048;
		}
		
	} else if (!strcasecmp(modes, "MODE2/2336")) {
		/* WAS 2352 in V1.361B still work?
		 * what if MODE2/2336 single track bin, still 2352 sectors?
		 */
		track->bstart = 16;
		track->bsize = 2336;
		track->extension = ext_iso;
		
	} else if (!strcasecmp(modes, "AUDIO")) {
		track->bstart = 0;
		track->bsize = 2352;
		track->audio = 1;
		if (towav)
			track->extension = ext_wav;
		else
			track->extension = ext_cdr;
	} else {
		printf("(?) ");
		track->bstart = 0;
		track->bsize = 2352;
		track->extension = ext_ugh;
	}
}

/*
 *	return a progress bar
 */

char *progressbar(float f, int l)
{
	static char s[80];
	int i, n;
	
	n = l * f;
	for (i = 0; i < n; i++) {
		s[i] = '*';
	}
	for (; i < l; i++) {
		s[i] = ' ';
	}
	s[i] = '\0';
	
	return s;
}

/*
 *	Write a track
 */

int writetrack(FILE *bf, struct track_t *track, char *bname)
{
	char *fname;
	FILE *f;
	char buf[SECTLEN+10];
	long sz, sect, realsz, reallen;
	char c, *p, *p2, *ep;
	int32_t l;
	int16_t i;
	float fl;
	
	if (!(fname = malloc(strlen(bname) + 8))) {
		fprintf(stderr, "main(): malloc() failed, out of memory\n");
		exit(4);
	}
	sprintf(fname, "%s%2.2d.%s", bname, track->num, track->extension);
	
	printf("%2d: %s ", track->num, fname);
	
	if (!(f = fopen(fname, "wb"))) {
		fprintf(stderr, " Could not fopen track file: %s\n", strerror(errno));
		exit(4);
	}
	
	if (fseek(bf, track->start, SEEK_SET)) {
		fprintf(stderr, " Could not fseek to track location: %s\n", strerror(errno));
		exit(4);
	}
	
	reallen = (track->stopsect - track->startsect + 1) * track->bsize;
	if (verbose) {
		printf("\n mmc sectors %ld->%ld (%ld)", track->startsect, track->stopsect, track->stopsect - track->startsect + 1);
		printf("\n mmc bytes %ld->%ld (%ld)", track->start, track->stop, track->stop - track->start + 1);
		printf("\n sector data at %d, %d bytes per sector", track->bstart, track->bsize);
		printf("\n real data %ld bytes", (track->stopsect - track->startsect + 1) * track->bsize);
		printf("\n");
	}

	printf("                                          ");
	
	if ((track->audio) && (towav)) {
		// RIFF header
		fputs("RIFF", f);
		l = htolel(reallen + WAV_DATA_HLEN + WAV_FORMAT_HLEN + 4);
		fwrite(&l, 4, 1, f);  // length of file, starting from WAVE
		fputs("WAVE", f);
		// FORMAT header
		fputs("fmt ", f);
		l = htolel(0x10);     // length of FORMAT header
		fwrite(&l, 4, 1, f);
		i = htoles(0x01);     // constant
		fwrite(&i, 2, 1, f);
		i = htoles(0x02);	// channels
		fwrite(&i, 2, 1, f);
		l = htolel(44100);	// sample rate
		fwrite(&l, 4, 1, f);
		l = htolel(44100 * 4);	// bytes per second
		fwrite(&l, 4, 1, f);
		i = htoles(4);		// bytes per sample
		fwrite(&i, 2, 1, f);
		i = htoles(2*8);	// bits per channel
		fwrite(&i, 2, 1, f);
		// DATA header
		fputs("data", f);
		l = htolel(reallen);
		fwrite(&l, 4, 1, f);
	}
	
	realsz = 0;
	sz = track->start;
	sect = track->startsect;
	fl = 0;
	while ((sect <= track->stopsect) && (fread(buf, SECTLEN, 1, bf) > 0)) {
		if (track->audio) {
			if (swabaudio) {
				/* swap low and high bytes */
				p = &buf[track->bstart];
				ep = p + track->bsize;
				while (p < ep) {
					p2 = p + 1;
					c = *p;
					*p = *p2;
					*p2 = c;
					p += 2;
				}
			}
		}
		if (fwrite(&buf[track->bstart], track->bsize, 1, f) < 1) {
			fprintf(stderr, " Could not write to track: %s\n", strerror(errno));
			exit(4);
		}
		sect++;
		sz += SECTLEN;
		realsz += track->bsize;
		if (((sz / SECTLEN) % 500) == 0) {
			fl = (float)realsz / (float)reallen;
			printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b%4ld/%-4ld MB  [%s] %3.0f %%", realsz/1024/1024, reallen/1024/1024, progressbar(fl, 20), fl * 100);
			fflush(stdout);
		}
	}
	
	fl = (float)realsz / (float)reallen;
	printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b%4ld/%-4ld MB  [%s] %3.0f %%", realsz/1024/1024, reallen/1024/1024, progressbar(1, 20), fl * 100);
	fflush(stdout);
	
	if (ferror(bf)) {
		fprintf(stderr, " Could not read from %s: %s\n", binfile, strerror(errno));
		exit(4);
	}
	
	if (fclose(f)) {
		fprintf(stderr, " Could not fclose track file: %s\n", strerror(errno));
		exit(4);
	}
	
	printf("\n");
	return 0;
}

/*
 *	Main
 */

int main(int argc, char **argv)
{
	char s[CUELLEN+1];
	char *p, *t;
	int i, idx;
	struct track_t *tracks = NULL;
	struct track_t *track = NULL;
	struct track_t *prevtrack = NULL;
	struct track_t **prevp = &tracks;
	
	FILE *binf, *cuef;
	
	printf("%s", VERSTR);
	
	parse_args(argc, argv);
	
	if (!((binf = fopen(binfile, "rb")))) {
		fprintf(stderr, "Could not open BIN %s: %s\n", binfile, strerror(errno));
		return 2;
	}
	
	if (!((cuef = fopen(cuefile, "rb")))) {
		fprintf(stderr, "Could not open CUE %s: %s\n", cuefile, strerror(errno));
		return 2;
	}
	
	printf("Reading the CUE file:\n");
	
	/* We don't really care about the first line. */
	if (!fgets(s, CUELLEN, cuef)) {
		fprintf(stderr, "Could not read first line from %s: %s\n", cuefile, strerror(errno));
		return 3;
	}
	
	i = 0;
	while (fgets(s, CUELLEN, cuef)) {
		while ((p = strchr(s, '\r')) || (p = strchr(s, '\n')))
			*p = '\0';
			
		if ((p = strstr(s, "TRACK"))) {
			printf("\nTrack ");
			if (!(p = strchr(p, ' '))) {
				fprintf(stderr, "... ouch, no space after TRACK.\n");
				continue;
			}
			p++;
			if (!(t = strchr(p, ' '))) {
				fprintf(stderr, "... ouch, no space after track number.\n");
				continue;
			}
			*t = '\0';
			
			prevtrack = track;
			if (!(track = malloc(sizeof(struct track_t)))) {
				fprintf(stderr, "main(): malloc() failed, out of memory\n");
				exit(4);
			}
			*prevp = track;
			prevp = &track->next;
			track->next = NULL;
			track->num = atoi(p);
			
			p = t + 1;
			printf("%2d: %-12.12s ", track->num, p);
			track->modes = strdup(p);
			track->extension = NULL;
			track->mode = 0;
			track->audio = 0;
			track->bsize = track->bstart = -1;
			track->bsize = -1;
			track->startsect = track->stopsect = -1;
			
			gettrackmode(track, p);
			
		} else if ((p = strstr(s, "INDEX"))) {
			if (!(p = strchr(p, ' '))) {
				printf("... ouch, no space after INDEX.\n");
				continue;
			}
			p++;
			if (!(t = strchr(p, ' '))) {
				printf("... ouch, no space after index number.\n");
				continue;
			}
			*t = '\0';
			t++;
			idx = atoi(p);
			printf(" %s %s", p, t);
			track->startsect = time2frames(t);
			track->start = track->startsect * SECTLEN;
			if (verbose)
				printf(" (startsect %ld ofs %ld)", track->startsect, track->start);
			if ((prevtrack) && (prevtrack->stopsect < 0)) {
				prevtrack->stopsect = track->startsect;
				prevtrack->stop = track->start - 1;
			}
		}
	}
	
	if (track) {
		fseek(binf, 0, SEEK_END);
		track->stop = ftell(binf);
		track->stopsect = track->stop / SECTLEN;
	}
	
	printf("\n\n");
	
	for (track = tracks; (track); track = track->next) {
		writetrack(binf, track, basefile);
    }
		
	fclose(binf);
	fclose(cuef);
	
    printf("End of Conversion\n\n");
	return 0;
}

