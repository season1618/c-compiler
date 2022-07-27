CFLAGS = -std=c11 -g -static
SRCS = main.c tokenize.c parse.c codegen.c
OBJS = $(SRCS:.c=.o)

main: $(OBJS)
	$(CC) -o main $(OBJS) $(LDFLAGS)

$(OBJS): dcl.h

stage2: main $(OBJS)
	$(CC) -E main.c > tmp.c
	./main tmp.c > main2.s
	$(CC) -E tokenize.c > tmp.c
	./main tmp.c > tokenize2.s
	$(CC) -E parse.c > tmp.c
	./main tmp.c > parse2.s
	$(CC) -E codegen.c > tmp.c
	./main tmp.c > codegen2.s
	$(CC) -o stage2 main2.s tokenize2.s parse2.s codegen2.s builtin.s $(LDFLAGS)

stage3: stage2 $(OBJS)
	$(CC) -E main.c > tmp.c
	./stage2 tmp.c > main3.s
	$(CC) -E tokenize.c > tmp.c
	./stage2 tmp.c > tokenize3.s
	$(CC) -E parse.c > tmp.c
	./stage2 tmp.c > parse3.s
	$(CC) -E codegen.c > tmp.c
	./stage2 tmp.c > codegen3.s
	$(CC) -o stage3 main3.s tokenize3.s parse3.s codegen3.s builtin.s $(LDFLAGS)

valid: stage2 stage3
	diff stage2 stage3

test: main test/test.c
	./main test/test.c > tmp.s
	$(CC) -o tmp tmp.s
	./tmp

test2: stage2 test/test.c
	./stage2 test/test.c > tmp.s
	$(CC) -o tmp tmp.s
	./tmp