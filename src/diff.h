#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include "utf8.h"
//#define DEBUG

struct edit_s {
    int *s1, *s2;
    int n;
    int diff;
    int cumulativediff;
    struct edit_s *next;
};

struct wordinfo_s {
    int *w;
    int wlen;
    int diff;
};

struct string2word_s {
    struct wordinfo_s *winfo;
    int numwords;
};

//number of words in string
int wordcount(char *s);
void remove_punct_and_make_lower_case(struct wordinfo_s *winfo);
void wordsplitarray(char *s, struct string2word_s *s2w);
void charsplitarray(char *s, struct string2word_s *s2w);
void parsestringarray(char *s, struct string2word_s *s2w, int character);
void wordnormsetup(struct wordinfo_s *unnormwords, int numwords, struct wordinfo_s **normwords);
void leven(struct string2word_s *s2w1, struct string2word_s *s2w2);
int alignment(char *s1, char *s2, struct string2word_s *s2w1, struct string2word_s *s2w2, int character);
void wordnormtakedown(struct wordinfo_s *normwords, int numwords);
void string2wordtakedown(struct string2word_s *s2w);
void printwords(struct string2word_s *s2w1, struct string2word_s *s2w2);
