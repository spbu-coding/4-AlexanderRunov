#include <stdio.h>
#include <stdbool.h>
#include "qdbmp1.c"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("not enough parameters");
        return -1;
    }
    if (argc > 3) {
        printf("redundant parameters found");
        return -1;
    }
    BMP *file_1, *file_2;
    file_1 = bmpReadFile(argv[1]);
    if (file_1 == NULL) {
        printf("could not read input file");
        return -1;
    }
    file_2 = bmpReadFile(argv[2]);
    if (file_2 == NULL) {
        printf("could not read input file");
        bmpFree(file_1);
        return -1;
    }
    if ((bmpGetWidth(file_1) != bmpGetWidth(file_2)) || (bmpGetHeight(file_1) != bmpGetHeight(file_2))) {
        printf("different sizes");
        bmpFree(file_1);
        bmpFree(file_2);
        return -1;
    }
    if (bmpGetDepth(file_1) != bmpGetDepth(file_2)) {
        printf("different depth");
        bmpFree(file_1);
        bmpFree(file_2);
        return -1;
    }
    bool equal = true;
    int n = 0;
    UCHAR r1, r2, b1, b2, g1, g2;
    for (UINT i = 0; i < bmpGetHeight(file_1); i++) {
        for (UINT j = 0; j < bmpGetWidth(file_1); j++) {
            bmpGetPixelRgb(file_1, j, i, &r1, &g1, &b1);
            bmpGetPixelRgb(file_2, j, i, &r2, &g2, &b2);
            if (r1 != r2 || g1 != g2 || b1 != b2) {
                equal = false;
                printf("%lu %lu\n", j, i);
                n++;
                if (n == 100) {
                    bmpFree(file_1);
                    bmpFree(file_2);
                    return -1;
                }
            }
        }
    }
    bmpFree(file_1);
    bmpFree(file_2);
    if (!equal)
        return -1;
    return 0;
}
