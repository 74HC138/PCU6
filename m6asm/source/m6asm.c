#include "m6asm.h"

uint8_t memory[1024];

char* sourceFile;
char* outputFile;
char* listFile;
FILE* source;
FILE* output;
FILE* list;
struct Constant* constants;
enum FileType fileType;

enum LogLevel logLevel = Normal;
int useColor = 1;

int main(int argc, char** argv) {
    
    int len = 0;
    while (Command[len] != NULL) len++;

    fileType = bin;

    for (int i = 1; i < argc; i++) {
        switch(getIndex(argv[i], Command, len)) {
            case 0:
            case 1:
                //set input file
                i++;
                if (i >= argc) {
                    printf("Parameter error!\n");
                    return -1;
                }
                sourceFile = argv[i];
                break;
            case 2:
            case 3:
                //set output file
                i++;
                if (i >= argc) {
                    printf("Parameter error!\n");
                    return -1;
                }
                outputFile = argv[i];
                break;
            case 4:
            case 5:
                //set list file
                i++;
                if (i >= argc) {
                    printf("Parameter error!\n");
                    return -1;
                }
                listFile = argv[i];
                break;
            case 6:
            case 7:
                fileType = ihex;
                break;
            case 8:
            case 9:
                fileType = bin;
                break;
            case 10:
            case 11:
                fileType = hex;
                break;
            case 12:
                displayHelp();
                return 0;
            case 13:
            case 14:
                logLevel = Verbose;
                break;
            case 15:
            case 16:
                logLevel = Debug;
                break;
            case 17:
            case 18:
                useColor = 0;
                break;
            case 19:
            case 20:
                logLevel = Silent;
                break;
            case 21:
            case 22:
                logLevel = ForceSilent;
                break;
            default:
                printf("Parameter error! Command %s not recognised.\n", argv[i]);
                return -1;
                break;
        }
    }

    if (sourceFile == NULL) {
        logMessage(Normal, error, "No input file given!\n");
        return -1;
    }

    source = fopen(sourceFile, "r");
    if (!source) {
        logMessage(Normal, error, "Cant open source file!\n");
        return -1;
    }

    if (listFile != NULL) {
        list = fopen(listFile, "w");
        logMessage(Debug, debug, "[Dbug] Opened List file\n");
    } else {
        list = NULL;
    }

    logMessage(Normal, info, "[i] First pass\n");
    if (assemble(0, source, 0, NULL) < 0) return -1;
    logMessage(Debug, debug, "[Dbug] First pass complete\n");
    
    fseek(source, 0, SEEK_SET);

    logMessage(Normal, info, "[i] Second pass\n");
    int size = assemble(1, source, 0, list);
    if (size < 0) return -1;
    logMessage(Verbose, info, "[i] Assembling complete\n");
    logMessage(Verbose, normal, "Assembled size: %i bytes\n", size);

    fclose(source);
    if (list != NULL) {
        fclose(list);
    }

    if (outputFile == NULL) {
        logMessage(Verbose, info, "[i] No output specified\n");
        return 0;
    }

    output = fopen(outputFile, "w");
    switch (fileType) {
        case ihex:
            for (int i = 0; i < 1024; i += 16) {
                fprintf(output, "%04x: ", i);
                for (int n = 0; n < 16; n++) {
                    fprintf(output, "%02x ", memory[i + n]);
                }
                fprintf(output, "\n");
            }
            break;
        case bin:
            fwrite((void*) memory, sizeof(uint8_t), 1024, output);
            break;
        case hex:
            for (int i = 0; i < 1024; i++) {
                fprintf(output, "%02x", memory[i]);
            }
            break;
        default:
            logMessage(Normal, error, "This should never be reached! Congratulation you broke my code!\n");
            break;
    }
    fclose(output);

    return 0;
}

