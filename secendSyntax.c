#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "precompiler.h"
#include "firstSyntax.h"
#include "secendSyntax.h"

/**
 * @brief Processes the provided tables and generates relevant output files.
 * 
 * This function processes the coding, symbol, entry, and extern tables,
 * generates the appropriate object, entry, and extern files based on the 
 * processed information.
 * 
 * @param codingHead Pointer to the head of the CodingTable list.
 * @param symbolHead Pointer to the head of the SymbolTable list.
 * @param entryHead Pointer to the head of the EntryTable list.
 * @param externHead Pointer to the head of the ExternTable list.
 * @param file_name Name of the output file.
 */
void secend_syntax(CodingTable* codingHead, SymbolTable* symbolHead, EntryTable* entryHead, ExternTable* externHead, const char *file_name) {
    CodingTable* currentCoding = codingHead;  /* Pointer to traverse CodingTable */
    SymbolTable* currentSymbol = symbolHead;  /* Pointer to traverse SymbolTable */
    EntryTable* currentEntry = entryHead;     /* Pointer to traverse EntryTable */
    ExternTable* currentExtern = externHead;  /* Pointer to traverse ExternTable */

    /* Match symbols in coding table and process tables */
    find_and_match_symbol(currentCoding, currentSymbol);
    process_coding_table(currentCoding, &currentExtern);
    update_entry_table(currentEntry, currentSymbol);
    
    /* Write processed tables to output files */
    coding_table_to_ob(file_name, currentCoding);
    
    /* Handle extern table if it exists */
    if (currentExtern != NULL) {
        extern_table_to_ext(file_name, currentExtern);
    }
    
    /* Handle entry table if it exists */
    if (currentEntry != NULL) {
        entry_table_to_ent(file_name, currentEntry);
    }
}

/**
 * @brief Converts a decimal number to a binary string with 12 bits and appends '010'.
 * 
 * @param num The decimal number to convert.
 * @return A dynamically allocated string representing the 15-bit binary.
 */
char* decimal_to_binary(int num) {
    char* binary = (char*)malloc(16 * sizeof(char));  /* Allocate space for 15 bits + null terminator */
    int i;

    /* Check for memory allocation failure */
    if (binary == NULL) {
        return NULL;
    }

    /* Mask the number to 12 bits */
    num &= 0xFFF;  /* Keep only the 12 least significant bits */

    /* Convert the number to a binary string (12 bits) */
    for (i = 0; i < 12; i++) {
        binary[i] = (num & (1 << (11 - i))) ? '1' : '0';  /* Fill from most significant bit */
    }

    /* Append fixed bits '010' to the binary string */
    binary[12] = '0';
    binary[13] = '1';
    binary[14] = '0';

    /* Null-terminate the string */
    binary[15] = '\0';

    return binary;
}

/**
 * @brief Matches symbols from the SymbolTable to the CodingTable and updates CodingTable entries.
 * 
 * This function looks for entries in the CodingTable that refer to symbols. It searches for 
 * matching symbols in the SymbolTable and updates the CodingTable entry with the corresponding
 * binary value.
 * 
 * @param codingTable Pointer to the CodingTable.
 * @param symbolTable Pointer to the SymbolTable.
 */
void find_and_match_symbol(CodingTable* codingTable, SymbolTable* symbolTable) {
    CodingTable* currentCoding = codingTable;
    SymbolTable* currentSymbol;
    char* binaryString;
    unsigned short newBits;

    /* Traverse through CodingTable */
    while (currentCoding != NULL) {
        /* Check if the current entry's type is "symbol" */
        if (strcmp(currentCoding->type, "symbol") == 0) {
            currentSymbol = symbolTable;
            
            /* Search for a matching symbol in SymbolTable */
            while (currentSymbol != NULL) {
                if (strcmp(currentSymbol->symbol, currentCoding->word) == 0) {
                    /* Match found, convert the symbol's binary value to a string */
                    binaryString = decimal_to_binary(currentSymbol->binary);

                    /* Convert the binary string to an unsigned short */
                    newBits = (unsigned short)strtol(binaryString, NULL, 2);

                    /* Update the CodingTable's bits with the new binary value */
                    currentCoding->bits = newBits;

                    /* Free the dynamically allocated binary string */
                    free(binaryString);
                    break;  /* Exit the loop after finding a match */
                }
                currentSymbol = currentSymbol->next;  /* Move to the next symbol */
            }
        }
        currentCoding = currentCoding->next;  /* Move to the next CodingTable entry */
    }
}

