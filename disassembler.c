#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disassembler.h"



int main(int argc, char *argv[])
{

	Config *config = parse_args(argc, argv); // parse args and find out what we are doing
	

	
	unsigned char *buffer = loadrom(config); // load the rom based on the config
	if(config->len == -1) { fprintf(stderr, "Error reading rom\n"); exit(1); }

	// if -x flag passed do code refs
	if(config->flag == 2) { 
		code_ref(config, argv[3], buffer); 
	}

	// disassemble the rom if not doing data refs
	else if (config->flag == 1) {
		char *data = traverse(buffer, config);
		disass_mips(buffer, data, config);
	}

	else if (config->flag == 0) {
		char *data = malloc(sizeof(char) * config->len); 
		memset(data, 1, config->len);	// flag all of it as code (cause lazy)
		disass_mips(buffer, data, config);
	}
	// prevent memory leaks
	free(config->filename);
	free(config);
	free(buffer);

	return 0;

}