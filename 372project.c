/*
 ============================================================================
 Name        : 372project.c
 Author      : Shannon Murphy
 Version     : 12/8/2015
 Description : Simulates the basic structure of a CPU
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "372project.h"

int regs[NUM_REGS]; // register array
int MEM[25];
int IOMEM[25];

/* Initialize registers and variables */
int IR = 0, PC = 0, MAR = 0, MDR = 0, ALU_A = 0, ALU_B = 0, SW = 0;
int opcode;
int regX = 0, regY = 0, regZ = 0, imm = 0, neg = 0, pos = 0, zero = 0, offset = 0;
int signBit = 0;
int state = FETCH;
int startAddress = 0;
int editAddress = 0;

/* I/O variables */
int IO_mode = 0;
char buffer;			// buffer to hold incoming characters

/*--------------------------------------------------------------------------------------------------------*/

/**
 * Helper method to print out all of the Debug-Monitor options.
 */
void printCommands() {
	printf("Commands: 1 = Load, 2 = Step, 3 = Run, 4 = Memory, 5 = Save, 6 = Edit, 9 = Terminate\n"
			"Enter: ");
}

/**
 * Print out the current contents of memory and all registers.
 */
void printMem(int start) {
	int i;
	printf("SC-4 Debug Monitor %31s\n\n", "Programmer: Shannon Murphy");
	printf(" Register File %21s %34s\n", "Memory Dump", "IO Memory Space");
	for(i = start; i < start + NUM_REGS; i++) {
		printf("% 2X: %08X %20.08X: %08X %20.08X: %08X\n", i, regs[i], i, MEM[i], i, IOMEM[i]);
	}
	printf("\nPC: %08d IR: 0x%08X String buffer: %c\n", PC, IR, buffer);
}

/**
 * Simulates the keyboard being polled to see if the ready bit is 1.
 */
void kbdLoop() {
	int i = 0;
	while(i < DEVICE_LOOP) {
		printMem(0);	// print IOMEM to show that nothing is changing until ready bit is 1
		i++;
	}
	IOMEM[KBD] = 1;
	printMem(0);
}

/**
 * Simulates the screen being polled to see if the ready bit is 1.
 */
void scrnLoop() {
	int i = 0;
	while(i < DEVICE_LOOP) {
		printMem(0);	// print IOMEM to show nothing is changing until ready bit is 1
		i++;
	}
	IOMEM[SCRN] = 1;
	printMem(0);
}

/**
 * Simulation of fetch, decode, execute cycle
 */
void runProgram(int stepMode) {
	PC = 0;
	state = FETCH;
	char stepKey;
	while (state != HALT) {
		switch (state) {
			case FETCH:
				if(stepMode) {
					scanf("%c", &stepKey);
					if(stepKey == '\n') {
						MAR = PC++;
						IR = MEM[MAR];
						state = DECODE;
					}
				} else if(stepMode == 0){
					MAR = PC++;
					IR = MEM[MAR];
					state = DECODE;
				}

				break;
			case DECODE:
				opcode = (IR & 0xF8000000) >> OPCODE_SHIFT;
				switch (opcode) {
				case ADD:
				case SUB:
				case AND:
				case OR:
					regZ = (IR & 0x7800000) >> Z_SHIFT;
					regX = (IR & 0x780000) >> X_SHIFT;
					regY = (IR & 0x78000) >> Y_SHIFT;
					ALU_A = regX;
					ALU_B = regY;
					state = EXECUTE;
					break;
				case LDI:
					regZ = (IR & 0x7800000) >> Z_SHIFT;
					imm = (IR & 0x7FFFFF) >> 0;
					signBit = (IR & 0x400000) >> 22;

					if(signBit == 1) {
						imm = imm | 0xFF800000;
					}

					state = EXECUTE;
					break;
				case ST:
					regZ = (IR & 0x7800000) >> Z_SHIFT;
					regX = (IR & 0x780000) >> X_SHIFT;

					offset = (IR & 0x7FFFF) >> 0;
					signBit = (IR & 0x40000) >> 18;

					if(signBit == 1) {
						offset = offset | 0xFFF80000;
					}

					state = EXECUTE;
					break;
				case BR:
				case BRZ:
					neg = (IR & 0x4000000) >> NEG_SHIFT;
					pos = (IR & 0x2000000) >> POS_SHIFT;
					zero = (IR & 0x1000000) >> Z_SHIFT;
					offset = (IR & 0xFFFFF) >> IMM_SHIFT;
					state = EXECUTE;
					break;
				case HALT:
					state = HALT;
					printf("In HALT microstate, program stopping.\n\n");
					break;
				}

				break;
			case EXECUTE:
				state = FETCH;
				switch (opcode) {
					case ADD:
						regs[regZ] = ALU_A + ALU_B;
						break;
					case SUB:
						regs[regZ] = ALU_A - ALU_B;
						break;
					case AND:
						regs[regZ] = ALU_A & ALU_B;
						break;
					case OR:
						regs[regZ] = ALU_A | ALU_B;
						break;
					case LDI:
						regs[regZ] = imm;
						break;
					case ST:
						if(IO_mode) {
							if(offset == KBD + 1) {		// keyboard
								kbdLoop();
								IOMEM[regs[regZ]+offset] = regs[regX];
								buffer = regs[regX];
								printMem(0);
								IOMEM[KBD] = 0;
							} else if(SCRN + 1) {		// screen
								buffer = 0;
								scrnLoop();
								IOMEM[regs[regZ]+offset] = regs[regX];
								printMem(0);
								IOMEM[SCRN] = 0;
							}
						} else {
							MEM[regs[regZ]+offset] = regs[regX];
						}
						break;
					case BR:
						PC = PC + offset;
						break;
					case BRZ:
						if(zero==1) {PC = PC+offset;}
						break;
				}
				printMem(startAddress);
				if(stepMode) {
					printf("\nThe program is in step mode. Hit 'enter' to step through the program.\n");
				}
				break;
		}
	}
}

