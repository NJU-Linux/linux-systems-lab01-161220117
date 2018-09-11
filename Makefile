cc = gcc
OBJ = mysh
all :
		@$(cc) -c -Wall ./*.c
		@echo complete *.c
		@$(cc) -o  $(OBJ) ./*.o -I /usr/lib/x86_64-linux-gnu/libreadline.so -lreadline -ltermcap -g
		@echo complete compiling...
		@rm -rf *.o
		@echo remove *.o
clean:
		rm -rf *.o

