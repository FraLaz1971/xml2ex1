#include <png.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv) {
    const char *fname;
    png_structp png_ptr;
    int width,x,idx,debug=0;
    int height,y;
    png_byte color_type;
    png_byte bit_depth;
    png_bytep *row_pointers;
    png_infop info_ptr;
    unsigned char **R;
    unsigned char **G;
    unsigned char **B;
    png_byte header[8];
    png_bytep row;
    FILE *fp;
    if (argc<2){
      fprintf(stderr,"usage:%s <file.png>\n",argv[0]);
      return 1;
    }
    fname=argv[1];
    fp = fopen(fname, "rb");
    if (!fp) { perror(fname); return 1; }
    if (debug) fprintf(stderr,"open file for reading\n");

    // --- Read header and verify PNG signature ---
    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8)) {
        fprintf(stderr, "%s is not a PNG file\n", fname);
        fclose(fp);
        return 1;
    }
    if (debug) fprintf(stderr,"png signature verified\n");
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) { fclose(fp); return 1; }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) { png_destroy_read_struct(&png_ptr, NULL, NULL); fclose(fp); return 1; }
    if (debug) fprintf(stderr,"png info struct created\n");

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return 1;
    }
    if (debug) fprintf(stderr,"error handled\n");

    png_init_io(png_ptr, fp);
    if (debug) fprintf(stderr,"png io inited\n");

    png_set_sig_bytes(png_ptr, 8);
    if (debug) fprintf(stderr,"png signature bytes set\n");
    png_read_info(png_ptr, info_ptr);
    if (debug) fprintf(stderr,"png info read\n");

    width  = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
    color_type = png_get_color_type(png_ptr, info_ptr);
    bit_depth  = png_get_bit_depth(png_ptr, info_ptr);

    if (color_type != PNG_COLOR_TYPE_RGB || bit_depth != 8) {
        fprintf(stderr, "Unsupported format: must be 8-bit RGB\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return 1;
    }

    // --- Allocate row buffers and read ---
    row_pointers = malloc(height * sizeof(png_bytep));
    for (y = 0; y < height; y++)
        row_pointers[y] = malloc(png_get_rowbytes(png_ptr, info_ptr));

    png_read_image(png_ptr, row_pointers);

    // --- Allocate RGB arrays ---
    R = malloc(height * sizeof(unsigned char *));
    G = malloc(height * sizeof(unsigned char *));
    B = malloc(height * sizeof(unsigned char *));
    for (y = 0; y < height; y++) {
        R[y] = malloc(width);
        G[y] = malloc(width);
        B[y] = malloc(width);
        row = row_pointers[y];
        for (x = 0; x < width; x++) {
            idx = x * 3;
            R[y][x] = row[idx + 0];
            G[y][x] = row[idx + 1];
            B[y][x] = row[idx + 2];
        }
    }

    printf("Read %s: %dx%d RGB image\n", fname, width, height);
    printf("Example pixels:\n");
    printf("  Top-left:      R=%3d G=%3d B=%3d\n", R[0][0], G[0][0], B[0][0]);
    printf("  Center:        R=%3d G=%3d B=%3d\n", R[height/2][width/2], G[height/2][width/2], B[height/2][width/2]);
    printf("  Bottom-right:  R=%3d G=%3d B=%3d\n", R[height-1][width-1], G[height-1][width-1], B[height-1][width-1]);

    // --- cleanup ---
    for (y = 0; y < height; y++) {
        free(row_pointers[y]);
        free(R[y]); free(G[y]); free(B[y]);
    }
    free(row_pointers);
    free(R); free(G); free(B);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
    return 0;
}