/**
 * @brief Frees nodes from the ExternTable where DecimalAddress is 0.
 * 
 * This function iterates through the ExternTable and deletes nodes where 
 * the DecimalAddress is 0.
 * 
 * @param externTable Double pointer to the ExternTable.
 */
void delete_old_extern(ExternTable** externTable) {
    ExternTable* current = *externTable;
    ExternTable* prev = NULL;

    /* Traverse the ExternTable */
    while (current != NULL) {
        if (current->DecimalAddress == 0) {
            /* Node matches the condition for deletion */
            ExternTable* temp = current;
            if (prev == NULL) {
                *externTable = current->next;  /* Adjust head if necessary */
            } else {
                prev->next = current->next;  /* Skip the node */
            }
            current = current->next;
            free(temp);  /* Free the memory */
        } else {
            /* Continue to the next node if it doesn't match */
            prev = current;
            current = current->next;
        }
    }
}

/**
 * @brief Adds a new node to the ExternTable.
 * 
 * @param externTable Double pointer to the ExternTable.
 * @param decimal The decimal address to store in the new node.
 * @param word The label associated with the decimal address.
 * @return Pointer to the newly created ExternTable node.
 */
ExternTable* add_to_extern_table(ExternTable** externTable, int decimal, const char* word) {
    ExternTable* newExtern = (ExternTable*)malloc(sizeof(ExternTable));  /* Allocate space for the new node */
    
    if (newExtern == NULL) {
        printf("Error: Memory allocation failed\n");
        return NULL;
    }

    /* Initialize the new node */
    newExtern->DecimalAddress = decimal;
    strncpy(newExtern->label, word, MAX_LINE_LENGTH);
    newExtern->next = NULL;

    /* Insert the new node at the end of the ExternTable */
    if (*externTable == NULL) {
        *externTable = newExtern;  /* If the table is empty, new node becomes the head */
    } else {
        ExternTable* temp = *externTable;
        while (temp->next != NULL) {
            temp = temp->next;  /* Traverse to the end of the list */
        }
        temp->next = newExtern;  /* Append the new node */
    }

    return newExtern;
}

/**
 * @brief Processes the CodingTable and updates the ExternTable with new entries.
 * 
 * This function processes each entry in the CodingTable. If an entry has the type "other",
 * it checks for matching labels in the ExternTable and adds a new entry if necessary.
 * 
 * @param codingTable Pointer to the CodingTable.
 * @param externTable Double pointer to the ExternTable.
 */
void process_coding_table(CodingTable* codingTable, ExternTable** externTable) {
    CodingTable* currentCoding = codingTable;
    ExternTable* currentExtern;
    int found;

    /* Traverse through the CodingTable */
    while (currentCoding != NULL) {
        /* Check if the entry type is "other" */
        if (strcmp(currentCoding->type, "other") == 0) {
            currentExtern = *externTable;
            found = 0;

            /* Search for a matching label in ExternTable */
            while (currentExtern != NULL) {
                if (strcmp(currentCoding->word, currentExtern->label) == 0) {
                    found = 1;  /* Match found */
                    break;
                }
                currentExtern = currentExtern->next;
            }

            /* If a match is found, add a new entry to ExternTable */
            if (found) {
                add_to_extern_table(externTable, currentCoding->DecimalAddress, currentCoding->word);
            } else {
                printf("Error: No matching label found for word '%s'\n", currentCoding->word);
            }
        }
        currentCoding = currentCoding->next;  /* Move to the next CodingTable entry */
    }
    
    /* Remove any outdated entries from the ExternTable */
    delete_old_extern(externTable);
}

