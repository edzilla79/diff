#include "diff.h"

//count unicode words separated by space
int wordcount(char *s) {
    int curchar;
    int i = 0;
    int prevspace = 1;
    int wc = 0;
    while ((curchar=u8_nextchar(s, &i)) != 0) {
        if (!isspace(curchar)) {
            //capture start of word
            if (prevspace) {
                wc ++;
            }
        }
        prevspace = isspace(curchar);
    }
    return wc;
}

//only remove punct and word ends
void remove_punct_and_make_lower_case(struct wordinfo_s *winfo)
{
    int i = 0;
    int j = 0;
    int wordlength = winfo->wlen;
    int *word = winfo->w;
    while (i < wordlength) {
       if (ispunct(word[i]) && \
           (((i != 0) && word[i-1] != ' ' && !ispunct(word[i-1])) ||
            ((i != wordlength-1) && word[i+1] != ' ' && !ispunct(word[i+1])))) {
          i++;
       }
       else if (isupper(word[i])) {
          word[j++] = tolower(word[i++]);
       }
       else if (i == j) {
          i++;
          j++;
       }
       else {
          word[j++] = word[i++];
       }
    }
    word[j] = '\0';
    winfo->wlen = j;
}

//parses unicode string into words 
void wordsplitarray(char *s, struct string2word_s *s2w) {
    int i = 0, j = 0;
    int curchar, prevspace = 1;
    int wc = 0;
    int startpos = 0, endpos = 0;

    s2w->numwords = wordcount(s);
    struct wordinfo_s *winfo = (struct wordinfo_s *)calloc(s2w->numwords, sizeof(struct wordinfo_s));
    s2w->winfo = winfo;

    while ((curchar=u8_nextchar(s, &i)) != 0) {
        if (isspace(curchar)) {
            //capture end of word
            if (!prevspace) {
                endpos = i-1;
                winfo[wc].w = (int *)calloc(endpos - startpos + 1, sizeof(int));
                winfo[wc].wlen = endpos - startpos;
                j = 0;
                while (startpos != endpos) {
                    winfo[wc].w[j++] = u8_nextchar(s, &startpos);
                }
                
                winfo[wc].w[j] = '\0';
                startpos ++;
                wc ++;
            }
            else {
                startpos = i;
            }
        }
        prevspace = isspace(curchar);
    }
    if (!prevspace) { //endword
        endpos = i;
        winfo[wc].w = (int *)calloc(endpos - startpos + 1, sizeof(int));
        winfo[wc].wlen = endpos - startpos;
        j = 0;
        while (startpos != endpos) {
            winfo[wc].w[j++] = u8_nextchar(s, &startpos);
        }
        winfo[wc].w[j] = '\0';
        printf("%ls\n", winfo[wc].w);
    }
}
//parses unicode string into char
void charsplitarray(char *s, struct string2word_s *s2w) {
    int i = 0;
    int wc = 0;
    int curchar;
    s2w->numwords = u8_strlen(s); 
    struct wordinfo_s *winfo = (struct wordinfo_s *)calloc(s2w->numwords, sizeof(struct wordinfo_s));
    s2w->winfo = winfo;
    while ((curchar = u8_nextchar(s, &i)) != 0) {
        winfo[wc].w = (int *)calloc(2, sizeof(int));
        winfo[wc].w[0] = curchar;
        winfo[wc].w[1] = '\0';
        winfo[wc].wlen = 1;
        wc ++;
    }    
}
//parses unicode string into units 
void parsestringarray(char *s, struct string2word_s *s2w, int character) {
    int wc;
    int startpos = 0, endpos = 0;
    int i = 0, j = 0; 
    int curchar;
    int prevspace = 1;

    if (!character) {
        wordsplitarray(s, s2w);
    }
    else {        
        charsplitarray(s, s2w);
    }
}
//mapping.  unnormalized words -> normalized words
void wordnormsetup(struct wordinfo_s *unnormwords, int numwords, struct wordinfo_s **normwords) {
    int i;
    struct wordinfo_s *nw = (struct wordinfo_s *)calloc((1 + numwords), sizeof(struct wordinfo_s));
    *normwords= nw;
    for (i = 0; i < numwords; i++) {
        nw[i].wlen = unnormwords[i].wlen;
        nw[i].w = (int *)calloc(nw[i].wlen + 1, sizeof(int));
        memcpy(nw[i].w, unnormwords[i].w, sizeof(int) * (nw[i].wlen + 1));
        remove_punct_and_make_lower_case(&(nw[i]));
    }
}

void wordnormtakedown(struct wordinfo_s *normwords, int numwords) {
    int i;
    for (i = 0; i < numwords; i++) {
        free(normwords[i].w);
    }
    free(normwords);
}

