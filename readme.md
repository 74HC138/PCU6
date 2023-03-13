# Programmable Computation Unit Version 6

The PCU6 is a minimal memory programmed computer that can handle nearly any task (as long as it fits within 256 instructions) as it is turing complete (even a bit more than that).

### Overview

The goal of the project is to create a rather minimal, jet easely programmable computer.

The PCU6 has 4 registers: The general A and B registers, the memory pointer X and the program counter PC. There are also 256 bytes of memory (work memory) indexed by eather the X register or an imidiate value, that acts like a pseudo register. The register also indexes the constant memory which can be loaded into the A or B register. The program memory is logicaly seperated from the constant and work memory, even though the program and constant memory share one ROM for simplicity.

For more information about the hardware, assembly and how to program the PCU6 look at the detailed documentation in the doc folder.

Also there might be further expansion to allow for more programm memory as 256 instructions are quite limiting.

### Machine Language

Every instruction for the PCU6 is 16bit wide. 8 of those is an imidiate constant, that can be used as a source for loading a register or in place of the B and X register in any operation.

| bit number | 15 .. 8   | 7  | 6 .. 4 | 3 .. 0 |
| ---------- | --------- | -- | ------ | ------ |
| function   | Parameter |BX/I| Source | Opcode |

The BX/I flag controlles of the B and X register should be used (1) or the imidiate value (0) in any operation that uses the B and X register.
The Source value specifies wich source of data is used for load instructions.
The Opcode field specifies the operation to be executet.

For more details look into the documentation in the doc folder

### Assembly

Included in this repo is a very rudimentary assembler that can assemble PCU6 assembly code. The structure of the assembly is a *bit* different then other CPUs.

The first field is the opcode, then the source, followed by the imidiate value and last of all the BX/I flag.

As an example:
```ldm a (0xff) [I]```
This instruction loads the register A into work memory at address 0xff.
```ldm a (0xff) [BX]```
While this instruction load the register A into work memory at the index X.

The assembler can also handle labels. These are indicated by a trailing `:`.
As an example:
```Loop:```

Also there are constants using the `.equ` command, origin address with `.org` and data using `.db`.

For more details look into the documentation in the doc folder

### Future expansion

To increase the real world usefullness of the PCU6 I plan on implimenting an EPC (extended PC) and EX (extended X) register. These would allow for 128kB of instruction memory, 64kb of constant memory and an extended work memory that could function as an interface to peripherals.

Also I plan on creating a PCB that would impliment the PCU6 with CMOS logic ICs but this might take some time. Im also not shure if i want to make an all in one system (so a single board computer/processor) or a more modular aproche that would allow for expansions (like more memory, peripherals, interrupts or even an MMU allowing for multitasking).

### ToDo's

- [x] create an assembler
- [ ] write good documentation
- [ ] write example code
- [ ] expand address space
- [ ] create working real life implimentation

### Disclaimer

I release this project with no waranty of it working, or breaking your system. If you base critical infrastructure on this project you are out of your mind and on your own.

### License

This project is licensed under GPL3. Do with the code and whatnot what you want. Clone, share, fork and improve if you want to, just give some credit to me and we are fine. 