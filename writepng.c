#include <png.h>
#include <dlfcn.h>

#include "writepng.h"

#if (PNG_LIBPNG_VER >= 10500)
 #define Z_BEST_COMPRESSION 9
#endif

static png_structp (*create_write_struct)(
					  png_const_charp user_png_ver, png_voidp error_ptr,
					  png_error_ptr error_fn, png_error_ptr warn_fn);
static png_infop (*create_info_struct)(png_structp png_ptr);
static void (*destroy_write_struct)(png_structpp png_ptr_ptr, png_infopp info_ptr_ptr);
static void (*init_io)(png_structp png_ptr, png_FILE_p fp);
static void (*set_IHDR)(png_structp png_ptr,
			png_infop info_ptr, png_uint_32 width, png_uint_32 height, int bit_depth,
			int color_type, int interlace_method, int compression_method,
			int filter_method);
static png_voidp (*_malloc)(png_structp png_ptr, png_uint_32 size);
static void (*destroy_write_struct)(png_structpp png_ptr_ptr, png_infopp info_ptr_ptr);
static void (*_free)(png_structp png_ptr, png_voidp ptr);
static void (*write_image)(png_structp png_ptr, png_bytepp image);
static void (*write_end)(png_structp png_ptr, png_infop info_ptr);
static void (*set_PLTE)(png_structp png_ptr, png_infop info_ptr, png_colorp palette, int num_palette);
static void (*set_compression_level)(png_structp png_ptr,
				     int level);
static void (*set_packing)(png_structp png_ptr);
static void (*write_info)(png_structp png_ptr, png_infop info_ptr);
static jmp_buf *(*set_longjmp_fn)(png_structp png_ptr, png_longjmp_ptr longjmp_fn, size_t jmp_buf_size);

void *handle;

int write_png_init(void)
{
    handle = dlopen(DLOPEN_LIBPNG, RTLD_LAZY);
    if (!handle) {
	return -1;
    }

    create_write_struct = dlsym(handle, "png_create_write_struct");
    if (!create_write_struct) {
	goto dlsymerr;
    }
    create_info_struct = dlsym(handle, "png_create_info_struct");
    if (!create_info_struct) {
	goto dlsymerr;
    }
    init_io = dlsym(handle, "png_init_io");
    if (!init_io) {
	goto dlsymerr;
    }
    set_IHDR = dlsym(handle, "png_set_IHDR");
    if (!set_IHDR) {
	goto dlsymerr;
    }
    _malloc = dlsym(handle, "png_malloc");
    if (!_malloc) {
	goto dlsymerr;
    }
    set_PLTE = dlsym(handle, "png_set_PLTE");
    if (!set_PLTE) {
	goto dlsymerr;
    }
    write_info = dlsym(handle, "png_write_info");
    if (!write_info) {
	goto dlsymerr;
    }
    set_packing = dlsym(handle, "png_set_packing");
    if (!set_packing) {
	goto dlsymerr;
    }
    set_compression_level = dlsym(handle, "png_set_compression_level");
    if (!set_compression_level) {
	goto dlsymerr;
    }
    write_image = dlsym(handle, "png_write_image");
    if (!write_image) {
	goto dlsymerr;
    }
    write_end = dlsym(handle, "png_write_end");
    if (!write_end) {
	goto dlsymerr;
    }
    _free = dlsym(handle, "png_free");
    if (!_free) {
	goto dlsymerr;
    }
    set_longjmp_fn = dlsym(handle, "png_set_longjmp_fn");
    if (!set_longjmp_fn) {
	goto dlsymerr;
    }
    destroy_write_struct = dlsym(handle, "png_destroy_write_struct");
    if (!destroy_write_struct) {
	goto dlsymerr;
    }

    return 0;

dlsymerr:
    dlclose(handle);
    return -2;
}

int write_png(char *pathstr, unsigned char *data, unsigned char *pal)
{
    png_structp png_ptr;
    png_infop info_ptr;
    png_colorp palette;
    png_bytep row_pointers[400];

    int i, result = 0;
    FILE *fp;

    fp = fopen(pathstr, "wb");
    if (fp == NULL) {
	return -1;
    }

    png_ptr = create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
	result = -2;
	goto err1;
    }
    info_ptr = create_info_struct(png_ptr);
    if (info_ptr == NULL) {
	result = -3;
	goto err1;
    }
#if (PNG_LIBPNG_VER >= 10500)
    if (setjmp(*set_longjmp_fn(png_ptr, longjmp, sizeof(jmp_buf)))) {
	/* if (setjmp(png_jmpbuf(png_ptr))) としたいところだが、、、、
	   png_jmpbuf がマクロなので *png_set...と展開されてしまうため手で展開 */
#else /* libpng 1.4 では以下のコードで動作していた */
    if (setjmp(*png_ptr->jmpbuf)) { /* quick hack? */
#endif
	result = -4;
	goto err2;
    }
    init_io(png_ptr, fp);
    set_IHDR(png_ptr, info_ptr, 640, 400, 4, PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    palette = (png_colorp)_malloc(png_ptr, 16 * sizeof(png_color));
    if (_malloc == NULL) {
	result = -5;
	goto err2;
    }
    for (i = 0; i < 16; i++) {
	palette[i].red = pal[i * 4 + 1];
	palette[i].green = pal[i * 4 + 2];
	palette[i].blue = pal[i * 4 + 3];
    }
    set_PLTE(png_ptr, info_ptr, palette, 16);
    write_info(png_ptr, info_ptr);
    set_packing(png_ptr);
    set_compression_level(png_ptr, Z_BEST_COMPRESSION);
    for (i = 0; i < 400; i++) {
	row_pointers[i] = &data[i * 640];
    }
    write_image(png_ptr, row_pointers);
    write_end(png_ptr, info_ptr);
    _free(png_ptr, palette);
err2:
    destroy_write_struct(&png_ptr, &info_ptr);
err1:
    fclose(fp);

    return result;
}
