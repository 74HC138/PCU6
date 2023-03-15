#ifndef M6ASM_H
#define M6ASM_H

#include <stdio.h>
#include <malloc.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

char* Command[] = {
    "-i",
    "--input",
    "-o",
    "--output",
    "-l",
    "--listing",
    "-ih",
    "--ihex",
    "-b",
    "--bin",
    "-h",
    "--hex",
    "--help",
    "-v",
    "--verbose",
    "-d",
    "--debug",
    "-n",
    "--noColor",
    "-s",
    "--silent",
    "-fs",
    "--forceSilent",
    NULL
};

enum FileType {
    ihex,
    hex,
    bin
};

char* OpcodeName[] = {
    "lda",
    "ldb",
    "ldx",
    "ldm",
    "jmp",
    "jcc",
    "cmp",
    "scf",
    "undef0",
    "undef1",
    "inx",
    "dex",
    "lar",
    "lbr",
    "undef2",
    "undef3"
};

enum Opcode {
    LDA = 0,
    LDB = 1,
    LDX = 2,
    LDM = 3,
    JMP = 4,
    JCC = 5,
    CMP = 6,
    SCF = 7,
    UNDEF0 = 8,
    UNDEF1 = 9,
    INX = 10,
    DEX = 11,
    LAR = 12,
    LBR = 13,
    UNDEF2 = 14,
    UNDEF3 = 15
};

char* SourceName[] = {
    "i",
    "a",
    "b",
    "+",
    "&",
    "|",
    "^",
    "m"
};

enum Source {
    Imidiate = 0,
    A = 1,
    B = 2,
    Add = 3,
    And = 4,
    Or = 5,
    Xor = 6,
    Memory = 7
};

struct Instruction {
    enum Opcode opcode;
    enum Source source;
    int BXI;
    uint8_t imidiate;
};

struct Constant {
    char* name;
    int value;
    struct Constant* nextConst;
};

enum LogType {
    normal,
    info,
    debug,
    error
};

enum LogLevel {
    ForceSilent = -2,
    Silent = -1,
    Normal = 0,
    Verbose = 1,
    Debug = 2
};

void logMessage(enum LogLevel level, enum LogType type, const char* format, ...);
void displayHelp();
int getIndex(char* str, char** tokenList, int listLen);
char* getToken(const char* str, int n);
int findConstant(char* name, int* value);
uint16_t toOpcode(struct Instruction instr);
char getLastChar(char* str, int* length);
int assemble(int pass, FILE* src, int startAdr, FILE* listing);
int handleDb(char *input_string, int index, unsigned char *memory);

#endif