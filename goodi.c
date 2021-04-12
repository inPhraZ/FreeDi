#include <stdio.h>
#include <stdlib.h>

void print_usage(int exit_code)
{
	printf("Usage:  goodi	[word]\n");
	exit(exit_code);
}

int main(int argc, char *argv[])
{
	if (argc == 1)
		print_usage(1);
	return 0;
}
