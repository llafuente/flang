### REPL

### Misc

* prevent double import (cache ast? / cycles?) - node->parent is a problem!!
* test utf8 function snowman!
* give very comprehensive error on Polymorphism


### defer

```
defer {
  statement will be evaluated before exit function scope
}
```

for performance this should be evaluated only once. Go don't do this...
let's investigate further before adding it.

### scope

Execute the block when scope ends
```
scope exit {

}
```

Execute the block when function scope ends
```
scope return {

}
```


### debugging

* variables

  stdout
  stdin
  stderr
  stdlog -> stdout (where $log, $$log, $watch print)

* log

  ```
  var x = 10;
  var y = 12;
  log x, y;
  // stdout: 10 12\n
  ```

* $$log

  Print each part of the expression

  ```
  $$log a + b + c
  // a + b + c -> 10 + 10 + 10 -> 30
  $$log callme(a, b)
  // callme(a, b) -> sum(15, 25) -> 40
  ```

* $watch

  tell compiler to $log every change on giving variable
  ```
  $watch x;
  ```
* defer $log

  $log all returns

* assert

  ```
  assert expr; // check against bool
  assert expr is value;
  assert expr isnt value;
  assert expr instanceof <type>;
  assert expr constains value;
  //negate
  assert y !comparisson x;
  ```

### builtin type: regex

### builtin type: string

### builtin type: array

* .capacity
* .length
* []
.push
.pop
.index_of
.has/contains (index_of != -1)

.map
.reduce
.filter
.remap
.find
.find_one
.find_last
.until
.each

### builtin type: object

Hash type.

### builtin type: model

model is a type that extend functionality from object adding:
* validation
* observers
* setter/getters
* virtuals

### Functions

* function template `fn ($t)`
  * recognize templates
  * search function with templates
  * check if the template can be applied and the function make sense
  * duplicate the function (ast_clone is still incomplete)

* operator overloading
  * + (add)
  * - (sub)
  * / (div)
  * * (mul)
  * | (pipe)
  * ==
  * !=
  * <
  * >
  * <=
  * >=

* MFC: member function call:
  * a.b(c) -> b(a,c)
  * a.x.b(c) -> b(a.x,c)

* lambda
  * function/callback header shortcut

    ```
    function callback(i8 a, i8 b)
    function xx(callback cb) {
      cb(10, 10);
    }
    xx(@{ // use the same header as declared -> callback(i8 a, i8 b)
      $log a;
      $log b;
    });
    ```

* named parameters

  ```
  struct t { i8 a, i8 b };
  function x(i8 a, i8 b) {
    // fancy staff
  }
  x(b:100, a: 50);
  ```

* expand struct into function arguments

  ```
  struct t { i8 a, i8 b };
  function x(i8 a, i8 b) {
    // fancy staff
  }

  function y(i8 a, i8 b, i8 c) {
    // fancy staff
  }

  function z(i8 c, i8 a, i8 b) {
    // fancy staff
  }

  function super(i8 c, i8 a, i8 d, i8 b) {
    // fancy staff
  }

  x(expand t);
  y(expand t, 10);
  z(10, expand t);
  super(expand t, c:10, d:20);
  ```


### Modules

* function, variables visibility.

* Specify a prefix for everything (to avoid c collisions)

  Prefix could be overriden in the import


### Return life cycle

need refinement.

* return life cycle
  * function (will be destroyed at the end of current function) (defer delete)
  * scope (will be destroyed when scope reach end)
  * program (will be destroyed on last scope)
  * default (user owned)
