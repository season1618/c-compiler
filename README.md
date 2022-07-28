# subset C compiler for x86-64 on Linux

[情報工学系 プログラミング創造演習(Creating Programming Projects](https://titech-cpp.github.io/)

[Cコンパイラ開発メモ](https://github.com/season1618/books/blob/main/c-compiler/index.md)

## Environment
wsl2

## Development

```
$ make
```

to build the stage1 compiler.

```
$ make stage2
```

to build the stage2 compiler.

```
$ make stage3
```

to build the stage3 compiler.

## Tests

```
$ make test
```

to compile `test/test.c` by the stage1 compiler and run it.

```
$ make test2
```

to compile `test/test.c` by the stage2 compiler and run it.

```
$ make valid
```

to validate that the stage2 compiler equals to the stage3 compiler.