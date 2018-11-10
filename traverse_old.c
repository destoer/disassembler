// <-----  a total redesign may be neccessary



// attempt to find what is code and what is data
// very simple algorithm atm

/* TODO */
// figure out which are jump locs and which is data when marking words
// fix dword label marker to work for all lw
// fix random stopping of dw at the end of the disassembly
// get data refs working for parsesub
// troubleshoot overflow / underflow issues...

// Add function prologue finding and seperation based on stack
// alloc and dealloc and jr ra <--------------



// Improve register emulation handling <-------------
// addu for known registers should give a resultant value
// if not it should just mention that an index has been taken 
// into a base dword
// other forms of complex handling should be added


#include "mips.h"
#include "stack.h"
#include "registers.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char *data; // for marking code
            // nearly every seciton of code works on it so allow it to be global 


// give everything a refactor 
// so indetiation and the braces
// function off the 'Register statements' <------------ Very large code save and will help troubleshoot the underflows
// attempt to remove parsesubs
// remove the global variables for the stack
// and make it return data to remove mips requirint a global var
// only the programcounter and the register array should be accessible from all functions..
// remove invalid or otherwhise out of range data references

// when hitting the end of a loc either jr ra is detected or a label is
// go back to start and parse it (
// improve data reference for code(not working for all may be broken from our org change) and mark it where its from next to the label ( will require a new struct )
// improve static code analysis  and remove the code ignoring absolute jumps

#define ENTRY 0x80000400

void parsesub(char *buffer,int length, unsigned int org);

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


void mkca(unsigned int address,int num, int length) // mark code at current pc (replace when we use array loop for jump tables)
{
	//if(address == 0x1af08-0x400) { printf("%x", op.opcodefull); exit(0); }
	if(address < length)
	{
		data[address] += num; 
		data[address+1] += num;
		data[address+2] += num;
		data[address+3] += num;	
	}
	else { printf("attempt to mark out of range address: %x\n", address); }
}

// sw lw lb sb




// causes data mis marking
void loadmark(int initialmark, int loopmark,int length,Xref xref,int org,unsigned int nref, FILE * out1)
{
	
	unsigned int calc = 0;
	if(registers[op.rs] > 0)  // if a set value
	{
					
		xref.type = DATA;
		xref.saddress = ProgramCounter + org;

		calc = registers[op.rs];
		if(op.imm > 0x8000) { calc -= 1 << 16; } // hmm why is diffrent to all the rest debug later.....
		calc += op.imm;
		xref.daddress = calc;
		
		/* DEBUGGING */
		// add error handling behavior for salvaging 
		// unaligned marks
		// for no just catch and prevent them
		if(calc % 4 != 0 || (calc - org) % 4 != 0)
		{
			printf("[ERROR] missalgined mark at %x\n",calc);
			return;
		}




		if(calc > org && calc < org + length) // check that the mark is actually valid
		{                                    
			fwrite(&xref,sizeof(Xref),1,out1);
			nref += 1;
		}
		calc -= org;

		// mark the data
		if(calc < length)
		{
			mkc(calc,initialmark,length);	
			calc += 4;

			while(data[calc] > 0 && data[calc] < 3 && calc < length)  // keep marking words until we hit another
			{
				mkc(calc,loopmark,length);
				calc+=4;
			}	
		}
	}
	if(loopmark > 10 || initialmark > 10)
	{
		printf("Loopmark exceeded values at %x:%x\n",ProgramCounter,calc);
		exit(1);
	}
}




// switch through 
	
