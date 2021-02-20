#include <stdio.h>
#include <string.h>
#include "qdbmp.h"
#include "qdbmp.c"


typedef struct {
    USHORT bfType;
    UINT bfSize;
    USHORT bfReserved1;
    USHORT bfReserved2;
    UINT bfOffBits;
    UINT biSize;
    UINT biWidth;
    UINT biHeight;
    USHORT biPlanes;
    USHORT biBitCount;
    UINT biCompression;
    UINT biSizeImage;
    UINT biXPelsPerMeter;
    UINT biYPelsPerMeter;
    UINT biClrUsed;
    UINT biClrImportant;
} BMPheader;

USHORT read_u16(FILE *fp) {
    unsigned char b0, b1;
    b0 = getc(fp);
    b1 = getc(fp);
    return ((b1 << 8) | b0);
}

UINT read_u32(FILE *fp) {
    unsigned char b0, b1, b2, b3;
    b0 = getc(fp);
    b1 = getc(fp);
    b2 = getc(fp);
    b3 = getc(fp);
    return ((((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}

int write_u16(USHORT x, FILE *f) {
    UCHAR little[2];

    little[1] = (UCHAR) ((x & 0xff00) >> 8);
    little[0] = (UCHAR) ((x & 0x00ff) >> 0);

    return (f && fwrite(little, 2, 1, f) == 1);
}

int write_u32(UINT x, FILE *f) {
    UCHAR little[4];

    little[3] = (UCHAR) ((x & 0xff000000) >> 24);
    little[2] = (UCHAR) ((x & 0x00ff0000) >> 16);
    little[1] = (UCHAR) ((x & 0x0000ff00) >> 8);
    little[0] = (UCHAR) ((x & 0x000000ff) >> 0);

    return (f && fwrite(little, 4, 1, f) == 1);
}

int NegativeBMP(const char *f_input, const char *f_output) {


    FILE *f = fopen(f_input, "rb");
    if (!f) {
        printf("could not open input file");
        return -1;
    }
    FILE *f1 = fopen(f_output, "wb");
    if (!f1) {
        printf("could not open output file");
        return -1;
    }
    BMPheader bh;

    bh.bfType = read_u16(f);
    bh.bfSize = read_u32(f);
    bh.bfReserved1 = read_u16(f);
    bh.bfReserved2 = read_u16(f);
    bh.bfOffBits = read_u32(f);
    bh.biSize = read_u32(f);
    bh.biWidth = read_u32(f);
    bh.biHeight = read_u32(f);
    bh.biPlanes = read_u16(f);
    bh.biBitCount = read_u16(f);
    bh.biCompression = read_u32(f);
    bh.biSizeImage = read_u32(f);
    bh.biXPelsPerMeter = read_u32(f);
    bh.biYPelsPerMeter = read_u32(f);
    bh.biClrUsed = read_u32(f);
    bh.biClrImportant = read_u32(f);

    if (bh.bfType != 0x4d42 && bh.bfType != 0x4349 && bh.bfType != 0x5450) {
        fclose(f);
        return -1;
    }

    if (
            bh.bfReserved1 != 0 ||
            bh.biPlanes != 1 ||
            (bh.biSize != 40 && bh.biSize != 108 && bh.biSize != 124) ||
            (bh.bfOffBits != 14 + bh.biSize && bh.bfOffBits != 1038 + bh.biSize) ||
            bh.biWidth < 1 || bh.biWidth > 10000 ||
            bh.biHeight < 1 || bh.biHeight > 10000 ||
            !(bh.biBitCount == 24 || bh.biBitCount == 8) ||
            bh.biCompression != 0) {
        fclose(f);
        return -1;
    }
    write_u16(bh.bfType, f1);
    write_u32(bh.bfSize, f1);
    write_u16(bh.bfReserved1, f1);
    write_u16(bh.bfReserved2, f1);
    write_u32(bh.bfOffBits, f1);
    write_u32(bh.biSize, f1);
    write_u32(bh.biWidth, f1);
    write_u32(bh.biHeight, f1);
    write_u16(bh.biPlanes, f1);
    write_u16(bh.biBitCount, f1);
    write_u32(bh.biCompression, f1);
    write_u32(bh.biSizeImage, f1);
    write_u32(bh.biXPelsPerMeter, f1);
    write_u32(bh.biYPelsPerMeter, f1);
    write_u32(bh.biClrUsed, f1);
    write_u32(bh.biClrImportant, f1);
    if (bh.biBitCount == 8) {
        UCHAR c;
        for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 4; j++) {
                fread(&c, 1, sizeof(UCHAR), f);
                if (j != 3)
                    c = ~c;
                fwrite(&c, 1, sizeof(UCHAR), f1);
            }
        }
        while (!feof(f)) {
            fread(&c, 1, sizeof(UCHAR), f);
            fwrite(&c, 1, sizeof(UCHAR), f1);
        }
    } else {
        UCHAR c;
        while (!feof(f)) {
            fread(&c, 1, sizeof(UCHAR), f);
            c = ~c;
            fwrite(&c, 1, sizeof(UCHAR), f1);
        }
    }
    fclose(f);
    fclose(f1);
    return 0;
}


int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("not enough parameters\nexpected format: \nconverter --mine | --theirs <input_name>.bmp <output_name>.bmp");
        return -1;
    } else if (argc > 4) {
        printf("redundant parameters found\nexpected format: \nconverter --mine | --theirs <input_name>.bmp <output_name>.bmp");
        return -1;
    }
    if (strcmp(argv[1], "--mine") == 0) {
        NegativeBMP(argv[2], argv[3]);
    } else if (strcmp(argv[1], "--theirs") == 0) {
        BMP *file;
        file = BMP_ReadFile(argv[2]);
        if (file == NULL) {
            printf("could not read input file");
            return -3;
        }
        if (BMP_GetDepth(file) == 24) {
            UCHAR R, G, B;
            for (UINT i = 0; i < BMP_GetWidth(file); i++)
                for (UINT j = 0; j < BMP_GetHeight(file); j++) {
                    BMP_GetPixelRGB(file, i, j, &R, &G, &B);
                    BMP_SetPixelRGB(file, i, j, ~R, ~G, ~B);
                }
        } else if (BMP_GetDepth(file) == 8) {
            UCHAR R, G, B;
            for (UINT i = 0; i < 256; i++) {
                BMP_GetPaletteColor(file, i, &R, &G, &B);
                BMP_SetPaletteColor(file, i, ~R, ~G, ~B);
            }
        } else {
            printf("unsupported image format");
            return -3;
        }
        BMP_WriteFile(file, argv[3]);
        BMP_Free(file);
    } else {
        printf("unknown algorithm name");
        return -1;
    }
    return 0;
}