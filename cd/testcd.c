#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>

int main()
{
	char *s = "..";
	char *pwd = malloc(64);
	if(!getcwd(pwd,64)){
		perror("getcwd error\n");
		return 1;
	}
	int ret = chdir(s);
	if(ret){
		printf("chdir error\n");
	}
	if(!getcwd(pwd, 64)){
		perror("getcwd error\n");
		return 1;
	}
	printf("pwd:%s\n", pwd);
	return 0;
}