// if there is a lui load the value
// if there is an load or store using a loaded value mark accordingly 
// if there is a branch delete the register state and take it pushing the current pc
// if there is a jr reg that is absolute take it 




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

	FILE *out1 = fopen(str1, "wb+"); // write refs
	FILE *out2 = fopen(str2, "wb+"); // write data marks
	if(out1 == NULL || out2 == NULL)
	{	
		printf("Failed to open a output file\n");
		exit(1); // exit with error if file not found
	} 	
	
	
	
	Xref xref;
	struct StackNode* root = NULL; // declare a local stack for saving pcs
	unsigned int nref = 0; // number of references in the code
	unsigned int calc; // used to calc the address of la
	unsigned int pc; // store the program calc
	unsigned int ent; // number of table entries
	unsigned int address; // address of table start
	unsigned int jumpa; // var for holding jump table addresses
	unsigned int jumpfinal;	// calculate effective jump addresses

	// factor off later with search and replace...
	unsigned int length = config->len;
	unsigned int org = config->org;
	Opcode op3; 
	printf("marking code and data...\n");

	// 0 data, 1 instruction, 3 jump, 5 jump table
	data = calloc(sizeof(char), length + 4); // allocate and zero out array of file length
	unsigned char *passed =  calloc(sizeof(char), length + 4); // check if a instruction has been seen or not

	for(;;) // add break after x amount of time
	{

		/*
		if(data[0x8001a3d0] != 0)
		{
			printf("[DEBUG] UNDEFINED MARK %x : %x\n", ProgramCounter,op.opcodefull);					
		}
		*/
		if(passed[ProgramCounter]) // if we have seen this instruction 
		{
			memset(registers,0,sizeof(registers)); // reset the register state
			ProgramCounter = pop(&root); // get our program Counter
			//printf("poped pc = %x off the stack\n", ProgramCounter); // debug
			if(ProgramCounter == 0) // if no more to pop(we are at the end)
			{
				fwrite(data,sizeof(char),config->len,out2);
				printf("number of references: %d", nref);
        			//fwrite(&nref,sizeof(int),1,out); // pull this off the end of the file to read shit in
				free(passed);
				fclose(out1);
				fclose(out2);
        			free(str1);
				free(str2);
				printf("Done determining code and data\n");
				return data;
			}	
		}

		
         
		do_op(&op,buffer,ProgramCounter);
		// mark instruciton as code
		if(data[ProgramCounter] < 3)
		{
			mkc(ProgramCounter,1,length);
		}
		passed[ProgramCounter] = 1;
		ProgramCounter += 4; // inc for bytes an instruciton takes
		
		// jumps may need revising
		switch(op.instruction) // if absolute jump dont save
		{					// if a conditonal save it on stack and jump
							// else do nothing
							
			case 0x0: // rtype potential registers jump
				
				if(op.funct == 0x8 || op.funct == 0x9) // reset test delay slot regardless
				{

					mkc(ProgramCounter,1,length);
					pc = registers[op.rs]; // save the register fix later 
					parsesub(buffer,length,org); // check delay slot instruction
					ProgramCounter += 4;
					
					// absolute jr / jalr reg
					if(pc > 0)
					{
						push(&root,ProgramCounter); // push our current address (may not want to do this...)
						ProgramCounter = pc; // load it with the register
						mkc(ProgramCounter,3,length); // mark as jump loc

					}
					
				}

				break;

			case 0x1: // bgez & bgezal & bltz & bltzal

				xref.type = CODE;
				xref.saddress = ProgramCounter + org;

				op.simm = (op.imm << 2);
				if(op.rt != 0 && op.rs != 0) // bltz ( branch will never be taken )
				{
					push(&root, ProgramCounter+0x4); // save our program counter
					parsesub(buffer,length,org); // check delay slot instruction

					ProgramCounter += op.simm;

					xref.daddress = ProgramCounter + org;
					fwrite(&xref,sizeof(Xref),1,out1);
					nref += 1;
					if(ProgramCounter < length)
					{
						mkc(ProgramCounter,3,length); // mark as a jump

					}
				}
				break;
			case 0x2: // jump

				xref.type = CODE;
				xref.saddress = ProgramCounter + org;
				
				jumpfinal = ((ProgramCounter & 0xf0000000) | (op.address << 2)); 
				// instruction following the jump is the delay slot MARK IT
				//mkc(ProgramCounter,1,length);
				//if( jumpfinal == 0x1af08) { printf("\n%x\n", ProgramCounter); printf("%x", op.opcodefull); }
				parsesub(buffer,length,org); // check delay slot instruction
				ProgramCounter = (jumpfinal - org&0xfffffff);


				xref.daddress = ProgramCounter + org;

				//printf("Writing %x, %x, %x\n",xref.type,xref.daddress,xref.saddress);
				fwrite(&xref,sizeof(Xref),1,out1);
				nref += 1;
				if(ProgramCounter < length)
				{
					//printf("pc: %x\n", pc);
					mkc(ProgramCounter,3,length); // mark as a jump
				} 
				break;
				
				 
			case 0x3: // jal (need to pop stored when a j r31 is seen allow for now)
				xref.type = CODE;
				xref.saddress = ProgramCounter + org;


				jumpfinal = ((ProgramCounter & 0xf0000000) | (op.address << 2)); 
				// next instruction is delay slot so is a garunteed instruction(we allready point to the next instruction)
				push(&root, ProgramCounter+0x4); // maybe this shouldn't be pushed?
				parsesub(buffer,length,org); // check delay slot instruction
				ProgramCounter = jumpfinal - org&0xfffffff;  

				xref.daddress = ProgramCounter + org;


				fwrite(&xref,sizeof(Xref),1,out1);
				nref += 1;
		
				if(ProgramCounter < length)
				{
					
					mkc(ProgramCounter,3,length); // mark as a jump
				}
				break;
				
			case 0x4 ... 0x7: // beq bne bgtz blez
				xref.type = CODE;
				xref.saddress = ProgramCounter + org;

                		op.simm = (op.imm << 2);
				push(&root, ProgramCounter+0x4); // save our program counter
				parsesub(buffer,length,org); // check delay slot instruction
				ProgramCounter += op.simm;

				xref.daddress = ProgramCounter + org;

				fwrite(&xref,sizeof(Xref),1,out1);
				nref += 1;

				if(ProgramCounter < length)
				{
					mkc(ProgramCounter,3,length); // mark as a jump
				}
				break;
			    
			case 0x9: // addiu (could resolve to la) // get the xref sorted for This
				if(registers[op.rt] > 0 && op.rt == op.rs) // if lui has occurred and we have addition that register
				{
					
					
					
					xref.type = DATA;
					xref.saddress = ProgramCounter + org;

					if(op.imm > 0x8000) { registers[op.rt]-= (1 << 16); } // minus one if msb set
					registers[op.rt] -= org; // convert to a buffer address
					registers[op.rt] += op.imm;	// add the lower immediate
					//printf("pc: %x,%x",ProgramCounter, registers[op.rt] + 0x80000400); exit(0);
					calc = registers[op.rt]; // set calc so register aint modified
					
					
				
					if(calc < length)
					{
						if(data[calc] < 3) // if not a jump loc
						{
							for(int i = 1; i < 6; i++)
							{
								do_op(&op,buffer,calc-(i*4));
								if(op.opcodefull == 0x3e00008) // if a ret follows it is likely a jump loc
								{
									mkc(calc,3,length);
								}
									
							}
						}
						
						if(data[calc] != 3) 
						{
							xref.daddress = calc+org;
							if(calc < length) // check that the mark is actually valid
							{                                    
								fwrite(&xref,sizeof(Xref),1,out1);
								nref += 1;
							}
	
							mkc(calc,-2,length); calc+=4;
							while(data[calc] > 0 && data[calc] != 3 &&  calc < length)  // keep bytes until we hit another or a word
							{

								mkc(calc,-1,length);
								calc+=4;
							} 							
						} 

					}	
				}
				
				break;	
			case 0xf: // lui
				// load it with the value
				registers[op.rt] = op.imm << 16;
				break;
				
		/*	case 0xb: // sltiu potential jump table start 	
				pc = ProgramCounter; // save the pc

				ent = op.imm; // get the number of table entries
				do_op(&op,buffer,ProgramCounter);
				
				if(!(op.instruction == 0x4 && (op.rs == 00 || op.rt == 00))) { ProgramCounter += 4; do_op(&op, buffer,ProgramCounter); }
				if(op.instruction == 0x4 && (op.rs == 00 || op.rt == 00)) // beqz
				{
					
					ProgramCounter += 4;
					do_op(&op,buffer,ProgramCounter);
				
					if(!(op.instruction == 0x0 && op.funct == 0x0 && op.shift == 0x2)) { ProgramCounter += 4; do_op(&op, buffer,ProgramCounter);}

					if(op.instruction == 0x0 && op.funct == 0x0 && op.shift == 0x2) // sll
					{
								
						
						ProgramCounter += 4;
						do_op(&op,buffer,ProgramCounter);
						
						if(op.instruction != 0xf) { ProgramCounter += 4; do_op(&op, buffer,ProgramCounter);  }
						if(op.instruction == 0xf) // lui
						{
							
							address = (op.imm << 16);
							ProgramCounter += 4;
							do_op(&op2,buffer,ProgramCounter);
							
							if(!(op2.instruction == 0x0 &&  op2.funct == 0x21 && op2.rd == op.rt)) { ProgramCounter += 4; do_op(&op2, buffer,ProgramCounter); }
							if(op2.instruction == 0x0 &&  op2.funct == 0x21 && op2.rd == op.rt) // addu same reg as lui
							{
								ProgramCounter += 4;
								do_op(&op2, buffer,ProgramCounter);
								
								if(op2.instruction != 0x23) { ProgramCounter += 4; do_op(&op2, buffer,ProgramCounter);  }
								if(op2.instruction == 0x23) // load word
								{
									if(op2.imm > 0x8000) { address -= (1 << 16); }
									
									address += op2.imm; // get the address
									ProgramCounter += 4;
									do_op(&op2, buffer,ProgramCounter);
									
									if(!(op2.instruction == 0x0 && op2.funct == 0x8)) { ProgramCounter += 4; do_op(&op2, buffer,ProgramCounter); }
									if(op2.instruction == 0x0 && op2.funct == 0x8)
									{

										printf("; Jump table detected at %x, %x cases, table at %x\n", ProgramCounter+0x400,ent,address);
										//exit(0);
										address -= org; // convert to our address scheme
										if(address > length) { printf("out of range address"); exit(0); }
										// mark the jump table addresses as dw data
										for(int i = 0; i < (ent * 4); i+= 4)
										{
											mkc(address+i,-1,length);
											jumpa = buffer[address + i] << 24;
											jumpa += buffer[address + i+1] << 16;
											jumpa += buffer[address + i+2] << 8;
											jumpa += buffer[address + i+3];
											jumpa -= org; // convert to our address scheme
											printf("; %x(%x)\n", jumpa,jumpa+org);
											if(jumpa < length) { push(&root, jumpa); } // save our address for code analysis
										}
										
										mkc(ProgramCounter,5,length);
										//	ProgramCounter += 4;
									}
								}	
							}			
						}			
					}	
				}
				ProgramCounter = pc; // restore the pc
				break;
			*/
			
			case 0x11: // potential floating point jump
				op.rs = op.opcodefull & 0x3E00000; op.rs = op.rs >> 21;
				if(op.rs == 0x8) { // floating point branch(needs testing)
					xref.type = CODE;
					xref.saddress = ProgramCounter + org;
					op.simm = (op.imm << 2);
					push(&root, ProgramCounter+0x4); // save our program counter
					parsesub(buffer,length,org); // check delay slot instruction
					ProgramCounter += op.simm;

					xref.daddress = ProgramCounter + org;

					fwrite(&xref,sizeof(Xref),1,out1);
					nref += 1;
					if(ProgramCounter < length)
					{
						mkc(ProgramCounter,3,length);
					}

				}
				break;

        	case 0x14 ... 0x17: // beql bnel blezl bgtzl

				xref.type = CODE;
				xref.saddress = ProgramCounter + org;

               			op.simm = (op.imm << 2);
				push(&root, ProgramCounter+0x4); // save our program counter
				parsesub(buffer,length,org); // check delay slot instruction
				ProgramCounter += op.simm;

				xref.daddress = ProgramCounter + org;

				fwrite(&xref,sizeof(Xref),1,out1);
				nref += 1;

				if(ProgramCounter < length)
				{
					mkc(ProgramCounter,3,length); // mark as a jump
				}
               			break;

			case 0x20: // load byte
				loadmark(0,0,length,xref,org,nref,out1);
				break;
				
			case 0x23: // load word
				loadmark(-2,-1,length,xref,org,nref,out1);		
				break;
			
			case 0x28: // store byte
				loadmark(0,0,length,xref,org,nref,out1);
				break;			
			case 0x2b: // store word ( register does not get modified )
				//if(ProgramCounter == 0x11c68) { exit(0); }
				loadmark(-2,-1,length,xref,org,nref,out1);
				break;

			// different to rest apparently!? (verify)
			case 0x31: // load word from memory to fp reg(lwc1)
			/*	printf("%x: lwc1",op.opcodefull); exit(1);
				if(registers[op.rs] > 0)
				{
					xref.type = DATA;
					xref.saddress = ProgramCounter + org;

					calc = registers[op.rs];
					if(op.imm > 0x8000) { calc -= 1; }

					calc += op.imm;
					xref.daddress = calc;
					if(calc > org && calc < org + length) // check that the mark is actually valid
					{                                    
						fwrite(&xref,sizeof(Xref),1,out1);
						nref += 1;
					}					
					
					
					if(calc < length)
					{
						mkc(calc,-2,length); // mark as word for now float eventually
								
						calc += 4;
						while(data[calc] > 0 && data[calc] < 3 && calc < length)  // keep bytes until we hit another or a word
						{

							mkc(calc,-1,length); // mark as float eventually
							calc+=4;
						}
							
					}
				} */
				loadmark(-2,-1,length,xref,org,nref,out1);
				break;				


			default: // not a jump
				break;

		}
		
		if(ProgramCounter > length) 
		{

			memset(registers,0,sizeof(registers)); // reset the register state

			ProgramCounter = pop(&root); // get our program Counter

			if(ProgramCounter == 0) // if no more to pop(we are at the end)
			{
				fwrite(data,sizeof(char),config->len,out2);
				printf("number of references: %d", nref);
        			//fwrite(&nref,sizeof(int),1,out); // pull this off the end of the file to read shit in
				free(passed);
				fclose(out1);
				fclose(out2);
				free(str2);
        			free(str1);
				free(passed);
				printf("Done determining code and data\n");
				return data;
			}
			
		}

	}

}






