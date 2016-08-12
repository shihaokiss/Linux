.PHONY:all
all:test1 test2 test3
test1:test1.c
	gcc -o $@ $^
test2:test2.c
	gcc -o $@ $^
test3:test3.c
	gcc -o $@ $^
.PHONY:clean
clean:
	rm -rf test1 test2 test3
