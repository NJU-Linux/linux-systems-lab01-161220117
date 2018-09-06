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

#define IF_BG 1
#define IN_DI 2
#define OUT_DI 4
#define OUT_DI_APPEND 8
#define IF_PIPE 16

struct parsed_cmd
{
	int flag;
	char* in_file;
	char* out_file;
	int para_count;
	char* para[64];
	char* command2;
	int command2_pos;
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
	p_cmd->flag = 0;
	p_cmd->para_count = 0;
	p_cmd->command2_pos = 0;
	p_cmd->in_file = NULL;
	p_cmd->out_file = NULL;
	for(int i = 0; i<64; i++){
		p_cmd->para[i] = NULL;
	}
	p_cmd->command2 = NULL;
	return;
}
int read_command()
{
	parsed_cmd_init();
	free(command);
	command = readline(prompt);
	//printf("%s\n", command);
	char *temp = malloc(16); temp = strtok(command, " ");
	while(temp != NULL){
		p_cmd->para[p_cmd->para_count] = malloc(sizeof(temp));
		strcpy(p_cmd->para[p_cmd->para_count++], temp);
		temp = strtok(NULL, " ");
	}
	printf("\033[46;37mthis is para:\033[0m\n");
	for(int i = 0; i<p_cmd->para_count; i++){
		printf("\033[46;37m%s\033[0m ", p_cmd->para[i]);
	}
	printf("\n");
	free(temp);
	for(int i = 0; i<p_cmd->para_count; i++){
		int len = strlen(p_cmd->para[i]);
		if(p_cmd->para[i][len-1] == '&'){
			p_cmd->flag |= IF_BG;
			p_cmd->para[i][len-1] = 0;
		}
		if(strstr(p_cmd->para[i], "|")){
			p_cmd->flag |= IF_PIPE;
			p_cmd->command2 = malloc(len);
			if(strlen(p_cmd->para[i]) == 1){
				p_cmd->command2 = malloc(strlen(p_cmd->para[i+1]));
				strcpy(p_cmd->command2, p_cmd->para[i+1]);
				p_cmd->command2_pos = i+1;
				printf("\033[46;37m%s\033[0m", p_cmd->command2);
			}
			else if(p_cmd->para[i][len-1] == '|'){
				p_cmd->para[i][len-1] = 0;
				p_cmd->command2 = malloc(strlen(p_cmd->para[i+1]));
				strcpy(p_cmd->command2, p_cmd->para[i+1]);
				p_cmd->command2_pos = i+1;
				printf("\033[46;37m%s\033[0m", p_cmd->command2);
			}
			else if(p_cmd->para[i][0] == '|'){
				p_cmd->command2 = malloc(strlen(p_cmd->para[i]));
				strcpy(p_cmd->command2, p_cmd->para[i]+1);
				p_cmd->command2_pos = i;
				printf("\033[46;37m%s\033[0m", p_cmd->command2);
			}
			else{
				char* tmp = strtok(p_cmd->para[i], "|");
				tmp = strtok(NULL, "|");
				p_cmd->command2 = malloc(strlen(tmp));
				strcpy(p_cmd->command2, tmp);
				printf("\033[46;37m%s\033[0m", p_cmd->command2);
			}
			printf("\n");
		}
		else if(!strcmp(p_cmd->para[i], "<<") || !strcmp(p_cmd->para[i], "<")){
			p_cmd->flag |= IN_DI;
			strcpy(p_cmd->in_file, p_cmd->para[i+1]);
		}
		else if(!strcmp(p_cmd->para[i], ">>")){
			p_cmd->flag |= OUT_DI_APPEND;
			strcpy(p_cmd->out_file, p_cmd->para[i+1]);
		}
		else if(!strcmp(p_cmd->para[i], ">")){
			p_cmd->flag |= OUT_DI;
			strcpy(p_cmd->out_file, p_cmd->para[i+1]);
		}
	}
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
