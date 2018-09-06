#ifdef __MYSHELL_H__
#define __MYSHELL_H__

int do_prompt();
int read_command();

struct parsed_cmd
{
	int if_bg;
	int if_redirect;
	int if_pipe;
	int para_count;
	char para[64][16];
};
#endif
