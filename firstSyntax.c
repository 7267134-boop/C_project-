#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "precompiler.h"
#include "firstSyntax.h"
#include "secendSyntax.h"

void first_syntax(const char *file_name) {
    FILE *source_file; /* Pointer to the file */
    CodingTable** table;
    SymbolTable** symbolHead;
    EntryTable** entryHead; 
    ExternTable** externHead;

    /* Allocate memory for the double pointer tables */
    table = (CodingTable**)malloc(sizeof(CodingTable*)); 
    if (table == NULL) {
        printf("Memory allocation failed for table.\n");
        exit(1);
    }

    symbolHead = (SymbolTable**)malloc(sizeof(SymbolTable*)); 
    if (symbolHead == NULL) {
        printf("Memory allocation failed for symbolHead.\n");
        free(table); /* Free previously allocated memory */
        exit(1);
    }

    entryHead = (EntryTable**)malloc(sizeof(EntryTable*)); 
    if (entryHead == NULL) {
        printf("Memory allocation failed for entryHead.\n");
        free(table);
        free(symbolHead);
        exit(1);
    }

    externHead = (ExternTable**)malloc(sizeof(ExternTable*)); 
    if (externHead == NULL) {
        printf("Memory allocation failed for externHead.\n");
        free(table);
        free(symbolHead);
        free(entryHead);
        exit(1);
    }

    /* Initialize all tables to NULL */
    *table = NULL;
    *symbolHead = NULL;
    *entryHead = NULL;
    *externHead = NULL;
    
    /* Open the file for reading */
    source_file = fopen(file_name, "r");

    /* Check if the file was opened successfully */
    if (source_file == NULL) {
        printf("Error: could not open source file %s.\n", file_name); /* Print error if file can't be opened */
        return; /* Exit function if file opening fails */
    }

    /* File processing logic goes here */
    printf("Processing file: %s\n", file_name); /* Indicate that file processing has started */

    process_first_pass(source_file, table, symbolHead, entryHead, externHead);
    secend_syntax(*table, *symbolHead, *entryHead, *externHead, file_name);


    /* Close the file after processing is complete */
    fclose(source_file); /* Always close the file to free resources */
}

void process_first_pass(FILE *file, CodingTable** table, SymbolTable** symbol, EntryTable** entryHead, ExternTable** externHead) {
    char line[MAX_LINE_LENGTH];
    char local_word[MAX_LINE_LENGTH];
    char extern_word[MAX_LINE_LENGTH];
    char local_line[MAX_LINE_LENGTH];
    char *word;
    int lineFile = 0;

    while (fgets(line, sizeof(line), file)) {
        remove_extra_spaces(line);
        strcpy(local_line, line); 
        word = strtok(line, " \t\n");  /* Split the line by delimiters */
        lineFile += 1;

        while (word != NULL) {
            /* Copy word to local_word */
            strcpy(local_word, word);
            word = strtok(NULL, " \t\n");
            
            if(word != NULL && get_next_word(local_word, local_line) == NULL){
                strcpy(local_word, word);
                word = strtok(NULL, " \t\n");
            }

            if(is_extern_entry(extern_word)) {
                strcpy(extern_word,local_word);
                continue;
            }
            
            if (is_first_symbol(local_word)) {
                handle_first_symbol(local_word, table, symbol); 
            } else if (is_symbol(local_word)) {
                handle_symbol(local_word, local_line, table);
            } else if (is_directive(local_word)) {
                handle_directive(local_word, line, table); 
            } else if (is_extern_entry(local_word)) {
                handle_extern_entry(local_word, word, local_line, table, externHead, entryHead);
            } else if (is_opcode(local_word)) {
                handle_opcode(local_word, word, local_line, table);
            } else if (is_register(local_word)) {
                handle_register(local_word, word, local_line, table);
            } else if (is_number(local_word)) {
                handle_number(local_word, local_line, table);
            } else if (is_letter(local_word)) {
                handle_letter(local_word, local_line, table);
            } else if (is_other(local_word)) {
                handle_other(local_word, local_line, &lineFile, table);
            }
            strcpy(extern_word,local_word);
        }
    }
}

