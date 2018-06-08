#pragma once
void disass_mips(unsigned char *buffer, int length, int flag,unsigned int org);
void rtype(void);
void traverse(unsigned char *buffer, int length, int flag, unsigned int org);
extern unsigned int ProgramCounter; // current instruction we are decoding
extern char *data;

// defs for ref type type whether it is from code or data

#define DATA 0
#define CODE 1
#define UNKNOWN 3


// used for storing references with to code and data
// array will be realloced for the references
// every  time a label is marked this will happen
// destination  address  passed as arg to flag -x to print the reference information ( address of the label )
// the surrounding code block from where it is referenced will be printed at this point unless it s from data
// this will require back tracing the label and saving of the data array after finished disassembly ( or an unknown opcode error )
// all matching entries will be printed, this will require searching of the entire array
// unless each reference is stored in the struct as a point to an array and realloced as required with
// a reference counter being incremented ( get working later )

// referencing instruction in each block should be clearly marked when printed 


// get this working with jump tables later but leave it for now

typedef struct
{
	unsigned int type; // what sort of reference is it data or code?
	unsigned int daddress; // destination address ( what is being referenced )
	unsigned int saddress; // source address ( where the reference comes from )
} Xref;


typedef struct {

	unsigned int opcodefull;  // opcode
	unsigned int instruction; //instruction bits
	unsigned int rs; // register source
	unsigned int rt; // register target
	unsigned int rd; // register data
	unsigned int shift; // register shift
	unsigned int funct;  // comtrol codes
	unsigned short imm; // immediate
	unsigned int address; // jump address
	unsigned int jumpfinal; // calculate final jump address
	unsigned int cond; // floating point conditon
	short simm; // signed imm
	
	//------------------- 
	// coprocssor stuff

	unsigned char pad;
	unsigned char sel;
	
} Opcode;

extern Opcode op; 
extern Opcode op2; 


extern void do_op(Opcode *op, unsigned char *buffer, unsigned int address); // load a struct with information about the opcode
extern void load(char *instruction); // same syntax for a store
extern void branch(char *instruction, unsigned int org);
extern void arithreg2(char *instruction); // divu rs, rt
extern void mkc(unsigned int address,int num,int length); // mark code at current pc (replace when we use array loop for jump tables)
extern void arithreg(char *instruction); // arithmetic instructions with registers ins, rd ,rs, rt
extern void arithimm(char *instruction); // arithmetic instructions with immediate ( ins, rt, rs, imm
extern void ffmt(char *instruction, int number) ;// print a floating instruction ( slightly slower but cleaner code )
