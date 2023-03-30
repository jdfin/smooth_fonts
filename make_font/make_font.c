#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <png.h>


struct GlyphInfo {
    int y_adv;
    int x_adv_max;
    int x_off_min;
    int x_off_max;
    int y_off_min;
    int y_off_max;
    struct {
        int off; // into output grayscale array
        int x, y; // in grayscale image (not output)
        int w, h;
        int x_off, y_off;
        int x_adv;
    } glyph[128];
} glyph_info;

// png image converted to 8-bit grayscale
uint8_t *image = NULL;

png_uint_32 image_height = 0;
png_uint_32 image_width = 0;

// total bytes of grayscale data we need in output for all glyphs
int gs_bytes;


// skip line in description file
static void skip_line(FILE *fp)
{
    char line[200];
    memset(line, 0, sizeof(line));
    assert(fgets(line, sizeof(line), fp) == line);
    int len = strlen(line);
    assert(line[len - 1] == '\n');
}


// read BMFont description file
static void read_description(const char *fn_root)
{
    char fn_buf[80];
    sprintf(fn_buf, "%s.fnt", fn_root);
    FILE *fp = fopen(fn_buf, "r");
    assert(fp != NULL);

    memset(&glyph_info, 0, sizeof(glyph_info));

    // "info" line
    skip_line(fp);

    // "common" line
    int base;
    assert(fscanf(fp, "common lineHeight=%d base=%d", &glyph_info.y_adv, &base) == 2);
    skip_line(fp);

    // "page" line
    skip_line(fp); // assuming page 0

    int glyph_cnt;
    assert(fscanf(fp, "chars count=%d\n", &glyph_cnt) == 1);

    // glyph's offset = -1 means it is not present
    for (int i = 0; i < 128; i++)
        glyph_info.glyph[i].off = -1;

    // initialize mins/maxs
    glyph_info.x_adv_max = INT8_MIN;
    glyph_info.x_off_min = INT8_MAX;
    glyph_info.x_off_max = INT8_MIN;
    glyph_info.y_off_min = INT8_MAX;
    glyph_info.y_off_max = INT8_MIN;

    gs_bytes = 0;
    for (int i = 0; i < glyph_cnt; i++) {
        int c; // character

        // read character id, used to index into glyph info
        assert(fscanf(fp, "char id=%d\n", &c) == 1);
        assert(c >= 0 && c < 128);

        // read glyph info
        assert(fscanf(fp, "x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d",
                      &glyph_info.glyph[c].x, &glyph_info.glyph[c].y,
                      &glyph_info.glyph[c].w, &glyph_info.glyph[c].h,
                      &glyph_info.glyph[c].x_off, &glyph_info.glyph[c].y_off,
                      &glyph_info.glyph[c].x_adv) == 7);

        // skip over page and chnl
        skip_line(fp);

        // where in the glyph data array this one is
        glyph_info.glyph[c].off = gs_bytes;

        // updates mins/maxs
        if (glyph_info.x_adv_max < glyph_info.glyph[c].x_adv)
            glyph_info.x_adv_max = glyph_info.glyph[c].x_adv;

        if (glyph_info.x_off_min > glyph_info.glyph[c].x_off)
            glyph_info.x_off_min = glyph_info.glyph[c].x_off;

        // x_off_max is really the max of (x_off + w), to see how far the
        // glyph might extend, similar for y_off_max
        if (glyph_info.x_off_max < (glyph_info.glyph[c].x_off + glyph_info.glyph[c].w))
            glyph_info.x_off_max = glyph_info.glyph[c].x_off + glyph_info.glyph[c].w;

        if (glyph_info.y_off_min > glyph_info.glyph[c].y_off)
            glyph_info.y_off_min = glyph_info.glyph[c].y_off;

        if (glyph_info.y_off_max < (glyph_info.glyph[c].y_off + glyph_info.glyph[c].h))
            glyph_info.y_off_max = glyph_info.glyph[c].y_off + glyph_info.glyph[c].h;

        // total bytes required in the glyph data array
        gs_bytes += (glyph_info.glyph[c].w * glyph_info.glyph[c].h);
    }

    assert(fclose(fp) == 0);
}