int is_first_symbol(char *word) {
    int len;
    if (is_symbol(word)) {
        len = strlen(word);  /* Get the length of the word */
        if (len > 0 && word[len - 1] == ':') {
            return 1;  /* Return 1 if there is a colon at the end */
        }
    }
    return 0;  /* Return 0 if there is no colon at the end */
}

int is_symbol(char *word) {
    int i;
    i = 0;
    /* Loop through each character in the string */
    while (word[i] != '\0') {
        if(word[i] == ':'){
            i++;
            continue;
        }
        /* Check if the character is an uppercase letter */
        if (!isupper(word[i])) {
            return 0;  /* Found an uppercase letter */
        }
        i++;
    }

    return 1;  /* No uppercase letters found */
}

void handle_first_symbol(const char *word, CodingTable **table, SymbolTable **symbolTable) {
    size_t len;
    char first_symbol[MAX_LINE_LENGTH];
    int symbol_address;

    if (word == NULL || table == NULL || symbolTable == NULL) {
        return;
    }

    /* Remove colon from the word if it exists */
    len = strlen(word);
    if (len == 0) {
        printf("Error: Empty symbol\n");
        return;
    }

    /* Ensure we don't copy the colon */
    if (word[len - 1] == ':') {
        strncpy(first_symbol, word, len - 1);  /* Copy symbol without the colon */
        first_symbol[len - 1] = '\0';          /* Ensure null termination */
    } else {
        strncpy(first_symbol, word, len);      /* Copy the whole word */
        first_symbol[len] = '\0';              /* Ensure null termination */
    }

    symbol_address = get_next_decimal_address(*table);
    append_symbol_node(symbolTable, first_symbol, symbol_address);
}

void handle_symbol(const char *word, const char *line, CodingTable **table) {
    append_node(table, "000000000000000", word, "symbol");
}

int is_directive(char *word) {
    return (strcmp(word, ".data") == 0 || strcmp(word, ".string") == 0);
}

void handle_directive(const char *word, const char *line, CodingTable **table) {
    /*if (strcmp(word, ".string") == 0) {
        handle_letter(word, line, table);
    } else {
        handle_number(word, line, table);
    }*/
}

int is_extern_entry(char *word) {
    return (strcmp(word, ".entry") == 0 || strcmp(word, ".extern") == 0);
}

void handle_extern_entry(const char *local_word, const char *word, const char *line, CodingTable **table, ExternTable** externHead, EntryTable** entryHead) {
    int DecimalAddress;   /* Use int directly, not a pointer */
    if (word == NULL || externHead == NULL) {
        printf("Error: head is NULL.\n");
        return;
    }
    /* Check if the directive is .entry */
    if (strcmp(local_word, ".entry") == 0) {
        DecimalAddress = UNKNOWN_ADDRESS;        /* Set DecimalAddress */
        append_entry_node(entryHead, DecimalAddress, word);  /* Append entry */
    
    } else {  /* If it's not .entry, handle .extern */
        DecimalAddress = UNKNOWN_ADDRESS;         /* Set DecimalAddress */
        append_extern_node(externHead, DecimalAddress, word); /* Append entry */
    }
}

int is_opcode(char *word) {
    char *opcodes[] = {"mov", "cmp", "add", "sub", "lea", "clr", "not", "inc", "dec", "jmp", "bne", "red", "prn", "jsr", "rts", "stop",  NULL};
    int i = 0;
    while (opcodes[i] != NULL) {
        if (strcmp(word, opcodes[i]) == 0) {
            return 1;
        }
        i++;
    }
    return 0;
}

