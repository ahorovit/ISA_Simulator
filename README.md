# ISA_Simulator
##Simple Instruction Set Simulator with example assembly input



  Simple Instruction Set Simulator (ISS): Loads assembly code in from file, simulates
	execution of limited set of instructions. Reports total cycle count, total # executed
	instructions, total number instructions in code, #hits to cache, # LD/ST executed
	System Specs:
	8-bit processor
	6 8-bit integer registers [R1,R6]
	byte-addressable 256Byte cache
	Supported instructions:
	MOV Rn, <num>  --> stores 8-bit integer [-128,127] in register Rn
	ADD Rn, Rm		 --> Adds registers and stores result in Rn
	ADD Rn, <num>	 --> Adds immediate to value stored in Rn
	CMP Rn, RM		 --> Tests for equality b/w registers, sets flag
	JE <Address>	 --> jumps to instruction at Address IF CMP flag is set
	JMP <Address>	 --> unconditional jump
	LD Rn, [Rm]		 --> Loads from address stored in Rm into Rn
	ST [Rm], Rn		 --> Stores contents of Rn into memory address in Rm
	Cycles per instruction:
	MOV, ADD, JMP	--> 1cycle
	CMP						--> 2cycle
	LD/ST					--> 2cycle (hit) or 40cycle (miss)	
