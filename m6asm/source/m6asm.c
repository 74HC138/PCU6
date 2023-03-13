#include "m6asm.h"

uint8_t memory[1024];
uint16_t* mem16 = (uint16_t*) memory;

char* sourceFile;
char* outputFile;
FILE* source;
FILE* output;
struct Constant* constants;


int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Not enough arguments!\n");
        printf("Usage:\n");
        printf("m6asm [source] [destination]\n");
        return -1;
    }
    sourceFile = argv[1];
    outputFile = argv[2];

    source = fopen(sourceFile, "r");
    if (!source) {
        printf("Cant open source file!\n");
        return -1;
    }

    printf("Pass 1\n");
    if (assemble(0) != 0) return -1;
    fseek(source, 0, SEEK_SET);
    printf("Pass 2\n");
    if (assemble(1) != 0) return -1;
    printf("Finished assembling\n");

    if (argc > 3) {
        if (strcmp(argv[3], "-hex") == 0) {
            output = fopen(outputFile, "w");
            for (int i = 0; i < 1024; i += 16) {
                fprintf(output, "%04x: ", i);
                for (int n = 0; n < 16; n++) {
                    fprintf(output, "%02x ", memory[i + n]);
                }
                fprintf(output, "\n");
            }
            fclose(output);
            fclose(source);
            printf("Outputfile is written\n");
            return 0;
        }
    }

    output = fopen(outputFile, "wb");
    fwrite((void*) memory, sizeof(uint8_t), 1024, output);
    fclose(output);
    fclose(source);
    printf("Outputfile is written\n");

    return 0;
}

char* getToken(const char* str, int n) {
    char* nth_word = NULL;
    char* temp_str = strdup(str); // make a copy of the string to avoid modifying the original
    
    if (temp_str != NULL) {
        char* delimiter = " \n";
        char* token = strtok(temp_str, delimiter); // get the first token
        
        for (int i = 1; i < n && token != NULL; i++) {
            token = strtok(NULL, delimiter); // get the next token
        }
        
        if (token != NULL) {
            nth_word = strdup(token); // make a copy of the nth token
        }
        
        free(temp_str); // free the memory allocated for the copy of the string
    }
    
    return nth_word;
}

int findConstant(char* name, int* value) {
    if (constants == NULL) return 0;
    struct Constant* ptr = constants;
    while (ptr != NULL) {
        if (strcmp(ptr->name, name) == 0) {
            //found match
            *value = ptr->value;
            return 1;
        }
        ptr = ptr->nextConst;
    }
    return 0;
}

uint16_t toOpcode(struct Instruction instr) {
    uint16_t opcode = (instr.opcode << 8) & 0x0f00;
    opcode |= (instr.source << 12) & 0x7000;
    if (instr.BXI) {
        opcode |= 0x8000;
    }
    opcode |= instr.imidiate & 0x00ff;
    return opcode;
}

char getLastChar(char* str, int* length) {
    int len = strlen(str);
    for (int i = len - 1; i >= 0; i--) {
        if (!isspace(str[i])) {
            if (length) {
                *length = len - 1;
            }
            return str[i];
        }
    }
    return '\0';
}

int handleDb(char *input_string, int index, unsigned char *memory) {
    char *token_start, *token_end;
    int added_length = 0;

    // Find the start of the input string after ".db"
    input_string = strstr(input_string, " ");
    if (input_string != NULL) {
        // Move the input string pointer past the opening quote
        input_string++;
    } else {
        return index;
    }

    printf("%s\n", input_string);

    // Tokenize the input string
    while (*input_string != '\0') {
        if (*input_string == '\"') {
            // The token is a string in quotes, so copy the substring between the quotes into the memory array
            printf("start string\n");
            token_start = input_string + 1;
            char* start = token_start;
            while (42) {
                token_end = strchr(start, '\"');
                if (*(token_end - 1) == '\\') {
                    start = token_end + 1;
                } else {
                    break;
                }
            }
            if (token_end != NULL) {
                int string_length = token_end - token_start;
                for (int i = 0; i < string_length; i++) {
                    if (token_start[i] == '\\') {
                        // Handle escape sequences
                        printf("escape sequence detected\n");
                        switch (token_start[i+1]) {
                            case 'n': memory[index + added_length] = '\n'; break;
                            case 't': memory[index + added_length] = '\t'; break;
                            case '0': memory[index + added_length] = '\0'; break;
                            case '\\': memory[index + added_length] = '\\'; break;
                            case '\"': memory[index + added_length] = '\"'; break;
                            case '\'': memory[index + added_length] = '\''; break;
                            default:  memory[index + added_length] = token_start[i+1]; break;
                        }
                        i++;
                    } else {
                        memory[index + added_length] = token_start[i];
                    }
                    added_length++;
                }
                input_string = token_end + 1;
            } else {
                // There is no closing quote, so return the current index
                return index + added_length;
            }
        } else if (*input_string == ',') {
            // The token is a comma, so skip it
            printf("token is comma\n");
            input_string++;
        } else {
            // The token is a number, so convert it to a byte and store it in the memory array
            printf("token is number\n");
            char* new_string = NULL;
            memory[index + added_length] = (unsigned char) strtol(input_string, &new_string, 0);
            if (input_string == new_string) {
                //that token could not be interpreted as a number
                printf("Error, %s is not a valid number!", new_string);
                return -1;
            }
            input_string = new_string;
            added_length++;
        }
    }

    return index + added_length;
}

