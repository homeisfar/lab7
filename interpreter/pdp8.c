/*
Ali Homafar
ah26482
Section: 58345

PDP429 - extended PDP8 interpreter
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "objinput.h"

void printVerbose();
int memoryDump();
/**********************
   Machine constants
***********************/

#define MEMSIZE 65536
#define NOPC 0x0000

/**********************
   Masks
***********************/
#define OPCODE_MASK 0xF000	/* Mask the opcode */
#define INDIRECTION 0x0200	/* Masks the indirection or group bit */
#define MEMMASK 0x00FF		/* Masks page address bits */
#define HIGHEIGHT 0xFF00	/* Masks 8 high order bits */
#define DEVICE 0x03F8		/* Masks the device for IOT */
#define REGSELECT 0x0C00	/* Masks register selection bits */
#define FULLBITS 0xFFFF 	/* Masks all the bits in a word */
#define REGTOREGSELECT 0x0E00	/* masks the subopcode for register-register ops */
#define REG_I 0x01C0		/* Masks all the bits for R-R i */
#define REG_J 0x0038		/* Masks all the bits for R-R j */
#define REG_K 0x0007		/* Masks all the bits for R-R k */
#define R_R_SUBCODE 0x0E00	/* Masks subopcode for R-R */

#define ZCBIT 0x0100

#define BIT_FOUR 0x080		/* Z/C or CLA */
#define BIT_FIVE 0x040		/* CLL or SMA */
#define BIT_SIX 0x020		/* CMA or SZA */
#define BIT_SEVEN 0x010		/* CML or SNL */

#define BIT_EIGHT 0x08		/* RAR or RSS */
#define BIT_NINE 0x004		/* RAL or OSR */
#define BIT_TEN 0x002		/* 0/1 or HLT */
#define BIT_ELEVEN 0x001	/* IAC or 0 */

#define BITSM 0x0200
#define BITSZ 0x0100
#define BITSNL 0x0080
#define BITRSS 0x0040
#define BITCL 0x0020
#define BITCLL 0x0010
#define BITCM 0x0008
#define BITCML 0x0004
#define BITDC 0x0002
#define BITIC 0x0001

/**********************
Registers & Cycle Timer
***********************/

static int REG[8]; 		/* Accumulator */
char instr_longhand[200];
/*
A 	0
B 	1
C 	2
D 	3
PC 	4
PSW 	5
SP 	6
SPL 7	
*/
static int REG_A = 0; 		/* Accumulator */
// static int REG_C = 0; 		/* Accumulator */
// static int REG_D = 0; 		/* Accumulator */
static unsigned REG_L = 0;		/* Link Register */

static unsigned SP = 0;		/* stack pointer */
static unsigned SPL = 0;	/* stack pointer limit */
static unsigned proc_word_status = 0;

static long long cycles = 0;	/* cycles */
static int memory[MEMSIZE];	/* memory */

/**********************
***********************/
FILE *input;
int run;				/* Whether interpreter is running or not */
int verbose;			/* Whether to output in verbose mode */
int dump; 				/* Whether to dump simulated memory */

/***********************
 *					   *
 ***********************/

int parseOpcode(int opcode) /* Take an instruction. Returns 0-15 */
{
	opcode = opcode & OPCODE_MASK;
	opcode = opcode >> 12;
	return opcode;
}

