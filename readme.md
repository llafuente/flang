
```bash
sh autogen.sh
./configure
make
```



## Compile process

* read file to string
* tokenize the string (tokenizer)
* from tokens create the AST (parser)
* type inference (pass-inference)
* Resolve dependencies in the AST (typesystem)
* generate IR from ast (codegen)
  * execute jit (exec-jit)
  * execute bitcode (exec-bit)
  * dump IR (exec-dump)
  * execuable (exec-full)


## tokenizer

Tokenize code with the information found in `tokens.c`.

### Notes

Tokenizer will add a new line token before EOF, that is also a token.

Example `var x;`

> VAR ' ' 'x' SEMICOLON NEWLINE EOF


## parser

From the tokenizer data, generate a simple AST.

It will report only lexer error mostly: "expected &lt;something&gt;"

## typesystem

Fill the gaps in the AST. Mostly implicit casting.

## codegen

From the AST generate code with `LLVM-C`
