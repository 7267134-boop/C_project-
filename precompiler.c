#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "precompiler.h"
#include "firstSyntax.h"

/**
 * @brief Checks if a word is a reserved keyword.
 *
 * The function compares the provided word against a list of reserved words
 * used in the program and returns whether the word is reserved.
 *
 * @param name The word to be checked.
 * @return int 1 if the word is reserved, 0 otherwise.
 */
int is_reserved_word(char *name) {
    int i;
    /* List of reserved words */
    const char *reserved_words[RESERVED_WORDS] = {
        "not", "clr", "inc", "dec", "jmp", "bne", "red", "prn",
        "jsr", "mov", "cmp", "add", "sub", "lea", "rts", "stop",
        "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "z", "macr",
        "endmacr", "data", "string", "entry", "exter"
    };

    /* Loop through reserved words to find a match */
    for (i = 0; i < RESERVED_WORDS; i++) {
        if (strcmp(name, reserved_words[i]) == 0) {
            return 1;  /* Return 1 if the word is reserved */
        }
    }

    return 0;  /* Return 0 if the word is not reserved */
}

/**
 * @brief Checks if a line is a valid macro definition.
 *
 * This function ensures that a macro definition line follows the correct syntax.
 * It validates both the macro declaration ("macr") and its termination ("endmacr").
 *
 * @param line The line to be checked.
 * @return int 1 if the line is valid, 0 otherwise.
 */
int check_macro_definition_line(char *line) {
    char *end, *start, *end_name;
    char mcro_name[MACRO];

    /* Remove trailing whitespace */
    end = line + strlen(line) - 1;
    while (end > line && isspace((unsigned char)*end)) end--;
    end[1] = '\0';

    /* Check if the line starts with "macr" */
    if (strncmp(line, "macr", 4) == 0) {
        /* Find the macro name */
        start = line + 4;
        while (*start == ' ') start++;
        end_name = start;
        while (*end_name != ' ' && *end_name != '\n' && *end_name != '\0') end_name++;
        strncpy(mcro_name, start, end_name - start);
        mcro_name[end_name - start] = '\0';

        /* Check if the macro name is valid */
        if (is_reserved_word(mcro_name)) {
            return 0; /* Invalid macro name */
        }

        /* Check for extra characters after the macro name */
        while (*end_name == ' ') end_name++;
        if (*end_name != '\0') {
            return 0; /* Extra characters after macro name */
        }
    } else if (strncmp(line, "endmacr", 7) == 0) {
        /* Check for extra characters after "endmacr" */
        start = line + 7;
        while (*start == ' ') start++;
        if (*start != '\0') {
            return 0; /* Extra characters after "endmacr" */
        }
    }

    return 1; /* Line is valid */
}

/**
 * @brief Processes a file and handles macros.
 *
 * The function reads the file, replaces macros, and writes the output to a new file.
 * It handles macro definitions, replacements, and validates macro names.
 *
 * @param file_name The name of the source file to be processed.
 * @param head Pointer to the head of the linked list for storing macros.
 * @return int 1 on success, 0 on failure.
 */