void handle_opcode(char *local_word, char *word, char *line, CodingTable **table) {
    char result[16];
    const char *opcode, *source, *target, *are;
    

    if (strcmp(local_word, "mov") == 0) {
        opcode = "0000";
        source = get_source(word);
        target = get_target(word, line);
        are = "100";
        concatenate_binary_str(opcode, source, target, are, result);
        append_node(table, result, "mov", "op");
    } else if (strcmp(local_word, "cmp") == 0) {
        opcode = "0001";
        source = get_source(word);
        target = get_target(word, line);
        are = "100";
        concatenate_binary_str(opcode, source, target, are, result);
        append_node(table, result, "cmp", "op");
    } else if (strcmp(local_word, "add") == 0) {
        opcode = "0010";
        source = get_source(word);
        target = get_target(word, line);
        are = "100"; 
        concatenate_binary_str(opcode, source, target, are, result);
        append_node(table, result, "add", "op");
    } else if (strcmp(local_word, "sub") == 0) {
        opcode = "0011";
        source = get_source(word);
        target = get_target(word, line);
        are = "100";    
        concatenate_binary_str(opcode, source, target, are, result);
        append_node(table, result, "sub", "op");
    } else if (strcmp(local_word, "lea") == 0) {
        opcode = "0100";
        source = get_source(word);
        target = get_target(word, line);
        are = "100";    
        concatenate_binary_str(opcode, source, target, are, result);
        append_node(table, result, "lea", "op");
    } else if (strcmp(local_word, "clr") == 0) {
        opcode = "0101";
        source = "0000";
        target = get_target(word, line);
        are = "100";    
        concatenate_binary_str(opcode, source, target, are, result);
        append_node(table, result, "clr", "op");
    } else if (strcmp(local_word, "not") == 0) {
        opcode = "0110";
        source = "0000";
        target = get_target(word, line);
        are = "100";    
        concatenate_binary_str(opcode, source, target, are, result);
        append_node(table, result, "not", "op");
    } else if (strcmp(local_word, "inc") == 0) {
        opcode = "0111";
        source = "0000";
        target = get_target(word, line);
        are = "100";    
        concatenate_binary_str(opcode, source, target, are, result);
        append_node(table, result, "inc", "op");
    } else if (strcmp(local_word, "dec") == 0) {
        opcode = "1000";
        source = "0000";
        target = get_target(word, line);
        are = "100";    
        concatenate_binary_str(opcode, source, target, are, result);
        append_node(table, result, "dec", "op");
    } else if (strcmp(local_word, "jmp") == 0) {
        opcode = "1001";
        source = "0000";
        target = get_target(word, line); 
        are = "100";   
        concatenate_binary_str(opcode, source, target, are, result);
        append_node(table, result, "jmp", "op");
    } else if (strcmp(local_word, "bne") == 0) {
        opcode = "1010";
        source = "0000";
        target = get_target(word, line); 
        are = "100";   
        concatenate_binary_str(opcode, source, target, are, result);
        append_node(table, result, "bne", "op");
    } else if (strcmp(local_word, "red") == 0) {
        opcode = "1011";
        source = "0000"; 
        target = get_target(word, line);
        are = "100";   
        concatenate_binary_str(opcode, source, target, are, result);
        append_node(table, result, "red", "op");
    } else if (strcmp(local_word, "prn") == 0) {
        opcode = "1100";
        source = "0000";
        target = get_target(word, line);
        are = "100";    
        concatenate_binary_str(opcode, source, target, are, result);
        append_node(table, result, "prn", "op");
    } else if (strcmp(local_word, "jsr") == 0) {
        opcode = "1101";
        source = "0000";
        target = get_target(word, line);
        are = "100";    
        concatenate_binary_str(opcode, source, target, are, result);
        append_node(table, result, "jsr", "op");
    } else if (strcmp(local_word, "rts") == 0) {
        append_node(table, "111000000000100", "rts", "op");
    } else if (strcmp(local_word, "stop") == 0) {
        append_node(table, "111100000000100", "stop", "op");
    }
}

