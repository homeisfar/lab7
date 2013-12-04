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
#define INDIRECTION 0x0100	/* Masks the indirection or group bit */
#define MEMMASK 0x00FF		/* Masks page address bits */
#define HIGHEIGHT 0x0FF0	/* Masks 8 high order bits */
#define DEVICE 0x03F8		/* Masks the device for IOT */
#define REGSELECT 0x0C00	/* Masks register selection bits */

#define ZCBIT 0x0100

#define BIT_FOUR 0x080		/* Z/C or CLA */
#define BIT_FIVE 0x040		/* CLL or SMA */
#define BIT_SIX 0x020		/* CMA or SZA */
#define BIT_SEVEN 0x010		/* CML or SNL */

#define BIT_EIGHT 0x08		/* RAR or RSS */
#define BIT_NINE 0x004		/* RAL or OSR */
#define BIT_TEN 0x002		/* 0/1 or HLT */
#define BIT_ELEVEN 0x001	/* IAC or 0 */

/**********************
Registers & Cycle Timer
***********************/

static int REG_A = 0; 		/* Accumulator */
static int REG_B = 0; 		/* Accumulator */
static int REG_C = 0; 		/* Accumulator */
static int REG_D = 0; 		/* Accumulator */
static unsigned REG_L = 0;		/* Link Register */

static unsigned pc = NOPC;	/* program counter */
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
    // printf("pc is %x", pc);

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
	 // int data;
	 // int blocksize;

		/* set all memory to 0 */
	for (i = 0; i < MEMSIZE; i++)
		 memory[i] = 0;

		Load_Binary_Object_File();

	// Token *magic = new_token();
	// get_token(input, magic, 4);

	// if(strstr(magic->string, "OBJG") == NULL){
	// 	fprintf(stderr, "This is not an appropriate PDP429 file.\n");
	// 	exit(1);
	// }
		
	//  get_token(input, magic, 2);

	//  /* shift 8 bits and join the two bytes together to get our value */
	//  pc |= magic->string[0];
	//  pc *= 256;
	//  pc |= magic->string[1];

	//  printf("%x PC\n", pc);

	//  /* There should always be a single byte to tell us the block size */
	//  while(poke(input) != -1){
	//  	get_token(input, magic, 1);
	//  	blocksize = 0;
	//  	blocksize |= magic->string[0];
	//  	i = 0;

	//  	printf("BLOCK SIZE? %x\n", blocksize);
	//  	 after the blocksize we know we're getting a mem addr 
	//  	get_token(input, magic, 2);
	//  	i |= magic->string[0];
	//  	i = i << 8;
	//  	i |= magic->string[1];

	//  	printf("MEM ADDR? %x%x\n", magic->string[0], magic->string[1]);
	//  	// get_token(input, magic, blocksize - 3);

	//  	int counter = 0;
	//  	while(counter < blocksize - 3){
	//  	get_token(input, magic, 2);
	//  	data = 0;
	// 	data |= magic->string[0];
	// 	// printf("DATA INDEX 0 %x\n", data);
	//  	data = data << 8;
	//  	data |= magic->string[1];
	//  	// printf("DATA INDEX 1 %x\n", data);
	//  	counter += 2;
	//  	memory[i] = data;
	//  	i++;

	//  	printf("DATA? %x%x\n", magic->string[0], magic->string[1]);
	//  	}
	 	
	//  }




	//  delete_token(magic);



 	
// 	unsigned int data;
// 	char line[20];
	

	
// 	while(fgets(line, 9, input) != NULL){
// 		/* If entrypoint is read, retrieve its value and assign to pc */
// 		if(line[0] == 'E' && line[1] == 'P' && pc == NOPC){
// 			sscanf(line, "%*s %x", &pc);
// 		}
// 		else if(line[0] == 'E'){
// 			printf("Invalid data. Multiple entry points detected.\n");
// 			exit(1);
// 		}
		
// 		/* Otherwise, put data into temporary variables and assign the data at the memory address */
// 		else{
// 			/* Scan the line for input or bad data */
// 			if(sscanf(line,"%x: %x",&i, &data) == 2){
// 				/* The data is formatted correctly but is off size. */
// 				if(i > 0xfff || data > 0xfff){
// 					printf("Invalid data. Larger than word size. Address: %x Data: %x\n", i, data);
// 					exit(1);
// 				}
// 				/* Assign data to memory */
// 				memory[i] = data;
// 			}
// 			/* Check to see if misread line is just a new line. Ignore it if so */
// 			else if(line[0] != '\n'){
// 					printf("Badly formatted data input. Exitting on %s\n", line);
// 					exit(1);
// 				}
// 		}
// 	}

