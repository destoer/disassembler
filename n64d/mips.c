  #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mips.h" 
#include "printing.h"

// Last updated on 22/04/2018


// many be better to have a seperate int and char buffer
// needs a refactor :)
// improve traversal disassembler(maybe add basic "emulation")
// get co-processor functions evaluated properly
// advanced (attempt to encode for pseudo-assembly instruction)
// add hex dumps as comments next to the asm incase the disassembler messes up ( will be by side when gui done )
// add support for decoding co-processor functions :)
// remove the debugging printing
// clean up cop1 WELL ( remove all the else if rs = for the letters and just function it )
// add generic shift func
// work on text editing for gui

// after done GUI and make interactive such as remkaring data saving memory  for data
// commenting hash checks on assembly vs original etc

unsigned int calc; 
unsigned int jumpfinal; // calculate final jump address
char *data; // mark what the code is
unsigned int ProgramCounter = 0; // address of current instruction
Opcode op; // define our opcode
Opcode op2; // used for resolving pseudo instructions





void do_op(Opcode *op, unsigned char *buffer, unsigned int address) // load a struct with information about the opcode
{
		// load our opcode with the full opcode
		op->opcodefull = buffer[address] << 24;
		op->opcodefull += buffer[address+1] << 16;
		op->opcodefull += buffer[address+2] << 8;
		op->opcodefull += buffer[address+3]; 

		

		// get our register fields
		// better to pull only what we need but work out later

		op->instruction = op->opcodefull & 0xFC000000;   op->instruction = op->instruction >> 26;
		op->rs = op->opcodefull & 0x3E00000; op->rs = op->rs >> 21;
		op->rt = op->opcodefull & 0x1F0000;	op->rt = op->rt >> 16;
		op->rd = op->opcodefull & 0xF800;	 op->rd = op->rd >> 11;
		op->shift = op->opcodefull & 0x7C0;  op->shift = op->shift >> 6; // may be wrong?
		op->funct = op->opcodefull & 0x3F;
		op->imm = op->opcodefull & 0xffff;
		//simm = opcodefull & 0xffff; may need in future ;)
		op->address = op->opcodefull & 0x3FFFFFF;	
		
	/*	if(op->rt > 31 || op->rs > 31 || op->rd > 31)
		{
			printf("Error out of range register");
			exit(0);
		} */
}


void inline load(char *instruction) // same syntax for a store
{
	printf("%s %s, $%x(%s)",instruction,r[op.rt],op.imm,r[op.rs]);
}

void branch(char *instruction, unsigned int org)
{
		op.simm = (op.imm << 2);
		jumpfinal = ProgramCounter + op.simm;
		printf("%s %s, %s, loc_%x ; %x",
		instruction,r[op.rs],r[op.rt],jumpfinal+org,op.simm);
}

void inline arithreg2(char *instruction) // divu rs, rt
{
	printf("%s %s, %s", instruction, r[op.rs], r[op.rt]);
}

void mkc(unsigned int address,int num,int length) // mark code at current pc (replace when we use array loop for jump tables)
{
	
		//if(num == 0) { printf("byte mark op %x: pc %x\n", op.opcodefull, ProgramCounter); return; }
		
		if(address < length)
		{
			data[address] = num;
			data[address+1] = num;
			data[address+2] = num;
			data[address+3] = num;	
		}	
		else { printf("error attempted to mark out of range address %x %x %x\n", address,ProgramCounter,op.opcodefull); }
}

void inline arithreg(char *instruction) // arithmetic instructions with registers ins, rd ,rs, rt
{
		printf("%s %s, %s, %s",instruction,r[op.rd],r[op.rs],r[op.rt]);
}


void inline arithimm(char *instruction) // arithmetic instructions with immediates ( ins, rt, rs, imm
{
	printf("%s %s, %s, $%x", instruction, r[op.rt],r[op.rs],op.imm);
}



// generic branch and arithmetic instructions here to keep code clean
// even if they are inlined