int is_register(char *word) {
    char *registers[] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", NULL};
    int i = 0;

    if (word == NULL) {
        return 0;
    }

    /* If the word starts with '#', skip it */
    if (word[0] == '#' || word[0] == '*') {
        word++;  /* Skip the '#' or '*' */
    }

    while (registers[i] != NULL) {
        if (strcmp(word, registers[i]) == 0) {
            return 1;  /* Match found */
        }
        i++;
    }
    return 0;  /* No match found */
}

void handle_register(const char *local_word, char *word, const char *line, CodingTable **table) {
    char result[16];
    char binary_str[5];
    char *lastWord;
    const char *binaryWord;
    const char *opcode, *source, *target, *are;

    lastWord = get_last_word(local_word, line);

    register_to_binary(local_word, binary_str);
    binaryWord = binary_str;  /* Store the binary string as a pointer */

    opcode = "000000";

    if (is_register(lastWord)) {
        register_to_binary(lastWord, binary_str);
        source = binary_str;
        target = binaryWord;
    } else if (strcmp(lastWord, "mov") == 0 ||
               strcmp(lastWord, "cmp") == 0 ||
               strcmp(lastWord, "add") == 0 ||
               strcmp(lastWord, "sub") == 0 ||
               strcmp(lastWord, "lea") == 0) {
        source = binaryWord;
        target = "000";
    } else {
        source = "000";
        target = binaryWord;
    }

    are = "100";  /* Set ARE field to "100" */

    /* Concatenate all parts into the result binary string */
    concatenate_binary_str(opcode, source, target, are, result);

    /* If the next word is not a register, add to the coding table */
    if (word == NULL || !is_register(word)) {
        append_node(table, result, local_word, "register");
    }
}

int is_number(char *word) {
    /* Skip the '#' or '*' character if present */
    if (word[0] == '#' || word[0] == '*' || word[0] == '-') {
        word++;  /* Point to the next character */
    }
    if (word[0] == '-') {
        word++;  /* Point to the next character */
    }
    
    /* Check if the remaining string starts with a digit */
    if (isdigit((unsigned char)word[0])) {
        return 1;
    }
    
    return 0;
}

void handle_number(const char *word, const char *line, CodingTable **table) {
    int num;
    char *result;
    char temp_word[MAX_LINE_LENGTH]; /* Temporary variable to store the string */
    /* Check if the string starts with '#' */
    if (word[0] == '#') {
        strcpy(temp_word, word + 1); /* Copy the string without the '#' sign */
        num = atoi(temp_word); /* Convert the string to an integer */
        result = integer_imm_to_binary(num); /* Convert the integer to binary */
        append_node(table, result, word, "integer_imm");
    } else {
        strcpy(temp_word, word); /* Copy the original string */
        num = atoi(temp_word); /* Convert the string to an integer */
        result = integer_to_binary(num); /* Convert the integer to binary */
        append_node(table, result, word, "integer");
    }
}

int is_letter(char *word) {
    if (word[0] == '"'){
        return 1;
    }
    return 0;
}

void handle_letter(const char *word, const char *line, CodingTable **table) {
    char binary[15]; /* 15 bits without null terminator */
    char single_char[2]; /* Array to hold a single character and null terminator */

    /* Iterate over each character in the word */
    while (*word != '\0') {
        if (*word == '"') {
           word++;
           continue;
        }
        
        /* Convert the current letter to its binary representation */
        letter_to_binary((int)*word, binary);
        
        /* Store the current character in single_char array */
        single_char[0] = *word;
        single_char[1] = '\0'; /* Null-terminate the single character */

        /* Append the binary representation and the single character to the coding table */
        append_node(table, binary, single_char, "letter");

        /* Move to the next character in the word */
        word++;
    }

    /* Handle the end-of-string '\0' case */
    letter_to_binary(0, binary); /* '\0' represented as all 0s in binary */
    append_node(table, binary, "\\0", "letter");
}

