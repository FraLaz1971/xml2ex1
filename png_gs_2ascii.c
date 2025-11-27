#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>

int main(int argc, char**argv) {
    int y,x,pos;
    FILE *fp;
    FILE *ofp;
    png_structp png;
    png_infop info;
    char ifname[256];
    char ofname[256];
    char base[256];
    char *result_ptr;
    int width;
    int height;
    png_byte color_type;
    png_byte bit_depth;
    png_bytep *rows;
    if(argc<2){
      fprintf(stderr,"user:%s <file.png>\n",argv[0]);
      return 1;
    }
    strcpy(ifname,argv[1]);
    fp = fopen(ifname, "rb");
    if(!fp) { perror("fopen"); return 1; }

    png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png) return 1;
    info = png_create_info_struct(png);
    if(!info) return 1;
    if(setjmp(png_jmpbuf(png))) return 1;

    png_init_io(png, fp);
    png_read_info(png, info);

    width = png_get_image_width(png, info);
    height = png_get_image_height(png, info);
    color_type = png_get_color_type(png, info);
    bit_depth = png_get_bit_depth(png, info);

    if (color_type != PNG_COLOR_TYPE_GRAY || bit_depth != 8) {
        fprintf(stderr, "Expected 8-bit grayscale image\n");
        return 1;
    }

    rows = malloc(sizeof(png_bytep) * height);
    for(y = 0; y < height; y++) {
        rows[y] = malloc(width);
    }

    png_read_image(png, rows);
    fclose(fp);
    printf("Read grayscale image %dx%d\n", width, height);
    printf("Example pixels:\n");
    printf("  Top-left (0,0): %3d\n", rows[0][0]);
    printf("  Center (%d,%d): %3d\n", width/2, height/2, rows[height/2][width/2]);
    printf("  Bottom-right (%d,%d): %3d\n", width-1, height-1, rows[height-1][width-1]);
    result_ptr=strstr(ifname,".");
    pos=result_ptr-ifname;
    strncpy(base,ifname,pos);
    base[pos]='\0';
    snprintf(ofname,255,"%s%4s",base,".asc");
    fprintf(stderr,"output file name: %s\n",ofname);
    ofp = fopen(ofname, "w");
    if(!ofp) {
        perror("error in opening output file\n");
        return 1;
    }

    for(y = height-1; y >= 0; y--) {
        for(x = 0; x < width; x++)
            fprintf(ofp,"%3d ",(unsigned char)rows[y][x]);
        fprintf(ofp,"\n");
    }
    fclose(ofp);
    for(y = 0; y < height; y++) free(rows[y]);
    free(rows);
    png_destroy_read_struct(&png, &info, NULL);
    return 0;
}
