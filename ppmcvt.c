#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <memory.h>
#include "pbm.h"

typedef struct {

    bool b; //pbm pgm
    unsigned int g;
    // rgb
    char *i;
    char *r;
    // sepia and mirror
    bool s;
    bool m;
    // thumbnail
    long t;
    long n;
    // output
    bool o;

    unsigned int numCommands;

    char *infilename;
    char *outfilename;
} Cmdline;

// PBM.h implementations
// ==========================================================================================

PPMImage * new_ppmimage(unsigned int width, unsigned int height, unsigned int max){

    //Difference here is that there are 3 maps to create, each map is a column of pixels.

    PPMImage *img = (PPMImage *) malloc(sizeof(PPMImage));
    int i, j;

    img->height = height;
    img->width = width;
    img->max = max;

    for (i = 0; i < 3; i++){
        img->pixmap[i] = (unsigned int **)malloc(height*sizeof(unsigned int *));
        for (j = 0; j < height; j++)
            img->pixmap[i][j] = (unsigned int*) malloc(width * sizeof(unsigned int));
    }

    return img;
}

PGMImage * new_pgmimage( unsigned int width, unsigned int height, unsigned int max){

    PGMImage *img = (PGMImage *) malloc(sizeof(PGMImage));
    int i;

    img->pixmap = (unsigned int **)malloc(height*sizeof(unsigned int *));
    img->height = height;
    img->width = width;
    img->max = max;
    for (i = 0; i < height; i++)
        img->pixmap[i] = (unsigned int *) malloc(width * sizeof(unsigned int));

    return img;

}

PBMImage * new_pbmimage( unsigned int width, unsigned int height){

    PBMImage *img = (PBMImage *) malloc(sizeof(PBMImage));
    int i;

    img->pixmap = (unsigned int **)malloc(height*sizeof(unsigned int *));
    img->height = height;
    img->width = width;
    for (i = 0; i < height; i++)
        img->pixmap[i] = (unsigned int *) malloc(width * sizeof(unsigned int));

    return img;

}

void del_ppmimage( PPMImage * img){
    // Will need to free every malloc for the pixmap
    int i, j;
    for (i = 0; i < 3; i++){
        for(j = 0; j < img->height; j++)
            free(img->pixmap[i][j]); // Free every row pointer in the map
        free(img->pixmap[i]); // Free the pointer to the row pointers
    }

    free(img);
}

void del_pgmimage( PGMImage * img){

    int i;
    for (i = 0; i < img->height; i++)
        free(img->pixmap[i]); // Free the pointer to the row pointers

    free(img);
}

void del_pbmimage( PBMImage * img){

    int i;
    for (i = 0; i < img->height; i++)
        free(img->pixmap[i]); // Free the pointer to the row pointers

    free(img);
}

// Transformations
// ================================ Transformations ================================

// sepia created
void * sepia(PPMImage * ppm) {

    int row, col, rgb;
    unsigned int newRed, newBlue, newGreen;
    float oldRed, oldBlue, oldGreen; // color IDs which colors are used.

    for (row = 0; row < ppm->height; row++){
        for (col = 0; col < ppm->width; col++){
            oldRed = (float)ppm->pixmap[0][row][col];
            oldGreen = (float)ppm->pixmap[1][row][col];
            oldBlue = (float)ppm->pixmap[2][row][col];

            newRed = 0.393*oldRed + 0.769*oldGreen + 0.189*oldBlue;
            newGreen = 0.349*oldRed + 0.686*oldGreen + 0.168*oldBlue;
            newBlue = 0.272*oldRed + 0.534*oldGreen + 0.131*oldBlue;

            if (newRed > ppm->max)
                newRed = ppm->max;

            if (newBlue > ppm->max)
                newBlue = ppm->max;

            if (newGreen > ppm->max)
                newGreen = ppm->max;

            ppm->pixmap[0][row][col] = newRed;
            ppm->pixmap[1][row][col] = newGreen;
            ppm->pixmap[2][row][col] = newBlue;

        }
    }

}

