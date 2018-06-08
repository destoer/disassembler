#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disassembler.h"
FILE *out;

// add to header
int org = 0x80000400;
unsigned int nref; // number of references in the code
Xref *xref;
// load an integer from 4 bytes
unsigned int bload(unsigned char * buffer, int pc)
{
		unsigned int temp = buffer[pc] << 24;
		temp += buffer[pc+1] << 16;
		temp += buffer[pc+2] << 8;
		temp += buffer[pc+3];
		
		return temp;
}




// byteswap a rom
void byteswap(unsigned char * buffer, unsigned int length)
{
	int temp;
	for(int j = 0; j < length; j++)
	{
		temp = buffer[j]; // save it in a temp var
		// swap the bytes
		buffer[j] = buffer[j+1]; 
		buffer[j+1] = temp;
		j++;
	}
}


// dump information out of the rom header and return start address
void headerdump(unsigned char *buffer, unsigned int length)
{
	puts("; HEADER DUMP: ");
	printf("; endianess: %x\n",buffer[0]);
	unsigned int calc = bload(buffer,0x04); // get the clockrate
	printf("; clockrate %x\n", calc);
	org = bload(buffer,0x08);
	printf("; program counter: %x\n", org);
	calc = bload(buffer,0x0c);
	printf("; release address: %x\n", calc);
	calc = bload(buffer,0x10);
	printf("; crc1: %x\n", calc);
	calc = bload(buffer,0x14);
	printf("; crc2: %x\n", calc);
	printf("; internal name: ");
	for(int i = 32; i < 52; i++) // print out the rom name
	{
		putchar(buffer[i]);
	}
	putchar(10);
	calc = bload(buffer, 0x38);
	printf("; media format: %x\n", calc);
	unsigned short calc2 = buffer[0x3c] << 8;
	calc2 += 0x3d;
	printf("; cartridge id: %x\n", calc2);
	printf("; country code: %x\n", buffer[0x3e]);
	printf("; version: %x\n", buffer[0x3f]);

}