/***********************
 *					   *
 ***********************/


 int get2(void)
{
    int c1 = getc(input);
    int c2 = getc(input);
    // if (debug) fprintf(stderr, "read two bytes: 0x%X, 0x%X\n", c1, c2);
    if ((c1 == EOF) || (c2 == EOF))
        {
            fprintf(stderr, "Premature EOF\n");
            exit(1);
        }
    if (c1 & (~0xff)) fprintf(stderr, "Extra high order bits for 0x%X\n", c1);
    if (c2 & (~0xff)) fprintf(stderr, "Extra high order bits for 0x%X\n", c2);
    int n = ((c1 & 0xff) << 8) | (c2 & 0xff);
    return(n);
}


 int Load_Binary_Object_File()
{
    int c1 = getc(input);
    int c2 = getc(input);
    int c3 = getc(input);
    int c4 = getc(input);
    // if (debug) fprintf(stderr, "read four bytes: 0x%X, 0x%X, 0x%X, 0x%X\n", c1, c2, c3, c4);

    if ((c1 != 'O') || (c2 != 'B') || (c3 != 'J') || (c4 != 'G'))
        {
            fprintf(stdout, "First four bytes are not OBJG: ");
            // fprintf(stdout, "%s", printrep(c1));
            // fprintf(stdout, "%s", printrep(c2));
            // fprintf(stdout, "%s", printrep(c3));
            // fprintf(stdout, "%s", printrep(c4));
            fprintf(stdout, " (%02X %02X %02X %02X)\n", c1, c2, c3, c4);

            exit(1);
        }

    int pc = get2();
    // printf("REG[4] is %x", REG[4]);

    int n;
    while ((n = getc(input)) != EOF)
        {
            // if (debug) fprintf(stderr, "Read next block of %d bytes\n", n);
            n = n - 1;
            int addr = get2(); n -= 2;
            while (n > 0)
                {
                    int data = get2(); n -= 2;  
                    // printf("DATA IS: %x\n", data);          
                    memory[addr] = data;
                    addr += 1;
                }
        }

    return(pc);
}


int initializeMemory()
{
	 int i;
		/* set all memory to 0 */
	for (i = 0; i < MEMSIZE; i++)
		 memory[i] = 0;

		REG[4] = Load_Binary_Object_File();
	return 0;
} 

/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */



void buildout(int reg, int instruction, int addr)
{
	if(instruction == 8){
	strcat(instr_longhand, "M[%04X");	
	}
	
}

/* function to retrieve an effective memory address */
int retrieveMemAddress(int instr, int di, int zc) 
{
	int memAddress = 0x0000;
	/* first determine the page of the memory address we seek */
	/* Get the five high order bits from PC if ZC is set */
	if(zc > 0)
		memAddress |= REG[4];

	memAddress &= HIGHEIGHT; /* clear out the low 8 bits */
	instr &= MEMMASK; /* retrieve the low 8 bits from our instruction */
	memAddress |= instr; /* OR the high eight bits to the low eight bits */

	if(di == 0)
		return memAddress;
	else
		return memory[memAddress];
}

/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

void push(int instr)
{
	memory[REG[6]] = instr;
	REG[6]--;
	if(REG[6] < REG[7])
		// exit(1);
		printf("OVERFLOW\n");
}

void pop(int instr)
{

	REG[6]++;
	if(REG[6] == 0)
		// exit(1);
		printf("OVERFLOW\n");
	memory[instr] = memory[REG[6]];
}

void retur()
{
	REG[6]++;
	if(REG[6] == 0)
		// exit(1);
		printf("OVERFLOW\n");
	REG[4] = memory[REG[6]];
}

int decodeSpecial(int instr, char *returnstr)
{
	int did_return = 0;
	char instr_shorthand[40];
	instr_shorthand[0] = '\0';
	if(instr == 0x0)
	{
		strcat(instr_shorthand, "NOP ");
	}

	if(instr == 0x01)
	{
		strcat(instr_shorthand, "HLT ");
		proc_word_status = proc_word_status & 0xfffe;
		run = 0;
		printVerbose(instr_shorthand);
		memoryDump();
		exit(0);
	}

	if(instr == 0x02)
	{
		strcat(instr_shorthand, "RET ");
		printVerbose(instr_shorthand);
		did_return++;
		retur();
	}
	strcpy(returnstr, instr_shorthand);
	return did_return;
}

int decodeRegister(int instr, char *returnstr)
{
	int reg_selected = instr & REGSELECT;
	reg_selected = reg_selected >> 10;
	char instr_shorthand[40];
	instr_shorthand[0] = '\0';

	if(reg_selected == 0)
	{
		strcat(instr_shorthand, "A ");
	}
	else if(reg_selected == 1)
	{
		strcat(instr_shorthand, "B ");
	}
	else if(reg_selected == 2)
	{
		strcat(instr_shorthand, "C ");
	}
	else if(reg_selected == 3)
	{
		strcat(instr_shorthand, "D ");
	}

	strcat(returnstr, instr_shorthand);
	return reg_selected;
}

