#include <stdio.h>
#include <string.h>
#include "beta.h"
#include "writepng.h"

int main(int argc, char **argv)
{
    int err, r;
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
    if (beta == NULL) {
        fprintf(stderr, "bi_make failed: %d\n", err);
        return -3;
    }
    if (beta->finfo != BETA_FI_NONE) {
        fprintf(stderr, "finfo: %d err: %d\n", beta->finfo, err);
    }

    r = write_png(strcat(argv[1], ".png"), beta->raw, beta->current_palette);
    if (r != 0) {
        fprintf(stderr, "write_png failed: %d\n", r);
    }
    bi_dispose(beta);

    return 0;
}
