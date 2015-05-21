
```bash
sh autogen.sh
./configure
make
```



## Compile process

* read file to string
* tokenize the string (tokenizer)
* from tokens create the AST (parser)
* Resolve dependencies in the AST (typesystem)
* generate IR from ast (codegen)
  * execute jit (exec-jit)
  * execute bitcode (exec-bit)
  * dump IR (exec-dump)
  * execuable (exec-full)


## tokenizer

Split a string with the information found in `tokens.c` and spaces.

Tokenizer will add a new line token before EOF, that is also a token.

Example `var x;`

> VAR ' ' 'x' SEMICOLON NEWLINE EOF


## parser

From the tokenizer data, generate a simple AST.

It will report only lexer error mostly: "expected &lt;something&gt;"

## codegen

Get the AST and generate code with `LLVM-C`
