//
// Created by human on 2/17/22.
//

#ifndef SECURITY_STRING_SUPPORT_H
#define SECURITY_STRING_SUPPORT_H

#include "stdlib.h"
#include <stdio.h>
#include "constants.h"

/**
 * as name suggests
 * @param f
 * @return a heap allocated next-line of file-ptr , make sure to free!
 */
char *read_next_line_in_heap(FILE *f) {
    char *line = NULL;
    size_t length = 0;
    size_t read_len = getline(&line, &length, f);
    if (read_len == -1) {
        free(line);
        return NULL;
    }
    return line;
}

/**
 * checks if given src string starts with dest string
 * @param src
 * @param dest
 * @return
 */
bool starts_with(char *src, char *dest) {
    return strncmp(src, dest, strlen(dest)) == 0;
}

FILE *replace_line_in_file(FILE *src, const char *src_loc, char *to_be_replaced, int index) {
    char tmp_name[1024 * 1024];
    memset(tmp_name, 0, 1024 * 1024);
    strcpy(tmp_name, src_loc);
    strncat(tmp_name, ".tmp", 5);
    FILE *tmp = fopen(tmp_name, "w");
    fseek(src, 0, SEEK_SET);
    int counter = 0;
    char *buff;
    while ((buff = read_next_line_in_heap(src)) != NULL) {
        if (counter++ == index)
            fputs(to_be_replaced, tmp);
        else
            fputs(buff, tmp);
        free(buff);
    }

    fflush(tmp);
    fclose(tmp);
    fclose(src);

    remove(src_loc);

    if (rename(tmp_name, src_loc) == -1)
        return NULL;

    return fopen(src_loc, "r+");
}

char *read_nth_line(FILE *f, int n) {
    fseek(f, 0, SEEK_SET);
    for (int i = 0; i < n - 1; ++i) {
        free(read_next_line_in_heap(f));
    }
    return read_next_line_in_heap(f);
}

char** split(char *str, char deli, int *_count) {
    size_t len_string = strlen(str);
    *_count = 1;
    for (int i = 0; i < len_string; ++i) {
        if(str[i] == deli) ++*_count;
    }

    char** ans = calloc(sizeof(char**),*_count);
    char* it = str;
    int i = 0;
    while (it[0] != '\0'){
        ans[i++] = it;
        size_t len_it = strlen(it);

        for (int j = 0; j < len_it; ++j) {
            if(it[j+1] == '\0'){
                it = &it[j+1];
                break;
            }else if(it[j] == deli){
                it[j] = '\0';
                it = &it[j+1];
                break;
            }
        }
    }


    return ans;
}


#endif //SECURITY_STRING_SUPPORT_H