/* used for when opcode == 14 -> register to register */
int decodeRegister_Register(int instr, char *returnstr)
{
	int reg_selected = instr & REGTOREGSELECT;
	reg_selected = reg_selected >> 9;
	char instr_shorthand[40];
	instr_shorthand[0] = '\0';

	if(reg_selected == 0)
	{
		strcat(instr_shorthand, "MOD ");
	}
	else if(reg_selected == 1)
	{
		strcat(instr_shorthand, "ADD ");
	}
	else if(reg_selected == 2)
	{
		strcat(instr_shorthand, "SUB ");
	}
	else if(reg_selected == 3)
	{
		strcat(instr_shorthand, "MUL ");
	}
	else if(reg_selected == 4)
	{
		strcat(instr_shorthand, "DIV ");
	}
	else if(reg_selected == 5)
	{
		strcat(instr_shorthand, "AND ");
	}
	else if(reg_selected == 6)
	{
		strcat(instr_shorthand, "OR ");
		 // printf("I GOT INTO SUBLAND SOMEHOW?\n\n");
	}
	else if(reg_selected == 7)
	{
		strcat(instr_shorthand, "XOR ");
	}

	strcpy(returnstr, instr_shorthand);
	return reg_selected;
}

int select_subop(int instr, int select)
{

	int reg_selected = 0;

	if(select == 0){
	reg_selected = instr & REG_I;
	reg_selected = reg_selected >> 6;
	}
	if(select == 1){
	reg_selected = instr & REG_J;
	reg_selected = reg_selected >> 3;
	}
	if(select == 2){
	reg_selected = instr & REG_K;
	}

	return reg_selected;
}


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */


/* Decode all the microcoded options from the operate instruction */
/* Perform those operations */
int decodeOperate(int instr, char *returnstr)
{
	// int group = instr & INDIRECTION;

	 char instr_shorthand[40];
	instr_shorthand[0] = '\0';
	int incrementpc =0;
	int regSelect = decodeRegister(instr, instr_shorthand);

	// printf("INSTRUCTION IS: %x", instr);


/* 
#define BITSM 0x0200
#define BITSZ 0x0100
#define BITSNL 0x0080
#define BITRSS 0x0040
#define BITCL 0x0020
#define BITCLL 0x0010
#define BITCM 0x0008
#define BITCML 0x0004
#define BITDC 0x0002
#define BITIC 0x0001
*/


		int skipzero = 0;
		int skiplink = 0;
		int skipneg = 0;
/*Reverse Skip Sense. If this bit is one, the SMA, SZA, and SNL subinstructions will skip on the opposite condition. 
That is, SMA skips on positive or zero, SZA skips on nonzero, and SNL skips if the Link is zero.*/

		if(instr & BITSM){
			strcat(instr_shorthand, "SMA ");
			skipneg = 1;
		}

		if(instr & BITSZ){
			strcat(instr_shorthand, "SZA ");
			skipzero = 1;
		}

		if(instr & BITSNL){
			strcat(instr_shorthand, "SNL ");
			skiplink = 1;
		}
		
		if(instr & BITRSS){
			strcat(instr_shorthand, "RSS ");
			if(((REG_A < 32768) && skipneg) || (REG_A != 0 && skipzero) || (REG_L == 0 && skiplink))
				incrementpc++;
		}
		else
			if(((REG_A > 32767) && skipneg) || (REG_A == 0 && skipzero) || (REG_L == 1 && skiplink))
				incrementpc++;

		if(instr & BITCL){
			strcat(instr_shorthand, "CLA ");
			REG[regSelect] = 0;
		}
		if(instr & BITCLL){
			strcat(instr_shorthand, "CLL ");
			REG_L = 0;
		}
		if(instr & BITCM){
			strcat(instr_shorthand, "CMA ");
			REG[regSelect] = ~REG[regSelect];
			REG[regSelect] &= 0xffff;
		}

		if(instr & BITCML){
			strcat(instr_shorthand, "CML ");
			REG_L = !REG_L;
		}

		if(instr & BITDC){
			strcat(instr_shorthand, "DC ");
			int overflowTrigger1 = 0;
			int overflowTrigger2 = 0;

			if(REG[regSelect] >= 32768)
				overflowTrigger1 = 1;
			 
			 REG[regSelect] -= 1;

			 	/* the value is now positive */
			 if(REG[regSelect] < 32768)
			 	overflowTrigger2 = 1;

			 if(overflowTrigger1 == 1 && overflowTrigger2 == 1)
			 	REG_L = !REG_L;

			 if(REG[regSelect] < 0){
			 	REG[regSelect] += 65536;
			 	REG_L = !REG_L;
			 }
		}

		if(instr & BITIC){
			strcat(instr_shorthand, "IC ");
			int overflowTrigger1 = 0;
			int overflowTrigger2 = 0;

			if(REG[regSelect] <= 32767)
				overflowTrigger1 = 1;
			 
			 REG[regSelect] += 1;

			 if(REG[regSelect] > 32767)
			 	overflowTrigger2 = 1;

			 if(overflowTrigger1 == 1 && overflowTrigger2 == 1)
			 	REG_L = !REG_L;

			 if(REG[regSelect] > 65535){
			 	REG[regSelect] -= 65536;
			 	REG_L = !REG_L;
			 }
			}




	strcpy(returnstr, instr_shorthand);
	return incrementpc;

}


