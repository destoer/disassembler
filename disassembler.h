#include "printing.h"
#include "mips.h"
#include "rom.h"
#include "lib.h"

#include <stdbool.h>
bool ioplist; // remove later debugging

// may be a better place to put this
#include <stdio.h>
#include <stdlib.h>

static const char type[3][6] = 
{
    "DATA",
    "CODE",
    "OTHER",
};

// add proper checks when loading in the db file as the programs crashes very easily
// when it isnt formatted properly


// scan for db files with conflicting data and code references
//



// look through the data references for destination addresses
// that have a conflicting type having both data and code refs
// as our system currently cannot reference code sections from data
void conflicting_ref(Config *config)
{


	unsigned int len2;
	unsigned int nref;


	char *str2 = malloc( strlen(config->filename) + 1 );
	strcpy(str2,config->filename);
	char *lastdot = strrchr(str2, '.');
	if(lastdot != NULL)
	{
			lastdot[1] = 'd';
			lastdot[2] = 'b';
			lastdot[3] = '\0';
	} 
	//printf("%s\n",str2);

				
		
	// load the db file in again
	FILE *fp2 = fopen(str2, "rb"); 		// load our data file in
	if(fp2 == NULL) 
	{
		fprintf(stderr, "unable to open file %s\n", str2);
		exit(1); // exit with error if file not found
	}
	
	
	// pull our length out of the file and malloc the buffers
	
	fseek(fp2,0,SEEK_END);
	len2 = ftell(fp2);
	fseek(fp2,-4, SEEK_END); // read the int of the end of the file
    	fread(&nref,1,sizeof(int),fp2); // get the number of references from the endo f the file
	rewind(fp2);
	Xref *xref = malloc(nref * sizeof(Xref));
	fread(xref,sizeof(Xref),nref,fp2);
	int conf = 0;
	printf("\ntotal refrences %d\n",nref);
	for(int i = 0; i < nref; i++)
	{
		unsigned int address = xref[i].daddress;
		unsigned int type = xref[i].type;
		for(int j = 0; j < nref; j++)
		{
			if(xref[j].daddress == address && xref[j].type != type)
			{
				conf++;
				puts("Conflicting type: ");
				printf("reference number %d\n",j+1);
		
				printf("destination %x\n", xref[j].daddress);
				printf("source %x\n", xref[j].saddress);
				//printf("type %s\n", xref[j].type);
			}
		}
	}
	printf("Total conflicting references %x",conf);			
}


