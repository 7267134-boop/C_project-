#ifndef SECENDSYNTAX_H
#define SECENDSYNTAX_H
#include <stdio.h>
#include <stdlib.h>
#include "defs.h"

void secend_syntax(CodingTable* codingHead, SymbolTable* symbolHead, EntryTable* entryHead, ExternTable* externHead, const char *file_name);
void print_binary(unsigned short value);
void find_and_match_symbol(CodingTable* codingTable, SymbolTable* symbolTable);
char* decimal_to_binary(int num);
void delete_old_extern(ExternTable** externTable);
ExternTable* add_to_extern_table(ExternTable** externTable, int decimal, const char* word);
void process_coding_table(CodingTable* codingTable, ExternTable** externTable);
void update_entry_table(EntryTable* entryTable, SymbolTable* symbolTable);
int binary_to_decimal(const char* binary_str);
void decimal_to_octal(int bits, char* octal_str);
void count_instructions_and_data(CodingTable* head, int* instruction_count, int* data_count);
void change_file_extension(const char* file_name, char* new_file_name, char* type);
void coding_table_to_ob(const char* file_name, CodingTable* head);
void extern_table_to_ext(const char* file_name, ExternTable* head);
void entry_table_to_ent(const char* file_name, EntryTable* head);

#endif
