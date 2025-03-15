#include <stdio.h>
#include <string.h>
#include "beta.h"
#include "writepng.h"

int main(int argc, char **argv)
{
  int err;
  betainfo *beta;

  if ((err = write_png_init()) != 0) {
    fprintf(stderr, "libpng not found?: %d\n", err);
    return -1;
  }

  if (argc != 2) {
    fprintf(stderr, "usage: beta2png filename\n");
    return -2;
  }
  beta = bi_make(argv[1], strlen(argv[1]), &err);
  write_png(strcat(argv[1], ".png"), beta->raw, beta->current_palette);
  bi_dispose(beta);
  printf("err: %d\n", err);
  return 0;
}