// if(pc == NOPC){
// 	printf("No entry point discovered. Bad input. Exitting.\n");
// 	exit(1);
// }
	
	return 0;
} 

/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */


/* function to retrieve an effective memory address */
int retrieveMemAddress(int instr, int di, int zc) 
{
	int memAddress = 0x0000;
	/* first determine the page of the memory address we seek */
	/* Get the five high order bits from PC if ZC is set */
	if(zc > 0)
		memAddress |= pc;

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

/* Decode all the microcoded options from the operate instruction */
/* Perform those operations */
int decodeOperate(int instr, char *returnstr)
{
	int group = instr & INDIRECTION;
	 char instr_shorthand[30];
	instr_shorthand[0] = '\0';
	int incrementpc =0;

	// printf("INSTRUCTION IS: %x", instr);

	/***GROUP 0 OPERATIONS***********************************/
	if(!group){
		if(instr & BIT_FOUR){
			strcat(instr_shorthand, "CLA ");
			REG_A = 0;
		}
		if(instr & BIT_FIVE){
			strcat(instr_shorthand, "CLL ");
			REG_L = 0;
		}
		if(instr & BIT_SIX){
			strcat(instr_shorthand, "CMA ");
			REG_A = ~REG_A;
			REG_A &= 0xfff;
		}
		if(instr & BIT_SEVEN){
			strcat(instr_shorthand, "CML ");
			REG_L = !REG_L;
		}
		if(instr & BIT_ELEVEN){
			strcat(instr_shorthand, "IAC ");
			int overflowTrigger1 = 0;
			int overflowTrigger2 = 0;

			if(REG_A <= 2047)
				overflowTrigger1 = 1;
			 
			 REG_A += 1;

			 if(REG_A > 2047)
			 	overflowTrigger2 = 1;

			 if(overflowTrigger1 == 1 && overflowTrigger2 == 1)
			 	REG_L = !REG_L;

			 if(REG_A > 4095){
			 	REG_A -= 4096;
			 	REG_L = !REG_L;
			 }
			
		}


		/* extra variables to determine early on if RAR and RAL have
		illegally been both set. If they've both been set, perform the operation
		but stop running the program. Print out error message. */
		int right = instr & BIT_EIGHT;
		int left = instr & BIT_NINE;
		int two = instr & BIT_TEN;
		two = !two;
		two = !two;
		
		if(right && left){
			run = 0;
			fprintf(stderr, "Illegally used RAL/RAR simultaneously. Exiting program.\n");
		}

		int tempbit = 0;
		if(right){
			if(two)
				strcat(instr_shorthand, "RTR ");
			else
				strcat(instr_shorthand, "RAR ");
			
			do{
				tempbit = REG_A & 0x001;
				REG_A = REG_A >> 1;
				if(REG_L == 1)
					REG_A |= 0x800;

				if(tempbit == 0)
					REG_L = 0;
				else REG_L = 1;
				two--;
			}
			while(two >= 0);
		}

		if(left){
			if(two)
				strcat(instr_shorthand, "RTL ");
			else
				strcat(instr_shorthand, "RAL ");

			do{
				tempbit = REG_A & 0x800;
				REG_A = REG_A << 1;
				REG_A += REG_L;
				if(tempbit == 0)
					REG_L = 0;
				else REG_L = 1;
				two--;
			}
			while(two >= 0);
		}
	}

	/***GROUP 1 OPERATIONS***********************************/
	else{
		int skipzero = 0;
		int skiplink = 0;
		int skipneg = 0;
/*Reverse Skip Sense. If this bit is one, the SMA, SZA, and SNL subinstructions will skip on the opposite condition. 
That is, SMA skips on positive or zero, SZA skips on nonzero, and SNL skips if the Link is zero.*/

		if(instr & BIT_FIVE){
			strcat(instr_shorthand, "SMA ");
			skipneg = 1;
		}

		if(instr & BIT_SIX){
			strcat(instr_shorthand, "SZA ");
			skipzero = 1;
		}

		if(instr & BIT_SEVEN){
			strcat(instr_shorthand, "SNL ");
			skiplink = 1;
		}
		
		if(instr & BIT_EIGHT){
			strcat(instr_shorthand, "RSS ");
			if(((REG_A < 2048) && skipneg) || (REG_A != 0 && skipzero) || (REG_L == 0 && skiplink))
				incrementpc++;
		}
		else
			if(((REG_A > 2047) && skipneg) || (REG_A == 0 && skipzero) || (REG_L == 1 && skiplink))
				incrementpc++;

		if(instr & BIT_FOUR){
			strcat(instr_shorthand, "CLA ");
			REG_A = 0;
		}

		if(instr & BIT_NINE)
			strcat(instr_shorthand, "OSR ");

		if(instr & BIT_TEN){
			strcat(instr_shorthand, "HLT ");
			run = 0;
			printVerbose(instr_shorthand);
			memoryDump();
			exit(0);
		}
	}

	strcpy(returnstr, instr_shorthand);
	return incrementpc;
}


/* ***************************************************************** */
/*                                                                   */
/*                                                                   */
/* ***************************************************************** */

int interpret() /* Runs the interpreter */
{
	int opcode = 0;			/* the value of the opcode */
	int instruction = 0;	/* the  */
	int addr = 0;			/* the effective address to operate on */
	int di = 0;				/* direction bit */
	int zc = 0;				/* page bit */
	int device = 0;			/* Which device has been selected */
	int increment_pc_reminder=0;	/* a number to increment the PC after certain operations */
	run = 2020; /* arbitrary positive value */
	char instr_shorthand[30];	/* A string to output the names of the instructions for verbose */

	while(run){
		di = 0;
		instr_shorthand[0] = '\0';
		instruction = memory[pc];
		opcode = parseOpcode(instruction);			
		int jumped = 0;
		increment_pc_reminder = 0; 
		
		/*If memory referencing operation, get d/i z/c */
		if((opcode > 1 && opcode < 10) || opcode == 11 || opcode == 12){
			di = instruction & INDIRECTION;
			zc = instruction & ZCBIT;
			addr = retrieveMemAddress(instruction, di, zc);

			/* if indirection, add one cycle for additional memory access */
			 if(di)
			 	 cycles++;
		}

		// if(opcode == 5 || opcode == 6 || opcode == 7)
		 	// cycles--;
		 // cycles += 2;
		
		/* Determine the operation to perform */
		switch(opcode){

			case 0: /* AND */
			strcat(instr_shorthand, "AND ");
			 REG_A &= memory[addr];
			break;
	/***********************************************************************/
			case 1: /* TAD */
			strcat(instr_shorthand, "TAD ");
			 REG_A += memory[addr];
			/*If we exceed the word size, set register back down */			 
			 if(REG_A > 4095){
			 	REG_A -= 4096;
			 	REG_L = !REG_L;
			 }
			break;
	/***********************************************************************/
			case 2: /* ISZ */
			strcat(instr_shorthand, "ISZ ");
			memory[addr] += 1;
			
			if(memory[addr] > 4095)
				memory[addr] = 0;

			 if(memory[addr] == 0)
			 	increment_pc_reminder++;
			break;
	/***********************************************************************/
			case 3: /* DCA */
			strcat(instr_shorthand, "DCA ");
			memory[addr] = REG_A;
			REG_A = 0;
			break;
	/***********************************************************************/
			case 4: /* JMS  */
			strcat(instr_shorthand, "JMS ");
			if(di)
				strcat(instr_shorthand, "I ");
			printVerbose(instr_shorthand);
//The address of the next location (program counter plus one) is stored at the effective address and 
			//the program counter is set to the effective address plus one.
			memory[addr] = pc+1;
			jumped++;
			pc = addr+1;
			
			break;
	/***********************************************************************/
			case 5: /* JMP */
			strcat(instr_shorthand, "JMP ");
			if(di)
				strcat(instr_shorthand, "I ");
			printVerbose(instr_shorthand);
			jumped++;
			 pc = addr;
			break;
	/***********************************************************************/
			case 6: /* IOT */
			strcat(instr_shorthand, "IOT ");
			device = memory[pc] & DEVICE;
			device = device >> 3;
			if(device == 3){
				REG_A = getc(stdin);
				REG_A &= 0xfff;
				strcat(instr_shorthand, "3 ");
			}
			else if(device == 4){
				putchar(REG_A & 0xff);
				strcat(instr_shorthand, "4 ");
			}
			/* If IOT to any device other than 3/4, stop running. */
			else run = 0;
			break;
	/***********************************************************************/
			case 7: /* operate */
			increment_pc_reminder = decodeOperate(memory[pc], instr_shorthand);
			break;
			
		} /* End switch case */
			
			if(!jumped){
				if(di)
				strcat(instr_shorthand, "I ");
			printVerbose(instr_shorthand);
			pc++;
			if(pc > 4095)
				pc = 0;
			pc+=increment_pc_reminder;
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
	printf("pc: %d\n", pc);
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
	 if(verbose)
	fprintf(stderr, "Time %lld: PC=0x%03X instruction = 0x%03X (%s), rA = 0x%03X, rL = %d\n", cycles, pc, memory[pc], s, REG_A, REG_L);
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
                            // interpret();
                        }
                }
        }
    
		memoryDump();

	exit(1);
}
