#include <png.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int main(int argc, char **argv) {
    const char *fname;
    png_structp png_ptr; /* the input png structure */
    png_structp png; /* the output png structure */
    int width,x,idx,debug=0;
    int height,y,pos,res;
    png_byte color_type;
    png_byte bit_depth;
    png_bytep *row_pointers;
    png_bytep *row_pointers_gs;
    png_infop info_ptr, info;
    char ofname[256];
    char base[256];
    char *result_ptr;
    unsigned char **R;
    unsigned char **G;
    unsigned char **B;
    png_byte header[8];
    png_bytep row;
    FILE *fp,*ofp;
    if (argc<2){
      fprintf(stderr,"usage:%s <file.png>\n",argv[0]);
      return 1;
    }
    fname=argv[1];
    fp = fopen(fname, "rb");
    if (!fp) { perror(fname); return 1; }
    if (debug) fprintf(stderr,"open file for reading\n");

    // --- Read header and verify PNG signature ---
    res = fread(header, 1, 8, fp);
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
    	// --- cleanup input png ---

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
    /* save to png grayscale */
    color_type = PNG_COLOR_TYPE_GRAY;
    /* create output file name*/
    result_ptr=strstr(fname,".");
    pos=result_ptr-fname;
    strncpy(base,fname,pos);
    base[pos]='\0';
    snprintf(ofname,255,"%s%7s",base,"_gs.png");
    fprintf(stderr,"output file name: %s\n",ofname);
    ofp = fopen(ofname, "wb");
    if(!ofp) { perror("ferror in opening output file\n"); return 1; }

    png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png) return 1;

    info = png_create_info_struct(png);
    if(!info) return 1;

    if(setjmp(png_jmpbuf(png))) return 1;

    png_init_io(png, ofp);

    png_set_IHDR(
        png, info, width, height,
        bit_depth, color_type,
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE
    );

    png_write_info(png, info);

    /* write the average value of the 3 channels*/
    row_pointers_gs = malloc(sizeof(png_bytep) * height);
    for (y = 0; y < height; y++) {
		row_pointers_gs[y] = malloc(width);  // 1 byte per pixel
		for (x = 0; x < width; x++) {
			row_pointers_gs[y][x] = (png_byte)((R[y][x]+G[y][x]+B[y][x])/3); // average value
		}
	}    
	png_write_image(png, row_pointers_gs);
    png_write_end(png, NULL);


    fclose(ofp);
    png_destroy_write_struct(&png, &info);
    for (y = 0; y < height; y++) {
        free(row_pointers[y]);
        free(R[y]); free(G[y]); free(B[y]);
    }
    free(row_pointers);
    free(R); free(G); free(B);

    for(int y = 0; y < height; y++) free(row_pointers_gs[y]);
    free(row_pointers_gs);

    printf("Wrote %s (%dx%d)\n", fname, width, height);

    return 0;
}
