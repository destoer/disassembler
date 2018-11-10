
// <--- research into mips pipline hazards and scanning for funciton prologues
// and other common coding patterns

// at a -rt option to command args to tell it to ignore unknown opcodes 
// so things can be tested


// experiment more with compiling mips from C to understand how its converted to asm
// will stop leading to assumptions about the code

// add function for data marks based on registers inlined and bundle them into a parsesub function

// traversal algo for the jump statements is fine the data operaitosn are the cause

// design the jump statements first and get them working well
// funciton of the emulation routines early on

#include "mips.h"
#include "stack.h"
#include "registers.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
char *data; // for marking code
            // nearly every seciton of code works on it so allow it to be global 
unsigned int registers[32] = {0}; //array for holding register values 
extern bool ioplist;
void mkc(unsigned int address,int num,int length) // mark code at current pc (replace when we use array loop for jump tables)
{

	if(address < length)
	{
		data[address] = num;
		data[address+1] = num;
		data[address+2] = num;
		data[address+3] = num;	
	}	
	else { printf("error attempted to mark out of range address %x %x %x\n", address,ProgramCounter,op.opcodefull); }
}


char *traverse(unsigned char *buffer, Config *config)
{
	// open up a file for writing out the db file
    char *str1 = malloc( strlen(config->filename) + 100 );
	char *str2 = malloc( strlen(config->filename) + 100);
	strcpy(str1,config->filename);

	char *lastdot = strrchr(str1, '.');
	if(lastdot != NULL)
	{
		lastdot[1] = 'd';
		lastdot[2] = 'r';
		lastdot[3] = '\0';
	} 

	strcpy(str2,str1);

	lastdot = strrchr(str2, '.');
	if(lastdot != NULL)
	{
		lastdot[1] = 'd';
		lastdot[2] = 'd';
		lastdot[3] = '\0';
	}
	
	lastdot = NULL;

	FILE *outx = fopen(str1, "wb+"); // write refs
	FILE *outd = fopen(str2, "wb+"); // write data marks
	if(outx == NULL || outd == NULL)
	{	
		printf("Failed to open a output file\n");
		exit(1); // exit with error if file not found
	} 	
	
    	// we no longer need our strings to free them
   	free(str1); str1 = NULL;
	free(str2); str2 = NULL;
	
	
	Xref xref;
	struct StackNode* root = NULL; // declare a local stack for saving pcs
	unsigned int nref = 0; // number of references in the code
	unsigned int calc; // used to calc the address of la
	unsigned int pc; // store the program calc
	unsigned int ent; // number of table entries
	unsigned int address; // address of table start
	unsigned int jumpa; // var for holding jump table addresses
	unsigned int jumpfinal;	// calculate effective jump addresses

	Opcode op3; 
	printf("marking code and data...\n");

	// 0 data, 1 instruction, 3 jump, 5 jump table
	data = calloc(sizeof(char), config->len + 4); // allocate and zero out array of file length
	unsigned char *passed =  calloc(sizeof(char), config->len + 4); // check if a instruction has been seen or not



//------------------------debug---------------------------------\\

    // file for unknown opcodes (remove after for debugging)
    FILE *fp = fopen("oplist.dat","a+");


    // read the contents into a buffer for searching
    fseek(fp, 0, SEEK_END); // get file size and set it to len
    int listlen = ftell(fp); // get length of file
    rewind(fp); // go to start of file
    bool found = false;
    unsigned int *oplist = malloc(sizeof(unsigned int) * listlen);
    fread(oplist, sizeof(int), listlen, fp); // load file into array buffer

    fseek(fp,0,SEEK_END); // goto the end so it can appended

	for(int i = 0; i < config->len;)
	{

        // while we have seen data at address
        // get a new address off our saved stack
        while(passed[i] == 1)
        {
            memset(registers,0,sizeof(registers)); // reset the register state
            i = pop(&root); // get a saved program counter
            if(i == 0) // no more instructions
            {
                goto done;
            }
        }

        // load the opcode fields in
        do_op(&op,buffer,i);

        // mark the instruction as code
        if(data[i] < 3)
		{
			mkc(i,1,config->len);
		}
        // marked as seen
		passed[i] = 1;
		i += 4; // inc for bytes an instruciton takes


        switch(op.instruction)
        {
            case 0x0: // rtype potential registers jump <-- do code ref for this
                if(op.funct == 0x8 || op.funct == 0x9) // reset test delay slot regardless
                {
                    mkc(i,1,config->len);
                    pc = registers[op.rs]; // save the register
                    // <--- do delay slot checking clear register state after it

                    i += 4;
                    // absolute jr / jalr reg
                    if(pc > 0)
                    {
                        push(&root,i); // push current address <-- remove later as this is                                     
                                       // a absolute jump
                        i = pc;
                        mkc(i,3,config->len); // mark as jump loc
                    }
                }
                break;

            case 0x1: // bgez & bgezal & bltzal
                xref.type = CODE;
                xref.saddress = i + config->org;

                op.simm = (op.imm << 2);
                if(op.rt != 0 && op.rs != 0) // bltz (never taken)
                {
                    push(&root,i+4);
                    // <--- delay slot check
                    i += op.simm;
                    xref.daddress = i + config->org; 
                    fwrite(&xref,sizeof(Xref),1,outx);
                    nref += 1;
                    if(i < config->len)
                    {
                        mkc(i,3,config->len);
                    }
                }
                break;


            case 0x2: // jump
                xref.type = CODE;
                xref.saddress = i + config->org;

                jumpfinal = ((i & 0xf0000000) | (op.address << 2));
                
                // <--- parse delay slot here
                i = (jumpfinal - config->org&0xfffffff);

                xref.daddress = i + config->org;

                fwrite(&xref,sizeof(Xref),1,outx);
                nref += 1;
                if( i < config->len)
                {
                    mkc(i,3,config->len);
                }
                break;

            case 0x3:  // jal (call)
                xref.type = CODE;
                xref.saddress = i + config->org;

                jumpfinal = ((i & 0xf0000000) | (op.address << 2));
                push(&root,i+4); // maybye it should not be pushed as it may not return if it
                                 // mangles r8
                // <--- parse delay slot here
                i = (jumpfinal - config->org&0xfffffff);

                xref.daddress = i + config->org;

                fwrite(&xref,sizeof(Xref),1,outx);
                nref += 1;

                if(i < config->len)
                {
                    mkc(i,3,config->len); // mark as jump
                }
                break;

            case 0x4 ... 0x7: // beq bne bgtz blez
                xref.type = CODE;
                xref.saddress = i + config->org;

                op.simm = (op.imm << 2);
                push(&root, i+4); // save the pc
                // <--- check the delay slot here
                i += op.simm;

                xref.daddress = i + config->org;
                fwrite(&xref,sizeof(Xref),1,outx);
                nref += 1;

                if(i < config->len)
                {
                    mkc(i,3,config->len); // mark as a jump
                }
                break;

        /*    case 0x9: // addiu (could resolve to la) / move
                if(op.rs == 0) // register zero is a move
                {
                    registers[op.rt] = op.imm;
                }

                // if we allready have a upper immediate loaded
                // could resolve to la
                else if(registers[op.rt] > 0 && op.rt == op.rs)
                {
                    
                    xref.type = DATA; // mark ref as data
                    xref.saddress = i + config->org;

                    if(op.imm > 0x8000) { registers[op.rt] -= (1<<16); }
                    registers[op.rt] += op.imm;	// add the lower immediate
                    calc = registers[op.rt] -= config->org; // convert to our buffer address without destroying the value

                    // perform the data marks
                    // need to smarter about this markign to account for self modifying code...
                    // (dont resort to checking for a nearby jr ra)
                    if(calc < config->len)
                    {
                        // if indeed valid mark the daddress else trash the current one
                        xref.daddress = registers[op.rt];
                        fwrite(&xref,sizeof(Xref),1,outx);
						nref += 1;

                        // mark until a label hit with dw (function early)
                        // make it check for data marks not properly aligned


                    }


                }

            */




            case 0x11: // potential floating point jump
                op.rs = op.opcodefull & 0x3E00000; op.rs = op.rs >> 21;
                if(op.rs == 0x8)
                {
                    xref.type = CODE;
                    xref.saddress = i + config->org;
                    op.simm = (op.imm << 2);
                    push(&root, i+0x4); // save pc
                    // <--- check the delay slot here

                    i += op.simm;

                    xref.daddress = i + config->org;
                    fwrite(&xref,sizeof(Xref),1,outx);
                    nref += 1;
                    if(i < config->len)
                    {
                        mkc(i,3,config->len);
                    }
                }
                break;

            case 0x14 ... 0x17: // beql bnel blezl bgtzl
                xref.type = CODE;
                xref.saddress = i + config->org;
                op.simm = (op.imm << 2);
                push(&root,i+0x4); // save the pc
                // <--- parse the delay slot here
                i += op.simm;

                xref.daddress = i + config->org;

                fwrite(&xref,sizeof(Xref),1,outx);
                nref += 1;

                if(i < config->len)
                {
                    mkc(i,3,config->len);
                }

            // mark it in the file if not encoutered before
            default:
				//printf("oplist is :%d\n",ioplist);
				//exit(1);
				if(ioplist == false) // mark in the oplist
				{
						found = false;
						// search the file if not found mark it and exit printing the info

						for(int j = 0; j < listlen / sizeof(int); j++)
						{
						    if(op.instruction == oplist[j])
						    {
						       found = true; break; 
						    }
						}

						if(found == false)
						{
						    fwrite(&op.instruction,sizeof(int),1,fp);

						        puts("Unknown traversed instruction");
								printf("pc: %02x:%02x ", i,i+config->org);
								printf("instruction: %02x ", op.instruction);
								printf("rs: %02x ",op.rs);
								printf("rt: %02x ", op.rt);
								printf("rd: %02x ", op.rd);
								printf("shift: %02x ", op.shift);
								printf("funct: %02x ", op.funct);
								printf("pad : %02x ", op.pad);
								printf("sel: %02x ",op.sel);
								printf("\nopcodefull: %02x \n", op.opcodefull);
						        //clean up
						        free(passed);
						        free(oplist);
                                free(config->filename);
                                free(config);
                                free(data);
                                free(buffer);
							    fclose(outd);
							    fclose(outx);
						        fclose(fp); // debug
						        exit(1);
						}
				} 
				break;
				    
        }        	
    }


done:
    printf("Done determining code and data\n");
    printf("number of references: %d", nref);

    // return our pointer to data and write out data
    fwrite(data,sizeof(char),config->len,outd);

    //clean up
    free(passed);
    free(oplist);
	fclose(outd);
	fclose(outx);
    fclose(fp); // debug


    return data;
}



