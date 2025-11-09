#include <stdio.h>
#include <stdlib.h>
#include <png.h>

int main(int argc, char** argv) {
    int width = 300, height = 200;
    png_byte color_type = PNG_COLOR_TYPE_GRAY;  // ← only 1 channel
    png_byte bit_depth = 8;
    if(argc<2){
      fprintf(stderr,"usage:%s <file.png>",argv[0]);
      return 1;
    }
    char *fname=argv[1];
    FILE *fp = fopen(fname, "wb");
    if(!fp) { perror("fopen"); return 1; }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png) return 1;

    png_infop info = png_create_info_struct(png);
    if(!info) return 1;

    if(setjmp(png_jmpbuf(png))) return 1;

    png_init_io(png, fp);

    png_set_IHDR(
        png, info, width, height,
        bit_depth, color_type,
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE
    );

    png_write_info(png, info);

    // Allocate grayscale data (height × width)
    png_bytep *row_pointers = malloc(sizeof(png_bytep) * height);
    for(int y = 0; y < height; y++) {
        row_pointers[y] = malloc(width);  // 1 byte per pixel
        for(int x = 0; x < width; x++) {
            row_pointers[y][x] = (png_byte)((x + y) % 256); // simple pattern
        }
    }

    png_write_image(png, row_pointers);
    png_write_end(png, NULL);

    for(int y = 0; y < height; y++) free(row_pointers[y]);
    free(row_pointers);

    fclose(fp);
    png_destroy_write_struct(&png, &info);

    printf("Wrote %s (%dx%d)\n", fname, width, height);
    return 0;
}

