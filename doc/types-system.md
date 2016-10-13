# flang type system

Flang is a strong typed language in it's core.

But flang is in fact loosely typed because:

* `unsafe_cast`. You can force the compiler to not check the casting you are
making... but unlike c/c++ you must write a different cast, so it's searchable.

* `pointers`. Pointer can be used to bypass type system in conjuction with
`unsafe_cast`. It's usage is discouraged but it's essential to glue c code into
flang.

## built-in types

### primitives

`i8`, `i16`, `i32`, `i64`: signed integers

`u8`, `u16`, `u32`, `u64`: unsigned integers

`f32`, `f64`: floating points

`bool`: boolean

`ptrdiff`: memory address values

`void`: no-type

`regex`: PERL compatible Regular Expressions (**TODO**)

```
var i8 char;
var f32 float;
var regex = /hell/g
```

### ref (references)

References are the prefered way to use pointer to a single object.

Reference are in fact pointers, but you cannot move them. No pointer arithmetic.
No braket access. And don't need to dereference :)

With this behaviour, overflow is prevented.

**TODO** Before dereference the pointer is checked against invalid values.

```
var i8* ref_to_char = malloc(sizeof(i8)); // **TODO** shorthand var new i8* ref_to_char;
var bool* ref_to_bool;

ref_to_char = 100;

var i8 sum = ref_to_char + 1;
$log sum; // stdout: $(i8) sum = 101
```

### array (Arrays) (**TODO**)

Arrays are the prefered way to use pointer to a contiguous memory.

An array is a thin struct around a template type.

Members

* length: array length (alias: len)
* capacity: allocated, max length (alias: cap)
* values (pointer to type) can be used directly using `arr[number]`

```
var array(i8) array_of_chars;
var i8[] array_of_chars2;

array_of_chars2[5] = 0;
log array_of_chars2.len;
```

### string (**partialy done**)

Strings do no extend array type. Because that way will inherit all
the array length meaning, that it's incorrect for strings.

So strings are implemented separately.

Members

* length: number of characters (alias: len)
* used: bytes used (alias: len)
* capacity: bytes allocated (alias: cap)
* values: pointer to bytes
* encoding: enum with supported encodings (alias: enc)

*NOTE*: unlike an array `operator[]` has a very distinct meaning.

Because `operator[]` return a new string, containing the character at
that position, that could be multi-byte.


string can handle multiple enconding transparently.

That doesn't mean it's UTF-8, has an encoding value so your application
should be aware of the encoding.

### ptr (Pointers)

Pointers... same as C/C++.

It's usage is discouraged, they exist to glue C code into flang without
creating unnecessary frictions.

*NOTE* That in flang `type*` is not a pointer, it's a reference.

We don't want people to use it much, so they are "*syntax anoying*".

```
var ptr(void) ptr_to_void;
var ptr(i8) c_string;
```

**TODO** **REVIEW** var new shorthand should be invalid for pointers?

### struct

Syntax

```
// normal plain struct
struct identifier {
  type identifier,
  alias identifier identifier,
};

// templated struct
template $tpl;
struct identifier($tpl[,]) {
  type identifier,
  alias identifier identifier,
};
```

#### structs extension (**TODO**)

It's just a syntax and casting help.

**REVIEW** extend can be used N times ?
**REVIEW** how we handle naming collisions ?

Example:

```
struct v2 {
  f32 x,
  f32 y,
};

struct v3_composition {
  v2 v2;
  f32 z;
}

struct v3 {
  extend v2;
  f32 z;
}

var v2 x2;
x2.x = 0;
x2.y = 1;

// this is a bit annoying...
var v3_composition x3c;
x3c.v2.x = 0;
x3c.v2.y = 1;
x3c.z = 2;

var v3 x3;
x3.x = 0;
x3.y = 1;
x3.z = 2;

#assert sizeof(v3_composition) == sizeof(v3), "both structs has the same size";

v2 cc = cast(v2) x3c; // invalid cast
v2 cc = x3c.v2; // ok, no cast needed
v2 cc = cast(v2) x3; // ok
```

#### unions (**TODO**)

This is neede to glue c code.
But we can modify some in the syntax like extend.


### list (**TODO** **REVIEW**)

Almost Contiguous memory.
Use multiple dynamic array.
Slower access, fast modification.

*Note*: this is not traditional linked-list.

Usage it's recommended for high modifications.

## Variable declaration

Examples

```
// full local variable declaration and initialization
var i8 a = 0;
// inference local variable declaration and initialization
var b = 0; // b it's a i64 atm
// full local variable declaration
var i16 c;
// inference local variable declaration
var d;
// local variable declaration and allocation
var new i8* a_ref_to_i8;
// full global variable declaration
global i32 g_man = 1337;

function x () {
  // coder hint for where the var cames.
  global g_man;

  return g_man;  
}

```

**TODO** var new ?;

## functions

Examples

```
fn say_hello() {
  printf("Hello!");
}
// fn and function are synomins
function say_hello2() {
  printf("Hello2!");
}
```

### function metadata

Each function has a internal ID, that it's the identifier that flang will use.
Because flang want to be a good citizen in C community, it also has to specify
the name that will be used when exported, you can use function metadata to
force this name, instead of a random one.

Example
```
[@uid=sum_i8]
fn sum(i8 a, i8 b): i8 {
  return a + b;
}

[@uid=sum_i16]
fn sum(i16 a, i16 b): i16 {
  return a + b;
}
```

*Note* All metadata will be stored in the type.

### functions variadic (**TODO**)

Example:
```
// variadic **TODO**
fn flang_printf(... list) : type {
  list.each($v, $k, {
    printf ($v);
  })
}
```

### functions macro (**TODO**)