void val_to_string(int value)
{
	char str[20];
	sprintf(str, "0x%04X", value);
	strcat(instr_longhand, str);
}

void reg_to_string(int value, int valueinvalue, int rev)
{
	char str[20];

if(rev == 0){
	if(value == 0)
		strcat(instr_longhand, "A");
	if(value == 1)
		strcat(instr_longhand, "B");
	if(value == 2)
		strcat(instr_longhand, "C");
	if(value == 3)
		strcat(instr_longhand, "D");
	if(value == 4)
		strcat(instr_longhand, "PC");
	if(value == 5)
		strcat(instr_longhand, "PSW");
	if(value == 6)
		strcat(instr_longhand, "SP");
	if(value == 7)
		strcat(instr_longhand, "SPL");
	if(value == 8)
		strcat(instr_longhand, "L");
	if(value == 9){
		strcat(instr_longhand, "M[");
		val_to_string(valueinvalue);
		strcat(instr_longhand, "]");
	}

	if(value < 8){
	strcat(instr_longhand, " -> ");
	val_to_string(valueinvalue);	
	}

	if(value == 9){
		strcat(instr_longhand, " -> ");
		val_to_string(memory[valueinvalue]);
	}
}

if(rev == 1){
	if(value < 8){
	val_to_string(valueinvalue);
	strcat(instr_longhand, " -> ");
	}

	if(value == 0)
		strcat(instr_longhand, "A");
	if(value == 1)
		strcat(instr_longhand, "B");
	if(value == 2)
		strcat(instr_longhand, "C");
	if(value == 3)
		strcat(instr_longhand, "D");
	if(value == 4)
		strcat(instr_longhand, "PC");
	if(value == 5)
		strcat(instr_longhand, "PSW");
	if(value == 6)
		strcat(instr_longhand, "SP");
	if(value == 7)
		strcat(instr_longhand, "SPL");
	if(value == 8)
		strcat(instr_longhand, "L");

}
	
}

/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

