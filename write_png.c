#include <stdio.h>
#include <stdlib.h>
#include "png.h"

int main(int argc, char **argv) {
    int width = 300, height = 200, y,x;
    png_structp png_ptr;
    png_infop info_ptr;
    char * fname;
    FILE *fp;
    png_bytep *row_pointers;
    if(argc<2){
      fprintf(stderr,"usage:%s <file.png>\n",argv[0]);
      return 1;
    }
    fname=argv[1];
    fp = fopen(fname, "wb");
    if (!fp) { perror(fname); return 1; }

    // --- Initialize write structures ---
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
	perror("error in creating write structure"); 
	fclose(fp); return 1; 
	}

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) { 
	perror("error in creating info structure"); 
	png_destroy_write_struct(&png_ptr, NULL); 
	fclose(fp); return 1; 
	}

    if (setjmp(png_jmpbuf(png_ptr))) {  // error handler
	perror("error in setting error handler"); 
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return 1;
    }

    png_init_io(png_ptr, fp);

    // --- Header: 8-bit RGB image ---
    png_set_IHDR(png_ptr, info_ptr,
                 width, height, 8,
                 PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);

    // --- Allocate and fill image rows ---
    row_pointers = malloc(height * sizeof(png_bytep));
    for (y = 0; y < height; y++) {
        row_pointers[y] = malloc(3 * width);
        for (x = 0; x < width; x++) {
            int idx = x * 3;
            row_pointers[y][idx + 0] = (x * 255) / (width - 1);   // R gradient
            row_pointers[y][idx + 1] = (y * 255) / (height - 1);  // G gradient
            row_pointers[y][idx + 2] = 128;                       // constant B
        }
    }

    // --- Write image and clean up ---
    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, NULL);

    for (y = 0; y < height; y++)
        free(row_pointers[y]);
    free(row_pointers);

    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);

    printf("Created %s successfully (%dx%d)\n",fname, width, height);
    return 0;
}