void ffmt(char *instruction, int number) // print a floating instruction ( slightly slower but cleaner code )
{
	// 	printf("add.s f%d, f%d, f%d", shift, rd, rt);
	//  printf("mov.s f%d, f%d", shift, rd);
	printf("%s", instruction);

	// print the fmt
	if(op.rs == 0x10) // single
	{
		printf(".s");
	}
	
	else if(op.rs == 0x11) // double
	{
		printf(".d");	
	}
	
	else if(op.rs == 0x14) // word
	{
		printf(".w");
	}
	
	else if(op.rs == 0x15) // long
	{
		printf(".l");
	}
	
	else
	{
		printf("unknown fmt %d", op.rs);
		exit(0);
	}
	
	// print the registers
	if(number == 3)
	{
		printf(" f%d, f%d, f%d", op.shift, op.rd, op.rt);
	}
	
	else if(number == 2)
	{
		printf(" f%d, f%d", op.shift, op.rd);
	}
	
	else
	{
		printf("invalid number of registers %d",number);
		exit(0);
	}

	
}




void disass_mips(unsigned char *buffer, int length, int flag, unsigned int org)
{
	

	//-------------------------------------
	// attempt to determine what is code 
	// and what is data
	// data is 0 instruction is 1
	if(flag == 1) // if a rom
	{
		traverse(buffer, length, flag,org);
			
		// if data follows it mark as -1 
		// even if it follows a -1 until another jump
		// detrace code
		// DOESENT WORK
	/*	for(int i = 0; i < length; i+=4)
		{
			if(data[i] == 4) 
			{
				// save the counter by here and mark latter
				calc = i;
				do_op(&op,buffer,i);
				while(op.opcodefull == 0) 
				{ 
					i += 4; 
					if(data[i] == 1 && op.opcodefull != 0) { data[calc] = 3;  break; } // it is a jump
				}
				
				
				if(data[calc] != 3) { data[calc] = -1; }
				
			}
		}  */
	}


	
	else if(flag == 0)// disassemble a binary ( may be better just to check for a flag than waste all the memory allocation )
	{
		data = malloc(sizeof(char) * length); 
		memset(data, 1, length);	// flag all of it as code (cause lazy)
	}
	//-------------------------------------
	// disassemble the file
	if(flag != 2)
		puts("\nstart:");
	
	while(ProgramCounter < length)
	{
		
		do_op(&op,buffer,ProgramCounter); // load with info
		
		// switch for instruction
		// may need checks rs if not 0 for immediate
		putchar('\t');
		
		if(data[ProgramCounter] > 0) // if an instruction
		{
			if(data[ProgramCounter] == 3) { printf("\rloc_%x:\n\t", ProgramCounter+org); }
			ProgramCounter += 4; // increment the program counter for bytes used
			switch(op.instruction)
			{
				case 0x0: // r type encoding
					rtype();	
					break;
				case 0x1: // bgez branch greater than equal zero( may not work )
					op.simm = (op.imm << 2);
					jumpfinal = ProgramCounter + op.simm;
					printf("bgez %s, loc_%x ; %x", r[op.rs],jumpfinal+org,op.simm);
					break;
				case 0x2: // jump
					jumpfinal = (ProgramCounter & 0xf0000000) | (op.address << 2); 
					if( (jumpfinal - org&0xfffffff) < length)
					{
						printf("j loc_%x", jumpfinal + (org&0xf0000000));
					}
					else
					{
						printf("j $%x ; out of range jump", jumpfinal);
					}
					break;
				case 0x3: // jump and link (call kinda...)
					jumpfinal = (ProgramCounter & 0xf0000000) | (op.address << 2); 
					if( ( jumpfinal - org&0xfffffff ) < length)
					{
						printf("jal loc_%x", jumpfinal + (org&0xf0000000));
						//printf("%x",jumpfinal + (org&0xf0000000));
						//exit(0);
					}
					else
					{
						printf("jal $%x ; out of range jump", jumpfinal);
					}
					printf(" ; %x", op.address << 2); // debug
					break;
				case 0x4: // branch on equal
					branch("beq",org);
					break;
				case 0x5: // bne
					branch("bne",org);
					break;
				case 0x6: // branch less than or equal to zero(wrong?)
					if(op.rt !=  0) 
					{
						printf("Error on blez\n");
						exit(1);
					}
					op.simm = (op.imm << 2);
					jumpfinal = ProgramCounter + op.simm;
					printf("blez %s, loc_%x ; %x", r[op.rs], jumpfinal+org, op.simm);
					break;
				case 0x7: // bgtz branch on greater than zero ( may not work )
					op.simm = (op.imm << 2);
					jumpfinal = ProgramCounter + op.simm;
					printf("bgtz %s, loc_%x ; %x",r[op.rs],jumpfinal+org,op.simm);
					break;
				case 0x8: // add immediate
					arithimm("addi");
					break;
			
				case 0x9: // add immediate unsigned
					arithimm("addiu");
					break;
				case 0xa: // set on less than immediate (signed)
					arithimm("slti");
					break;
				case 0xe: // xori xor immediate
					arithimm("xori");
					break;
					//COPROCESSORS NEEDS TO BE FIXED
				case 0x10: // co-processor 0 instruction (get details on the actual func later) not in order fix later
					//printf("cop0 ");
					op.sel = op.opcodefull & 0x3;
					op.pad = op.opcodefull & 0x7F8; op.pad = op.pad  >> 3;
					switch(op.rs) // rs has the actual instruction bits
					{

						case 0x0: // MFC0(move from co-processor zero)
							// assume rd is the large number
							// and sel selects the minor  // figure out the register encoding(keep assembling shit)
							printf("mfc0 %s, r%d", r[op.rt],op.rd+op.sel);
							break; 
								
						case 0x4: // mtc0
							printf("mtc0 %s, r%d", r[op.rt],op.rd+op.sel); 
							break;
						case 0x10: // c0 (may conflict) 
							if(op.funct == 0x1) // read indexed tlb entry
							{
								printf("tlbr");
							}
							else if(op.funct == 0x2)
							{
								printf("tlbwi ");
							}
							else if(op.funct == 0x18) // exception return
							{
								printf("eret");
							}
							else if(op.funct == 0x8) // prone tlb for matching entry
							{
								printf("tlbp");
							}
							else
							{
								
								printf("unknown  co-processor0 opcode\n");
								printf("pc: %02x ", ProgramCounter);
								printf("instruction: %02x ", op.instruction);
								printf("rs: %02x ", op.rs);
								printf("rt: %02x ", op.rt);
								printf("rd: %02x ", op.rd);
								printf("shift: %02x ", op.shift);
								printf("funct: %02x ", op.funct);
								printf("pad : %02x ",op.pad);
								printf("sel: %02x ", op.sel);
								printf("\nopcodefull: %02x ", op.opcodefull);
								exit(0);
							}
							
							break;
							
						default: // unknown opcode
							printf("unknown  co-processor0 opcode\n");
							printf("pc: %02x ", ProgramCounter);
							printf("instruction: %02x ", op.instruction);
							printf("rs: %02x ",op.rs);
							printf("rt: %02x ", op.rt);
							printf("rd: %02x ", op.rd);
							printf("shift: %02x ", op.shift);
							printf("funct: %02x ", op.funct);
							printf("pad : %02x ", op.pad);
							printf("sel: %02x ",op.sel);
							printf("\nopcodefull: %02x ", op.opcodefull);
							exit(1);
							break;
					}

					break;
					//00010000
					//   ^ means its a coprocessor
						
					//right most bits are the coprecssor being used
					// other 26 bits are the function on the coprocessor
					// figure this out later :)
					
				case 0x11: // cop1 (floating point co-processor) // this will require extensive checking not in order fix later
					
					
					if(op.funct == 0)
					{
						switch(op.rs)
						{
							
							case 0x0: // move word from floating point
								printf("mfc1 %s, f%d", r[op.rt],op.rd);
								break;
							case 0x1: // dmfc1 (move fs into rt)
								printf("dmfc1 %s, f%d", r[op.rt],op.rd);
								break;
							case 0x2:
								// is r%d for some odd reason!?
								printf("cfc1 %s, r%d", r[op.rt],op.rd);
								break;
							case 0x4:
								printf("mtc1 %s, f%d", r[op.rt],op.rd);
								break;
							case 0x5: // copy doubleword form gpr to fpr
								printf("dmtc1 %s, f%d", r[op.rt],op.rd);
								break;
							case 0x6:  // control word to floating point
								printf("ctc1 %s, r%d", r[op.rt],op.rd);
								break;
							
							default:
							
								// put these in the standard loop after it works xd
								if(op.rs == 0x10) // add .s
								{
									printf("add.s f%d, f%d, f%d", op.shift, op.rd, op.rt);
									
									
								}
								
								else if(op.rs == 0x11)
								{
									printf("add.d f%d, f%d, f%d", op.shift, op.rd, op.rt);
								}


							/*
								pc: 4a74 instruction: 11 fmt: 02 ft: 0b fs: 1f fd: 00 funct: 00
								opcodefull: 444bf800
							
							*/
								
							
								else
								{
									printf("\nfunct = 0\n");
									printf("unknown  co-processor1 opcode\n");
									printf("pc: %02x ", ProgramCounter);
									printf("instruction: %02x ", op.instruction);
									printf("fmt: %02x ", op.rs);
									printf("ft: %02x ", op.rt);
									printf("fs: %02x ", op.rd);
									printf("fd: %02x ", op.shift);
									printf("funct: %02x ", op.funct);
									//printf("pad : %02x ", pad);
									//printf("sel: %02x ", sel);
									printf("\nopcodefull: %02x ", op.opcodefull);
									exit(0);	
								}
								break;
							
							
						}
					}
					
					else // look out for floating point branch as its hard to account for
					{
						if(op.rs != 0x8)  // not a floating point jump
						{				
							switch(op.funct)
							{
								
								
								case 0x1: // sub.s / sub.d
									ffmt("sub",3);
								
									break;
							
								
								case 0x2: // mul.s / mul.d
									ffmt("mul",3);
									break;
									
								case 0x3: // div.s / div.d
									ffmt("div",3);
									break;
								case 0x6: // mov
									ffmt("mov",2);
									break;
								case 0x7: // floating point negative
									ffmt("neg",2);
									break;
								case 0x9: // trunc.l ( may not work...)
									ffmt("trunc.l",2);
									break;

									
								case 0xd: // trunc.w floating truncate to word fixed point
									ffmt("trunc.w",2);
									break;
								case 0x20: // cvt.s ( floating point convert to single floating point)
									ffmt("cvt.s",2);
									break;
								case 0x21: // floating point convert to double floating point ( could be wrong)
								
									ffmt("cvt.d",2);
									break;
								case 0x24: // floating point to word fixed point
									ffmt("cvt.w",2);
									break;
								case 0x25: // floating point convert to long point
									ffmt("cvt.l",2);
									
									break;
									
								default:
									if( ( (op.opcodefull & 0xf0) >> 4 ) == 3 )// c (floating point compare) may require chacking for true and false on the code
									{								// to do the inverse eg unorderd becomes ordered
										printf("c.");
										op.cond = op.opcodefull & 15;
										if((op.cond & 8) == 0) // 4th bit is 0
										{
											switch(op.cond)
											{
												case 0: // False
													printf("f");
													break;
												case 1: // unorderd
													printf("un");
													break;
												case 2: // equal
													printf("eq");
													break;
												case 3: // unorder or equal
													printf("ueq");
													break;
												case 4: // orderd or less than
													printf("olt");
													break;
												case 5: // unorderd less than
													printf("ult");
													break;
												case 6: // ordered or less than equal
													printf("ole");
													break;
												case 7: // uordered or less than or equal
													printf("ule");
													break;
												default:
													printf("unknown conditon %x\n", op.cond);
													printf("opcodefull %x", op.opcodefull);
													exit(0);												
											}
										}
										
										else // 4th bit is 1
										{
											op.cond &= 3; // remove the cond 3rd bit
											switch(op.cond)
											{
												case 0: // siginaling false
													printf("sf");
													break;
												case 1: // not greater than or less than or equal
													printf("ngle");
													break;
												case 2: // siginaling equal
													printf("seq");
													break;
												case 3: // not greater than or less than
													printf("ngl");
													break;
												case 4: //less than
													printf("lt");
													break;
												case 5: // not greater thanb or equal
													printf("nge");
													break;
												case 6: // less than or equal
													printf("le");
													break;
												case 7: // not greater than
													printf("ngt");
													break;
												default:
													printf("unknown conditon %x\n", op.cond);
													printf("opcodefull %x", op.opcodefull);
													exit(0);
											}
										}
										
										if(op.rs == 0x11)
										{
											printf(".d");
										}
										
										if(op.rs == 0x10)
										{
											printf(".s");
										}
										printf(" f%d, f%d", op.rd, op.rt);  // may require format cc,fs,ft instead of fs,ft

									}
									
								
								
									else 
									{ // is an  actual invalid opcode
										
										printf("\nfunct != 0\n");
										printf("unknown  co-processor1 opcode\n");
										printf("pc: %02x ", ProgramCounter);
										printf("instruction: %02x ", op.instruction);
										printf("fmt: %02x ", op.rs);
										printf("ft: %02x ", op.rt);
										printf("fs: %02x ", op.rd);
										printf("fd: %02x ", op.shift);
										printf("funct: %02x ", op.funct);
										//printf("pad : %02x ", pad);
										//printf("sel: %02x ", sel);
										printf("\nopcodefull: %02x ",op.opcodefull);
										exit(0);
									}
							}
						}
						
						else // rs of 8 means its a floating point jump
						{ // floating point jump(this is gonna be a major pain to test(may require cc printing too)
							op.cond = op.rt; // reuse cond to store cc
							op.cond = op.cond & 3; // get first two bits(may require to get cc first laster stored in 3 msb)
							
							switch(op.cond)
							{
								case 0: // branch on fp false
									printf("bc1f");
									break;
								case 1: // branch on fp true
									printf("bc1t");
									break;
								case 2: // branch on fp false likely
									printf("bc1fl");
									break;
								case 3: // branch on fp true likely
									printf("bc1tl");
									break;
							}
							// print the jump(may not be correct)
							op.simm = (op.imm << 2);
							jumpfinal = ProgramCounter + op.simm;
							printf(" loc_%x", jumpfinal+org);
							printf(" ; %x", op.simm); // debug
						}
						
					}
					break;
			
				case 0xb: // set on less than immediate unsigned
					arithimm("sltiu");
					break;
				case 0xc: // and immediate
					arithimm("andi");
					break;
				case 0xd: // bitwise or immediate
					if(op.rs != 0)
					{
						arithimm("ori");
					}
					
					// attempt to resolve to li
					else
					{
						printf("li %s, $%x",r[op.rt],op.imm);
					}
					break;
			
				case 0xf: // load upper immediate imm shifted 16 bits rest of gpr zeroed out
					do_op(&op2, buffer,ProgramCounter); // get our info 
					
					if(op2.instruction == 0x9 && op2.rt == op.rt && data[ProgramCounter] < 3) // resolve to la(needs support for jump tables)
					{ 
						if(op2.imm > 0x8000) { op.imm -= 1; } // minus one if msb set
						calc = (op.imm) << 16; 
						calc += op2.imm; // may require |=
						printf("la %s, $%x", r[op2.rt], calc);
						ProgramCounter += 4; // accounts for two instructions
						
					}
					
					else 
					{
						printf("lui %s, $%0x",r[op.rt], op.imm); // may be wrong ( $%0x )
					}
					
					break;
				case 0x14: // branch on equal likely
					branch("beql",org);
					break;
				case 0x15: // bnel // branch on not equal or likely (may not work)
					branch("bnel",org);
					break;
				case 0x16: // blezl ( may not work )
					op.simm = (op.imm << 2);
					jumpfinal = ProgramCounter + op.simm;
					printf("blezl %s, loc_%x ; %x",r[op.rs],jumpfinal+org,op.simm);
 					break;
				case 0x17: // branch on greater than zero likely ( may not work )
					op.simm = (op.imm << 2);
					jumpfinal = ProgramCounter + op.simm;
					printf("bgtzl %s, loc_%x ; %x",r[op.rs],jumpfinal+org,op.simm);
					break;
				case 0x19: // double word add immediate unsigned
					arithimm("daddiu");
					break;
				case 0x20: // load byte
					load("lb");
					break;
				case 0x21: // lh load halfword
					load("lh");
					break;
				case 0x22: // lwl load word left
					load("lwl");
					break;
					
				case 0x23: // load word
					load("lw");
					break;

				case 0x24: // load byte unsigned(?)
					load("lbu");
					break;
				case 0x25:
					load("lhu"); // load halfword unsigned
					break;
				case 0x26: // lwr load word right
					load("lwr");
					break; 
				case 0x28: // store byte
					load("sb");
					break;
					
				case 0x2a: // swl store word left
					load("swl");
					break;
				
				case 0x29: // store halfword
					load("sh");
					break;

				case 0x2b: // store word
					load("sw");
					break;
				case 0x2e:
					load("swr");
					break;
					// 00010000
				case 0x2f: //cache (may be wrong)
					printf("cache $%x, $%x(%s)", op.rt,op.imm,r[op.rs]);
					break;
				case 0x31: // load word to floating point
					printf("lwc1 f%d, $%x(%s)",op.rt,op.imm,r[op.rs]);
					break; 
				case 0x35: // load double word to coprocessor
					printf("ldc1 f%d, $%x(%s)",op.rt,op.imm,r[op.rs]);
					break;
				case 0x37: // load doubleword (untested)
					load("ld");
					break;
				case 0x39: // store word from floating point(likely dont work)
					printf("swc1 f%d, $%x(%s)",op.rt,op.imm,r[op.rs]);
					break;
				case 0x3d: // store doubleword from floating point
					printf("sdc1 f%d, $%x(%s)",op.rt,op.imm,r[op.rs]);
					break;
				case 0x3f: // store doubleword
					load("sd");
					break;
					
			default: // unknown opcode
					printf("unknown opcode\n");
					printf("pc: %02x ", ProgramCounter);
					printf("instruction: %02x ", op.instruction);
					printf("rs: %02x ", op.rs);
					printf("rt: %02x ", op.rt);
					printf("rd: %02x ", op.rd);
					printf("shift: %02x ", op.shift);
					printf("funct: %02x ", op.funct);
					printf("\nopcodefull: %02x ", op.opcodefull);
					exit(1);
					break;
			
			}
			if(data[ProgramCounter-4] == 5) { printf(" ; jump table detected "); }
			putchar(10);
		}

		else // is data 
		{
			putchar('\r');
			
			while( data[ProgramCounter] <= 0 && ProgramCounter < length) // while still data and less than length print data
			{


				
					//printf("; %08x: ", ProgramCounter+0x400); // debug
					if(data[ProgramCounter] == 0)
					{
						printf("\tdb $%x\n", buffer[ProgramCounter]);
						ProgramCounter++;
					}
					
					else if(data[ProgramCounter] == -1)
					{
						calc = buffer[ProgramCounter] << 24;
						calc += buffer[ProgramCounter+1] << 16;
						calc += buffer[ProgramCounter+2] << 8;
						calc += buffer[ProgramCounter+3];						
						printf("\tdw $%x\n", calc);
						ProgramCounter += 4;
						
					}
					
					else if(data[ProgramCounter] == -2) // initial data mrk
					{
						printf("dword_%x:\n", ProgramCounter+org);
						calc = buffer[ProgramCounter] << 24;
						calc += buffer[ProgramCounter+1] << 16;
						calc += buffer[ProgramCounter+2] << 8;
						calc += buffer[ProgramCounter+3];						
						printf("\tdw $%x\n", calc);
						ProgramCounter += 4;
					}
			}
		}
 
		

	}
	// prevent memory leaks
	// free(data);
}



