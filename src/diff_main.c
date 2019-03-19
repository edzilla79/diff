#include <stdio.h>
#include <errno.h>
#include "diff.h"

int main( int argc, char **argv) {
    int i, j;
    int **w1, **w2;
    char s1[8192];
    char s2[8192];
    struct string2word_s s2w1, s2w2;
    FILE *fptr;
    int character = 0;
    setlocale(LC_ALL, "en_US.UTF-8");

    if ((argc != 3) || ((argv[2][0] != '0') && (argv[2][0] != '1'))) {
        printf("Usage: %s TESTFILE <COMPARISON_TYPE(0 FOR WORD, 1 FOR CHAR)>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    fptr = fopen(argv[1], "r");
    if (fptr == NULL) {
        printf("fopen failed, errno = %d\n", errno);
        exit(EXIT_FAILURE);
    }
    character = (int)(argv[2][0] - '0');

    while (fgets(s1, sizeof(s1), fptr) != 0) {
        fgets(s2, sizeof(s2), fptr);
        alignment(s1, s2, &s2w1, &s2w2, character);
        printwords(&s2w1, &s2w2);
        string2wordtakedown(&s2w1);
        string2wordtakedown(&s2w2);
    }
    fclose(fptr);
    return 1;
}   
