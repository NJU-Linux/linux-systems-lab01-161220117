# lab1
## My simple shell
**<font size = 5>WARNING: Do NOT clone, download or copy any codes in this repository!!!!</font size = 5>** 
- ***To use this simple shell, you need to install the readline library.***
- After make you will get a executable program named njush.
- Please enter "help" to get help information.
- Use "exit" to exit this program.
- Use "cd XX" or "cd" to change directory.
- Support the simplified bash history commands stated in the document.
- Support only one program run in background, such as "ls -a&".
- Support multiple pipes. It is required spaces before and after the command, such as "ls -a >> a.txt | ls -l | wc -l".
- Support multiple redirections. It is required spaces before and after the command, such as "ls -a > a.txt | ls -l > b.txt | wc -l < c.txt", "ls -a | wc -l > a.txt"

***<font size = 5>NOTICE</font size = 5>***
- 后面测试发现在tmux中有颜色的字体能正常显示，在正常shell中运行输入的内容会把前面输出的prompt给吞掉，如果需要有颜色的可以在tmux中运行，否则可以把do_prompt()函数中的第60行注释替换为第61行，第72行注释替换为第73行，即可正常显示没有颜色的版本
