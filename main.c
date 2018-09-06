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
#define maxn_command 1024
#define maxn_prompt 2048

struct parsed_cmd
{
	int if_bg;
	int if_redirect;
	int if_pipe;
	int para_count;
	char* para[64];
};
char current_dir[maxn_dirname];
char hostname[maxn_hostname];
char prompt[maxn_prompt];
struct passwd* pass_wd;
char *command;
struct parsed_cmd* p_cmd;

int do_prompt()
{
	if(!gethostname(hostname, sizeof(hostname))){
		sprintf(prompt, "[myshell]%s@%s:", pass_wd->pw_name, hostname);
	} 
	else{
		sprintf(prompt, "[myshell]%s@???:", pass_wd->pw_name);
	}
	if( !strncmp(current_dir, pass_wd->pw_dir, strlen(pass_wd->pw_dir))){
		char temp_dir[maxn_dirname];
		temp_dir[0] = '~';
		strcat(temp_dir+1, current_dir+strlen(pass_wd->pw_dir));
		strcpy(current_dir, temp_dir);
	}
	sprintf(prompt+strlen(prompt), "%s$ ", current_dir);
	//printf("%s", prompt);
	return 0;
}
void parsed_cmd_init()
{
	p_cmd->if_bg = 0;
	p_cmd->if_redirect = 0;
	p_cmd->if_pipe = 0;
	p_cmd->para_count = 0;
	return;
}
int read_command()
{
	parsed_cmd_init();
	free(command);
	command = readline(prompt);
	//printf("%s\n", command);
	char *temp = malloc(16); temp = strtok(command, " ");
	p_cmd->para[0] = malloc(sizeof(temp));
	printf("hahah\n");
	strcpy(p_cmd->para[0], temp);
	while(temp != NULL){
		p_cmd->para_count++;
		temp =strtok(NULL, " ");
		p_cmd->para[p_cmd->para_count] = malloc(sizeof(temp));
		strcpy(p_cmd->para[p_cmd->para_count], temp);
	}	
	for(int i = 0; i<p_cmd->para_count; i++){
		printf("%s ", p_cmd->para[i]);
	}
	printf("\n");
	free(temp);
	return 0;
}
int main(int argc, char* argv[])
{
	for (int i = 0; i < argc; i++) {
		assert(argv[i]); // specification
		printf("argv[%d] = %s\n", i, argv[i]);
 		} 
	//getcwd(current_dir, sizeof(current_dir));//初始化认为在~中,当用到cd的时候再更换看当前目录
	p_cmd = malloc(sizeof(struct parsed_cmd));
	pass_wd = malloc(sizeof(struct passwd));
	pass_wd = getpwuid(getuid());
	strncpy(current_dir, pass_wd->pw_dir, sizeof(current_dir));

	do_prompt();
	read_command();
	return 0;
}
