#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

int main()
{
	char *s = "/home/tsm";
	char *pwd = malloc(64);
	if(!getcwd(pwd,64)){
		perror("getcwd error");
		return 1;
	}
	int ret = chdir(s);
	if(ret){
		printf("chdir error\n");
	}
	return 0;
}
