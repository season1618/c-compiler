assert(){
    expected="$1"
    input="$2"

    ./main "$input" > tmp.s
    cc -o tmp tmp.s foo.o
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 1 "main(){ return 1; }"
assert 1 "main(){ +1; }"
assert 1 "main(){ - -1;}"
assert 6 "main(){2*3;}"
assert 3 "main(){return 5*(9-6)/15+ 2;}"
assert 15 "main(){-3*-5;}"
assert 24 "main(){return-(3 + 5)*2 + 40;}"
assert 1 "main(){4* 6== 7+17;}"
assert 1 "main(){3*5 <4*4;}"
assert 1 "main(){34>=34;}"
assert 1 "main(){abc = 1;}"
assert 2 "main(){a = b=2;}"
assert 1 "main(){ a = 0; a = a + 1 ; return a; }"
assert 1 "main(){ {} return 1; }"
assert 1 "main(){ if(1== 3) 0; else 1; }"
assert 1 "main(){ a = 1; if(a==0){return a;}else if(a==1){return a;}else{return a;} }"
assert 4 "main(){ a = 0; while(a<4) a=a+1; return a; }"
assert 4 "main(){ a = 0; for(i = 0; i<4;i=i+1) a=a+1; return a; }"
assert 1 "main(){ a = 0; foo(); return 1; }"
assert 28 "plus(a, b, c, d, e, f, g){ return a + b + c + d + e + f + g; } main(){ return plus(1,2,3,4,5,6,7); }"
assert 2 "sub(a, b){ return a - b; } main(){ return sub(3, 1); }"
assert 13 "fibonacci(n){ if(n == 0) return 0; else if(n == 1) return 1; else return fibonacci(n-1) + fibonacci(n-2); } main(){ return fibonacci(7); }"
assert 1 "main(){ a = 1; b = &a; return *b; }"
echo OK