void logMessage(int level, enum LogType type, const char* format, ...) {
    if (level <= logLevel || (level >= -1 && type == error)) {
        if (useColor != 0) {
            switch (type) {
                default:
                case normal:
                    printf("\x1b[0m"); //reset text to normal
                    break;
                case info:
                    printf("\x1b[33m"); //set text to yellow
                    break;
                case debug:
                    printf("\x1b[34m"); //set text to blue
                    break;
                case error:
                    printf("\x1b[91m\x1b[1m"); //set text to bold red
                    break;
            }
        }
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
        if (useColor != 0) printf("\033[0m"); //reset color back to normal
    }
}

void displayHelp() {
    printf("Help:\n");
    printf("This utility is used to assemble code for the PCU6.\n");
    printf("Available commands are:\n");
    printf("-i\t--input\t\tset input file\n");
    printf("-o\t--output\tset output file\n");
    printf("-l\t--listing\tset listing file\n");
    printf("-ih\t--ihex\t\tset intel hex as output type\n");
    printf("-b\t--bin\t\tset binary as output file\n");
    printf("-h\t--hex\t\tset raw hex as output file\n");
    printf("\t--help\t\tdisplay this message\n");
    printf("-v\t--verbose\tdisplay verbose output\n");
    printf("-d\t--debug\t\tdisplay debug information\n");
    printf("-n\t--noColor\tdont use color for output\n");
    printf("-s\t--silent\tdisplay no output except error output\n");
    printf("-fs\t--forceSilent\tdisplay no output at all\n");
    return;
}

int getIndex(char* str, char** tokenList, int listLen) {
    for (int i = 0; i < listLen; i++) {
        if (strcmp(str, tokenList[i]) == 0) return i;
    }
    return -1;
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

    logMessage(Debug, debug, "[Dbug] .db string: %s\n", input_string);

    // Tokenize the input string
    while (*input_string != '\0') {
        if (*input_string == '\"') {
            // The token is a string in quotes, so copy the substring between the quotes into the memory array
            logMessage(Debug, debug, "[Dbug] start of string\n");
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
                        logMessage(Debug, debug, "[Dbug] start of escape sequence\n");
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
            input_string++;
        } else {
            // The token is a number, so convert it to a byte and store it in the memory array
            char* new_string = NULL;
            memory[index + added_length] = (unsigned char) strtol(input_string, &new_string, 0);
            if (input_string == new_string) {
                //that token could not be interpreted as a number
                logMessage(Normal, error, "Error, %s ist not a valid number!\n", new_string);
                return -1;
            }
            input_string = new_string;
            added_length++;
        }
    }

    return index + added_length;
}

