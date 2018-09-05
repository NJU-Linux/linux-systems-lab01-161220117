cc = gcc
OBJ = l40ab1
all :
		@echo $(cc)  -c  ./*.c
		@echo $(cc)  -o $(OBJ)  ./*.o  -I /usr/lib/x86_64-linux-gnu/libreadline.so  -lreadline -ltermcap  -g
		@echo git add .
		@echo git commit -m "compiling"
		@# echo rm -rf  *.o
clean:
		@echo rm -rf *.o