int is_other(char *word) {
    if(!is_symbol(word) && !is_directive(word) && !is_extern_entry(word) && !is_opcode(word) && !is_register(word) && !is_number(word) && !is_letter(word)) {
        return 1;
    }
    return 0;
}

void handle_other(const char *word, const char *line, int *lineFile, CodingTable **table) {
    append_node(table, "000000000000001", word, "other");
}

/* Function to convert a binary string to a 15-bit unsigned short */
unsigned short binary_to_bits(const char* binary_str) {
    unsigned short bits = 0;
    int i;
    for (i = 0; i < 15 && binary_str[i] != '\0'; i++) {
        bits = (bits << 1) | (binary_str[i] - '0'); /* Convert each char '0'/'1' to bit */
    }
    return bits;
}

void append_node(CodingTable** head, const char* binary_str, const char* word, const char* type) {
    int DecimalAddress = 100; /* Start DecimalAddress from 100 */
    CodingTable* new_node;

    /* Check if binary_str, word, and type are valid (non-null) */
    if (binary_str == NULL || word == NULL || type == NULL) {
        printf("Error: Invalid input provided to append_node!\n");
        return;
    }

    /* Traverse the list to find the last node and increment the DecimalAddress */
    if (*head != NULL) {
        CodingTable* temp = *head;
        while (temp->next != NULL) {
            temp = temp->next; /* Traverse to the last node */
        }
        DecimalAddress = temp->DecimalAddress + 1; /* Increment DecimalAddress based on the last node */
    }

    /* Allocate memory for the new node */
    new_node = (CodingTable*)malloc(sizeof(CodingTable)); 
    if (new_node == NULL) {
        printf("Memory allocation failed!\n");
        exit(1); /* Exit if memory allocation fails */
    }

    /* Initialize the new node with the provided data */
    new_node->DecimalAddress = DecimalAddress; /* Set the DecimalAddress */

    /* Convert the binary string to a 15-bit value */
    new_node->bits = binary_to_bits(binary_str); /* Assuming binary_to_bits handles the conversion properly */
    
    /* Copy the word and type strings, ensuring null-termination */
    strncpy(new_node->word, word, MAX_LINE_LENGTH - 1); /* Copy the word */
    new_node->word[MAX_LINE_LENGTH - 1] = '\0'; /* Ensure null-termination of the word */
    
    strncpy(new_node->type, type, MAX_LINE_LENGTH - 1); /* Copy the type */
    new_node->type[MAX_LINE_LENGTH - 1] = '\0'; /* Ensure null-termination of the type */
    
    new_node->next = NULL; /* Initialize the next pointer to NULL */

    /* Append the new node to the list */
    if (*head == NULL) {
        *head = new_node; /* If the list is empty, set the new node as the head */
    } else {
        CodingTable* temp = *head;
        while (temp->next != NULL) {
            temp = temp->next; /* Traverse to the last node */
        }
        temp->next = new_node; /* Append the new node at the end */
    }
}

/* Function to create and append a node to the extern_table */
ExternTable* append_extern_node(ExternTable** head, int DecimalAddress, const char* label) {
    ExternTable* new_node;
    ExternTable* current;

    /* Allocate memory for the new node */
    new_node = (ExternTable*)malloc(sizeof(ExternTable));
    if (new_node == NULL) {
        printf("Memory allocation failed!\n");
        exit(1); /* Exit if memory allocation fails */
    }

    /* Initialize the new node */
    new_node->DecimalAddress = DecimalAddress;           /* Set the DecimalAddress */
    strncpy(new_node->label, label, MAX_LINE_LENGTH);    /* Copy the label into the node */
    new_node->label[MAX_LINE_LENGTH - 1] = '\0';         /* Ensure null-termination of the label */
    new_node->next = NULL;                               /* Initialize the next pointer to NULL */

    /* If the list is empty, make the new node the head */
    if (*head == NULL) {
        *head = new_node;  /* New node is now the head */
        return *head;
    }

    /* Traverse to the end of the list */
    current = *head;
    while (current->next != NULL) {
        current = current->next;
    }

    /* Append the new node at the end of the list */
    current->next = new_node;

    /* Return the head of the list */
    return current;
}

