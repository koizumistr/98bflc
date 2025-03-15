#include <stddef.h>
#include "raw.h"

static int read_bit(char c, int i);

int set_raw(unsigned char *raw, unsigned char **rgbe)
{
    int i, j, i8;
    unsigned char r_tmp, g_tmp, b_tmp, e_tmp;

    for (i = 0; i < 32000; i++) {
	i8 = i * 8;
	for (j= 0; j < 8; j++) {
	    r_tmp = read_bit(rgbe[0][i], j);
	    g_tmp = read_bit(rgbe[1][i], j);
	    b_tmp = read_bit(rgbe[2][i], j);
	    e_tmp = read_bit(rgbe[3][i], j);
	    raw[i8 + j] = (e_tmp << 3) | (g_tmp << 2) | (r_tmp << 1) | b_tmp;
	}
    }
    return 0;
}

static int read_bit(char c, int i)
{
    return (c & (1 << (7 - i))) >> (7 - i);
}

int pal98togpal(unsigned char *pal98, unsigned char *palout)
{
    int color_num, cn3, cn4;

    if (pal98 == NULL || palout == NULL)
	return -1;

    for (color_num = 0; color_num < 16; color_num++) {
	cn3 = color_num * 3;
	cn4 = color_num * 4;
	palout[cn4] = 0;
	palout[cn4 + 1] = (pal98[cn3] << 4) | pal98[cn3];
	palout[cn4 + 2] = (pal98[cn3 + 1] << 4) | pal98[cn3 + 1];
	palout[cn4 + 3] = (pal98[cn3 + 2] << 4) | pal98[cn3 + 2];
    }
    return 0;
}
