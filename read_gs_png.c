#include <stdio.h>
#include <stdlib.h>
#include <png.h>

int main(int argc, char**argv) {
    if(argc<2){
      fprintf(stderr,"user:%s <file.png>",argv[0]);
      return 1;
    }
    FILE *fp = fopen(argv[1], "rb");
    if(!fp) { perror("fopen"); return 1; }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png) return 1;
    png_infop info = png_create_info_struct(png);
    if(!info) return 1;
    if(setjmp(png_jmpbuf(png))) return 1;

    png_init_io(png, fp);
    png_read_info(png, info);

    int width = png_get_image_width(png, info);
    int height = png_get_image_height(png, info);
    png_byte color_type = png_get_color_type(png, info);
    png_byte bit_depth = png_get_bit_depth(png, info);

    if (color_type != PNG_COLOR_TYPE_GRAY || bit_depth != 8) {
        fprintf(stderr, "Expected 8-bit grayscale image\n");
        return 1;
    }

    png_bytep *rows = malloc(sizeof(png_bytep) * height);
    for(int y = 0; y < height; y++) {
        rows[y] = malloc(width);
    }

    png_read_image(png, rows);

    printf("Read grayscale image %dx%d\n", width, height);
    printf("Example pixels:\n");
    printf("  Top-left (0,0): %3d\n", rows[0][0]);
    printf("  Center (%d,%d): %3d\n", width/2, height/2, rows[height/2][width/2]);
    printf("  Bottom-right (%d,%d): %3d\n", width-1, height-1, rows[height-1][width-1]);

    for(int y = 0; y < height; y++) free(rows[y]);
    free(rows);
    fclose(fp);
    png_destroy_read_struct(&png, &info, NULL);
    return 0;
}
