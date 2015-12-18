# Flang

flang is an experimental language for mangling data.

take a look if you want but it not useable yet.

## Usage

requisites (at least (what i'm using right now))
* bison (GNU bison) 3.0.4
* flex 2.5.37
* autoconf (GNU Autoconf) 2.69
* Clang 3.6.2
* [https://github.com/llafuente/string.c](string.c) latest in the git


```bash
sh gen-grammar.sh
sh autogen.sh
./configure
make
# enjoy :)
```

# Generate documentation

Documentation of the language is not ready, but the code is documented.

```bash
sh gen-doc.sh
```

`doc.md` will contain the documentation (it's not in the git yet).

## Compile process

* Flex/Bison parse the file and build the AST
* Typesystem traverse the AST filling the gaps
  * Register types
  * Apply types to all nodes
    * Inference
    * Implicit casting
  * Reduce the AST to it's minimal form
* Codegen AST usgin LLVM-C
  * execute jit (exec-jit)
  * execute bitcode (exec-bit)
  * dump IR (exec-dump)
  * execuable (exec-full)

## Support

* Types.
  * Integers (any size and signed) and boolean
  * Structs (no union)
  * Functions

* Modules (in global scope, like #include in c)
* Function polymorphism
* Function pointer call
* Pointers and pointer arithmetic
* Basic inference
* All c operators and same precedence (expect conditional expression)
* autocast function. Bypass the typesystem casting alowing custom casts.
* debug capabilities
  * $log: print the code and the return value of the given expression

## Todo

Current todo list.
* struct/function template (nothing fancy just type template)
* stdin, stdout, stderr, stdlog
* $$log: print the code, the expression with all value inputs and the final value of the expression.

For the future (not far i hope)
* switch
* Lambdas
* Type Any (box/unbox/unboxif)
* conditional expression (? :)
* operator overloading
* export c header (flang is compatible with c)
* REPL
* Modules
  * global prefix: prefix functions/global variables.
  * import 'as'. (like a namespace)
* structs
  * setter(struct, string, type value)
  * getter(struct, string) : type
  * keys(struct) : string[]
  * struct["xxxx"] = expr -> setter(struct, string, expr)
  * debug(struct)
  * default initialization
  * extends (simple hierarchy)
  * union
* functions
  * default arguments
  * call with named arguments
  * get_arguments (name & type)
  * arguments (like JS in an array of any)
  * keyword to force return type to be assigned
* unvar, initialized variable
* Underscore in literals 999_99_9999L
* until -> while(!)
* break; Terminates the loop (most internal if none specified).
* redo; Restarts this iteration, without checking loop condition.
* next | continue; Jumps to next iteration.
* retry; If retry appears in the iterator, the block, or the body of the for expression, restarts the invocation of the iterator call. Arguments to the iterator is re-evaluated.
* forever{}
* object
  * Hashmap
  * store Any
* model
  * is an object
  * has a validation scheme
  * have a state: pristine, dirty, error
  * can get changes since last pristine
  * have setters & getters
  * observers
  * autocast to object
* native regex using pcr2
* io using libuv


> fori = 0 < x {} -> for var i=0; i < x; ++i {}

## fix

> ++(5 + 6)

> ReferenceError: Invalid left-hand side expression in prefix operation

## C differences

Apart from syntax...

* No operator `,`
* Braces are mandatory, for version control reasons
* Types cannot demote without casting.
* Function polymorphism is allowed. So proper naming must be used for the C interface.

```
#id=sum_i8_i8
fn sum(i8 a, i8 b): i8 {
  return a + b;
}
#id=sum_i32_i32
fn sum(i32 a, i32 b): i32 {
  return a + b;
}
```

* There is no typedef.

  Normally `struct a{...}` has: `typedef struct a a_t`.

  Flang declaration of a type is the definition (with the same name).

  That implies that there is no need to use `struct a`, `a` is enough.

  Any type works the same even functions.
