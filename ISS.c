#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
/*
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

*/

// Assembly code constraints
#define MAX_INSTR 512   // Arbitrary code size limit
int entryPt = 1024; // stores min instruction index (starts at max)
int exitPt = -1;		// stores max instruction index

// Hardware variables
int isEqual = 0;  		// CMP result flag (0 means unequal, 1 means equal)
int8_t registers[6];	// 6 8-bit registers
int8_t cache[256];		// 256B byte-addressable memory
uint8_t valid[256] = {0};	// valid bits for cache


// Counters for output
int executed = 0;
int cycles = 0;
int hits = 0;
int LdSt = 0;

// functions for parsing/executing assembly code
void parse_assembly(char *filename, char opCodes[], int8_t operands[][2]);
void exec_assembly(char opCodes[], int8_t operands[][2]);


int main(int argc, char* argv[]) {

	char opCodes[MAX_INSTR];					// store instructions as characters
	int8_t operands[MAX_INSTR][2];		// store operands as 8-bit integers

	/* Check input arguments */
	if( argc != 2)
	{printf("enter assembly code filename\n");exit(0);}
	
	// decompose assembly into char operations and int operands
	parse_assembly(argv[1], opCodes, operands);

	// Execute assembly code
	exec_assembly(opCodes, operands);		
		
		
	printf("Total number of instructions in the code: %d\n", exitPt - entryPt + 1);	
	printf("Total number of executed instructions: %d\n", executed);
	printf("Total number of clock cycles: %d\n", cycles);
	printf("Number of hits to local memory: %d\n", hits);
	printf("Total number of executed LD/ST instructions: %d\n", LdSt);

/*
	// print out parsed assembly
	int i,j;
	for (i=entryPt; i <= exitPt; i++) {
		printf("%d: [%c]", i, opCodes[i]);
		for (j = 0; j < 2; j++) {
			printf("[%d]", operands[i][j]);
			if(opCodes[i] == 'j' || opCodes[i] == 'J') j++;
		}
		printf("\n");
	}
*/


	return 0;
}

void exec_assembly(char opCodes[], int8_t operands[][2]) {

	int ip = entryPt;	//	Instruction pointer 

	while (ip <= exitPt) {
//		printf("%d\n", ip);		
		executed++;  //Increment executed instruction count

		switch( opCodes[ip] ) {

			case 'm':  // MOV
				registers[ operands[ip][0] ] = operands[ip][1];
				cycles++;
				ip++;
			break;

      case 'a':  // ADD (Register)
				registers[ operands[ip][0] ] = 	registers[ operands[ip][0] ] + registers[ operands[ip][1] ];
				cycles++;
				ip++;
      break;

      case 'A':	 // ADD (Immediate)
				registers[ operands[ip][0] ] = 	registers[ operands[ip][0] ] + operands[ip][1];
				cycles++;
				ip++;
      break;

      case 'l':  // LD
				registers[ operands[ip][0] ] = cache[ operands[ip][1] ];
				if(valid[ registers[ operands[ip][1] ] ] ==0) { // cache miss
					cycles += 40;
					valid[ registers[ operands[ip][1] ] ] = 1;
				}
				else {				// cache hit
					cycles += 2;
					hits++;
				}
				ip++;
				LdSt++;
      break;

      case 's':  // ST
				cache[ operands[ip][0] ] = registers[ operands[ip][1] ];
				if(valid[ registers[ operands[ip][0] ] ] ==0) { // cache miss
					cycles += 40;
					valid[ registers[ operands[ip][0] ] ] = 1;
				}
				else {					// cache hit
					cycles += 2;
					hits++;
				}
				ip++;
				LdSt++;
      break;

      case 'c':  // CMP
				if (registers[ operands[ip][0] ] == registers[ operands[ip][1] ])				
					isEqual = 1;
				else
					isEqual = 0;
				ip++;
				cycles+=2;
      break;

      case 'j':  // JE (conditional)
				if(isEqual == 1)
					ip = operands[ip][0];
				else
					ip++;
				cycles++;
      break;

      case 'J':  // JMP (unconditional)
				ip = operands[ip][0];
				cycles++;
			break;
		}
	}	

}


void parse_assembly(char *filename, char opCodes[], int8_t operands[][2]) {

	// File i/o
	FILE *assembly = fopen(filename, "r");
  char line[256];						  /* buffer for each line */

	// Line parsing
	char *token;		//holds current substring
	const char tab[2] = "\t";
	const char spc[2] = " ";
	const char com[2] = ",";
	const char nln[2] = "\n";
	const char brk[2] = "]";
	
	int index;
	char op;

	// Read each line from file, parse into operands
	while (fgets(line, sizeof(line), assembly)) {

		// Grab line number
		token = strtok(line, tab);	

		// findindex, update program bounds
		index = atoi(token);
		if ( entryPt > index) entryPt = index;
		if ( exitPt < index) exitPt = index;

		// Grab operation
		token = strtok(NULL, spc);			

		// Encode operation as character
		if(strcmp("MOV", token) == 0) op = 'm';
		else if(strcmp("ADD", token) == 0) op = 'a';  //AddImmediate updates further down
		else if(strcmp("LD", token) == 0) op = 'l';
		else if(strcmp("ST", token) == 0) op = 's';
		else if(strcmp("CMP", token) == 0) op = 'c';
		else if(strcmp("JE", token) == 0) op = 'j';
		else if(strcmp("JMP", token) == 0) op = 'J';
		else{printf("invalid instruction\n"); exit(0);}


		switch (op) {
			case 'm':
				// Grab first operand
				token = strtok(NULL, com);			
				token++;		// Remove 'R'
				operands[index][0] = (int8_t)atoi(token);

	      // Grab second operand --> Never a register
	      token = strtok(NULL, spc);
				operands[index][1] = (int8_t)atoi(token);
			break;

			case 'a':
        // Grab first operand
        token = strtok(NULL, com);
        token++;    	// Remove 'R'
        operands[index][0] = (int8_t)atoi(token);

        // Grab second operand  --> may be register or Immediate
        token = strtok(NULL, spc);
				if(token[0] == 'R')
					token++;  // If register, remove 'R'
				else 
					op = 'A';  // Else, redesignate as Add Immediate 

        operands[index][1] = (int8_t)atoi(token);
			break;

			case 'c':  
        // Grab first operand
        token = strtok(NULL, com);
        token++;      // Remove 'R'
        operands[index][0] = (int8_t)atoi(token);

	      // Grab second operand --> Always a register
	      token = strtok(NULL, spc);
				token++;		// Remove 'R'
				operands[index][1] = (int8_t)atoi(token);
			break;

			case 'l':
        // Grab first operand
        token = strtok(NULL, com);
        token++;      // Remove 'R'
        operands[index][0] = (int8_t)atoi(token);

        // Grab second operand --> Always memory indirect
        token = strtok(NULL, brk);
        token += 3;    // Remove ' [R'
        operands[index][1] = (int8_t)atoi(token);
			break;

			case 's':
        // Grab first operand --> Always memory indirect
        token = strtok(NULL, brk);
        token += 2;    // Remove "[R"
        operands[index][0] = (int8_t)atoi(token);
				
        // Grab second operand --> Always register
        token = strtok(NULL, brk);
        token += 3;    // Remove ", R"
        operands[index][1] = (int8_t)atoi(token);
			break;

			case 'j': case 'J':

				// Grab ONLY operand --> never a register
				token = strtok(NULL, spc);			
				operands[index][0] = (int8_t)atoi(token);			
			break;
		}

		opCodes[index] = op;  // store opCode
	}
    fclose(assembly);
}