void string2wordtakedown(struct string2word_s *s2w) {
    int i;
    for (i = 0; i < s2w->numwords; i++) {
        free(s2w->winfo[i].w);
    }
    free(s2w->winfo);    
}

//does levenstein alignment to find best path
void leven(struct string2word_s *s2w1, struct string2word_s *s2w2)
{
    int i, j, la, lb;
    la = s2w1->numwords, lb = s2w2->numwords;
    struct wordinfo_s *ta, *tb;
    struct edit_s **tbl = (struct edit_s **)calloc((1 + la), sizeof(struct edit_s *));
    wordnormsetup(s2w1->winfo, la, &ta);
    wordnormsetup(s2w2->winfo, lb, &tb);

    for (i = 0; i < 1 + la; i++) {
        tbl[i] = (struct edit_s *)calloc((1 + lb), sizeof(struct edit_s));
        tbl[i][0].s1 = (i != 0) ? s2w1->winfo[i-1].w : NULL;
        tbl[i][0].s2 = NULL;
        tbl[i][0].n = i;
        tbl[i][0].next = (i != 0) ? &(tbl[i-1][0]) : NULL;
        tbl[i][0].diff = 1;
        tbl[i][0].cumulativediff = i;
        #ifdef DEBUG
            printf("size=%d %d %ls %ls %d\n", i, 0, tbl[i][0].s1, tbl[i][0].s2, s2w1->winfo[i-1].wlen);
        #endif
    }
    for (i = 0; i < 1 + lb; i++) {
        tbl[0][i].s1 = NULL;
        tbl[0][i].s2 = (i != 0)? s2w2->winfo[i-1].w : NULL;
        tbl[0][i].n = i;
        tbl[0][i].next = (i != 0) ? &(tbl[0][i-1]) : NULL;
        tbl[0][i].diff = 1;
        tbl[0][i].cumulativediff = i;
        #ifdef DEBUG
            printf("size=%d %d %ls %ls %d\n", 0, i, tbl[0][i].s1, tbl[0][i].s2, tbl[0][i].diff);
        #endif
    }
    int diff = 0;
    for (i = 1; i < 1+la; i++) {
        for (j = 1; j < 1+lb; j++) {                 
            diff = (ta[i-1].wlen != tb[j-1].wlen) || 
                (memcmp(ta[i-1].w, tb[j-1].w, ta[i-1].wlen * sizeof(int)) != 0);

                #ifdef DEBUG
                    printf("size=%d %d %ls %ls\n", ta[i-1].wlen, tb[j-1].wlen, ta[i-1].w, tb[j-1].w);
                #endif

            if ((tbl[i-1][j-1].n + diff > tbl[i-1][j].n + 1) || 
                ((tbl[i-1][j-1].n + diff == tbl[i-1][j].n + 1) && 
                 (tbl[i-1][j-1].cumulativediff > tbl[i-1][j].cumulativediff))) {
                if ((tbl[i-1][j].n + 1 > tbl[i][j-1].n + 1) ||
                    ((tbl[i-1][j].n + 1 == tbl[i][j-1].n + 1) && 
                     (tbl[i-1][j].cumulativediff > tbl[i][j-1].cumulativediff))) {
                    tbl[i][j].n = tbl[i][j-1].n + 1;
                    tbl[i][j].s1 = NULL;
                    tbl[i][j].s2 = s2w2->winfo[j-1].w;//insertion
                    tbl[i][j].next = &tbl[i][j-1];
                    tbl[i][j].diff = 1;
                    tbl[i][j].cumulativediff += 1;
                    #ifdef DEBUG
                        printf("[%d][%d] from [%d][%d]. s1=%ls s2=%ls diff =%d ptr=%x\n", 
                          i, j, i, j-1, tbl[i][j-1].s1, tbl[i][j-1].s2, tbl[i][j-1].diff,tbl[i][j].next);
                    #endif
                }
                else {
                    tbl[i][j].n = tbl[i-1][j].n + 1;
                    tbl[i][j].s1 = s2w1->winfo[i-1].w;//deletion
                    tbl[i][j].s2 = NULL;
                    tbl[i][j].next = &tbl[i-1][j];
                    tbl[i][j].diff = 1;
                    tbl[i][j].cumulativediff += 1;
                    #ifdef DEBUG
                        printf("[%d][%d] from [%d][%d]. s1=%ls s2=%ls diff =%d ptr=%x\n", 
                            i, j, i-1, j, tbl[i-1][j].s1, tbl[i-1][j].s2, tbl[i-1][j].diff,tbl[i][j].next);
                    #endif
                }
            }
            else {
                if ((tbl[i-1][j-1].n + diff > tbl[i][j-1].n + 1) ||
                    ((tbl[i-1][j-1].n + diff == tbl[i][j-1].n + 1) && 
                     (tbl[i-1][j-1].cumulativediff > tbl[i][j-1].cumulativediff))) {
                    tbl[i][j].n = tbl[i][j-1].n + 1;
                    tbl[i][j].s1 = NULL;
                    tbl[i][j].s2 = s2w2->winfo[j-1].w; //insertion
                    tbl[i][j].next = &tbl[i][j-1];
                    tbl[i][j].diff = 1;
                    tbl[i][j].cumulativediff += 1;
                    #ifdef DEBUG
                        printf("[%d][%d] from [%d][%d]. s1=%ls s2=%ls diff =%d ptr=%x\n", 
                            i, j, i, j-1, tbl[i][j-1].s1, tbl[i][j-1].s2, tbl[i][j-1].diff,tbl[i][j].next);
                    #endif
                }
                else {
                    tbl[i][j].n = tbl[i-1][j-1].n + diff;
                    tbl[i][j].s1 = s2w1->winfo[i-1].w;//substitution or good
                    tbl[i][j].s2 = s2w2->winfo[j-1].w;
                    tbl[i][j].next = &tbl[i-1][j-1];
                    tbl[i][j].diff = diff;
                    tbl[i][j].cumulativediff += diff;
                    #ifdef DEBUG
                        printf("[%d][%d] from [%d][%d]. s1=%ls s2=%ls diff =%d ptr=%x\n", 
                            i, j, i-1, j-1, tbl[i-1][j-1].s1, tbl[i-1][j-1].s2, tbl[i-1][j-1].diff,tbl[i][j].next);
                    #endif
                }
            }
            #ifdef DEBUG
                printf("[%d][%d] s1=%ls s2=%ls diff=%d\n", i, j, tbl[i][j].s1, tbl[i][j].s2, tbl[i][j].diff);
            #endif
        }
    }
    #ifdef DEBUG
        for (i = 0; i <= la; i++){
            for (j = 0; j <= lb; j++) {
                printf("[%d][%d]\n", i, j);
                printf("%ls %ls n=%d diff=%d\n", tbl[i][j].s1, tbl[i][j].s2, tbl[i][j].n, tbl[i][j].diff);
            }
        }
    #endif

    struct edit_s *tedit = &tbl[la][lb];
    tedit = &tbl[la][lb];
    int ai = la-1, bi = lb-1 ;
    while (tedit) {
        if ((ai != -1) && (tedit->s1 != NULL)) {
            s2w1->winfo[ai].diff = tedit->diff;
            ai --;
        }
        if ((bi != -1) && (tedit->s2 != NULL)) {
            #ifdef DEBUG
                printf("b=%d ptr=%x\n", bi, (unsigned int)tedit);
            #endif
            s2w2->winfo[bi].diff = tedit->diff;
            bi --;
        }
        tedit = tedit->next;    
    }    
          
    for (i = 0; i < 1+la; i++) {
        free(tbl[i]);
    }
    free(tbl);
    wordnormtakedown(ta, la);
    wordnormtakedown(tb, lb);
}
//alignment of 2 strings and its best path
int alignment(char *s1, char *s2, struct string2word_s *s2w1, struct string2word_s *s2w2, int character) {
    setlocale(LC_ALL, "en_US.UTF-8");
    parsestringarray(s1, s2w1, character);
    parsestringarray(s2, s2w2, character);
    leven(s2w1, s2w2); 
    return 1;
}