/**
 * @brief Updates the EntryTable with addresses from the SymbolTable.
 * 
 * This function updates each entry in the EntryTable by searching for matching labels
 * in the SymbolTable. If a match is found, it updates the DecimalAddress of the entry.
 * 
 * @param entryTable Pointer to the EntryTable.
 * @param symbolTable Pointer to the SymbolTable.
 */
void update_entry_table(EntryTable* entryTable, SymbolTable* symbolTable) {
    EntryTable* currentEntry = entryTable;

    /* Traverse the EntryTable */
    while (currentEntry != NULL) {
        SymbolTable* currentSymbol = symbolTable;
        int found = 0;  /* Flag to check if a match is found */

        /* Search for the label in the SymbolTable */
        while (currentSymbol != NULL) {
            if (strcmp(currentEntry->label, currentSymbol->symbol) == 0) {
                /* Match found, update the DecimalAddress */
                currentEntry->DecimalAddress = currentSymbol->binary;
                found = 1;
                break;
            }
            currentSymbol = currentSymbol->next;  /* Move to the next symbol */
        }

        /* Print an error if no match is found */
        if (!found) {
            printf("Error: Label '%s' not found in SymbolTable\n", currentEntry->label);
        }

        currentEntry = currentEntry->next;  /* Move to the next entry */
    }
}
/**
 * @brief Converts a 15-bit sequence to a 5-digit octal string.
 * 
 * This function takes a 15-bit unsigned short integer and converts it into a 5-digit 
 * octal string with leading zeros. The result is stored in the provided string buffer.
 * 
 * @param bits The 15-bit unsigned short integer to convert.
 * @param octal_str The buffer to store the resulting 5-digit octal string.
 */
void bits_to_octal(unsigned short bits, char* octal_str) {
    sprintf(octal_str, "%05o", bits);  /* Ensure 5-digit octal with leading zeros */
}

/**
 * @brief Counts the number of instructions and data in the CodingTable.
 * 
 * This function iterates through the CodingTable and counts the number of entries
 * categorized as instructions (based on types "integer" or "letter") and data.
 * 
 * @param head Pointer to the head of the CodingTable list.
 * @param instruction_count Pointer to store the number of instructions.
 * @param data_count Pointer to store the number of data entries.
 */
void count_instructions_and_data(CodingTable* head, int* instruction_count, int* data_count) {
    CodingTable* current = head;
    *instruction_count = 0;
    *data_count = 0;

    /* Traverse the CodingTable to count instructions and data */
    while (current != NULL) {
        if (strcmp(current->type, "integer") == 0 || strcmp(current->type, "letter") == 0) {
            (*instruction_count)++;  /* Increment instruction count */
        } else {
            (*data_count)++;  /* Increment data count */
        }
        current = current->next;
    }
}

/**
 * @brief Changes the file extension based on the specified type.
 * 
 * This function replaces the file extension of the provided file name with a new extension,
 * depending on the type ("coding", "extern", or "entry").
 * 
 * @param file_name The original file name.
 * @param new_file_name The buffer to store the modified file name with the new extension.
 * @param type The type that determines the new file extension ("coding" -> .ob, "extern" -> .ext, "entry" -> .ent).
 */
void change_file_extension(const char* file_name, char* new_file_name, char* type) {
    char *dot;
    strcpy(new_file_name, file_name);

    /* Find the last dot in the file name */
    dot = strrchr(new_file_name, '.');
    
    if (strcmp(type, "coding") == 0) {
        if (dot != NULL) {
            /* Replace everything after the dot with .ob */
            strcpy(dot, ".ob");
        } else {
            /* If no dot is found, append .ob */
            strcat(new_file_name, ".ob");
        }
    } else if (strcmp(type, "extern") == 0) {
        if (dot != NULL) {
            /* Replace everything after the dot with .ext */
            strcpy(dot, ".ext");
        } else {
            strcat(new_file_name, ".ext");
        }
    } else if (strcmp(type, "entry") == 0) {
        if (dot != NULL) {
            /* Replace everything after the dot with .ent */
            strcpy(dot, ".ent");
        } else {
            strcat(new_file_name, ".ent");
        }
    }
}

