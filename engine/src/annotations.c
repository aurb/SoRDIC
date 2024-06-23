/*  Software Rendering Demo Engine In C
    Copyright (C) 2024 Andrzej Urbaniak

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#include "engine.h"

FLOAT *ant_t = NULL;
INT *ant_data = NULL;
INT **ant_v = NULL;
FLOAT *ant_last_t = NULL;
INT *ant_last_v = NULL;
INT *ant_prev_v = NULL;
INT ant_count = 0; //total count of annotation records
INT ant_channel_count = 0; //total count of channels
INT next_index = 0;

FLOAT* annotations_last_t() {
    return ant_last_t;
}

INT* annotations_last_v() {
    return ant_last_v;
}

INT* annotations_prev_v() {
    return ant_prev_v;
}

void annotations_track(FLOAT t) {
    int i;
    while(next_index < ant_count && t > ant_t[next_index]) {
#ifdef LOG_ANNOTATIONS
        printf("%f ", ant_t[next_index]);
#endif
        for(i = 0; i < ant_channel_count; i++) {
            //if for processed timestamp and channel i
            //annotation was detected - store it
            if (ant_v[next_index][i] != -1) {
                ant_last_t[i] = ant_t[next_index];
                ant_prev_v[i] = ant_last_v[i];
                ant_last_v[i] = ant_v[next_index][i];
#ifdef LOG_ANNOTATIONS
                printf("%d ", ant_last_v[i]);
#endif
            }
#ifdef LOG_ANNOTATIONS
            else {
                printf("- ");
            }
#endif
        }
#ifdef LOG_ANNOTATIONS
        printf("\n");
#endif
        next_index++;
    }

}

char* next_annotation_token(char **ant_str) {
    if (*ant_str == NULL) {
        return NULL;
    }
    char *token_start = *ant_str;
    char *token_end = *ant_str;

    //Find next occurence of delimiter and return as token
    //characters from start to that delimiter
    while (*token_end) {
        if (*token_end == ';') {
            *token_end = '\0';
            *ant_str = token_end + 1;
            return token_start;
        }
        token_end++;
    }

    //End of annotation string reached without detecting delimiter.
    //Set Annotation pointer to NULL to prevent any further execution of this function.
    //Return last part of string as final token.
    *ant_str = NULL;
    return token_start;
}

int annotations_read(const char *filename) {
    FILE *annotations_file = fopen(filename, "r");
    int column_cnt = 0, line_cnt = 0;
    char line_buf[100];
    char c, *t = NULL;
    INT i = 0, j = 0;
    if (annotations_file == NULL) {
        printf("Error opening %s\n", filename);
        return 1;
    }

    while ((c = fgetc(annotations_file)) != '\n') {
        if (c == ';') {
            column_cnt++;
        }
    }
    fseek(annotations_file, 0L, SEEK_SET);
    while ((c = fgetc(annotations_file)) != EOF) {
        if (c == '\n') {
            line_cnt++;
        }
    }
    fseek(annotations_file, 0L, SEEK_SET);
    ant_channel_count = column_cnt-1;

    ant_t = calloc(line_cnt, sizeof(FLOAT));
    ant_data = calloc(line_cnt*ant_channel_count, sizeof(INT));
    ant_v = calloc(line_cnt, sizeof(INT*));
    for (i=0; i < line_cnt; i++) {
        ant_v[i] = ant_data + i*ant_channel_count;
    }
    ant_last_t = calloc(ant_channel_count, sizeof(FLOAT));
    ant_last_v = calloc(ant_channel_count, sizeof(INT));
    ant_prev_v = calloc(ant_channel_count, sizeof(INT));
    for (i=0; i < ant_channel_count; i++) {
        ant_last_t[i] = -1.0;
        ant_last_v[i] = -1;
        ant_prev_v[i] = -1;
    }

    ant_count = line_cnt;

    i = 0;
    char *rest = NULL;
    while (fgets(line_buf, sizeof(line_buf), annotations_file)) {
        rest = line_buf;
        t = next_annotation_token(&rest);
        if (t == NULL || t[0] == '\0') //skip empty lines OR lines with no timestamp
            continue;
        else {
            ant_t[i] = atof(t);
        }

        j = 0;
        while ((t = next_annotation_token(&rest)) != NULL) {
            if (t[0] != '\0')
                ant_v[i][j] = atoi(t);
            else
                ant_v[i][j] = -1;
            j++; //next channel/column
        }
        i++; //next line
    }

    fclose(annotations_file);
    return 0;
}

void annotations_free() {
    free(ant_t);
    free(ant_data);
    free(ant_v);
    free(ant_last_t);
    free(ant_last_v);
    free(ant_prev_v);
}