int assemble(int pass, FILE* src, int startAdr, FILE* listing) {
    int adr = startAdr;
    char *line = (char*) malloc(1024);
    size_t len;
    int lineCount = 0;

    while (42) {
        if(getline(&line, &len, src) == -1) {
            break;
        }
        lineCount++;
        logMessage(Debug, debug, "[Dbug] Evaluating line %i\n", lineCount);
        char* token = getToken(line, 1);
        if (token == NULL) goto finished;
        if (token[0] == ';') {
            if (listing != NULL && pass == 1) {
                fprintf(listing, "%s", line);
            }
            goto finished;
        }
        if (len >= 2) {
            if (token[0] == '/' && token[1] == '/') {
                if (listing != NULL && pass == 1) {
                    fprintf(listing, "%s", line);
                }
                goto finished;
            }
        }

        if (strcmp(token, ".equ") == 0) {
            //define a constant
            logMessage(Debug, debug, "[Dbug] found a constant difinition\n");
            if (listing != NULL) {
                fprintf(listing, "%s", line);
            }
            if (pass == 0) {
                char* name = getToken(line, 2);
                char* value = getToken(line, 3);
                int v;
                if (value[0] == '\'') {
                    //the constant is a char
                    v = value[1];
                } else {
                    //the constant is a number
                    v = strtol(value, NULL, 0);
                }
                logMessage(Debug, debug, "[Dbug] constant name: %s  constant value: %i\n", name, v);

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
            char* value = getToken(line, 2);
            int v = strtol(value, NULL, 0);
            adr = v;
            logMessage(Debug, debug, "[Dbug] found .org. Origin at %i\n", adr);
            if (listing != NULL && pass == 1) {
                fprintf(listing, "%s", line);
            }
            goto finished;
        }
        if (strcmp(token, ".db") == 0) {
            int newAdr = handleDb(line, adr, memory);
            if (newAdr < 0) return -1;
            if (listing != 0 && pass == 1) {
                fprintf(listing, "%s", line);
                for (int i = adr; i < newAdr; i++) {
                    fprintf(listing, "%02x ", memory[adr]);
                }
                fprintf(listing, "\n");
            }
            adr = newAdr;
            goto finished;
        }

        int length = 0;
        if (getLastChar(token, &length) == ':') {
            if (pass == 0) {
                logMessage(Debug, debug, "[Dbug] found a label. Value is %i.\n", adr);
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
            if (listing != NULL) {
                fprintf(listing, "%s", line);
            }
            goto finished;
        }

        struct Instruction inst;
        for (int i = 0; i < 16; i++) {
            if (strcmp(token, OpcodeName[i]) == 0) {
                logMessage(Debug, debug, "[Dbug] found matching opcode\n");
                //this line contains an opcode

                if (pass == 0) {
                    adr += 2; //no need to assemble the instruction on first pass, just advance the address
                    if (adr > 1024) {
                        //oh oh, we ran out of space
                        logMessage(Normal, error, "Error, not enough memory to assemble\n");
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

                        if (listing != NULL) {
                            fprintf(listing, "%04x: %02x %02x  %s", adr, memory[adr], memory[adr + 1], line);
                        }

                        adr += 2;
                        goto finished;
                    }
                    
                    char* src = getToken(line, 2);
                    for (int n = 0; n < 8; n++) {
                        if (strcmp(src, SourceName[n]) == 0) {
                            //source found
                            logMessage(Debug, debug, "[Dbug] matching source found\n");

                            inst.source = n;
                            char* imidiate = getToken(line, 3);
                            imidiate++;
                            imidiate[strlen(imidiate) - 1] = 0; //discard the first and last character
                            int value = 0;
                            if (imidiate[0] == '\'') {
                                //imidiate value is a char
                                value = imidiate[1];
                            } else {
                                if (!findConstant(imidiate, &value)) { //try to find constant with matching name
                                    value = strtol(imidiate, NULL, 0); //if that fails try to convert the string to a number
                                }
                            }
                            inst.imidiate = value;

                            logMessage(Debug, debug, "[Dbug] imidiate value: %i\n", value);

                            inst.BXI = 0;
                            char* bxi = getToken(line, 4);
                            if (bxi != NULL) {
                                if (strcmp(bxi, "[BX]") == 0) {
                                    logMessage(Debug, debug, "[Dbug] BX flag 1\n");
                                    inst.BXI = 1;
                                } else {
                                    logMessage(Debug, debug, "[Dbug] BX flag 0\n");
                                    inst.BXI = 0;
                                }
                            } else {
                                inst.BXI = 0;
                                logMessage(Debug, debug, "[Dbug] BX flag 0\n");
                            }

                            uint16_t op = toOpcode(inst);
                            memory[adr] = op & 0x00ff;
                            memory[adr + 1] = (op >> 8) & 0x00ff;

                            if (listing != NULL) {
                                fprintf(listing, "%04x: %02x %02x  %s", adr, memory[adr], memory[adr + 1], line);
                            }

                            adr += 2;
                            goto finished;
                        }
                    }
                    logMessage(Normal, error, "Error on line %i! Source not recognised.\n", lineCount);
                    return -1;
                }
            }
        }

        logMessage(Normal, error, "Error on line %i\n", lineCount);
        return -1;
        finished:
    }
    return adr;
}