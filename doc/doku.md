#CPU MK6

##Overview

The CPU Mark 6 core has 4 registers: A, B, X and PC.
The A and B registers are general purpose registers.
The X register is the index register into the volatile memory.
The PC register is the index into the non volatile memory used for program storage.

##Instruction set

Every instruction has a fixed with of 16 bits.
| 15      8 | 7  | 6    4 | 3    0 |
| Parameter |BX/I| Source | Opcode |

Parameter: 8 bit imidiate data
BX/I: Switch between B or X register and imidiate data for arythemtic operations and memory indexing. (If 0 -> MEM[i] or A + i, If 1 -> MEM[X] or A + B)
Source: Selector for data source
	0: (i) Imidiate data
	1: (a) A register
	2: (b) B register
	3: (+) A + B/I + Carry
	4: (&) A and B/I
	5: (|) A or B/I
	6: (^) A xor B/I
	7: (m) MEM[X/I]
Opcode:
	0: LDA src (imidiate) {[BX/I]}  Load src to register A
	1: LDB src (imidiate) {[BX/I]}  Load src to register B
	2: LDX src (imidiate) {[BX/I]}  Load src to register X
	3: LDM src (imidiate) {[BX/I]}  Load src to memory[X/I]
	4: JMP src (imidiate) {[BX/I]}  Jump to src
	5: JCC src (imidiate) {[BX/I]}  Jump to src if CF is set
	6: CMP src (imidiate) {[BX/I]}  Perform load without target and update flags
	7: SCF (imidiate CC)		Set Conditional flag if CC is true
	8: 
	9: 
	A: INX				Increment register X
	B: DEX				Decrement register X
	C: LAR [BX/I]			Load data rom memory from X/I to A
	D: LBR [BX/I]			Load data rom memory from X/I to B
	E:
	F:
	
SCF (imidiate CC):
CC bits:
	0: Invert Carry
	1: Invert Zerro
	2: Include Carry
	3: Include Zero
	4: Invert Shift Flag
	5: Include Shift Flag
	
##Programs

###Counter

Start:
00 0000  LDA i (0) //load 0 to A
Loop:
01 0130  LDA + (1) [I] //add 1 to A
02 0507  SCF (b00000101) //if carry not set
03 0005  JCC i (Loop) //jump to Loop if CC true
Halt:
04 0304  JMP i (Halt) //jump to halt

###Text Output

Start:
00 0002  LDX i (0)
Loop:
01 008C  LAR (0) [BX]
02 0807  SCF (b00001000)
03 0605  JCC i (End)
04 FF13  LDM a (ff) [I]
05 000A  INX
06 0004  JMP i (Loop)
End:
07 0604  JMP i (End)

###Multiplication
LDA i (0)
LDB i (5)
LDX i (4)
Loop:
LDA + (0) [BX]
DEX
SCF (b00001010) //test if zero flag is clear
JCC i (Loop)
Halt:
JMP i (Halt)



	
	
