#include<assert.h>
#include<readline/readline.h>
#include<readline/history.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/types.h>
#include<pwd.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>

#include"myshell.h"

#define DEBUG

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
	char* line[64];
	char* command1;
	char* para1[32]; int para1_cnt;
	char* command2;
	char* para2[32]; int para2_cnt;
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
	p_cmd->in_file = NULL;
	p_cmd->out_file = NULL;
	for(int i = 0; i<64; i++){
		p_cmd->line[i] = NULL;
	}
	p_cmd->command1 = NULL;
	p_cmd->command2 = NULL;
	for(int i = 0; i<32; i++){
		p_cmd->para1[i] = NULL;
		p_cmd->para2[i] = NULL;
	}
	p_cmd->para1_cnt = 0; p_cmd->para2_cnt = 0;
	return;
}
int read_command()
{
	parsed_cmd_init();
	free(command);
	command = readline(prompt);
	char *temp = malloc(16); temp = strtok(command, " ");
	while(temp != NULL){
		p_cmd->line[p_cmd->para_count] = malloc(sizeof(temp));
		strcpy(p_cmd->line[p_cmd->para_count++], temp);
		temp = strtok(NULL, " ");
	}
	/*printf("\033[46;37mthis is para:\033[0m\n");
	for(int i = 0; i<p_cmd->para_count; i++){
		printf("\033[46;37m%s\033[0m ", p_cmd->para[i]);
	}
	printf("\n");*/
	free(temp);
	int which_cmd = 1;
	int is_para = 1;
	for(int i = 0; i<p_cmd->para_count; i++){
		int len = strlen(p_cmd->line[i]);
		if(!i){
			p_cmd->command1 = malloc(len);
			if(p_cmd->line[0][len-1] == '&' && is_para){
				p_cmd->flag |= IF_BG;
				p_cmd->line[0][len-1] = 0;
			}
			strcpy(p_cmd->command1, p_cmd->line[0]);
		}
		else{
			if(p_cmd->line[i][len-1] == '&'){
				p_cmd->flag |= IF_BG;
				p_cmd->line[i][len-1] = 0;
				if(which_cmd == 1){
					p_cmd->para1[p_cmd->para1_cnt] = malloc(len);
					strcpy(p_cmd->para1[p_cmd->para1_cnt++], p_cmd->line[i]);
				}
				else{
					p_cmd->para2[p_cmd->para2_cnt] = malloc(len);
					strcpy(p_cmd->para2[p_cmd->para2_cnt++], p_cmd->line[i]);
				}
			}
			if(strstr(p_cmd->line[i], "|")){
				p_cmd->flag |= IF_PIPE;
				p_cmd->command2 = malloc(len);
				if(strlen(p_cmd->line[i]) == 1){
					p_cmd->command2 = malloc(strlen(p_cmd->line[i+1]));
					strcpy(p_cmd->command2, p_cmd->line[++i]);
					which_cmd = 2;
				}
				else if(p_cmd->line[i][len-1] == '|'){
					p_cmd->line[i][len-1] = 0;
					p_cmd->command2 = malloc(strlen(p_cmd->line[i+1]));
					strcpy(p_cmd->command2, p_cmd->line[++i]);
					which_cmd = 2;
				}
				else if(p_cmd->line[i][0] == '|'){
					p_cmd->command2 = malloc(strlen(p_cmd->line[i]));
					strcpy(p_cmd->command2, p_cmd->line[i]+1);
					which_cmd = 2;
				}
				else{
					char* tmp = strtok(p_cmd->line[i], "|");
					p_cmd->para1[p_cmd->para1_cnt] = malloc(strlen(tmp));
					strcpy(p_cmd->para1[p_cmd->para1_cnt++], tmp);
					tmp = strtok(NULL, "|");
					p_cmd->command2 = malloc(strlen(tmp));
					strcpy(p_cmd->command2, tmp);
					which_cmd = 2;
				}
			}
			else if(!strcmp(p_cmd->line[i], "<<") || !strcmp(p_cmd->line[i], "<")){
				p_cmd->flag |= IN_DI;
				is_para = 0;
				p_cmd->in_file = malloc(strlen(p_cmd->line[i+1]));
				strcpy(p_cmd->in_file, p_cmd->line[++i]);
			}
			else if(!strcmp(p_cmd->line[i], ">>")){
				p_cmd->flag |= OUT_DI_APPEND;
				is_para = 0;
				p_cmd->out_file = malloc(strlen(p_cmd->line[i+1]));
				strcpy(p_cmd->out_file, p_cmd->line[++i]);
			}
			else if(!strcmp(p_cmd->line[i], ">")){
				p_cmd->flag |= OUT_DI;
				is_para = 0;
				p_cmd->out_file = malloc(strlen(p_cmd->line[i+1]));
				strcpy(p_cmd->out_file, p_cmd->line[++i]);
			}
			else{
				if(is_para && which_cmd == 1){
					p_cmd->para1[p_cmd->para1_cnt] = malloc(len);
					strcpy(p_cmd->para1[p_cmd->para1_cnt++], p_cmd->line[i]);
				}
				else if(is_para && which_cmd == 2){
					p_cmd->para2[p_cmd->para2_cnt] = malloc(len);
					strcpy(p_cmd->para2[p_cmd->para2_cnt++], p_cmd->line[i]);
				}
			}
		}
	}
#ifdef DEBUG
	printf("this is cmd1\n");
	printf("%s ", p_cmd->command1);
	for(int i = 0; i<p_cmd->para1_cnt; i++){
		printf("%s ", p_cmd->para1[i]);
	}
	printf("\n");
	if(p_cmd->para2_cnt){
		printf("this is cmd2\n");
		printf("%s ", p_cmd->command2);
		for(int i = 0; i<p_cmd->para2_cnt; i++){
			printf("%s ", p_cmd->para2[i]);
		}
		printf("\n");
	}
	if(p_cmd->in_file){
		printf("in file :%s\n", p_cmd->in_file);
	}
	else if(p_cmd->out_file){
		printf("out file:%s\n", p_cmd->out_file);
	}
#endif
	return 0;
}
void cd_command()
{
	char* dest_dir;
	if(p_cmd->para1[0]){
		
		dest_dir = malloc(strlen(p_cmd->para1[0]));
		strcpy(dest_dir, p_cmd->para1[0]);
		printf("para1 in cd:%s\n", p_cmd->para1[0]);
		int ret = chdir(dest_dir);
		if(ret){
			printf("\033[41;37mchange directory is failed!\033[0m\n");
		}
	}
	return;
}
void do_command()
{
	int pipefd[2] = {0, 0};
	int in_fd = -1; int out_fd = -1;
	int status;
	if(!strcmp(p_cmd->command1, "exit")){
		exit(0);
	}
	else if(!strcmp(p_cmd->command1, "help")){
		printf("\033[42;37mVersion:0.0.1\033[0m");
		printf("\033[42;37mthis is my simple shell\033[0m\n");
		printf("\033[42;37mnow support simple cmd, redirection in one process, pipe between two processes\033[0m\n");
	}
	else if(!strcmp(p_cmd->command1, "cd")){
		cd_command();	
	}
	else{	
		if(p_cmd->flag & IF_PIPE){	//创建管道连接两个进程
			if(pipe(pipefd)){
				printf("\033[41;37mERROR when pipe!\033[0m\n");
				exit(1);
			}
		}
		pid_t pid = fork();
		/*子进程*/
		if(pid == 0){
			if(p_cmd->flag & IF_PIPE){
				//子进程command1关读口0把写口与输出关联
				close(pipefd[0]);
				dup2(pipefd[1], STDOUT_FILENO);
			}
			else if(p_cmd->flag & OUT_DI){
					out_fd = open(p_cmd->out_file, O_WRONLY|O_CREAT, 0666);
				dup2(out_fd, STDOUT_FILENO);
			}
			else if(p_cmd->flag & OUT_DI_APPEND){
				out_fd = open(p_cmd->out_file, O_WRONLY|O_CREAT|O_TRUNC, 0666);
				dup2(out_fd, STDOUT_FILENO);
			}
			else if(p_cmd->flag & IN_DI){
				//in_fd = open(p_cmd->in_file, O_CREATE|O_RDONLY)
				in_fd = open(p_cmd->in_file, O_RDONLY, 0666);
				dup2(in_fd, STDIN_FILENO);
			}
			execvp(p_cmd->command1, p_cmd->para1);
		}
		/*父进程*/
		else{
			if(p_cmd->flag & IF_PIPE){
				//子进程2command2关写口1把读口与输出关联
				pid_t pid2 = fork();
				if(pid2 == 0){
					close(pipefd[1]);
					dup2(pipefd[0], STDIN_FILENO);
					execvp(p_cmd->command2, p_cmd->para2);
				}
				else{
					waitpid(pid2, &status, 0);
				}
			}
			/*else if(p_cmd->flag & IF_BG){
				
			}*/
			else{
				waitpid(pid, &status, 0);
			}
			exit(0);	
		}
	}
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
	do_command();
	return 0;
}