int main(int argc, char *argv[])
{
	//-------------------------------------------------
	// check our args
	if(argc != 3 && argc != 4)
	{
		fprintf(stderr,"usage: %s <flag> <file>\n", argv[0]);
		exit(1);
	}

	int flag = -1;

	if(0 == strcmp(argv[1], "-b"))
	{
		flag = 0; // disassemble a binary
	}

	else if(0 == strcmp(argv[1], "-r"))
	{
		flag = 1; // disassemble a rom
	}
	
	
	else if(0 == strcmp(argv[1], "-x")) // print reference information
	{
		if(argc != 4) { printf("invalid number of args\n"); exit(0); }
		flag = 2;
		unsigned int len; // length of  rom file
		unsigned int len2; // length of our database file
		printf("; Loading program\n");
		unsigned char *buffer;
		// open the file and convert for db file

		//printf("%s\n", argv[2]); // rom name is passed
		char *str2 = malloc( strlen(argv[2]) );
		strcpy(str2,argv[2]);
		char *lastdot = strrchr(str2, '.');
		if(lastdot != NULL)
		{
			lastdot[1] = 'd';
			lastdot[2] = 'b';
			lastdot[3] = '\0';
		} 
		//printf("%s\n",str2);
		// load the rom in we need the header parsing code here FIX IMMEDIATLY 
		FILE *fp;
		fp = fopen(argv[2], "rb");
		
		if(fp == NULL)
		{
			fprintf(stderr, "unable to open file %s", argv[2]);
			exit(1); // exit with error if file not found
		}

		fseek(fp, 0, SEEK_END); // get file size and set it to len
		len = ftell(fp);
		printf("; Program length is %d (bytes)\n", len );
		rewind(fp); // go to start of file
		buffer = calloc(sizeof(char), len); // allocate and zero out array of file length
		
		// do our header stuff
		if(len < 0x1000) { printf("no boot code likely not a rom\n"); return 1; } // catch error
		len = len - 0x1000;
		unsigned char *buffer2 = calloc(sizeof(char), 0x1000); // store boot code
		buffer = calloc(sizeof(char), len); // allocate and zero out array of file length
		fread(buffer2,1,0x1000,fp); // read our bootcode in
		fread(buffer,1,len, fp);	// read the rest in
		// check if we need to byteswap our code and if so do it
		unsigned int temp = buffer2[0] << 24;
		temp += buffer2[1] << 16;
		temp += buffer2[2] << 8;
		temp += buffer2[3];
		
		if(temp != 0x80371240) // if not matching rom must be byteswapped
		{
			puts("; byteswapping rom...");
			byteswap(buffer,len);
			byteswap(buffer2,0x1000);
		}
		headerdump(buffer2,len); // dump info out of the rom header
		
		free(buffer2); // dont write it out this time and just free it as we dont need it

				
		
		// load the db file in again
		FILE *fp2 = fopen(str2, "rb"); 		// load our data file in
		if(fp2 == NULL)
		{
			fprintf(stderr, "unable to open file %s\n", str2);
			exit(1); // exit with error if file not found
		}		
		fseek(fp2,0,SEEK_END);
		len2 = ftell(fp2);
		rewind(fp2);
		data =  malloc( len );
		fread(data, 1, len, fp2);			// this will store our data info etc
		Xref *xref; 					// ref array
		len2 = len2 - len; 				// find how much our data ref takes up
		xref = malloc(len2);				// malloc space for our data refs
		nref = len2 / sizeof(Xref);			// find how many entries
		fread(xref,sizeof(Xref),nref,fp2);		// load our reference data in
		fclose(fp); fclose(fp2);			// close our files are we are done with them
		unsigned int search = (unsigned int)strtol(argv[3], NULL, 16);		// convert hex string to int
		printf("searching for references pointing to: %x\n", search);			// address we want to find data refs for
		

		// search for all valid matching references 
		// print the struct out
		// and disassemble the containing code block
		unsigned int x; // what reference number we are on
		unsigned int j;
		printf("\ntotal refrences %d\n",nref);
		for(x = 0; x < nref; x++)
		{	
			//printf("%x = %x?\n",xref[x].daddress,search);
			if(xref[x].daddress == search)
			{
				printf("reference number %d\n",j+1);
		
				printf("destination %x\n", xref[x].daddress);
				printf("source %x\n", xref[x].saddress);
				printf("type %x\n", xref[x].type);
				data[0] = 3; // mark the start label
				//printf("looking at %x\n",xref[x].saddress-org);
				int i = xref[x].saddress-org-0x4;
				while( i >= 0)
				{
					//printf("address: %x\n", i);
					if(data[i] == 3)
					{
						j++;
						
						//printf("label found at %x\n", i + org); 			// pass to disassembly with initial point
												// and length ( end label - current one )
												// pass the loaded data file
												// and display the code blocks instructions
												// need a new flag value
						ProgramCounter = i;
						//printf("reference at : %x\n",xref[x].saddress);
						disass_mips(buffer, (xref[x].saddress-org), flag,org);
						putchar(10);
						break;
						
					}
					i -= 4;
					
				}
			} 
		}
		

							// prevent memory leaks		  
		free(xref);
		free(data);
		free(buffer);
		exit(0); // never goes beyond this point
	}

	else  // not a valid flag
	{
		fprintf(stderr, "error %s is not a known flag\n", argv[1]);
		exit(1);
	}

	
	

	// ------------------------------------------------
	// load in the file
	unsigned int len; // length of file
	printf("; Loading program\n");

	// open the file
	FILE *fp;
	fp = fopen(argv[2], "rb");
	
	if(fp == NULL)
	{
		fprintf(stderr, "unable to open file %s", argv[2]);
		exit(1); // exit with error if file not found
	}
	
	fseek(fp, 0, SEEK_END); // get file size and set it to len
	len = ftell(fp);
	printf("; Program length is %d (bytes)\n", len );
	rewind(fp); // go to start of file

	
	
	unsigned char *buffer;
	if(flag == 0)
	{
		buffer = calloc(sizeof(char), len); // allocate and zero out array of file length
		// inilitaze a buffer equal to len of file
		fread(buffer, 1, len, fp); // load file into array buffer
		fclose(fp); // close the file as it is no longer needed


		
		

		// hex dump the contents
		// hexdump(buffer,len);

	}

	else // is a rom 
	{
		if(len < 0x1000) { printf("no boot code likely not a rom\n"); return 1; } // catch error
		len = len - 0x1000;
		unsigned char *buffer2 = calloc(sizeof(char), 0x1000); // store boot code
		buffer = calloc(sizeof(char), len); // allocate and zero out array of file length
		fread(buffer2,1,0x1000,fp); // read our bootcode in
		fread(buffer,1,len, fp);	// read the rest in
		// check if we need to byteswap our code and if so do it
		unsigned int temp = buffer2[0] << 24;
		temp += buffer2[1] << 16;
		temp += buffer2[2] << 8;
		temp += buffer2[3];
		
		if(temp != 0x80371240) // if not matching rom must be byteswapped
		{
			puts("; byteswapping rom...");
			byteswap(buffer,len);
			byteswap(buffer2,0x1000);
		}
		headerdump(buffer2,len); // dump info out of the rom header
		
		FILE *out = fopen("boot.out", "wb+");
		if(out == NULL)
		{	
			printf("Failed to open a output file\n");
			exit(1); // exit with error if file not found
		}
		fwrite(buffer2,sizeof(char),0x1000,out); // write out our bootcode
		fclose(fp);
		fclose(out);

	}

	// disassemble the rom
	
	disass_mips(buffer, len, flag,org);


	// save the file
	char *str = malloc( strlen(argv[2]) );
	strcpy(str,argv[2]);
	char *lastdot = strrchr(str, '.');
	if(lastdot != NULL)
	{
		lastdot[1] = 'd';
		lastdot[2] = 'b';
		lastdot[3] = '\0';
	} 


	out = fopen(str, "wb+");
	if(out == NULL)
	{	
		printf("Failed to open a output file\n");
		exit(1); // exit with error if file not found
	}
	printf("%s\n",str);
	fwrite(data,sizeof(char),len,out);
	printf("%d %d\n",len*sizeof(char),nref * sizeof(Xref));
	fwrite(xref,sizeof(Xref),nref,out);
	fclose(out);
	free(data);
	free(buffer);
	return 0;

}