void * mirror(PPMImage * ppm){
    unsigned int row, color, i, j;

    for ( row = 0; row < ppm->height; row++) {
        i = 0;
        j = ppm->width -1;
        while (i < j){
            for (color = 0; color < 3; color++){
                ppm->pixmap[color][row][j] = ppm->pixmap[color][row][i];
            }
            i += 1;
            j -= 1;
        }

    }

}

PPMImage * thumbnail(PPMImage * ppm, int n){
    unsigned int row, col, color, newrow, newcol;

    PPMImage *newPPM = new_ppmimage((ppm->width + n - 1)/n, (ppm->height + n - 1)/n, ppm->max);

    row = 0;
    col = 0;

    for (newrow = 0; newrow < newPPM->height; newrow++){
        for (newcol = 0; newcol < newPPM->width; newcol++){
            for (color = 0; color < 3; color++) {
                    newPPM->pixmap[color][newrow][newcol] = ppm->pixmap[color][row][col];
            }
            if (col + n >= ppm->width){
                col = 0;
                row += n;
            }else {
                col += n;
            }
        }
    }

    return newPPM;
}

void * n_thumbnails(PPMImage *ppm, int n){
    unsigned int row, col, color;
    PPMImage *thumb = thumbnail(ppm, n);

    for (row = 0; row < ppm->height; row++) {
        for (col = 0; col < ppm->width; col++) {
            for (color = 0; color < 3; color++) {
                ppm->pixmap[color][row][col] = thumb->pixmap[color][row % thumb->height][col % thumb->width];
            }
        }
    }

}



// The modularized version of i and r didn't pass the diff test, so they will be their own commands

void * command_i(PPMImage * ppm, char * out, int colorCode){
    unsigned int row, col, rgb;
    PPMImage *newppm = new_ppmimage(ppm->width, ppm->height, ppm->max);
    for(row = 0; row < ppm->height; row++){
        for(col = 0; col < ppm->width; col++){
            for(rgb = 0; rgb < 3; rgb++){
                if (rgb == colorCode){
                    newppm->pixmap[colorCode][row][col] = ppm->pixmap[colorCode][row][col];
                } else {
                    newppm->pixmap[rgb][row][col] = 0;
                }
            }
        }
    }

    write_ppmfile(newppm, out);
    del_ppmimage(ppm);
    del_ppmimage(newppm);
    exit(0);
}

void * command_r(PPMImage * ppm, char * out, int colorCode){
    unsigned int row, col, rgb;
    PPMImage *newppm = new_ppmimage(ppm->width, ppm->height, ppm->max);
    for(row = 0; row < ppm->height; row++){
        for(col = 0; col < ppm->width; col++){
            for(rgb = 0; rgb < 3; rgb++){
                if (rgb != colorCode){
                    newppm->pixmap[rgb][row][col] = ppm->pixmap[rgb][row][col];
                } else {
                    newppm->pixmap[colorCode][row][col] = 0;
                }
            }
        }
    }

    write_ppmfile(newppm, out);
    del_ppmimage(ppm);
    del_ppmimage(newppm);
    exit(0);
}

