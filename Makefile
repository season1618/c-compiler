CFLAGS = -std=c11 -g -static
SRCS = main.c tokenize.c parse.c codegen.c
OBJS = $(SRCS:.c=.o)

stage1: $(OBJS)
	$(CC) -o stage1 $(OBJS) $(LDFLAGS)

$(OBJS): dcl.h

stage2: stage1 $(SRCS)
	$(CC) -E main.c > tmp.c
	./stage1 tmp.c > main2.s
	$(CC) -E tokenize.c > tmp.c
	./stage1 tmp.c > tokenize2.s
	$(CC) -E parse.c > tmp.c
	./stage1 tmp.c > parse2.s
	$(CC) -E codegen.c > tmp.c
	./stage1 tmp.c > codegen2.s
	$(CC) -o stage2 main2.s tokenize2.s parse2.s codegen2.s

stage3: stage2 $(SRCS)
	$(CC) -E main.c > tmp.c
	./stage2 tmp.c > main3.s
	$(CC) -E tokenize.c > tmp.c
	./stage2 tmp.c > tokenize3.s
	$(CC) -E parse.c > tmp.c
	./stage2 tmp.c > parse3.s
	$(CC) -E codegen.c > tmp.c
	./stage2 tmp.c > codegen3.s
	$(CC) -o stage3 main3.s tokenize3.s parse3.s codegen3.s

valid: stage2 stage3
	diff main2.s main3.s
	diff tokenize2.s tokenize3.s
	diff parse2.s parse3.s
	diff codegen2.s codegen3.s

test1: stage1 test/test.c
	./stage1 test/test.c > tmp.s
	$(CC) -o tmp tmp.s
	./tmp

test2: stage2 test/test.c
	./stage2 test/test.c > tmp.s
	$(CC) -o tmp tmp.s
	./tmp

clean:
	rm *.s *.o stage1 stage2 stage3 tmp