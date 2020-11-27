#hello:hello.o
#	gcc hello.o -o hello
#hello.o:hello.c
#	gcc -c hello.c -o hello.o
#

src = $(wildcard ./src/*.c)
obj = $(patsubst ./src/%.c,./obj/%.o,$(src))

inc_path = ./inc
myArgs = -Wall -g

ALL:a.out

a.out:$(obj)
	gcc $^ -o $@ $(myArgs)

$(obj):./obj/%.o:./src/%.c
	gcc -c $< -o $@ $(myArgs) -I $(inc_path)

#hello.o:hello.c
#	gcc -c $< -o $@

#add.o:add.c
#	gcc -c $< -o $@

#sub.o:sub.c
#	gcc -c $< -o $@

#mul.o:mul.c
#	gcc -c mul.c -o mul.o



clean:
	-rm -rf $(obj) a.out

.PHONY:clean ALL
