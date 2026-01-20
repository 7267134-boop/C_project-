#ifndef PRECOMPILER_H
#define PRECOMPILER_H
#include <stdio.h>
#include <stdlib.h>
#include "defs.h"




typedef struct node {
    char *name;
    char *content;
    struct node *next;
} node;

int is_reserved_word(char *name);
int check_macro_definition_line(char *line);
int process_file(char *file_name, node **head);
int snprintf(char *str, size_t size, const char *format, ...);


#endif