// -b transformation from PPM to PBM
PBMImage * to_pbm(PPMImage * ppm, const unsigned int colors[], int numColors){

    // Check the thing about maybe needing to malloc for colors/how to reference values in colors

    PBMImage *pbm = new_pbmimage(ppm->width, ppm->height);
    int row, col, rgb;
    unsigned int color; // avg_rgb stores the avg of the channels, color IDs which colors are used.
    float avg_rgb;

    // Transforming the ppm pixmap to pbm

    for (row = 0; row < pbm->height; row++){
        for (col = 0; col < pbm->width; col++){
            avg_rgb = 0;
            for (rgb = 0; rgb < numColors; rgb++){
                color = colors[rgb];
                avg_rgb += ppm->pixmap[color][row][col];
            }
            avg_rgb /= (float)numColors; // Get the average rgb for the pixel
            if (avg_rgb < ppm->max/2.0) {
                pbm->pixmap[row][col] = 1;
            } else {
                pbm->pixmap[row][col] = 0;
            }
        }
    }

    return pbm;
    // By here, we anticipate that we have built out the PBM pixmap.
    // Since we should have applied all our transformations by now
    // we could create the pixmap at this instance below but to avoid
    // the need to pass extra parameters from cmdLine, write will be called in main.

    // Write the pbm file later at the very end of the program.
}

PGMImage * to_pgm(PPMImage * ppm, const unsigned int colors[], int numColors, long pgmMax){

    // Check the thing about maybe needing to malloc for colors/how to reference values in colors

    PGMImage *pgm = new_pgmimage(ppm->width, ppm->height, pgmMax);
    int row, col;
    unsigned int color; // avg_rgb stores the avg of the channels, color IDs which colors are used.
    float avg_rgb;
    size_t rgb;


    // Transforming the ppm pixmap to pgm

    for (row = 0; row < pgm->height; row++){
        for (col = 0; col < pgm->width; col++){
            avg_rgb = 0;
            for (rgb = 0; rgb < numColors; rgb++){
                color = colors[rgb];
                avg_rgb += ppm->pixmap[color][row][col];
            }
            avg_rgb /= (float)numColors; // Get the average rgb for the pixel
            pgm->pixmap[row][col] = avg_rgb/(float)ppm->max * pgmMax;

        }
    }

    return pgm;
    // By here, we anticipate that we have built out the PBM pixmap.
    // Since we should have applied all our transformations by now
    // we could create the pixmap at this instance below but to avoid
    // the need to pass extra parameters from cmdline, write will be called in main.

    // Write the pgm file later at the very end of the program.
}

// ================================ CMD and Main ================================

Cmdline parse_cmdline(int argc, char *argv[]){
    int opt;
    Cmdline cmd;

    if (argc < 2){
        fprintf(stderr, "Usage: ppmcvt [-bgirsmtno] [FILE]\n");
        exit(-1);
    }

    //initializing to allow for error handling and checking
    cmd.b = false;
    cmd.s = false;
    cmd.m = false;
    cmd.o = false;
    cmd.i = "";
    cmd.r = "";
    cmd.g = 0; // Need to be initialized for null check.
    cmd.t = 0;
    cmd.n = 0;
    cmd.numCommands = 0;


    while ((opt = getopt(argc, argv, "bg:i:r:smt:n:o:")) != -1){
        switch (opt) {
            case 'b':
                cmd.b = true;
                cmd.numCommands += 1;
                break;
            case 'g':
                cmd.g = (unsigned int)strtol(optarg, NULL, 10); //catch the error

                if(cmd.g > 65535){
                    fprintf(stderr, "Error: Invalid max grayscale pixel value: %s; must be less than 65,536\n", optarg); // No accounting for 0?
                    exit(-1);
                }
                cmd.numCommands += 1;

                break;
            case 'i':
                if (strcmp(optarg, "red") == 0 || strcmp(optarg, "blue") == 0 || strcmp(optarg, "green") == 0){
                    cmd.i = optarg;
                } else{
                    fprintf(stderr, "Error: Invalid channel specification (%s); should be 'red' 'green' or 'blue'\n", optarg);
                    exit(-1);
                }
                cmd.numCommands += 1;
                break;
            case 'r':
                cmd.numCommands += 1;
                if (strcmp(optarg, "red") == 0 || strcmp(optarg, "blue") == 0 || strcmp(optarg, "green") == 0){
                    cmd.r = optarg; break;
                } else{
                    fprintf(stderr, "Error: Invalid channel specification (%s); should be 'red' 'green' or 'blue'\n", optarg);
                    exit(-1);
                }
            case 's':
                cmd.s = true;
                cmd.numCommands += 1;
                break;
            case 'm':
                cmd.m = true;
                cmd.numCommands += 1;
                break;
            case 't':
                cmd.t = strtol(optarg, NULL, 10); // t and n are related to the thumbnail
                if (cmd.t <= 8 && cmd.t >= 1){
                    cmd.numCommands += 1;
                    break;
                } else {
                    fprintf(stderr, "Error: invalid scale factor: %s; must be 1-8\n", optarg);
                    exit(-1);
                }
            case 'n':
                cmd.n = strtol(optarg, NULL, 10);
                if (cmd.n <= 8 && cmd.n >= 1){
                    cmd.numCommands += 1;
                    break;
                }else {
                    fprintf(stderr, "Error: invalid scale factor: %s; must be 1-8\n", optarg);
                    exit(-1);
                }
            case 'o':
                cmd.outfilename = optarg;
                cmd.o = true;
                break; // Write output image to the specified file
        }
    }

    if (cmd.numCommands > 1){
        fprintf(stderr, "Error: Multiple transformations specified\n");
        exit(-1);
    }
    // Get the input and output filenames
    cmd.infilename = argv[optind]; // Place a check here for having executed -o in the cmd.

    return cmd;
}

