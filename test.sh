assert(){
    expected="$1"
    input="$2"

    ./main "$input" > tmp.s
    cc -o tmp tmp.s foo.c
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 1 "int main(){ return 1; }"
assert 1 "int main(){ +1; }"
assert 1 "int main(){ - -1;}"
assert 6 "int main(){2*3;}"
assert 3 "int main(){return 5*(9-6)/15+ 2;}"
assert 15 "int main(){-3*-5;}"
assert 24 "int main(){return-(3 + 5)*2 + 40;}"
assert 1 "int main(){4* 6== 7+17;}"
assert 1 "int main(){3*5 <4*4;}"
assert 1 "int main(){34>=34;}"
assert 1 "int main(){int abc; abc = 1;}"
assert 2 "int main(){int a; int b; a = b=2;}"
assert 1 "int main(){ int a; a = 0; a = a + 1 ; return a; }"
assert 1 "int main(){ {} return 1; }"
assert 1 "int main(){ if(1== 3) 0; else 1; }"
assert 1 "int main(){ int a; a = 1; if(a == 0){return 0;}else if(a==1){return 1;}else{return 2;} }"
assert 4 "int main(){ int a; a = 0; while(a<4) a=a+1; return a; }"
assert 4 "int main(){ int i; for(i = 0; i < 4; i = i+1) 1; return i; }"
assert 1 "int main(){ foo(); return 1; }"
assert 28 "int plus(int a, int b, int c, int d, int e, int f, int g){ return a + b + c + d + e + f + g; } int main(){ return plus(1,2,3,4,5,6,7); }"
assert 2 "int sub(int a, int b){ return a - b; } int main(){ return sub(3, 1); }"
assert 13 "int fibonacci(int n){ if(n == 0) return 0; else if(n == 1) return 1; else return fibonacci(n-1) + fibonacci(n-2); } int main(){ return fibonacci(7); }"
assert 1 "int main(){ int a; a = 0; int *b; b = &a; int **c; c = &b; **c = 1; return *b; }"
assert 1 "int *func(){ int a; a = 1; int *b; b = &a; return b; } int main(){ return *func(); }"
assert 1 "int**func(){ int a; a = 1; int *b; b = &a; int **c; c = &b; return c; } int main(){ return **func(); }"
assert 3 "int main(){ int *p; p = alloc(); return *(p + 3); }"
assert 4 "int main(){ return sizeof 1; }"
assert 8 "int main(){ int *p; return sizeof p; }"
assert 40 "int main(){ int a[10]; return sizeof a; }"
assert 3 "int main(){ int a[2]; *a = 1; *(a + 1) = 2; int *p; p = a; return *p + *(p + 1); }"

echo OK