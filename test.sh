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

assert 1 "return 1;"
assert 1 "+1;"
assert 1 "- -1;"
assert 6 "2*3;"
assert 2 "-1+3;"
assert 47 'return 5+6*7;'
assert 3 'return 5*(9-6)/15+ 2;'
assert 15 "-3*-5;"
assert 24 "return-(3 + 5)*2 + 40;"
assert 1 "4* 6== 7+17;"
assert 1 "3*5 <4*4;"
assert 1 "34>=34;"
assert 1 "abc = 1;"
assert 2 "a = b=2;"
assert 2 "a = 1; b = a+1;"
assert 1 "a = 0; a = a + 1 ; return a;"
assert 2 "if(1 <3){ a = 1; b = 2; return a*b; }else return 0;"
assert 1 "if(1== 3) 0; else 1;"
assert 1 "a = 1; if(a==0){return a;}else if(a==1){return a;}else{return a;}"
assert 4 "a = 0; while(a<4) a=a+1; return a;"
assert 4 "a = 0; for(i = 0; i<4;i=i+1) a=a+1; return a;"
assert 1 "a = 0; foo(); return 1;"
assert 28 "return plus( 1,2, 3,4,5, 6, 7);"

echo OK