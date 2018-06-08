#include <stdio.h>

void hexdump(unsigned char *buffer,int len)
{
	// needs improving with a proper printing libary because faster just to print hte hex then print the ascii at a ceartin amoutn away in frambuffer
	// but printf doesent allow for functionalitly like that ncurses does
	printf("hex dumping contents\n");
	// do first part of initial line so it prints properly
	int i = 0;
	int j = 0;
	printf("%07X: ", i); 
	for(i = 0; i < 16; i++)
	{
		printf("%02x%02x ", buffer[i],buffer[i+1]);
		i++;
	}

	for(i = 16; i < len; i++) // hex dumper add ascii conversion here
	{
		if(i % 16 == 0) 
		{
			
			// if within the printable ascii range
			for(j = i - 16; j < i; j++)
			{
				if(buffer[j] >= 32 && buffer[j] < 127) { putchar(buffer[j]); }
				else { putchar('.'); }
			}
			putchar(10); 
			printf("%07X: ", i); 
		}
		printf("%02x%02x ", buffer[i],buffer[i+1]);
		i++;
		
	}
	// print the last line
	for(; j < i; j++)
	{
		if(buffer[j] >= 32 && buffer[j] < 127) { putchar(buffer[j]); }
		else { putchar('.'); }
	}
	putchar(10);
}

// obselete kept in for older versions

/*void printreg(unsigned int reg) // print out a register based on bit value
{
	
	switch(reg)
	{
		case 0: printf("r0"); break;
		case 1: printf("at"); break;
		case 2: printf("v0"); break;
		case 3: printf("v1"); break;
		case 4: printf("a0"); break;
		case 5: printf("a1"); break;
		case 6: printf("a2"); break;
		case 7: printf("a3"); break;
		case 8: printf("t0"); break;
		case 9: printf("t1"); break;
		case 10: printf("t2"); break;
		case 11: printf("t3"); break;
		case 12: printf("t4"); break;
		case 13: printf("t5"); break;
		case 14: printf("t6"); break;
		case 15: printf("t7"); break;
		case 16: printf("s0"); break;
		case 17: printf("s1"); break;
		case 18: printf("s2"); break;
		case 19: printf("s3"); break; 
		case 20: printf("s4"); break;
		case 21: printf("s5"); break;
		case 22: printf("s6"); break;
		case 23: printf("s7"); break;
		case 24: printf("t8"); break;
		case 25: printf("t9"); break;
		case 26: printf("k0"); break;
		case 27: printf("k1"); break;
		case 28: printf("gp"); break;
		case 29: printf("sp"); break;
		case 30: printf("s8"); break;
		case 31: printf("ra"); break;
	}

} */