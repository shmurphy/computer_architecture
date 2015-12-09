/*
 ============================================================================
 Name        : 372project.h
 Author      : Shannon Murphy
 Version     : 12/8/2015
 ============================================================================
 */

#ifndef FSM_H_
#define FSM_H_

// States
#define FETCH 0
#define DECODE 1
#define EXECUTE 2

// Opcodes
#define LDI 1
#define ST 4
#define ADD 8
#define SUB 9
#define AND 10
#define OR 11
#define NOT 7
#define BR 2
#define BRZ 17
#define HALT 30 //A
#define NUM_REGS 16

// bit shifting constants
#define OPCODE_SHIFT 27
#define IMM_SHIFT 3
#define Z_SHIFT 23
#define X_SHIFT 19
#define Y_SHIFT 15
#define NEG_SHIFT 25
#define POS_SHIFT 24
#define ZERO_SHIFT 23

// I/O device offsets
#define IOBASE 0xB0000000
#define TIMER0 0x0
#define KBD    0x4
#define SCRN   0x8
#define COM1   0xC
#define DEVICE_LOOP 3

void printRegs();

#endif /* FSM_H_ */