// print BMFont description file
static void print_description()
{
    printf("y_adv=%d\n", glyph_info.y_adv);
    for (int i = 0; i < 128; i++) {
        if (glyph_info.glyph[i].off < 0)
            continue;
        printf("%d: off=%d w=%d h=%d xo=%d yo=%d xa=%d\n",
               i, glyph_info.glyph[i].off,
               glyph_info.glyph[i].w, glyph_info.glyph[i].h,
               glyph_info.glyph[i].x_off, glyph_info.glyph[i].y_off,
               glyph_info.glyph[i].x_adv);
    }
}


// read png into global image[]
static void read_png(const char *fn_root)
{
    char fn_buf[80];
    //sprintf(fn_buf, "%s.png", fn_root); // fontbuilder
    sprintf(fn_buf, "%s_0.png", fn_root); // bmfont
    //printf("reading %s\n", fn_buf);
    FILE *fp = fopen(fn_buf, "rb");
    assert(fp != NULL);

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    assert(png_ptr != NULL);

    png_infop info_ptr = png_create_info_struct(png_ptr);
    assert(info_ptr != NULL);

    png_init_io(png_ptr, fp);

    png_read_info(png_ptr, info_ptr);

    assert(png_get_IHDR(png_ptr, info_ptr, &image_width, &image_height,
                        NULL, NULL, NULL, NULL, NULL) == 1);

    //printf("height=%d width=%d\n", image_height, image_width);

    png_bytepp row_ptrs = png_malloc(png_ptr, image_height * sizeof(png_bytep));
    assert(row_ptrs != NULL);
    for (int r = 0; r < image_height; r++) {
        row_ptrs[r] = png_malloc(png_ptr, png_get_rowbytes(png_ptr, info_ptr));
        assert(row_ptrs[r] != NULL);
    }

    png_read_image(png_ptr, row_ptrs);

    image = malloc(image_height * image_width * sizeof(uint8_t));
    assert(image != NULL);

    memset(image, 0, sizeof(image));
    for (int r = 0; r < image_height; r++)
        for (int c = 0; c < image_width; c++)
            image[r * image_width + c] = row_ptrs[r][c];

    for (int r = 0; r < image_height; r++) {
        png_free(png_ptr, row_ptrs[r]);
        row_ptrs[r] = NULL;
    }

    png_free(png_ptr, row_ptrs);
    row_ptrs = NULL;

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    assert(fclose(fp) == 0);

} // read_png()


// make sure an int can be stored at int8_t
int as_int8(int v)
{
    assert(v >= INT8_MIN && v <= INT8_MAX);
    return v;
}


