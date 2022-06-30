CFLAGS = -std=c11 -g -static
SRCS = main.c tokenize.c parse.c codegen.c
OBJS = $(SRCS:.c=.o)

main: $(OBJS)
	$(CC) -o main $(OBJS) $(LDFLAGS)

$(OBJS): dcl.h

test: main test/test.c
	./main test/test.c > tmp.s
	$(CC) -o tmp tmp.s foo.c
	./tmp