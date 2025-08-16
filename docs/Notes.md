# Clang query notes

* Use godbolt.org for AST exploaration
* Set compiler to clang and compiler flags to `-Xclang -ast-dump -fsyntax-only`
* Some settings for clang query
```
set traversal     IgnoreUnlessSpelledInSource
set bind-root     true
set print-matcher true
enable output     dump

```


## Stencil

`clang/Tooling/Transformer/Stencil.h` <- contains various functions help to create rewrite.
Some useful ones:
* node
* name
* ifBound
* access