int interpret() /* Runs the interpreter */
{
	int opcode = 0;				/* the value of the opcode */
	int instruction = 0;		/* the  */
	int addr = 0;				/* the effective address to operate on */
	int di = 0;					/* direction bit */
	int zc = 0;					/* page bit */
	int device = 0;				/* Which device has been selected */
	int increment_pc_reminder = 0;	/* a number to increment the PC after certain operations */
	run = 100;				 	/* arbitrary positive value */
	char instr_shorthand[40];	/* A string to output the names of the instructions for verbose */
	int subopcode = 0;
	int subreg1 = 0;
	int subreg2 = 0;
	int subreg3 = 0;

	while(run--){
		di = 0;
		instr_shorthand[0] = '\0';
		instr_longhand[0] = '\0';
		instruction = memory[REG[4]];
		opcode = parseOpcode(instruction);			
		int jumped = 0;
		int regSelect = 0;
		increment_pc_reminder = 0; 
		
		/*If memory referencing operation, get d/i z/c */
		if((opcode > 1 && opcode < 10) || opcode == 11 || opcode == 12){
			di = instruction & INDIRECTION;
			zc = instruction & ZCBIT;
			addr = retrieveMemAddress(instruction, di, zc);

			/* if indirection, add one cycle for additional memory access */
			 // if(di)
			 // 	 cycles++;
		}

		printf("PC IS: %x. EXECUTING: %x\n", REG[4], memory[REG[4]]);

		/* Determine the operation to perform */
		switch(opcode){

			case 0: /* Special Instructions */

			jumped+= decodeSpecial(memory[REG[4]], instr_shorthand);
				// printVerbose(instr_shorthand);

			// strcat(instr_shorthand, "AND ");
			//  REG_A &= memory[addr];
			break;
	/***********************************************************************/
			case 1: /* ADD */
			strcat(instr_shorthand, "ADD");

			regSelect = decodeRegister(memory[REG[4]], instr_shorthand);
			
			// reg_to_string(regSelect);
			// strcat(instr_longhand, " -> ");
			// val_to_string(REG[regSelect]);
			// strcat(instr_longhand, ", M[");
			// val_to_string(addr);
			// strcat(instr_longhand, "]");
			// strcat(instr_longhand, " -> ");
			// val_to_string(memory[addr]);
			// strcat(instr_longhand, ", ");
			
			REG[regSelect] += memory[addr];
			if(REG[regSelect] > 65535){
				REG[regSelect] -= 65536;
				// val_to_string(!REG_L);
				// strcat(instr_longhand, " -> L, ");

				REG_L = !REG_L;
			}
			// val_to_string(REG[regSelect]);
			// strcat(instr_longhand, " -> ");
			// reg_to_string(regSelect);

			break;
	/***********************************************************************/
			case 2: /* SUB */
			strcat(instr_shorthand, "SUB");
			regSelect = decodeRegister(memory[REG[4]], instr_shorthand);

			REG[regSelect] = REG[regSelect] - memory[addr];
			
			if(REG[regSelect] < 0)
			{
				REG[regSelect] += 0xffff;
				REG_L = !REG_L;
			}
			break;
	/***********************************************************************/
			case 3: /* MUL */
			strcat(instr_shorthand, "MUL");
			regSelect = decodeRegister(memory[REG[4]], instr_shorthand);

			REG[regSelect] *= memory[addr];

			if(REG[regSelect] > 65535){
				REG[regSelect] = REG[regSelect] & FULLBITS;
				REG_L = !REG_L;
			}
			break;
	/***********************************************************************/
			case 4: /* DIV  */
			strcat(instr_shorthand, "DIV");
			
			regSelect = decodeRegister(memory[REG[4]], instr_shorthand);
			if(memory[addr] == 0)
				REG[regSelect] = 0;
			else
			REG[regSelect] /= memory[addr];


			break;
	/***********************************************************************/
			case 5: /* AND */
			strcat(instr_shorthand, "AND");
			regSelect = decodeRegister(memory[REG[4]], instr_shorthand);

			// reg_to_string(regSelect);
			// strcat(instr_longhand, " -> ");
			// val_to_string(REG[regSelect]);
			// strcat(instr_longhand, ", M[");
			// val_to_string(addr);
			// strcat(instr_longhand, "]");
			// strcat(instr_longhand, " -> ");
			// val_to_string(memory[addr]);
			// strcat(instr_longhand, ", ");
			

			REG[regSelect] = REG[regSelect] & memory[addr];
			break;
	/***********************************************************************/
			case 6: /* OR */
			strcat(instr_shorthand, "OR");
			regSelect = decodeRegister(memory[REG[4]], instr_shorthand);
			REG[regSelect] = REG[regSelect] | memory[addr];
			break;
	/***********************************************************************/
			case 7: /* XOR */
			strcat(instr_shorthand, "XOR");
			regSelect = decodeRegister(memory[REG[4]], instr_shorthand);
			REG[regSelect] = REG[regSelect] ^ memory[addr];
			break;
	/***********************************************************************/
	/***********************************************************************/
			case 8: /* LD */
			strcat(instr_shorthand, "LD");
			regSelect = decodeRegister(memory[REG[4]], instr_shorthand);

			reg_to_string(9, addr, 0);
			
			REG[regSelect] = memory[addr];

			strcat(instr_longhand, ", ");
			val_to_string(memory[addr]);
			strcat(instr_longhand, " -> ");
			reg_to_string(regSelect, addr, 1);
			

			// printf("REG SELECTED !!! %x\n", regSelect);
			// printf("ADDR IS:%x MEMORY AT THAT ADDR IS%x\n\n\n",addr, memory[addr]);
			
			break;
	/***********************************************************************/
	/***********************************************************************/
			case 9: /* ST */
			strcat(instr_shorthand, "ST");
			regSelect = decodeRegister(memory[REG[4]], instr_shorthand);
			memory[addr] = REG[regSelect];
			break;
	/***********************************************************************/
	/***********************************************************************/
			case 10: /* IOT */
			regSelect = decodeRegister(memory[REG[4]], instr_shorthand);
			/* strcpy to clear out the register selection string */
			strcpy(instr_shorthand, "IOT ");
			device = memory[REG[4]] & DEVICE;
			device = device >> 3;
			if(device == 3){
				REG[regSelect] = getc(stdin);
				REG[regSelect] &= FULLBITS;
				strcat(instr_shorthand, "3 ");
			}
			else if(device == 4){
				putchar(REG[regSelect] & 0xff);
				strcat(instr_shorthand, "4 ");
			}
			/* If IOT to any device other than 3/4, stop running. */
			else run = 0;
			break;
	/***********************************************************************/
	/***********************************************************************/
			case 11: /* ISZ JMP or CALL 1100*/
			regSelect = decodeRegister(memory[REG[4]], instr_shorthand);

			if(regSelect == 0){
				strcpy(instr_shorthand, "ISZ ");
			memory[addr] += 1;
			
			if(memory[addr] > 65535)
				memory[addr] = 0;

			 if(memory[addr] == 0)
			 	increment_pc_reminder++;
			}

			if(regSelect == 1){
				strcpy(instr_shorthand, "JMP ");
			if(di)
				strcat(instr_shorthand, "I ");
			printVerbose(instr_shorthand);
			jumped++;
			 REG[4] = addr;
			}

			if(regSelect == 2){

				// printf("ZC IS: : :: :  : %x  ", addr);
				strcpy(instr_shorthand, "CALL ");
					if(di)
				strcat(instr_shorthand, "I ");
				push(REG[4] + 1);
				printVerbose(instr_shorthand);
				jumped++;
				REG[4] = addr;
			}

			break;
	/***********************************************************************/
	/***********************************************************************/
			case 12: /* PUSH / POP */
			regSelect = decodeRegister(memory[REG[4]], instr_shorthand);

			if(regSelect == 0){
				strcpy(instr_shorthand, "PUSH ");
				push(memory[REG[4]]);
			}
			if(regSelect == 1)
				strcpy(instr_shorthand, "POP ");

			pop(addr);

			break;
	/***********************************************************************/
	/***********************************************************************/
			case 13: /* NOTHING HAPPENS HERE */
			break;
	/***********************************************************************/
	/***********************************************************************/
			case 14: /* REGISTER TO REGISTER 1110 */
			subopcode = decodeRegister_Register(memory[REG[4]], instr_shorthand);
			subreg1 = select_subop(memory[REG[4]], 0);
			subreg2 = select_subop(memory[REG[4]], 1);
			subreg3 = select_subop(memory[REG[4]], 2);

/*1110.000	 MOD	 Reg[j] % Reg[k] -> Reg[i]
1110.001	 ADD	 Reg[j] + Reg[k] -> Reg[i]
1110.010	 SUB	 Reg[j] - Reg[k] -> Reg[i]
1110.011	 MUL	 Reg[j] * Reg[k] -> Reg[i]
1110.100	 DIV	 Reg[j] / Reg[k] -> Reg[i]
1110.101	 AND	 Reg[j] & Reg[k] -> Reg[i]
1110.110	 OR	 Reg[j] | Reg[k] -> Reg[i]
1110.111	 XOR	 Reg[j] ^ Reg[k] -> Reg[i]*/

			if(subopcode == 0){
				if(REG[subreg3] == 0)
					REG[subreg1] = 0;
				else
					REG[subreg1] = REG[subreg2] % REG[subreg3];
			}
			if(subopcode == 1){
				REG[subreg1] = REG[subreg2] + REG[subreg3];
			}
			if(subopcode == 2){
				REG[subreg1] = REG[subreg2] - REG[subreg3];
			}
			if(subopcode == 3){
				REG[subreg1] = REG[subreg2] * REG[subreg3];
				REG[subreg1] &= FULLBITS;
			}
			if(subopcode == 4){
				if(REG[subreg3] == 0)
					REG[subreg1] = 0;
				else
				REG[subreg1] = REG[subreg2] / REG[subreg3];
			}
			if(subopcode == 5){
				REG[subreg1] = REG[subreg2] & REG[subreg3];

			// reg_to_string(regSelect);
			// strcat(instr_longhand, " -> ");
			// val_to_string(REG[regSelect]);
			// strcat(instr_longhand, ", M[");
			// val_to_string(addr);
			// strcat(instr_longhand, "]");
			// strcat(instr_longhand, " -> ");
			// val_to_string(memory[addr]);
			// strcat(instr_longhand, ", ");
			



			}
			if(subopcode == 6){
				// printf("THIS%x THAT%x AND THE OTHER%x\n",subreg1, REG[subreg2], subreg3);
				REG[subreg1] = REG[subreg2] | REG[subreg3];
			}
			if(subopcode == 7){
				REG[subreg1] = REG[subreg2] ^ REG[subreg3];
			}

			if(REG[subreg1] > 65535){
				REG[subreg1] -= 65536;
				REG_L = ~REG_L;
			}

			if(REG[subreg1] < 0){
				REG[subreg2] += 65536;
				REG_L = ~REG_L;
			}
			break;
	/***********************************************************************/
	/***********************************************************************/
			case 15: /* operate */
			strcat(instr_shorthand, "OPERATE ");

			increment_pc_reminder = decodeOperate(memory[REG[4]], instr_shorthand);
			// regSelect = decodeRegister(memory[REG[4]], instr_shorthand);
			// REG[regSelect] = REG[regSelect] ^ memory[addr];
			break;
	/***********************************************************************/
			
		} /* End switch case */
			
			cycles++;
			if(!jumped){
				if(di)
				strcat(instr_shorthand, "I ");
			printVerbose(instr_shorthand);
			REG[4]++;
			if(REG[4] > 65535)
				REG[4] = 0;
			REG[4]+=increment_pc_reminder;
		}

	}
	return 0;
}

