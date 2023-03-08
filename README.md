#  Chroma
Simple interpreter written in C
---
### 1. Introduction
```js
// fibonacci.chr
function fib(N) {
    if (N < 2) {
        return 1;
    }
    else {
        return fib(N - 1) + fib(N - 2);
    }
}
```
Chroma is a simple programming language that supports variables, arrays, functions and classes and more.
It is planned to be compatible with C.

Some external features such as basic IO can be imported from C libraries.

### 2. Features
Chroma has 6 basic datatypes; `number`, `string`, `array`, `function`, `class`, `object`.
Chroma stores data in a `struct` named `Value`, and each variable holds the reference for `Value`, in other word,
`Value*`.

`number` is equal to `double` type in C. Chroma does not have a seperate `int` type; `number` itself includes both `int` and `double`.

`string` is equal to `char*` type in C. `string` can be indexed, and concatenated by `+`.

`array` is an array of `Value*`, therefore, and datatypes can be stored in a single array.
`array` should be initialized by a given length.

`function` and `class` are technically implemented same in chroma, as they are both `invokable`.
This means these datatypes can be invoked(called), with creates an `object`.
However, a `function` returns the `return` value of the `object`, while `class` returns the `object` itself.


Chroma `function` supports major features that other programming language does;
recursion, closures and more.