/* Function to create and append a node to the entry_table list */
EntryTable* append_entry_node(EntryTable** head, int DecimalAddress, const char* label) {
    EntryTable* new_node;
    EntryTable* current;

    /* Allocate memory for the new node */
    new_node = (EntryTable*)malloc(sizeof(EntryTable));
    if (new_node == NULL) {
        printf("Memory allocation failed!\n");
        exit(1); /* Exit if memory allocation fails */
    }

    /* Initialize the new node */
    new_node->DecimalAddress = DecimalAddress;           /* Set the DecimalAddress */
    strncpy(new_node->label, label, MAX_LINE_LENGTH);    /* Copy the label into the node */
    new_node->label[MAX_LINE_LENGTH - 1] = '\0';         /* Ensure null-termination of the label */
    new_node->next = NULL;                               /* Initialize the next pointer to NULL */

    /* If the list is empty, make the new node the head */
    if (*head == NULL) {
        *head = new_node;  /* New node is now the head */
        return *head;
    }

    /* Traverse to the end of the list */
    current = *head;
    while (current->next != NULL) {
        current = current->next;
    }

    /* Append the new node at the end of the list */
    current->next = new_node;

    /* Return the head of the list */
    return current;
}

int get_next_decimal_address(CodingTable* table) {
    int initial_address;
    CodingTable* temp;
    initial_address = 100;  /* Starting address if list is empty */
    

    /* If the list is empty, return the initial address */
    if (table == NULL) {
        return initial_address;
    }
    temp = table;

    /* Traverse to the last node */
    while (temp != NULL) {
        if (temp->next == NULL) {
            return temp->DecimalAddress + 1; 
        }
        temp = temp->next;
    }
    
    return initial_address; 
}

/* Function to create and append a node to the symbolTable */
SymbolTable* append_symbol_node(SymbolTable** head, const char* symbol, unsigned short binary) {
    SymbolTable* new_node;
    SymbolTable* current;

    /* Allocate memory for the new node */
    new_node = (SymbolTable*)malloc(sizeof(SymbolTable));
    if (new_node == NULL) {
        printf("Error: Memory allocation failed!\n");
        return *head; /* Return head without changes */
    }

    /* Check if the input symbol is valid */
    if (symbol == NULL) {
        printf("Error: Invalid symbol input (NULL)!\n");
        free(new_node); /* Free allocated memory in case of error */
        return *head; /* Return head without changes */
    }

    /* Copy the symbol into the new node's symbol field */
    strncpy(new_node->symbol, symbol, MAX_LINE_LENGTH - 1); /* Copy the symbol */
    new_node->symbol[MAX_LINE_LENGTH - 1] = '\0'; /* Ensure null termination */

    /* Set the binary value and initialize the next pointer */
    new_node->binary = binary;
    new_node->next = NULL;

    /* If the list is empty, make the new node the head */
    if (*head == NULL) {
        *head = new_node;
        return *head;  /* New node becomes the head */
    }

    /* Traverse to the end of the list */
    current = *head;
    while (current->next != NULL) {
        current = current->next;
    }

    /* Append the new node at the end of the list */
    current->next = new_node;

    /* Return the head of the list */
    return current;
}