int main(int argc, char *argv[]) {

    Cmdline cmd = parse_cmdline(argc, argv);
    PPMImage *ppm = read_ppmfile(cmd.infilename);

    unsigned int numColors; // Keeping track for color transformations

    // Tracks the color channels in use for conversion
    unsigned int color[3];
    if (strcmp(cmd.i, "") != 0){
        if(strcmp(cmd.i, "red") == 0) {
            command_i(ppm, cmd.outfilename, 0);
        } else if(strcmp(cmd.i, "green") == 0) {
            command_i(ppm, cmd.outfilename, 1);
        } else if (strcmp(cmd.i, "blue") == 0){
            command_i(ppm, cmd.outfilename, 2);
        }
        exit(0);
    } else if (strcmp(cmd.r, "") != 0){
        numColors = 2;
        if(strcmp(cmd.r, "red") == 0) {
           command_r(ppm, cmd.outfilename, 0);
        } else if(strcmp(cmd.r, "green") == 0) {
            command_r(ppm, cmd.outfilename, 1);
        } else if(strcmp(cmd.r, "blue") == 0) {
            command_r(ppm, cmd.outfilename, 2);
        }
    } else{
        numColors = 3;
        for (int i = 0; i < 3; i++)
            color[i] = i;
    }

    if(cmd.m) {
        // mirror
        mirror(ppm);
    }

    if(cmd.s){
        sepia(ppm);
    }

    if(cmd.b == true){
        PBMImage * pbm = to_pbm(ppm, color, numColors); // drop numColors if static allowed
        write_pbmfile(pbm, cmd.outfilename);
        del_ppmimage(ppm);
        del_pbmimage(pbm);
    } else if(cmd.g != 0){
        PGMImage * pgm = to_pgm(ppm, color, numColors, cmd.g); // drop numColors if static allowed
        write_pgmfile(pgm, cmd.outfilename);
        del_ppmimage(ppm);
        del_pgmimage(pgm);
    } else{
        if(cmd.t != 0){
            PPMImage *PPMThumb = new_ppmimage(ppm->width/cmd.t, ppm->height/cmd.t, ppm->max);
            PPMThumb = thumbnail(ppm, cmd.t);
            write_ppmfile(PPMThumb, cmd.outfilename);
            del_ppmimage(ppm);
            del_ppmimage(PPMThumb);
        } else {
            if (cmd.n != 0) {
                n_thumbnails(ppm, cmd.n);
            }
            write_ppmfile(ppm, cmd.outfilename);
            del_ppmimage(ppm);
        }

    }

}
