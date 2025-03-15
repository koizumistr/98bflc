#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "raw.h"
#include "beta.h"

char *ext_list[] = {".R1", ".G1", ".B1", ".E1"};

betainfo *bi_make(char *path, int pathlen, int *err)
{
    int i, fr_res;
    betainfo *result;
    char *rgb_n, *w_fname[4];
    unsigned char *rgbe[4];
    FILE *fp;
    unsigned char default_palette[3 * 16] =
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x07, 0x00, 0x00, 0x07, 0x07,
         0x07, 0x00, 0x00, 0x07, 0x00, 0x07, 0x07, 0x07, 0x00, 0x07, 0x07, 0x07,
         0x04, 0x04, 0x04, 0x00, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x00, 0x0f, 0x0f,
         0x0f, 0x00, 0x00, 0x0f, 0x00, 0x0f, 0x0f, 0x0f, 0x00, 0x0f, 0x0f, 0x0f};

    result = NULL;
    rgb_n = NULL;
    for (i = 0; i < 4; i++) {
	w_fname[i] = NULL;
	rgbe[i] = NULL;
    }

    result = (betainfo *)malloc(sizeof(betainfo));
    if (result == NULL) {
	*err = BETA_E_NOMEM;
	goto error;
    }
    result->raw = NULL;
    result->path = (char *)malloc(pathlen);
    if (result->path == NULL) {
	*err = BETA_E_NOMEM;
	goto error;
    }
    strncpy(result->path, path, pathlen);
    result->pathlen = pathlen;

    rgb_n = (char *)malloc(pathlen + 5); /* 4 + 1(for NULL) */
    if (rgb_n == NULL) {
	*err = BETA_E_NOMEM;
	goto error;
    }
    strncpy(rgb_n, path, pathlen);
    rgb_n[pathlen] = '\0';
    strcat(rgb_n, ".RGB");
#ifdef DEBUG_98BFL
    printf("RGB: %s\n", rgb_n);
#endif

    for (i = 0; i < 4; i++) {
	w_fname[i] = (char *)malloc(pathlen + 4); /* 3 + 1(for NULL) */
	if (w_fname[i] == NULL) {
	    *err = BETA_E_NOMEM;
	    goto error;
	}
	strncpy(w_fname[i], path, pathlen);
	w_fname[i][pathlen] = '\0';
	strcat(w_fname[i], ext_list[i]);

#ifdef DEBUG_98BFL
	printf("%d: %s\n", i, w_fname[i]);
#endif
    }

    result->finfo = BETA_FI_NONE;

    fp = fopen(rgb_n, "r");
    if (fp != NULL) {
	fr_res = fread(result->palette, 1, 3 * 16, fp);
	if (fr_res < 3 * 16) {
	    memcpy(result->palette + fr_res, default_palette + fr_res, 3 * 16 - fr_res);
	    result->finfo |= BETA_FI_SHORTRGB;
	}
	fclose(fp);
	free(rgb_n);
	rgb_n = NULL;
    } else {
	*err = BETA_E_NORGB;
	memcpy(result->palette, default_palette, 3 * 16);
	result->finfo |= BETA_FI_NORGB;
    }

    for (i = 0; i < 4; i++) {
	fp = fopen(w_fname[i], "r");
	if (fp == NULL) {
	    goto error;
	}
	rgbe[i] = (unsigned char *)malloc(32000 * sizeof(unsigned char));
	if (rgbe[i] == NULL) {
	    *err = BETA_E_NOMEM;
	    goto error;
	}
	fr_res = fread(rgbe[i], 1, 32000, fp);
	if (fr_res < 32000) {
	    result->finfo |= BETA_FI_SHORTX1 << i;
	}
	fclose(fp);
	free(w_fname[i]);
	w_fname[i] = NULL;
    }

    if (pal98togpal(result->palette, result->current_palette) != 0) {
	goto error;
    }

    result->raw = (unsigned char *)malloc(32000 * 8 * sizeof(unsigned char));
    if (result->raw == NULL) {
	*err = BETA_E_NOMEM;
	goto error;
    }
    set_raw(result->raw, rgbe);

    for (i = 0; i < 16; i++) {
	result->occurrence[i] = 0;
    }
    result->occurrence_calced = 0;

end:
    if (rgb_n != NULL)
	free(rgb_n);
    for (i = 0; i < 4; i++) {
	if (w_fname[i] != NULL)
	    free(w_fname[i]);
	if (rgbe[i] != NULL)
	    free(rgbe[i]);
    }
    return result;

error:
    bi_dispose(result);
    result = NULL;
    goto end;
}

int bi_get_bitmap(betainfo *beta, char *data, int mag)
{
    int i, j, m, n, cur_x, p;

    if (mag > 1) {
	for (i = 0; i < 400; i++) {
	    cur_x = i * mag;
	    for (j = 0; j < 640; j++) {
		p = cur_x * 640 * mag + j * mag;
		memcpy(data + p * 4, beta->current_palette + 4 * beta->raw[640 * i + j], 4);
		for (m = 1; m < mag; m++) {
		    memcpy(data + (p + m) * 4, data + p * 4, 4);
		}
	    }
	    for (n = 1; n < mag; n++) {
		memcpy(data + (cur_x + n) * 640 * 4 * mag, data + cur_x * 640 * 4 * mag, 640 * 4 * mag);
	    }
	}
    } else if (mag < -1) { /* -Z: 1/Zに縮小 */
	for (i = 0; i < 400; i += -mag) {
	    cur_x = i / -mag;
	    for (j = 0; j < 640; j += -mag) {
		p = (cur_x * 640 + j) / -mag;
		memcpy(data + p * 4, beta->current_palette + 4 * beta->raw[640 * i + j], 4);
	    }
	}
    } else { /* -1 と 0 は 1 として扱う */
	for (i = 0; i < 32000 * 8; i++) {
	    memcpy(data + i * 4, beta->current_palette + 4 * beta->raw[i], 4);
	}
    }

    return 0;
}

int bi_set_palette(betainfo *beta, unsigned char *pal)
{
    int i, j;

    for (i = 0; i < 16; i++) {
	for (j = 0; j < 4; j++) {
	    beta->current_palette[4 * i + j] = pal[4 * i + j];
	}
    }
    return 0;
}

int bi_get_palette(betainfo *beta, unsigned char *pal)
{
    int i, j;

    for (i = 0; i < 16; i++) {
	for (j = 0; j < 4; j++) {
	    pal[4 * i + j] = beta->current_palette[4 * i + j];
	}
    }
    return 0;
}

int bi_get_original_palette(betainfo *beta, unsigned char *pal)
{
    int i, j;
    unsigned char palette[4 * 16];

    pal98togpal(beta->palette, palette);
    for (i = 0; i < 16; i++) {
	for (j = 0; j < 4; j++) {
	    pal[4 * i + j] = palette[4 * i + j];
	}
    }
    return 0;
}

int bi_reset_palette(betainfo *beta)
{
    return pal98togpal(beta->palette, beta->current_palette);
}

int bi_calc_occurence(betainfo *beta)
{
    int i;

    if (!beta->occurrence_calced) {
	for (i = 0; i < 256000 /* 640x400 */; i++) {
	    beta->occurrence[beta->raw[i]]++;
	}
	beta->occurrence_calced = 1;
    }
    return 0;
}

void bi_dispose(betainfo *beta)
{
    if (beta->path != NULL) {
	free(beta->path);
	beta->path = NULL;
    }
    if (beta->raw != NULL) {
	free(beta->raw);
	beta->raw = NULL;
    }
    free(beta);
}
