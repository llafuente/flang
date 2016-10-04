# Welcome to flang

Beaware!
There is a lot **TODO** / **REVIEW** /**??**.
Documentation is also under development.

## hello-world.fl

In every language we all start with that.

We are deeply sorry for no having a long hello world program.

hello-world.fl
```flang
printf("hello world!");
```

```bash
# flang is a compiled language, so compile
flang hello-world.fl
# execute
./hello-world
hello world!
```


## REPL (**TODO**)

If you prefer you can use our REPL

```bash
flang
> "hello world!";
"hello world!"
> printf("hello world!")
"hello world!"
0
```


## Paradigms / Features

* [Functional](https://en.wikipedia.org/wiki/Functional_programming)
* [Structured](https://en.wikipedia.org/wiki/Structured_programming)
* [Generic](https://en.wikipedia.org/wiki/Generic_programming)
* [Inference](https://en.wikipedia.org/wiki/Type_inference)
* [Polymorphism](https://en.wikipedia.org/wiki/Polymorphism_%28computer_science%29)
* [Operator overloading](https://en.wikipedia.org/wiki/Operator_overloading)

## Differences with known languages

Before the critics start. Here are the list of things you will not
find in the language, there is a reason for everything, will be explainded
in other chapters.

* There is no comma (`,`) operator
* There is no arrow `->` operator for pointers use: `pointer[0].`
* There are no classes or prototypes (you won't missed them)
* Strong typed (unles `unsafe_cast` is used)
* No identifiers shadowning (repeat! identifiers: variables, types and function must be unique in their scope, except for polymorphism)
* assignament expression cannot be used as function arguments or inside `if`, `do`, `while` etc. Assignament are only allowed at block level.


## TOC

Take some air and dive deep into flang!

* [type system](types-system.md)
* [debugging](debugging.md)
