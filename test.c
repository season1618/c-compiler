int printf();
// int len();

// int plus(int a, int b, int c, int d, int e, int f, int g){
//     return a + b + c + d + e + f + g;
// }

// int sub(int a, int b){
//     return a - b;
// }

// int fibonacci(int n){
//     if(n == 0) return 0;
//     else if(n == 1) return 1;
//     else return fibonacci(n-1) + fibonacci(n-2);
// }

// int *alloc();

// int *func1(){
//     int a; a = 1;
//     int *b; b = &a;
//     return b;
// }

// int**func2(){
//     int a; a = 1;
//     int *b; b = &a;
//     int **c; c = &b;
//     return c;
// }

// int test_if(){
//     int a; a = 1;
//     if(a == 0){
//         return 0;
//     }else if(a==1){
//         return 1;
//     }else{
//         return 2;
//     }
// }

// int test_while(){
//     int a; a = 0;
//     while(a<4) a=a+1;
//     return a;
// }

// int test_for(){
//     int i;
//     for(i = 0; i < 4; i = i+1) 1;
//     return i;
// }

// int arr2[2];

// int assert(int x, int y, char *s){
//     if(x == y){
//         printf("%.*s is %d\n", 1, s, y);
//     }else{
//         printf("%.*s is not %d\n", 1, s, y);
//     }
// }

int main(){
    int a; a = 1;
    int b; b = 2;
    // printf("a = %d OK\n", a);
    if(a == 1){
        if(b == 2){
        printf("a = %d OK\n", a);
        printf("b = %d OK\n", b);}
    }else{
        printf("a = %d NG\n", a);
        printf("b = %d NG\n", b);
    }
    // int x; x = 2;
    // int abc; abc = 1; printf("%d %d\n", x, abc);
    // assert(abc, 1, "abc");
    // assert(5*(9-6)/15+ 2, 3, "5*(9-6)/15+ 2");
    // assert(-(3 + 5)*2 + 40, 24, "-(3 + 5)*2 + 40");
    // assert(4* 6== 7+17, 1, "4* 6== 7+17");
    
    // int a;
    // int b;
    // b = 2;
    // a = b;
    // assert(a, 2, "a"); printf("%d %d\n",a, b);
    // a = 0;
    // a = a + 1;
    // assert(a, 1, "a");
    // a = 0;
    // int *p; p = &a;
    // int **q; q = &p;
    // **q = 1;
    // assert(**q, 1, "**q");
    // p = alloc();
    // assert(*(p + 3), 3, "*(p + 3)");
    // assert(*func1(), 1, "*func1()");
    // assert(**func2(), 1, "**func2()");
    // int arr[10];
    // assert(sizeof arr, 40, "sizeof arr");
    // int arr1[2]; *arr1 = 1; *(arr1 + 1) = 2;
    // assert(*arr1 + *(arr1 + 1), 3, "*arr1 + *(arr1 + 1)");
    // arr2[0] = 1;
    // arr2[1] = 2;
    // assert(arr2[0] + arr2[1], 3, "arr2[0] + arr2[1]");
    // char c; c = 12;
    // assert(c, 12, "c");
    // char *s; s = "abcdef";
    // assert(s[0], 97, "s[0]");
    
    // assert(sub(3, 1), 2, "sub(3, 1)");
    // assert(fibonacci(7), 13, "fibonacci(7)");
    // assert(test_if(), 1, "if statement");
    // assert(test_while(), 4, "while statement");
    // assert(test_for(), 4, "for statement");
    // assert(sizeof 1, 4, "sizeof 1");
    // assert(sizeof p, 8, "sizeof p");
    
    // printf("OK\n");
}