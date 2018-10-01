#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib.h"





// load an integer from 4 bytes
unsigned int bload(unsigned char * buffer, int pc) {
		unsigned int temp = buffer[pc] << 24;
		temp += buffer[pc+1] << 16;
		temp += buffer[pc+2] << 8;
		temp += buffer[pc+3];
		return temp;
}

// dump information out of the rom header and return start address
// return the program origin
unsigned int headerdump(unsigned char *buffer, unsigned int length) {
	puts("; HEADER DUMP: ");
	printf("; endianess: %x\n",buffer[0]);
	unsigned int calc = bload(buffer,0x04); // get the clockrate
	printf("; clockrate %x\n", calc);
	unsigned int org = bload(buffer,0x08);
	printf("; program counter: %x\n", org);
	calc = bload(buffer,0x0c);
	printf("; release address: %x\n", calc);
	calc = bload(buffer,0x10);
	printf("; crc1: %x\n", calc);
	calc = bload(buffer,0x14);
	printf("; crc2: %x\n", calc);
	printf("; internal name: ");
	for(int i = 32; i < 52; i++) { // print out the rom name
		putchar(buffer[i]);
	}
	putchar(10);
	calc = bload(buffer, 0x38);
	printf("; media format: %x\n", calc);
	unsigned short calc2 = buffer[0x3c] << 8;
	calc2 += buffer[0x3d];
	printf("; cartridge id: %x\n", calc2);
	printf("; country code: %x\n", buffer[0x3e]);
	printf("; version: %x\n", buffer[0x3f]);
	return org;
}

// byteswap a rom
void byteswap(unsigned char * buffer, unsigned int length) {
	int temp;
	for(int j = 0; j < length; j++) {
		temp = buffer[j]; // save it in a temp var
		// swap the bytes
		buffer[j] = buffer[j+1]; 
		buffer[j+1] = temp;
		j++;
	}
}





unsigned char *loadrom(Config *config) { // load the rom in and return the buffer to main
	
	

	
	

	// ------------------------------------------------
	// load in the file
	printf("; Loading program\n");

	// open the file
	FILE *fp;
	fp = fopen(config->filename, "rb");
	
	if(fp == NULL)
	{
		fprintf(stderr, "unable to open file %s", config->filename);
		exit(1); // exit with error if file not found
	}
	
	fseek(fp, 0, SEEK_END); // get file size and set it to len
	config->len = ftell(fp); // get length of file
	printf("; Program length is %d (bytes)\n", config->len );
	rewind(fp); // go to start of file

	
	
	unsigned char *buffer;
	if(config->flag == 1 || 2) { // if its a rom
		if(config->len < 0x1000) { printf("no boot code likely not a rom\n");  exit(1); } // catch error
		config->len -= 0x1000;
		unsigned char *buffer2 = calloc(sizeof(char), 0x1000); // store boot code
		fread(buffer2,1,0x1000,fp); // read our bootcode in
		// check if we need to byteswap our code and if so do it
		unsigned int temp = bload(buffer2,0);
		
		
		// now read the rom in
		buffer = calloc(sizeof(char), config->len + 4); // allocate and zero out array of file length
		dieifnull("unable to allocate space for rom",buffer);
		// inilitaze a buffer equal to len of file
		fread(buffer, 1, config->len, fp); // load file into array buffer
		fclose(fp); // close the file as it is no longer needed



		if(temp != 0x80371240) { // if not matching rom must be byteswapped
		
			puts("; byteswapping rom...");
			byteswap(buffer,config->len);
			byteswap(buffer2,0x1000);
		}
		
		config->org = headerdump(buffer2,config->len); // dump info out of the rom header
		FILE *out = fopen("boot.out", "wb+");
		if(out == NULL)
		{	
			printf("Failed to open a output file\n");
			exit(1); // exit with error if file not found
		}
		fwrite(buffer2,sizeof(char),0x1000,out); // write out our bootcode
		// clean up after outselves
		free(buffer2);
		fclose(out);

	}




	return buffer;
}
