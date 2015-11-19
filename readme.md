# Flang

flang is an experimental language for mangling data.

take a look if you want but it not useable yet.

## Usage

requisites (at least)
* bison (GNU bison) 3.0.4
* flex 2.5.37
* autoconf (GNU Autoconf) 2.69
* Clang 3.6.2


```bash
sh gen-grammar.sh
sh autogen.sh
./configure
make
# enjoy :)
```



## Compile process

* flex/bison parse the file and build the AST
* typesystem pass through the AST filling the gaps
* generate IR from ast (codegen)
  * execute jit (exec-jit)
  * execute bitcode (exec-bit)
  * dump IR (exec-dump)
  * execuable (exec-full)
