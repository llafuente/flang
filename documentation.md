### autocast

When there is no possible built-in casting, flang look for a special function called `autocast` that should recieve one param and return a specific type.
If the correct, exact function is found, it will be used to cast.

Example:

```flang
struct a_t {
  i8 value;
};

var a_t a;
a.value = 10;
var i8 x;
x = 5;

// x = a; it's no possible without autocast.
function autocast (a_t t): i8 {
  return t.value;
}

x = a;
assert(x == 10); // true

```