/* Function to get the next word after the current word */
char* get_next_word(const char* current_word, const char* line) {
    const char *start_ptr, *end_ptr;
    char *next_word;
    int current_word_len = strlen(current_word);
    int word_len;
    int found_current_word = 0;
    /* Create a copy of the line */
    char* line_copy = (char*)malloc(strlen(line) + 1);
    if (line_copy == NULL) {
        return NULL; /* Memory allocation failed */
    }
    strcpy(line_copy, line); /* Copy the line */

    /* Iterate over the copied line */
    start_ptr = line_copy;
    while (*start_ptr != '\0') {
        /* Skip over spaces, tabs, and newlines */
        while (*start_ptr == ' ' || *start_ptr == '\t' || *start_ptr == '\n') {
            start_ptr++;
        }

        /* If we reach the end of the line, break */
        if (*start_ptr == '\0') {
            break;
        }

        /* Find the end of the current word */
        end_ptr = start_ptr;
        while (*end_ptr != ' ' && *end_ptr != '\t' && *end_ptr != '\n' && *end_ptr != '\0') {
            end_ptr++;
        }

        /* Calculate the length of the current word */
        word_len = end_ptr - start_ptr;

        /* Check if we found the current word */
        if (!found_current_word && strncmp(start_ptr, current_word, word_len) == 0 && current_word_len == word_len) {
            found_current_word = 1; /* Mark that current_word is found */
        } else if (found_current_word) {
            /* Allocate memory for the next word */
            next_word = (char*)malloc(word_len + 1);
            if (next_word == NULL) {
                free(line_copy);
                return NULL; /* Memory allocation failed */
            }

            /* Copy the next word into the allocated memory */
            strncpy(next_word, start_ptr, word_len);
            next_word[word_len] = '\0'; /* Null-terminate the string */

            free(line_copy); /* Free the line copy */
            return next_word; /* Return the next word */
        }

        /* Move to the next word */
        start_ptr = end_ptr;
    }

    /* Free the line copy */
    free(line_copy);

    /* Return NULL if no next word is found */
    return NULL;
}

