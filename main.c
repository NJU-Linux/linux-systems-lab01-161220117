#include<stdio.h>
#include<assert.h>
#include<readline/readline.h>
#include <readline/history.h>

int main(int argc, char* argv[])
{
	for (int i = 0; i < argc; i++) {
		assert(argv[i]); // specification
		printf("argv[%d] = %s\n", i, argv[i]);
		}
    char a[] = "please input a string\n";
	char *input = readline(a); 
	printf("%s\n",input);
	return 0;
}
