./main test.c > tmp.s
cc -o tmp tmp.s foo.c
./tmp