void rtype(void)
{

	switch(op.funct)
	{
		case 0x0: // shift left logical
			if(op.rs == 0 && op.imm == 0 && op.rt == 0 && op.shift == 0)  // effectively a nop
			{
				printf("nop");
			}
			else // an actual shift ( may not work )
			{
				printf("sll %s, %s, $%x", r[op.rd], r[op.rt],op.shift);
			}
			break;
		//case 0x1: // invalid instruction for mips 3
			
		case 0x2: // shift right logical
			printf("srl %s, %s, $%x", r[op.rd],r[op.rt],op.shift);
			break;
		case 0x3: // shift right arithmetic
			printf("sra %s, %s, $%x",r[op.rd],r[op.rt],op.shift);
			break;

		case 0x4: // shift left logical variable
			printf("sllv %s, %s, %s", r[op.rd],r[op.rt],r[op.rs]);
			break;
		case 0x6: // shift word right logical variable
			printf("srlv %s, %s, %s", r[op.rd],r[op.rt],r[op.rs]);			
			break;			
	
		case 0x8: // jump register (the bane of recursive disassemblers)
			printf("jr %s", r[op.rs]);
			break;
		case 0x9: // jump and link register(also bane of recursive disassembler)
			printf("jalr %s, %s", r[op.rd], r[op.rs]);
			break;
		case 0xd: // break(raise a breakpoint) // may not work
			printf("break $%x", ( (op.opcodefull & 0x3FFFFC0) >> 6) ); // (bits 6 to 25)
			break;
		case 0xf: // sync
			printf("sync $%x", op.shift);
			break;
		case 0x10: // mfhi (move from hi) 
			printf("mfhi %s", r[op.rd]);
			break;
		case 0x11: //mthi (move to hi)
			printf("mthi %s", r[op.rs]);
			break;
		case 0x12: // move lo from register
			printf("mflo %s", r[op.rd]);
			break;
		case 0x13: // move to lo register
			printf("mtlo %s", r[op.rs]);
			break;
		case 0x14: // double shift left logical variable
			printf("dsllv %s, %s, %s", r[op.rd],r[op.rt],r[op.rs]);
			break;			
		case 0x16: // doubleword shift right logical variable
			printf("dsrlv %s, %s, %s", r[op.rd],r[op.rt],r[op.rs]);
			break;
		case 0x17: // doubleword shift right arithmetic variable
			printf("dsrav %s, %s, %s", r[op.rd],r[op.rt],r[op.rs]);
			break;		
		case 0x1a: // divide
			arithreg2("div");
			break;
		case 0x1b: // divide (unsigned)
			arithreg2("divu");
			break;
		case 0x1d: // double word multiply unsigned
			arithreg2("dmultu");
			break;			
		case 0x1e: // doubleword divide
			arithreg2("ddiv");
			break;		
		case 0x1f: // doubleword divide unsigned
			arithreg2("ddivu");
			break;
		case 0x19: // multiply unsigned word
			arithreg2("multu");
			break;
		case 0x20: // add
			arithreg("add");
			break;
		case 0x21: // add unsigned
			arithreg("addu");
			break;
		case 0x22: // sub
			arithreg("sub");
			break;
		case 0x23: // subu
			arithreg("subu");
			break;
		case 0x24: // and
			arithreg("and");
			break;
		case 0x25: // or
			if(op.rs == 0)//resolve as move pseudo instruction
			{

				printf("move %s, %s",r[op.rd],r[op.rt]);
			}
			
			else if(op.rt == 0)
			{
				printf("move %s, %s",r[op.rd],r[op.rs]);
			}
			
			else
			{
				arithreg("or");
				
			}
			
			break;
		case 0x26: // xor
			arithreg("xor");
			break;
		case 0x27: // not or // could be not
			arithreg("nor");
			break;
		case 0x2a: // set on less than signed
			arithreg("slt");
			break;
		case 0x2b: // set on less than unsigned may be wrong
			arithreg("sltu");
			break;
		case 0x2d: // doubleword addition unsigned
			arithreg("daddu");
			break;		
		case 0x3c: // doubleword shift left logical plus 42
			printf("dsll32 %s, %s, $%x",r[op.rd],r[op.rt],op.shift);
			break;
			
		case 0x3f: // doubleword shift right arithmetic
			printf("dsra %s, %s, $%x",r[op.rd],r[op.rt],op.shift);
			break;

		default:
			printf("unknown r type instruction\n");
			printf("pc: %02x ", ProgramCounter);
			printf("instruction: %02x ", op.instruction);
			printf("rs: %02x ", op.rs);
			printf("rt: %02x ", op.rt);
			printf("rd: %02x ", op.rd);
			printf("shift: %02x ", op.shift);
			printf("funct: %02x ", op.funct);
			printf("\nopcodefull: %02x ", op.opcodefull);
			exit(1);
			break;

	}

	
}