/*---------------------------------------------------
* func: code_ref
* purpose: resolve references to parts of the program
* and print where they come from 
----------------------------------------------------*/
void code_ref(Config *config, char *address, unsigned char *buffer) 
{

	/*
	// hijack control for debugging 
	conflicting_ref(config);
	exit(1);
	*/

	unsigned int xlen; // len of x.dr
	unsigned int dlen; // len of x.dd
	unsigned int nref; // number of refs in x.dr
	printf("; Loading program\n");


	// open the file and convert for db file
	// old implementation is far more concise
	// but this wont crash with malformed files
	

	//printf("%s\n", argv[2]); // rom name is passed
	char *datastr = malloc( strlen(config->filename) + 1 );
	strcpy(datastr,config->filename);
	char *xrefstr = malloc(strlen(datastr)+1);
	strcpy(xrefstr,datastr);

	char *lastdot = strrchr(datastr, '.');
	if(lastdot != NULL)
	{
			lastdot[1] = 'd';
			lastdot[2] = 'd';
			lastdot[3] = '\0';
	} 

	lastdot = strchr(xrefstr, '.');
	if(lastdot != NULL)
	{
		lastdot[1] = 'd';
		lastdot[2] = 'r';
		lastdot[3] = '\0';
	}
	lastdot = NULL;



	FILE *fp1 = fopen(xrefstr,"rb");
	FILE *fp2 = fopen(datastr,"rb");


	if(fp1 == NULL)
	{
		fprintf(stderr, "Unable to open file: %s",xrefstr);
		exit(1);
	}

	if(fp2 == NULL)
	{
		fprintf(stderr, "Unable to open file: %s", datastr);
		exit(1);
	}

	fseek(fp1,0,SEEK_END);
	fseek(fp2,0,SEEK_END);

	xlen = ftell(fp1);
	dlen = ftell(fp2);

	rewind(fp1);
	rewind(fp2);

	Xref *xref = malloc(xlen);
	char *data = malloc(dlen);

	dieifnull("unable to malloc for files",data);
	dieifnull("unable to malloc for files",xref);

	nref = xlen / sizeof(Xref); // get number of references

	// not a complete struct read in
	if(xlen % sizeof(Xref) != 0)
	{
		fprintf(stderr, "Malformed xref file\n");
		exit(1);
	}

	fread(data, 1, dlen, fp2);
	fread(xref,sizeof(Xref),nref,fp1);

	free(datastr);
	free(xrefstr);

	datastr = NULL;
	xrefstr = NULL;

	fclose(fp1);
	fclose(fp2);



	unsigned int search = hex2int(address);
	if(search < config->org) { puts("Reference is less than program origin did you enter the right value?"); goto done; }
	if(search > config->org + config->len) { puts("Reference exceeds the program origin did you enter the right value?"); goto done; }
	printf("searching for references pointing to: %x\n", search);			// address we want to find data refs for
		

	// search for all valid matching references 
	// print the struct out
	// and disassemble the containing code block
	unsigned int x; // what reference number we are on
	unsigned int j = 0;
	printf("\ntotal refrences %d\n",nref);
	for(x = 0; x < nref; x++) 
	{	
		//printf("%x = %x?\n",xref[x].daddress,search);
		if(xref[x].daddress == search) 
		{
			printf("reference number %d\n",j+1);
		
			printf("destination %x\n", xref[x].daddress);
			printf("source %x\n", xref[x].saddress);
			printf("type %s\n", type[xref[x].type]);
			//data[0] = 3; // mark the start label
			//printf("looking at %x\n",xref[x].saddress-org);
			int i = xref[x].saddress-config->org-0x4;
			printf("[DEBUG] initial address %x: ",i+config->org);
			while( i >= 0) 
			{
				printf("address: %x = %x\n", i+config->org,data[i]);
				if(data[i] == 3) 
				{
					j++;
						
					printf("label found at %x\n", i + config->org); 			// pass to disassembly with initial point
											// and length ( end label - current one )
											// pass the loaded data file
											// and display the code blocks instructions
											// need a new flag value
					ProgramCounter = i;
					unsigned int temp = config->len;
					//printf("reference at : %x\n",xref[x].saddress);
					config->len = (xref[x].saddress - config->org);
					if(config->len  <= temp && config->len > 0)
			                {
                       
						disass_mips(buffer, data, config);
						config->len = temp;
						putchar(10);
					}

					break;
						
				}
				i -= 4;
					
			}
			printf("[DEBUG] End address: %x\n",i+config->org);
		}
	}
		
	puts("Done all matching references found.");

	// prevent memory leaks	
done:	  
	free(xref);
	free(data);
	return;

}



/*-----------------------------
* func: parse_args
* purpose: parse passed args for 
* flags and filename etc
------------------------------*/
Config *parse_args(int argc, char *argv[]) {

	if(argc != 3 && argc != 4) 
	{
		fprintf(stderr,"usage: %s <flag> <file>\n", argv[0]);
		exit(1);
	}
	
	// figure out what kind of disassembly we are doing
	int flag = -1;

	if(0 == strcmp(argv[1], "-b")) 
	{
		flag = 0; // disassemble a binary
	}

	else if(0 == strcmp(argv[1], "-r")) 
	{
		flag = 1; // disassemble a rom
	}

	else if(0 == strcmp(argv[1], "-rt")) // ignore the oplist
	{
		//printf("SHOULD BE FUCKING SET");
		//exit(1);
		flag = 1;
		ioplist = true; // just use an extern as we are lazy and its debugging code
	}
	
	// print reference information
	else if(0 == strcmp(argv[1], "-x"))  
	{ 
	
		if(argc != 4) { printf("invalid number of args\n"); exit(0); }
		flag = 2;
	}

	else  // invalid flag
	{
		fprintf(stderr, "error %s is not a known flag\n", argv[1]);
		exit(1);
	}

	
	
	// malloc and return our information in a Config struct
	
	Config *config = malloc(sizeof(Config)); // malloc our struct
	config->flag = flag; // set the flag
	// copy the filename out of argv
	config->filename = calloc(strlen(argv[2]) + 1, sizeof(char));
	strcpy(config->filename, argv[2]);
	// load the program origin as default for now may be changed in parseheader
	config->org = 0x80000400;
	config->len = -1; // indicate its not yet set
	return config;	

}