int process_file(char *file_name, node **head) {
    FILE *fp, *fp_new;
    char str[MAX_LINE_LENGTH];
    int inside_mcro = 0;
    char *mcro_name = NULL;
    char *mcro_content = NULL;
    int mcro_length = 0;
    node *new_node, *current;
    char new_file_name[MAX_LINE_LENGTH];
    char *dot;
    char *trimmed_str;
    char as[] = ".as";
    char file_as[MAX_LINE_LENGTH];

    /* Debug print: file name */
    printf("Opening source file: %s\n", file_name);

    strcpy(file_as, file_name);
    strcat(file_as, as);

    /* Open source file for reading */
    if ((fp = fopen(file_as, "r")) == NULL) {
        printf("Error opening file %s.\n", file_name);
        return 0;
    }

    /* Generate new file name with .am extension */
    dot = strrchr(file_name, '.');
    if (dot != NULL) {
        *dot = '\0';
    }
    snprintf(new_file_name, MAX_LINE_LENGTH, "%s.am", file_name);

    /* Open new file for writing */
    printf("Creating new file: %s\n", new_file_name);
    if ((fp_new = fopen(new_file_name, "w")) == NULL) {
        printf("Error creating file %s.\n", new_file_name);
        fclose(fp);
        return 0;
    }

    /* Read the file line by line */
    while (fgets(str, MAX_LINE_LENGTH, fp) != NULL) {
        /* Remove leading whitespace */
        trimmed_str = str;
        while (isspace((unsigned char)*trimmed_str)) {
            trimmed_str++;
        }

        /* Skip empty lines or lines starting with ';' */
        if (*trimmed_str == '\0' || *trimmed_str == ';') {
            continue; /* Skip unwanted lines */
        }

        /* Check if the line defines a macro */
        if (strncmp(trimmed_str, "macr", 4) == 0) {
            if (!check_macro_definition_line(trimmed_str)) {
                fprintf(stdout, "Error:");
                fclose(fp);
                fclose(fp_new);
                remove(new_file_name);  /* Remove the new file in case of error */
                return 0;
            }
            inside_mcro = 1;
            mcro_name = (char *)malloc(strlen(trimmed_str + 5) + 1); /* Save macro name */
            if (is_reserved_word(mcro_name)) {
                printf("Memory allocation error.\n");
                fclose(fp);
                fclose(fp_new);
                remove(new_file_name);  /* Remove the new file in case of error */
                return 0;
            }
            if (mcro_name == NULL) {
                printf("Memory allocation error.\n");
                fclose(fp);
                fclose(fp_new);
                remove(new_file_name);  /* Remove the new file in case of error */
                return 0;
            }
            strcpy(mcro_name, trimmed_str + 5);
            mcro_name[strcspn(mcro_name, "\n")] = '\0'; /* Remove newline from macro name */
            mcro_content = (char *)malloc(MAX_LINE_LENGTH * sizeof(char));
            if (mcro_content == NULL) {
                printf("Memory allocation error.\n");
                free(mcro_name);
                fclose(fp);
                fclose(fp_new);
                remove(new_file_name);  /* Remove the new file in case of error */
                return 0;
            }
            mcro_length = 0;
            continue;
        }

        /* Handle end of macro definition */
        if (inside_mcro) {
            if (strncmp(trimmed_str, "endmacr", 7) == 0) {
                if (!check_macro_definition_line(trimmed_str)) {
                    fprintf(stdout, "Error:");
                    fclose(fp);
                    free(mcro_name);
                    free(mcro_content);
                    fclose(fp_new);
                    remove(new_file_name);  /* Remove the new file in case of error */
                    return 0;
                }
                inside_mcro = 0;

                /* Save the macro content in a linked list */
                new_node = (node *)malloc(sizeof(node));
                if (new_node == NULL) {
                    printf("Memory allocation error.\n");
                    free(mcro_name);
                    free(mcro_content);
                    fclose(fp);
                    fclose(fp_new);
                    remove(new_file_name);  /* Remove the new file in case of error */
                    return 0;
                }
                new_node->name = mcro_name;
                new_node->content = mcro_content;
                new_node->next = NULL;

                /* Insert the macro node into the linked list */
                if (*head == NULL) {
                    *head = new_node;
                } else {
                    current = *head;
                    while (current->next != NULL) {
                        current = current->next;
                    }
                    current->next = new_node;
                }
                continue;
            } else {
                /* Add content to the macro */
                strcpy(mcro_content + mcro_length, str);
                mcro_length += strlen(str);
                continue;
            }
        }

        /* Replace macros in memory */
        current = *head;
        while (current != NULL) {
            if (strstr(trimmed_str, current->name) != NULL) {
                fputs(current->content, fp_new); /* Replace with macro content */
                break;
            }
            current = current->next;
        }

        /* Write line if no macro replacement occurred */
        if (current == NULL) {
            fputs(str, fp_new);
        }
    }

    /* Close both files */
    fclose(fp);
    fclose(fp_new);
    first_syntax(new_file_name);
    return 1;
}

