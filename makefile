all: ll1.bin

ll1.bin: main.c symbol.c file_util.c list.c parser.c
	gcc -g -W $^ -o $@

clean:
	rm ll1.bin
