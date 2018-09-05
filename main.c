#include<assert.h>
#include<readline/readline.h>
#include<readline/history.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<pwd.h>
#include<string.h>
#include<unistd.h>

#include"myshell.h"

#define maxn_dirname 1024
#define maxn_hostname 256

char current_dir[maxn_dirname];
char hostname[maxn_hostname];
struct passwd* pass_wd;
int display_prompt()
{
	if(!gethostname(hostname, sizeof(hostname))){
		printf("[myshell]%s@%s:", pass_wd->pw_name, hostname);
	}
}
int main(int argc, char* argv[])
{
	for (int i = 0; i < argc; i++) {
		assert(argv[i]); // specification
		printf("argv[%d] = %s\n", i, argv[i]);
 		} 
	//getcwd(current_dir, sizeof(current_dir));//初始化认为在~中,当用到cd的时候再更换看当前目录
	pass_wd = malloc(sizeof(struct passwd));
	pass_wd = getpwuid(getuid());
	strncpy(current_dir, pass_wd->pw_dir, sizeof(current_dir));

	//printf("pw_name:%s\npw_passwd:%s\npw_gecos:%s\npw_dir:%s\npw_shell:%s\n", pass_wd->pw_name, pass_wd->pw_passwd, pass_wd->pw_gecos, pass_wd->pw_dir, pass_wd->pw_shell);
	printf("current_dir:%s\n", current_dir);	
	return 0;
}