/**
 * @brief Writes the CodingTable to a .ob file with octal conversion and counts.
 * 
 * This function writes the entries of the CodingTable to a file with a .ob extension. 
 * It also converts the 15-bit binary data into a 5-digit octal string and writes the 
 * instruction and data counts at the top of the file.
 * 
 * @param file_name The original file name.
 * @param head Pointer to the head of the CodingTable list.
 */
void coding_table_to_ob(const char* file_name, CodingTable* head) {
    FILE *file;
    char octal_str[6];  /* Buffer to store the 5-digit octal string */
    int instruction_count = 0, data_count = 0;
    char new_file_name[MAX_LINE_LENGTH];
    CodingTable* current;
    char* type = "coding";

    /* Change the file extension to .ob */
    change_file_extension(file_name, new_file_name, type);

    /* Count instructions and data */
    count_instructions_and_data(head, &instruction_count, &data_count);

    /* Open the new .ob file for writing */
    file = fopen(new_file_name, "w");
    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }

    /* Write instruction and data counts at the top */
    fprintf(file, "  %d %d\n", data_count, instruction_count);

    /* Iterate through the table and write each entry */
    current = head;
    while (current != NULL) {
        /* Convert the 15-bit sequence to a 5-digit octal string */
        bits_to_octal(current->bits, octal_str);

        /* Write the DecimalAddress and octal sequence to the file */
        fprintf(file, "%04d %s\n", current->DecimalAddress, octal_str);

        /* Move to the next entry */
        current = current->next;
    }

    /* Close the file */
    fclose(file);
}

/**
 * @brief Writes the ExternTable to a .ext file.
 * 
 * This function writes the entries of the ExternTable to a file with a .ext extension. 
 * Each entry consists of a label and its corresponding DecimalAddress.
 * 
 * @param file_name The original file name.
 * @param head Pointer to the head of the ExternTable list.
 */
void extern_table_to_ext(const char* file_name, ExternTable* head) {
    FILE *file;
    char new_file_name[MAX_LINE_LENGTH];
    ExternTable* current;
    char* type = "extern";

    /* Change the file extension to .ext */
    change_file_extension(file_name, new_file_name, type);

    /* Open the new .ext file for writing */
    file = fopen(new_file_name, "w");
    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }

    /* Iterate through the table and write each entry */
    current = head;
    while (current != NULL) {
        /* Write the label and DecimalAddress to the file */
        fprintf(file, "%s %04d\n", current->label, current->DecimalAddress);
        current = current->next;
    }

    /* Close the file */
    fclose(file);
}

/**
 * @brief Writes the EntryTable to a .ent file.
 * 
 * This function writes the entries of the EntryTable to a file with a .ent extension. 
 * Each entry consists of a label and its corresponding DecimalAddress.
 * 
 * @param file_name The original file name.
 * @param head Pointer to the head of the EntryTable list.
 */
void entry_table_to_ent(const char* file_name, EntryTable* head) {
    FILE *file;
    char new_file_name[MAX_LINE_LENGTH];
    EntryTable* current;
    char* type = "entry";

    /* Change the file extension to .ent */
    change_file_extension(file_name, new_file_name, type);

    /* Open the new .ent file for writing */
    file = fopen(new_file_name, "w");
    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }

    /* Iterate through the table and write each entry */
    current = head;
    while (current != NULL) {
        /* Write the label and DecimalAddress to the file */
        fprintf(file, "%s %04d\n", current->label, current->DecimalAddress);
        current = current->next;
    }

    /* Close the file */
    fclose(file);
}

