#ifndef _WRITEPNG_H_
#define _WRITEPNG_H_

#define DLOPEN_LIBPNG "/opt/homebrew/lib/libpng.dylib"
/*
  for GhostBSD(FreeBSD)
#define DLOPEN_LIBPNG "libpng.so"
*/

int write_png_init(void);
int write_png(char *pathstr, unsigned char *data, unsigned char *pal);
#endif /* _WRITEPNG_H_ */
