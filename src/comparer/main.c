#include <stdio.h>
#include <stdbool.h>
#include "qdbmp.h"
#include "qdbmp.c"
int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("a");
        return -1;
    }
    if (argc > 3) {
        printf("a");
        return -1;
    }
    BMP *file_1, *file_2;
    file_1 = BMP_ReadFile(argv[1]);
    if (file_1 == NULL) {
        printf("a");
        return -1;
    }
    file_2 = BMP_ReadFile(argv[2]);
    if (file_2 == NULL) {
        printf("a");
        BMP_Free(file_1);
        return -1;
    }
    if ((BMP_GetWidth(file_1) != BMP_GetWidth(file_2)) || (BMP_GetHeight(file_1) != BMP_GetHeight(file_2))) {
        printf("a");
        BMP_Free(file_1);
        BMP_Free(file_2);
        return -1;
    }
    if (BMP_GetDepth(file_1) != BMP_GetDepth(file_2)) {
        printf("a");
        BMP_Free(file_1);
        BMP_Free(file_2);
        return -1;
    }
    bool equal = true;
    int n = 0;
    UCHAR r1, r2, b1, b2, g1, g2;
    for (int i = 0; i < BMP_GetHeight(file_1); i++) {
        for (int j = 0; j < BMP_GetWidth(file_1); j++) {
            BMP_GetPixelRGB(file_1, j, i, &r1, &g1, &b1);
            BMP_GetPixelRGB(file_2, j, i, &r2, &g2, &b2);
            if (r1 != r2 || g1 != g2 || b1 != b2) {
                equal = false;
                printf("%d %d\n", j, i);
                n++;
                if (n == 100) {
                    BMP_Free(file_1);
                    BMP_Free(file_2);
                    return -1;
                }
            }
        }
    }
    BMP_Free(file_1);
    BMP_Free(file_2);
    if (!equal)
        return -1;
    printf("совпадает");
    return 0;
}