/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

int memoryDump()
{
	if(dump){
	int i;
	printf("Exit data.\nRegisters:\n");
	printf("A: %d\n", REG_A);
	printf("L: %d\n", REG_L);
	printf("REG[4]: %d\n", REG[4]);
	printf("Time: %lld\n\n", cycles);
	printf("%s\n", "Full contents of memory in hex:");

	for (i = 0; i < MEMSIZE; i++)
		printf("%04x %04x\n", i, memory[i]);
	}
	return 0;
}


/***********************
 *					   *
 ***********************/

void scanargs(char *s)
{
    /* check each character of the option list for
       its meaning. */

    while (*++s != '\0')
        switch (*s)
            {

            case 'v': /* verbose option */
                verbose = 1;
                break;

            case 'd': /* dump memory */
                dump = 1;
                break;
            default:
                fprintf (stderr,"PROGR: Bad option %c\n", *s);
                fprintf (stderr,"usage: PROGR [-v] file\n");
                exit(1);
            }
}

void printVerbose(char *s)
{
	 int a = strlen(s);
	 s[a-1] = '\0';
	 if(verbose){
	// fprintf(stderr, "Time %lld: PC=0x%03X instruction = 0x%03X (%s), rA = 0x%03X, rL = %d\n", cycles, REG[4], memory[REG[4]], s, REG_A, REG_L);
            fprintf(stderr, "Time %3lld: PC=0x%04X instruction = 0x%04X (%s)",
                    cycles, REG[4], memory[REG[4]], s);
            // char *regs = ...;
            // if (regs != NULL)
                fprintf(stderr, ": %s", instr_longhand);
            fprintf(stderr, "\n");
        }
}

int main(int argc, char **argv)
{ 

    while (argc > 1)
        {
            argc--, argv++;
            if (**argv == '-')
                scanargs(*argv);
            else
                {
                    input = fopen(*argv,"r");

                    if(dump)
                    printf("Opening: %s\n", *argv);
                    
                    if (input == NULL)
                        {
                            fprintf (stderr, "Can't open %s. Exit program.\n",*argv);
                            exit(1);
                        }
                    else
                        {
                        	initializeMemory();
                            fclose(input);
                             interpret();
                        }
                }
        }
    
		memoryDump();

	exit(1);
}