/* Function to get the word before the target word */
char* get_last_word(const char* word, const char* line) {
    char* last_word = NULL;
    char* current_word = NULL;
    char line_copy[256]; /* Declare a fixed size array for line copy, compatible with C90 */
    
    /* Copy the input line to avoid modifying the original */
    strncpy(line_copy, line, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0'; /* Ensure null-termination */
    
    /* Extract the first word from the line */
    current_word = strtok(line_copy, " ,\t\n");
    
    /* Loop through the words in the line */
    while (current_word != NULL) {
        if (strcmp(current_word, word) == 0) {
            return last_word; /* Return the word before the target word */
        }
        last_word = current_word; /* Store the current word as the last word */
        current_word = strtok(NULL, " ,\t\n"); /* Move to the next word */
    }
    
    return NULL; /* Return NULL if the target word is not found */
}

/* Function to concatenate four binary strings into result */
void concatenate_binary_str(const char *opcode, const char *source, const char *target, const char *are, char *result) {
    /* Ensure the result is an empty string initially */
    if (result != NULL) {
        result[0] = '\0';

        /* Check that none of the input strings are NULL */
        if (opcode != NULL) {
            strcat(result, opcode);
        }

        if (source != NULL) {
            strcat(result, source);
        }

        if (target != NULL) {
            strcat(result, target);
        }

        if (are != NULL) {
            strcat(result, are);
        }
    }
}

char* get_source(char *word) {
    char *source_word;
    int i;
    i = 0;
    source_word = word;

    if (source_word == NULL) {
        return NULL;
    }

    while (source_word[i] != '\0') {
        if ('#' == source_word[0]) {
            return "0001";
        } else if (is_symbol(source_word) || is_other(source_word)){
            return "0010";
        } else if ('*' == source_word[0]) {
            return "0100";
        } else if(is_register(source_word)){
            return "1000";
        }
        i++;
    }
    return NULL;
}

char* get_target(char *word, char *line) {

    char *target_word; 
    int i;
    target_word = get_next_word(word, line);
    if (target_word == NULL) {
        target_word = word;
    }
    
    i = 0;
    while (target_word[i] != '\0') {
        if (target_word[0] == '#') {
            return "0001";
        } else if (is_symbol(target_word) || is_other(target_word)){
            return "0010";
        } else if (target_word[0] == '*') {
            return "0100";
        } else if(is_register(target_word)) {
            return "1000";
        }
        i++;
    }
    return NULL;
}

void register_to_binary(const char* number_str, char* binary_str) {
    int number;
    int i;

    /* If the word starts with '#', skip it */
    if (number_str[0] == '#' || number_str[0] == '*') {
        number_str++;  /* Skip the '# or *' by pointing to the next character */
    }
    /* Convert the string to an integer */
    number = number_str[1] - '0';  /* Assuming the format is always "rX", where X is the number. */

    /* Convert the number to binary string */
    for (i = 2; i >= 0; i--) {
        binary_str[i] = (number & 1) ? '1' : '0';  /* Set the bit (rightmost first) */
        number >>= 1;  /* Shift right to get the next bit */
    }
    binary_str[3] = '\0';  /* Null-terminate the string */
}

char* integer_to_binary(int num) {
    char* binary = (char*)malloc(16 * sizeof(char));  /* Array of 12 bits + null terminator */
    int i;
    if (binary == NULL) {
        return NULL;  /* Allocation failed */
    }

    /* If the number is negative, convert it using two's complement */
    if (num < 0) {
        num = (1 << 15) + num;  /* Add 2^15 to represent the negative number */
    }

    for (i = 14; i >= 0; i--) {
        binary[i] = (num & 1) ? '1' : '0';
        num >>= 1;
    }
    
    binary[15] = '\0';  /* Null terminator for string */

    return binary;
}

char* integer_imm_to_binary(int num) {
    char* final_binary = (char*)malloc(16 * sizeof(char));  /* Allocate array for 15 bits + null terminator */
    int i;

    if (final_binary == NULL) {
        return NULL;  /* Allocation failed */
    }

    /* Handle negative numbers using two's complement for 12 bits */
    if (num < 0) {
        num = (1 << 12) + num;  /* Convert to two's complement for 12 bits */
    } else {
        num &= 0xFFF;  /* Mask the number to 12 bits for positive numbers */
    }

    /* Fill the first 12 bits (index 0 to 11) */
    for (i = 0; i < 12; i++) {
        final_binary[i] = (num & (1 << (11 - i))) ? '1' : '0';  /* Extract each bit from the most significant to the least */
    }

    /* Add the last 3 bits '100' (index 12, 13, and 14) */
    final_binary[12] = '1';
    final_binary[13] = '0';
    final_binary[14] = '0';

    final_binary[15] = '\0';  /* Null-terminate the string */

    return final_binary;
}

void letter_to_binary(int n, char binary[15]) {
    int i;
    /* Convert each ASCII value to a 15-bit binary string */
    for (i = 15 - 1; i >= 0; i--) {
        binary[i] = (n & 1) ? '1' : '0';  /* Set '1' or '0' depending on the bit */
        n >>= 1;  /* Shift right to process the next bit */
    }
}

/* Function to remove extra spaces from a string */
void remove_extra_spaces(char* str) {
    int i = 0, j = 0;
    int len = strlen(str);
    int space_found = 0;

    /* Skip leading spaces */
    while (isspace(str[i])) {
        i++;
    }

    while (i < len) {

        if (str[i] == ',') {
                str[i] = ' '; 
        }
        /* Copy characters other than spaces */
        if (!isspace(str[i])) {
            str[j++] = str[i];
            space_found = 0;
        } 
        /* Copy a single space if multiple spaces are found */
        else if (!space_found) {
            str[j++] = ' ';
            space_found = 1;
        }
        i++;
    }

    /* Remove trailing space if present */
    if (j > 0 && str[j - 1] == ' ') {
        j--;
    }

    /* Null-terminate the string */
    str[j] = '\0';
}