/**
 * Opens up a file and reads the data into memory.
 * Also clears the registers so that new data can be read into them.
 */
int openFile(char* fileName) {
	FILE *program;
	program =fopen(fileName,"r");
	if (!program) {
		return 1;
	}
	int u;
	int i;
	for(i = 0; i <= 25; i++) {
		fscanf(program, "%x", &u);
		MEM[i] = u;
	}
	fclose(program);
	return 0;
}

/**
 * Scans for which option the user selects
 */
int getUserSelection() {
	int userSelection;
	scanf("%d", &userSelection);
	return userSelection;
}

/**
 * Saves the contents of memory to a file, with start and end points specified by the user.
 */
void saveFile(char* fileName) {
	int start;
	int end;
	int i;
	FILE *save;
	save =fopen(fileName,"w");

	printf("Starting address: ");
	scanf("%x", &start);
	printf("Ending address: ");
	scanf("%x", &end);

	for(i = start; i <= end; i++) {
		fprintf(save, "%08X: %08X\n", i, MEM[i]);
	}

	fclose(save);
}

/**
 * Runs the program until the user enters a 9 for HALT.
 */
void startProgram() {
	int k;
	for(k = 0; k < NUM_REGS; k++) {
		MEM[k] = rand() % 0xFFFFFFFF;
		regs[k] = rand() % 0xFFFFFFFF;
//		IOMEM[k] = rand() % 0xFFFFFFFF;
	}

	int halt = 0; // flag to stop the simulation
	int stepMode = 0; // flag to step through program
	int userSelection; // represents which menu option the user selects

	while (!halt) {
		printMem(startAddress);
		printCommands();
		userSelection = getUserSelection();
		int fileSuccess = 1; // flag to determine whether file was successfully read
		switch(userSelection) {
			case 1:
				while(fileSuccess) {
					printf("Enter name of file to load: ");
					char fileName[20];
					scanf("%s", &fileName);
					if(fileName[0] == 'I') {
						IO_mode = 1;
					}
					fileSuccess = openFile(fileName);
					if(fileSuccess) {
						printf("That file does not exist... Try again.\n");}
				}
				break;
			case 2:
				stepMode = 1;
				runProgram(stepMode);
				break;
			case 3:
				stepMode = 0;
				printf("\nProgram running...\n");
				runProgram(stepMode);
				break;
			case 4:
				printf("Starting memory address: ");
				scanf("%x", &startAddress);
				if(startAddress >= 0xB0000000) {
					startAddress = startAddress - 0xB0000000;
				}

				printf("STARTING ADDRESS: %X\n", startAddress);
				break;
			case 5:
				printf("Enter name of file to save: ");
				char fileName[20];
				scanf("%s", &fileName);
				saveFile(fileName);
				break;
			case 6:
				printf("Enter the address you would like to edit: ");
				scanf("%x", &editAddress);
				printf("Enter the value you would like to place in address %X: ", editAddress);
				int editValue = 0;
				scanf("%x", &editValue);
				if(editAddress >= 0xB0000000) {
					IOMEM[editAddress -  0xB0000000] = editValue;
				} else {
					MEM[editAddress] = editValue;
				}
				break;
			case 9:
				printf("Program terminating.");
				halt = 1;
				break;
		}
	}
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    srand(time(NULL));	// on startup, randomize the values of MEM and registers
    startProgram();
    return 0;
}
