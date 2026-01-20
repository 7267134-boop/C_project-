#ifndef FIRSTSYNTAX_H
#define FIRSTSYNTAX_H
#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

typedef struct CodingTable {
    int DecimalAddress;         /* Decimal address in the list */
    unsigned short bits;        /* 15-bit sequence stored as a short */
    char word[MAX_LINE_LENGTH]; /* Word associated with the node */
    char type[MAX_LINE_LENGTH]; /* New field for type */
    struct CodingTable* next;   /* Pointer to the next node */
} CodingTable;

typedef struct SymbolTable {
    char symbol[MAX_LINE_LENGTH];  /* Symbol name */
    unsigned short binary;         /* Binary value */
    struct SymbolTable* next;      /* Pointer to the next node */
} SymbolTable;

typedef struct EntryTable {
    int DecimalAddress;                /* Decimal Address (ID) */
    char label[MAX_LINE_LENGTH];      /* Label (Name) */
    struct EntryTable* next;           /* Pointer to the next node */
} EntryTable;

typedef struct ExternTable {
    int DecimalAddress;                /* Decimal Address (ID) */
    char label[MAX_LINE_LENGTH];      /* Label (Name) */
    struct ExternTable* next;          /* Pointer to the next node */
} ExternTable;

void first_syntax(const char *file_name);
void process_first_pass(FILE *file, CodingTable** table, SymbolTable** symbol, EntryTable** entryHead, ExternTable** externHead);
int is_first_symbol(char *line);
int is_symbol(char *word);
void handle_first_symbol(const char *word, CodingTable **table, SymbolTable **symbolTable);
void handle_symbol(const char *word, const char *line, CodingTable **table);
int is_directive(char *word);
void handle_directive(const char *word, const char *line, CodingTable **table);
int is_extern_entry(char *word);
void handle_extern_entry(const char *local_word, const char *word, const char *line, CodingTable **table, ExternTable** externHead, EntryTable** entryHead);
int is_opcode(char *word);
void handle_opcode(char *local_word, char *word, char *line, CodingTable **table);
int is_register(char *word);
void handle_register(const char *local_word, char *word, const char *line, CodingTable **table);
int is_number(char *word);
void handle_number(const char *word, const char *line, CodingTable **table);
int is_letter(char *word);
void handle_letter(const char *word, const char *line, CodingTable **table);
int is_other();
void handle_other(const char *word, const char *line, int *lineFile ,CodingTable **table);
unsigned short binary_to_bits(const char* binary_str);
void append_node(CodingTable** head, const char* binary_str, const char* word, const char* type);
EntryTable* append_entry_node(EntryTable** head, int DecimalAddress, const char* label);
ExternTable* append_extern_node(ExternTable** head, int DecimalAddress, const char* label);
int get_next_decimal_address(CodingTable* head);
SymbolTable* append_symbol_node(SymbolTable** head, const char* symbol, unsigned short binary);
char* get_next_word(const char* word, const char* line);
char* get_last_word(const char* word, const char* line);
void concatenate_binary_str(const char *opcode, const char *source, const char *target, const char *are, char *result);
char* get_source(char *word);
char* get_target(char *word, char *line);
void register_to_binary(const char* number_str, char* binary_str);
char* integer_to_binary(int num);
char* integer_imm_to_binary(int num);
void letter_to_binary(int n, char binary[15]);
void remove_extra_spaces(char* str);


#endif