// parse the delay slot instruction
// get xref support for parsesub...
void parsesub(char *buffer,int length,unsigned int org)
{

	unsigned int calc;
	// switch through 
	
	// if there is a lui load the value
	// if there is an load or store using a loaded value mark accordingly 

	
	

	

		
		do_op(&op,buffer,ProgramCounter);
		mkc(ProgramCounter,1,length);
		
		switch(op.instruction)
		{


			case 0xf: // lui
				// load it with the value
				registers[op.rt] = (op.imm << 16);
				break;
		

/*			case 0x20: // load byte
				if(registers[op.rs] > 0)
				{
					calc = registers[op.rs];
					if(op.imm > 0x8000) { calc -= 1; }

					calc += op.imm;
					calc = registers[op.rs];
						
					calc -= 0x80000400;
						
					if(calc < length)
					{
						mkc(calc,0,length);
								
						calc += 4;
						while(data[calc] > 0 && data[calc] < 3 && calc < length)  // keep bytes until we hit another or a word
						{

							mkc(calc,0,length);
							calc+=4;
						}
							
					}
				}
				break;
	*/					
		case 0x23: // load word
					
				if(registers[op.rs] > 0)  // if a set value
				{
					calc = registers[op.rs];
					if(op.imm > 0x8000) { calc  -= 16 << 1; }
					calc += op.imm;
					calc -= org;
						
					// mark the data
					if(calc < length)
					{
						//printf("calc %x\n",calc); 
						mkc(calc,-2,length);
								
						calc += 4;
					/*	while(data[calc] > 0 && data[calc] < 3 && calc < length)  // keep marking words until we hit another
						{

							mkc(calc,-1,length);
							calc+=4;
						}		*/
					}
				}
				break;
/*			case 0x28: // store byte
				if(registers[op.rs] > 0)  // if a set value
				{
					calc = registers[op.rs];
					if(op.imm > 0x8000) { calc -= 1; }
					calc += op.imm;
					calc -= 0x80000400;
						
					// mark the data
					if(calc < length)
					{
						mkc(calc,0,length); // mark as byte
								
						calc += 4;
						while(data[calc] > 0 && data[calc] < 3 && calc < length)  // keep marking words until we hit another
						{
							mkc(calc,0,length);
							calc+=4;
						}		
					}
				}
				break;
*/				
			case 0x2b: // store word ( register does not get modified )
				//if(ProgramCounter == 0x11c68) { exit(0); }
				if(registers[op.rs] > 0)  // if a set value
				{
					calc = registers[op.rs];
					if(op.imm > 0x8000) { calc -= 1 << 16; }
					calc += op.imm;
					calc -= org;
					
					
					// mark the data
					if(calc < length)
					{
						mkc(calc,-2,length);
								
						calc += 4;
					/*	while(data[calc] > 0 && data[calc] < 3 && calc < length)  // keep marking words until we hit another
						{
							mkc(calc,-1,length);
							calc+=4;
						}
					*/
						
					}
				}
				break;
			case 0x31: // load word from memory to fp reg(lwc1)
				if(registers[op.rs] > 0)
				{
					calc = registers[op.rs];
					if(op.imm > 0x8000) { calc -= 1; }

					calc += op.imm;
					calc -= org;
					if(calc < length)
					{
						mkc(calc,-1,length); // mark as word for now float eventually
								
						calc += 4;
						while(data[calc] > 0 && data[calc] < 3 && calc < length)  // keep bytes until we hit another or a word
						{

							mkc(calc,-1,length); // mark as float eventually
								calc+=4;
						}
							
					}
				}
				break;


			default: // not a jump
				break;

		}
		memset( registers,0,sizeof(registers) );
}
