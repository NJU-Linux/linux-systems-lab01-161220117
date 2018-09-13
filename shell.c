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
	char* line[64];	//用para_count和line实现multiple pipe的实现
	char* command1;
	char* para1[32]; int para1_cnt;
	char* command2;
	char* para2[32]; int para2_cnt;
};
char current_dir[maxn_dirname];
char full_cur_dir[maxn_dirname];
char hostname[maxn_hostname];
char prompt[maxn_prompt];
struct passwd* pass_wd;
char *command;
struct parsed_cmd* p_cmd;

int history_length;
int history_base;

int pipe_cnt;
int cmd_cnt;
int cmd_pos[64];

int do_prompt()
{
	if(!gethostname(hostname, sizeof(hostname))){
		sprintf(prompt, "\033[44;37m[myshell]\033[0m\033[1m\033[32m%s@%s\033[0m\033[0m:", pass_wd->pw_name, hostname);
	} 
	else{
		sprintf(prompt, "\033[44;37m[myshell]\033[0m\033[1m\033[32m;1m%s@???\033[0m\033[0m:", pass_wd->pw_name);
	}
	if( !strncmp(current_dir, pass_wd->pw_dir, strlen(pass_wd->pw_dir))){
		char temp_dir[maxn_dirname];
		temp_dir[0] = '~';
		strcat(temp_dir+1, current_dir+strlen(pass_wd->pw_dir));
		strcpy(current_dir, temp_dir);
	}
	sprintf(prompt+strlen(prompt), "\033[1m\033[34m%s\033[0m\033[0m$ ", current_dir);
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
void parse_command()
{
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
			p_cmd->para1[p_cmd->para1_cnt] = malloc(len);
			strcpy(p_cmd->para1[p_cmd->para1_cnt++], p_cmd->line[0]);
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
					p_cmd->para2[p_cmd->para2_cnt] = malloc(strlen(p_cmd->line[i+1]));
					strcpy(p_cmd->command2, p_cmd->line[++i]);
					strcpy(p_cmd->para2[p_cmd->para2_cnt++], p_cmd->line[i]);
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
	cmd_pos[cmd_cnt++] = 0;	//第一个指令从0开始
	for(int i = 0; i<p_cmd->para_count; i++){
		if(!strcmp(p_cmd->line[i], "|")){
			pipe_cnt++;
			p_cmd->line[i] = NULL;
			cmd_pos[cmd_cnt++] = i+1;
		}
	}
/*#ifdef DEBUG
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
#endif*/
	return;
}
int read_command()	//最后一个参数后面要NULL才可以,第一个参数要直接是指令才行
{
	parsed_cmd_init();
	free(command);
	command = readline(prompt);
	add_history(command);
	write_history(NULL);
	parse_command();
#ifdef DEBUG
	for(int i = 0; i<p_cmd->para_count; i++){
		printf("line %d:%s ", i, p_cmd->line[i]);
	}
	printf("\n");
#endif
	return 0;
}
void cd_command()
{
	char* dest_dir;
	if(p_cmd->para1[1]){
		/*!!!!!!!!!!!!!!!!!一定要初始化!!!!!!!!!!!!!!!!!*/
		char* home_dir = NULL;
		/*!!!!!!!!!!!!!!!!!一定要初始化!!!!!!!!!!!!!!!!!*/
		if(!strncmp(p_cmd->para1[1], "~", 1)){
			home_dir = malloc(strlen(pass_wd->pw_dir));
			strcpy(home_dir, pass_wd->pw_dir);
		}
		if(home_dir){
			dest_dir = malloc(strlen(p_cmd->para1[1]) + strlen(home_dir));
			strcpy(dest_dir, home_dir);
			strcat(dest_dir, p_cmd->para1[1]+1);
		}
		else{
			dest_dir = malloc(strlen(p_cmd->para1[1]));
			strcpy(dest_dir, p_cmd->para1[1]);
		}
		int ret = chdir(dest_dir);
		if(ret){
			printf("\033[41;37mplease check the diretory name you entered\033[0m\n");
		}
		if(!strncmp(current_dir, "~", 1)){
			if(strncmp(dest_dir, "~", 1) || strncmp(dest_dir, "/", 1))
				strcat(current_dir, dest_dir);
		}
		else{
			strcpy(current_dir, dest_dir);
		}
/*#ifdef DEBUG
		char* pwd = malloc(64);
		if(!getcwd(pwd, 64)){
			perror("getcwd error\n");
			exit(1);
		}
		printf("pwd:%s\n", pwd);
		free(pwd);
#endif*/
		if(home_dir)
			free(home_dir);
	}
	return;
}
void history_command()
{
	HIST_ENTRY** histr;
	histr = history_list();
	if(!strcmp(p_cmd->command1, "history")){
		if(!p_cmd->para1[1]){
			int i = 0;
			if(history_length > 100){
				i = history_length - 100;
			}
			else{
				i = 1;
			}
			while(histr[i] != NULL && i<= history_length){
				printf("%d: %s\n", i, histr[i]->line);
				i++;
			}
		}
		else{
			if(!strcmp(p_cmd->para1[1], "-c")){
				clear_history();
				if(write_history(NULL)){
					perror("write_history error\n");
				}
			}
			else if(p_cmd->para1[1][0] > 47 && p_cmd->para1[1][0] < 58){
				int cnt = atoi(p_cmd->para1[1]);
				int i = 0;
				if(history_length > cnt){
					i = history_length - cnt;
				}
				else{
					i = 1;	//从0还是从1
				}
				while(histr[i] != NULL && i <= history_length){
					printf("%d: %s\n", i, histr[i]->line);
					i++;
				}
			}
		}	
	}
	else{
		if(!strcmp(p_cmd->command1, "!!")){
			HIST_ENTRY* entry = histr[0];
			int i = 1;
			while(histr[i+1]!=NULL){
				entry = histr[i++];
			}
			char* his_cmd = entry->line;
			parsed_cmd_init();
			free(command);
			command = malloc(strlen(his_cmd));
			strcpy(command, his_cmd);
			parse_command();
		}
		else{
			char* search_entity = malloc(strlen(p_cmd->command1));
			strcpy(search_entity, p_cmd->command1+1);
			HIST_ENTRY* entry = histr[1];
			int i = 2;
			while(histr[i]!=NULL){
				entry = histr[i++];
			}
			int offset = where_history();
			history_set_pos(offset);
			history_search_prefix(search_entity, -1);
			entry = current_history();
			printf("current history:%s\n", entry->line);
		}
	}
	return;
}
void pipe_command()
{
	int status;
	int pipefd_odd[2];
	int pipefd_even[2];
	int in_fd = -1, out_fd = -1;
	pid_t pid;
	for(int i_cmd = 0; i_cmd<cmd_cnt; ){
		int if_odd = 0;
		if(i_cmd%2 == 0){
			pipe(pipefd_even);
		}
		else{
			if_odd = 1;
			pipe(pipefd_odd);
		}
		char* pcmd[64] = {}; int pcmd_cnt = 0;
		//int i_current_pos = cmd_pos[i_cmd];
		pid = fork();
		/*子进程*/
		if(pid == 0){
			//处理管道
			if(i_cmd == 0){	//第一个指令把1与输出关联
				dup2(pipefd_even[1], STDOUT_FILENO);
			}
			else if(i_cmd == cmd_cnt - 1){	//最后一个指令把0与输入关联
				if(cmd_cnt % 2 != 0){	//这里注意数的是一共有多少指令！！
					dup2(pipefd_odd[0], STDIN_FILENO);
				}
				else{
					dup2(pipefd_even[0], STDIN_FILENO);
				}
			}
			else{	//中间指令，如果为奇，把偶的读与输入关联，奇的写与输出关联
				if (if_odd){
					dup2(pipefd_even[0],STDIN_FILENO); 
					dup2(pipefd_odd[1],STDOUT_FILENO);
				}
				else{
					dup2(pipefd_odd[0], STDIN_FILENO);
					dup2(pipefd_even[1], STDOUT_FILENO);
				}
			}
			//处理重定向
			int i_current_pos = cmd_pos[i_cmd];
			while(p_cmd->line[i_current_pos]){
				if(!strcmp(p_cmd->line[i_current_pos], "<") || !strcmp(p_cmd->line[i_current_pos], "<<")){
					in_fd = open(p_cmd->line[++i_current_pos], O_RDONLY, 0666);
					dup2(in_fd, STDIN_FILENO);	
					break;	
				}
				else if(!strcmp(p_cmd->line[i_current_pos], ">")){
					out_fd = open(p_cmd->line[++i_current_pos], O_WRONLY|O_CREAT, 0666);
					dup2(out_fd, STDOUT_FILENO);
					break;
				}
				else if(!strcmp(p_cmd->line[i_current_pos], ">>")){
					out_fd = open(p_cmd->line[++i_current_pos], O_WRONLY|O_CREAT|O_TRUNC, 0666);
					dup2(out_fd, STDOUT_FILENO);
					break;
				}
				pcmd[pcmd_cnt] = malloc(strlen(p_cmd->line[i_current_pos]));
				strcpy(pcmd[pcmd_cnt++], p_cmd->line[i_current_pos]);
				i_current_pos++;
			}
			/*下面这段printf不能加！！！！一加了也相当于被算到stdout了！会使比如第二个指令是wc -l的指令多算！！！*/
			/*for(int i = 0; i<=pcmd_cnt; i++){
				printf("pcmd:%s ", pcmd[i]);
			}
			printf("\n");*/
			execvp(pcmd[0], pcmd);
		}
		//父进程
		else{
			if(in_fd != -1){
				close(in_fd);
			}
			if(out_fd != -1){
				close(out_fd);
			}
			if(i_cmd == 0){
				close(pipefd_even[1]);
			}
			else if(i_cmd == cmd_cnt - 1){
				if(cmd_cnt % 2 != 0){
					close(pipefd_odd[0]);
				}
				else{
					close(pipefd_even[0]);
				}
			}		
			else{
				if(i_cmd % 2 != 0){
					close(pipefd_even[0]);
					close(pipefd_odd[1]);
				}
				else{
					close(pipefd_odd[0]);
					close(pipefd_even[1]);
				}
			}			
			waitpid(pid, &status, 0);
			i_cmd++;
		}	
	}
}
void do_command()
{
	//int pipefd[2] = {0, 0};
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
	else if(!strcmp(p_cmd->command1, "history") || (!strncmp(p_cmd->command1, "!", 1) && strcmp(p_cmd->command1, "!!"))){
		history_command();
	}
	else if(!strcmp(p_cmd->command1, "pwd")){
		printf("%s\n", full_cur_dir);
	}
	else{	
		if(!strcmp(p_cmd->command1, "!!")){
			history_command();
		}
		if(p_cmd->flag & IF_PIPE){
			pipe_command();
		}
		else{
			pid_t pid = fork();
			/*子进程*/
			if(pid == 0){
				if(p_cmd->flag & OUT_DI){
						out_fd = open(p_cmd->out_file, O_WRONLY|O_CREAT, 0666);
					dup2(out_fd, STDOUT_FILENO);
				}
				else if(p_cmd->flag & OUT_DI_APPEND){
					out_fd = open(p_cmd->out_file, O_WRONLY|O_CREAT|O_TRUNC, 0666);
					dup2(out_fd, STDOUT_FILENO);
				}
				else if(p_cmd->flag & IN_DI){
					//in_fd = open(p_cmd->in_file, O_CREATE|O_RDONLY)
					in_fd = open(p_cmd->in_file, O_RDONLY|O_CREAT, 0666);
					dup2(in_fd, STDIN_FILENO);
				}
				execvp(p_cmd->command1, p_cmd->para1);
			}
			/*父进程*/
			else{
				if(p_cmd->flag & IF_BG){
					printf("[child pid]:%d\n", pid);	
				}
				else{
					if(out_fd != -1){
						close(out_fd);
					}
					if(in_fd != -1){
						close(in_fd);
					}
					waitpid(pid, &status, 0);
				}
			}
		}
	}
}
int main(int argc, char* argv[])
{
	for (int i = 0; i < argc; i++) {
		assert(argv[i]); // specification
		printf("argv[%d] = %s\n", i, argv[i]);
 		} 
	//初始化认为在~中,当用到cd的时候再更换看当前目录
	p_cmd = malloc(sizeof(struct parsed_cmd));
	pass_wd = malloc(sizeof(struct passwd));
	pass_wd = getpwuid(getuid());
	strncpy(current_dir, pass_wd->pw_dir, sizeof(current_dir));
	chdir(pass_wd->pw_dir);
	read_history(NULL);
	while(1){	
		pipe_cnt = 0; cmd_cnt = 0;
		for(int i = 0; i<64; i++){
			cmd_pos[i] = 0;
		}
		do_prompt();
		strcpy(full_cur_dir, pass_wd->pw_dir);
		strcat(full_cur_dir, "/");
		if(strcmp(current_dir, "~")){
			strcat(full_cur_dir, current_dir);
		}
		read_command();
		do_command();	
	}

	return 0;
}