// write .h and .cpp files for font
static void write_code(const char *fn_root)
{
    char fn_buf[80];
    FILE *fp;

    // font.h defines "struct Font" and looks something like this:
    //
    // -----8<-----
    //#pragma once
    //
    // #include <stdint.h>
    //
    // struct Font {
    //     int8_t y_adv;
    //     int8_t x_adv_max;
    //     int8_t x_off_min;
    //     int8_t x_off_max;
    //     int8_t y_off_min;
    //     int8_t y_off_max;
    //     struct {
    //         int32_t off;
    //         int8_t w;
    //         int8_t h;
    //         int8_t x_off;
    //         int8_t y_off;
    //         int8_t x_adv;
    //     } info[128];
    //     const uint8_t *data;
    // ...some accessor functions, no more data
    // };
    // -----8<-----

    // font's .h file

    sprintf(fn_buf, "%s.h", fn_root);
    fp = fopen(fn_buf, "w");
    assert(fp != NULL);

    fprintf(fp, "#pragma once\n");
    fprintf(fp, "\n");
    fprintf(fp, "#include \"font.h\"\n");
    fprintf(fp, "\n");
    fprintf(fp, "const int %s_max_height = %d;\n", fn_root, glyph_info.y_adv);
    fprintf(fp, "const int %s_max_width = %d;\n", fn_root, glyph_info.x_adv_max);
    fprintf(fp, "\n");
    fprintf(fp, "extern const struct Font %s;\n", fn_root);

    assert(fclose(fp) == 0);

    // font's .cpp file
    //
    // This is cpp (instead of c) for the member functions in struct Font

    sprintf(fn_buf, "%s.cpp", fn_root);
    fp = fopen(fn_buf, "w");
    assert(fp != NULL);

    fprintf(fp, "#include <stdint.h>\n");
    fprintf(fp, "#include \"%s.h\"\n", fn_root);
    fprintf(fp, "\n");
    fprintf(fp, "extern const uint8_t %s_data[];\n", fn_root);
    fprintf(fp, "\n");
    fprintf(fp, "const Font %s = {\n", fn_root);
    fprintf(fp, "    %d, // int8_t y_adv\n", as_int8(glyph_info.y_adv));
    fprintf(fp, "    %d, // int8_t x_adv_max\n", as_int8(glyph_info.x_adv_max));
    fprintf(fp, "    %d, // int8_t x_off_min\n", as_int8(glyph_info.x_off_min));
    fprintf(fp, "    %d, // int8_t x_off_max (x_off + w)\n", as_int8(glyph_info.x_off_max));
    fprintf(fp, "    %d, // int8_t y_off_min\n", as_int8(glyph_info.y_off_min));
    fprintf(fp, "    %d, // int8_t y_off_max (y_off + h)\n", as_int8(glyph_info.y_off_max));
    fprintf(fp, "    { // info[]\n");
    fprintf(fp, "        // i32 off, i8 w, i8 h, i8 x_off, i8 y_off, i8 x_adv\n");
    for (int i = 0; i < 128; i++) {
        fprintf(fp, "        { %d, %d, %d, %d, %d, %d },", glyph_info.glyph[i].off,
                as_int8(glyph_info.glyph[i].w), as_int8(glyph_info.glyph[i].h),
                as_int8(glyph_info.glyph[i].x_off), as_int8(glyph_info.glyph[i].y_off),
                as_int8(glyph_info.glyph[i].x_adv));
        fprintf(fp, " // %d", i);
        if (isprint(i))
            fprintf(fp, " '%c'", (char)i);
        fprintf(fp, "\n");
    }
    fprintf(fp, "    },\n");
    fprintf(fp, "    %s_data // const uint8_t *data\n", fn_root);
    fprintf(fp, "};\n");
    fprintf(fp, "\n");
    fprintf(fp, "const uint8_t %s_data[%d] = {\n", fn_root, gs_bytes);
    for (int i = 0; i < 128; i++) {
        if (glyph_info.glyph[i].off < 0)
            continue;
        fprintf(fp, "    // %d", i);
        if (isprint(i))
            fprintf(fp, " '%c'", (char)i);
        fprintf(fp, "\n");
        // top left corner of glyph in image[]
        int top = glyph_info.glyph[i].y;
        int left = glyph_info.glyph[i].x;
        for (int gr = 0; gr < glyph_info.glyph[i].h; gr++) {
            fprintf(fp, "   ");
            for (int gc = 0; gc < glyph_info.glyph[i].w; gc++) {
                int r = top + gr;
                int c = left + gc;
                fprintf(fp, " %d,",  image[r * image_width + c]);
            }
            fprintf(fp, "\n");
        }
    }
    fprintf(fp, "};\n");

    assert(fclose(fp) == 0);

} // write_code()


int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("usage: %s <filename_root>\n", argv[0]);
        exit(1);
    }

    read_description(argv[1]);

    //print_description();

    read_png(argv[1]);

    write_code(argv[1]);

    return 0;
}