int assemble(int pass) {
    int adr = 0;
    char *line = (char*) malloc(1024);
    size_t len;
    int lineCount = 0;

    while (42) {
        if(getline(&line, &len, source) == -1) {
            break;
        }
        lineCount++;
        printf("Evaluating line %i\n", lineCount);
        char* token = getToken(line, 1);
        if (token == NULL) goto finished;
        printf("%s\n", token);

        if (strcmp(token, ".equ") == 0) {
            //define a constant
            printf("found a const definition\n");
            if (pass == 0) {
                char* name = getToken(line, 2);
                char* value = getToken(line, 3);
                int v = strtol(value, NULL, 0);
                printf("const name: %s\nconst value: %i\n", name, v);

                if (constants == NULL) {
                    constants = (struct Constant*) malloc(sizeof(struct Constant));
                    constants->name = (char*) malloc(strlen(name));
                    strcpy(constants->name, name);
                    constants->value = v;
                    constants->nextConst = NULL;
                } else {
                    struct Constant* ptr = constants;
                    while (ptr->nextConst != NULL) {
                        ptr = ptr->nextConst;
                    }
                    ptr->nextConst = (struct Constant*) malloc(sizeof(struct Constant));
                    ptr = ptr->nextConst;
                    ptr->name = (char*) malloc(strlen(name));
                    strcpy(ptr->name, name);
                    ptr->value = v;
                    ptr->nextConst = NULL;
                }
            }
            goto finished;
        }
        if (strcmp(token, ".org") == 0) {
            //define origin
            printf("found origin\n");
            char* value = getToken(line, 2);
            int v = strtol(value, NULL, 0);
            adr = v;
            goto finished;
        }
        if (strcmp(token, ".db") == 0) {
            adr = handleDb(line, adr, memory);
            if (adr < 0) return -1;
            goto finished;
        }

        int length = 0;
        if (getLastChar(token, &length) == ':') {
            if (pass == 0) {
                printf("this is a constant\n");
                printf("name length is %i\n", length);
                printf("value is %i\n", adr);
                //this is a constant
                if (constants == NULL) {
                    constants = (struct Constant*) malloc(sizeof(struct Constant));
                    constants->name = (char*) malloc(length + 1);
                    strncpy(constants->name, token, length);
                    constants->value = adr;
                    constants->nextConst = NULL;
                } else {
                    struct Constant* ptr = constants;
                    while (ptr->nextConst != NULL) {
                        ptr = ptr->nextConst;
                    }
                    ptr->nextConst = (struct Constant*) malloc(sizeof(struct Constant));
                    ptr = ptr->nextConst;
                    ptr->name = (char*) malloc(length + 1);
                    strncpy(ptr->name, token, length);
                    ptr->value = adr;
                    ptr->nextConst = NULL;
                }
            }
            goto finished;
        }

        struct Instruction inst;
        for (int i = 0; i < 16; i++) {
            if (strcmp(token, OpcodeName[i]) == 0) {
                printf("match found\n");
                //this line contains an opcode

                if (pass == 0) {
                    adr += 2; //no need to assemble the instruction on first pass, just advance the address
                    if (adr > 1024) {
                        //oh oh, we ran out of space
                        printf("Error, not enought memory to compile!\n");
                        return -1;
                    }
                    goto finished;
                } else {
                    //in second pass the instruction gets interpreted fully
                    inst.opcode = i;
                    if (inst.opcode == DEX || inst.opcode == INX) {
                        inst.imidiate = 0;
                        inst.source = 0;
                        inst.BXI = 0;

                        uint16_t op = toOpcode(inst);
                        memory[adr] = op & 0x00ff;
                        memory[adr + 1] = (op >> 8) & 0x00ff;

                        adr += 2;
                        goto finished;
                    }
                    
                    char* src = getToken(line, 2);
                    printf("source token: %s\n", src);
                    for (int n = 0; n < 8; n++) {
                        if (strcmp(src, SourceName[n]) == 0) {
                            //source found
                            printf("matching source found\n");

                            inst.source = n;
                            char* imidiate = getToken(line, 3);
                            imidiate++;
                            imidiate[strlen(imidiate) - 1] = 0; //discard the first and last character
                            printf("imidiate token: %s\n", imidiate);
                            int value = 0;
                            if (!findConstant(imidiate, &value)) { //try to find constant with matching name
                                value = strtol(imidiate, NULL, 0); //if that fails try to convert the string to a number
                            }

                            if (inst.opcode == JMP || inst.opcode == JCC) {
                                inst.imidiate = (uint8_t) (value >> 1) - 1;
                            } else {
                                inst.imidiate = value;
                            }

                            printf("imidiate value: %i\n", value);

                            char* bxi = getToken(line, 4);
                            if (bxi != NULL) {
                                if (strcmp(bxi, "[BX]") == 0) {
                                    inst.BXI = 1;
                                }
                            } else {
                                inst.BXI = 0;
                            }

                            uint16_t op = toOpcode(inst);
                            memory[adr] = op & 0x00ff;
                            memory[adr + 1] = (op >> 8) & 0x00ff;

                            adr += 2;
                            goto finished;
                        }
                    }
                    printf("Error on line %i! Source not found.\n", lineCount);
                    return -1;
                }
            }
        }

        printf("Error on line %i\n", lineCount);
        return -1;
        finished:
    }
    return 0;
}