Macro function are not 100% c macro functions. They have types.

The only difference it's they accept `block` of code as input.

```
fn macro each(array arr, $tpl v, u64 i, block blk) {
  i = 0;
  u64 max = arr.length;
  for (; i < max; ++i) {
    transclude blk;
  }
}
```

### function implicit return (**TODO**)

Example

```
// c is declared in the scope and will be implicit returned
template $tpl;
fn div ($tpl a, $tpl b) : $tpl c {
  c = nan;
  if (b == 0) {
    printf("Invalid operand b");
  } else {
    c = a / b;
  }
}
```


### function default values (**TODO**)

Example

```
// c is declared in the scope and will be implicit returned
fn sum (u64 a, u64 b = 0) : u64 {
  return a + b;
}
```

### function assert values (**TODO**)

Example

```
// c is declared in the scope and will be implicit returned
fn sum (u64* a != 0, u64* b != 0) : u64 {
  // implicit add assert(a != 0, "argument a must be != 0");
  // implicit add assert(b != 0, "argument a must be != 0");
  return a + b;
}
```

## ffi

Foreign function interface.

To include a C library into flang you need to declare all functions you want
to import.

Example:

```
ffi fn printf(ptr(i8) format, ...) : i32;
ffi fn malloc(u64 size) : ptr(void);
ffi fn free(ptr(void) ptr) : void;
```

**REVIEW** compilation process is made outside flang atm. So coder include
a functions list and also have to include the library in the compilation line.
flang should be able to compile and have something to add a library to
compilation.

**TODO** there is no method in include variables.

## casting

Number casting rules are easy.

* downcast must be explicit
* upcast it's implicit.
* From unsigned to signed must be explicit.

### implicit casting

You can specify when two types can coerce in one direction, declaring an
implicit cast function, so you don't have to type explicit everywhere.

> With a great power comes a great responsibility. (Ben Parker, RIP)


Example:

```flang
fn implicit cast (type_from xx) : type_to {
  return xx.member_of_to;
}
```

## operator overloading

List of valid operators: `+`, `-`, `*`, `/`, `[]`

Example:

```
function operator + (v2 a, v2 b) : v2 {
  var v2 r;
  r.x = a.x + b.x;
  r.y = a.y + b.y;
  return r;
}
```

### Operator [] (access read)

Access data from a type, for example an array.

Example:

```
function operator [] (v2* v, u64 index) {
  #assert index > 2, "Out of bounds";

  return (unsafe_cast(ptr(f32)) v)[index];
}
```

### Operator []= (access modify)

Set data into a type, must recieve and return a reference.

Example:

```flang
struct vec2 { f32 x, f32 y, };
function operator []=(vec2* a, u64 index) : f32* {
  var ptr(f32) x = unsafe_cast(ptr(f32)) a;

  return &x[index];
}

var vec2 v;
v.x = 1.1;
v.y = 2.2;
$log v;
v[0] = 5;
$log v;
```

output
```
$(struct vec2 { f32 x, f32 y, }) v = {x = 1.100000, y = 2.200000}
$(struct vec2 { f32 x, f32 y, }) v = {x = 5.000000, y = 2.200000}
```


### Operator new (allocate a reference or pointer)

Example:
```
struct test_struct {
  ptr(i8) list,
};

fn operator new (test_struct* t) {
  t.list = calloc(10 * sizeof(i8));
}

var new test_struct* tt;
tt.list[0] = 72;
```

When using operator new there is an implicit allocation of the
current pointer.

See what the compiler does:
```
// var new test_struct* tt;
var test_struct* tt;
tt = malloc(sizeof(test_struct*))
operator_new(tt);
```

## templates

Templates need to be declared unlike other languages.
We recommended to prefix templates with '$'.

Examples:

```
template $tpl;

// now the template can be used in structs and functions
// see below for examples
```

### Implement templates.

Most languages delay implementation of a template until it's use.

Flang allow both behaviours.

Templates are implemented from left to right.
Here is an example that illustrate that behaviour.

```
var i64 num64 = 10;

template $tpl;
struct st_tpl($tpl) { $tpl values, };
function x (st_tpl a, $tpl b) {}

var st_tpl(i8) ai8;
x(ai8, num64);
//     ^-- type error, explicit cast required between (i64) to (i8)
```

This error it's because after implement the function: $tpl will be i8,
and num64 cannot be downcasted, so typesystem force you to cast down
yourself.


Flang has some type restrictions when you try to implement a templates.

A template can be of any type.
```
template $tpl;
function print_type($tpl a) { printf("%lu", typeof($tpl)); }

print_type(10);
struct v2 { f32 x, f32 y}
print_type(v2);

```

But a templated struct can only be implemented by another struct
```
template $tpl;
struct v2 { $tpl x, $tpl y}
function print_type(v2 a) { printf("%lu", typeof(v2)); }

var v2(f32) v2f;
print_type(v2f);

print_type(10);
// ^-- type error, cannot implement type (struct v2 {  x,  y, }) into (i64). A struct is required.
```

The same occurs with pointers and references.
```
template $tpl;
struct v2 { $tpl x, $tpl y}
function print_type(v2* a) { printf("%lu", typeof(v2)); }

var v2(f32) v2f;
print_type(v2f);
// ^-- type error, cannot implement type (ref(struct v2 {  x,  y, })) into (struct v2_f32 { f32 x, f32 y, }). A reference is required.
```

**TODO** implement template with a type, and everywhere it's used get
implemented.

### Freeze template (**TODO** **REVIEW**)

The template won't be implemented anymore.

```
template freeze $tpl;
template unfreeze $tpl;
```

It's just a secure method to be sure that you implement the template before
it's use, and it's type secure.
