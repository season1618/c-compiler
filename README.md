# subset C compiler for x86-64 on Linux

- [情報工学系 プログラミング創造演習(Creating Programming Projects)](https://titech-cpp.github.io/)
- [Cコンパイラ開発メモ](https://season1618.github.io/note/cs/c-compiler/)

## Environment
WSL2: Ubuntu-20.04

## Development

```
$ make stageN
```

to build the stage N (N = 1,2,3) compiler.

## Test

```
$ make testN
```

to compile `test/test.c` by the stageN (N = 1,2) compiler and run it.

```
$ make valid
```

to validate that the stage2 compiler equals to the stage3 compiler.
