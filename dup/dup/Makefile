.PHONY:all
all:my_dup my_dup2
my_dup:my_dup.c
	gcc -o $@ $^
my_dup2:my_dup2.c
	gcc -o $@ $^
.PHONY:clean
clean:
	rm -rf my_dup my_dup2 log