void printwords(struct string2word_s *s2w1, struct string2word_s *s2w2) {
    int i;
    printf("Transcript1: ");
    for (i = 0; i < s2w1->numwords; i++) {
        printf("(%ls,", s2w1->winfo[i].w);
        printf("%d) ", s2w1->winfo[i].diff);
    }
    printf("\n");
    printf("Transcript2: ");
    for (i = 0; i < s2w2->numwords; i++) {
        printf("(%ls,", s2w2->winfo[i].w);
        printf("%d) ", s2w2->winfo[i].diff);
    }
    printf("\n");  
}
/*
int main() {
    int i, j ;
    int **w1, **w2;
    char s1[8192];
    char s2[8192];
    struct string2word_s s2w1, s2w2;
    FILE *fptr = fopen("testset.txt", "r"); //need to set to character = 0 mode in alignment
    //FILE *fptr = fopen("chineseout.csv", "r");
    setlocale(LC_ALL, "en_US.UTF-8");
    while (fgets(s1, sizeof(s1), fptr) != 0) {
        fgets(s2, sizeof(s2), fptr);
        alignment(s1, s2, &s2w1, &s2w2, 0); 

        printwords(&s2w1, &s2w2);
        string2wordtakedown(&s2w1);
        string2wordtakedown(&s2w2);

    }
    fclose(fptr);
}*/
