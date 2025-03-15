#ifndef _BETA_H_
#define _BETA_H_

#define BETA_E_NOMEM (-1)
#define BETA_E_NORGB 1

#define BETA_FI_NONE  0x0000
#define BETA_FI_NOX1  0x0001
/*
#define BETA_FI_NOR1  0x0001
#define BETA_FI_NOG1  0x0002
#define BETA_FI_NOB1  0x0004
#define BETA_FI_NOE1  0x0008
*/
#define BETA_FI_NORGB 0x0010

#define BETA_FI_SHORTX1  0x0100
/*
#define BETA_FI_SHORTR1  0x0100
#define BETA_FI_SHORTG1  0x0200
#define BETA_FI_SHORTB1  0x0400
#define BETA_FI_SHORTE1  0x0800
*/
#define BETA_FI_SHORTRGB 0x1000

struct betainfo_ {
    char *path;
    int pathlen;

    unsigned int finfo;

    unsigned char palette[3 * 16];
    unsigned char *raw;

    unsigned char current_palette[4 * 16];

    int occurrence_calced;
    int occurrence[16];
};
typedef struct betainfo_ betainfo;

betainfo *bi_make(char *path, int pathlen, int *err);
int bi_get_bitmap(betainfo *beta, char *data, int mag);
int bi_set_palette(betainfo *beta, unsigned char *pal);
int bi_get_palette(betainfo *beta, unsigned char *pal);
int bi_get_original_palette(betainfo *beta, unsigned char *pal);
int bi_reset_palette(betainfo *beta);
int bi_calc_occurence(betainfo *beta);
void bi_dispose(betainfo *beta);

#define bi_get_occurrence(beta) (beta)->occurrence

#endif /* _BETA_H_ */
