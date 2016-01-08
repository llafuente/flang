Unsorted list of under-study feature/syntax

### conditional chain continuation

```
object
.isModified()?
.save();
```

Equivalent to:
```
if (object.isModified()) { object.save(); }
```

```
object
.save()
.no_error()?
.return();
```

Equivalent to:
```
var tmp = object.save();
if (tmp.no_error()) { tmp.return(); }
```

### safe object access

```
if (?object.xxx.yyy.zzz == true)
```

Equivalent to:
```
if (
  object != 0 &&
  object.xxx != 0 &&
  object.xxx.yyy != 0 &&
  object.xxx.yyy.zzz == true)
```

### access scope vars by name

```
var name = 100;
assert(name == scope["name"]);
```

### access function arguments by name and id

```
function (a, b, c) {
  assert(arguments.length == 3);
  assert(arguments.a == a);
  assert(arguments.b == b);
  assert(arguments.c == c);
  assert(arguments[0] == a);
  assert(arguments[1] == b);
  assert(arguments[2] == c);
}
```

### if shorthand

```
where? {
  blablabla!
}
```
### return if not null shorthand

```
return? value;
```

Equivalent to:
```
if (value != null) {
  return value;
}
```

### operator ?: as Null-coalescing Operator (no ??)

```
var a = 1;
var b = 0;
var c;
assert( (a ?: 10) == 1);
assert( (b ?: 10) == 10);
```

### struct initialization

```
struct name {
  i8 literal = value,
}
```
value must be a constant

### operator ===

check both sides are in the same memory address, fast ==


### statement debug

* before statement type (if, while, for, return...)
* after statement type (if, while, for, return...)

```
before return {
  do your staff!
}

before return single-stmt;

before exit {
  // something before exit the program,
  // maybe not really "exit"
  // something before exit() ?
}
```

### more operators

```
a == b || a == c || a == d
a ||== (a, b, c)

a == b && a == c && a == d
a &&== (a, b, c)
```

Real usage:
```
if (callA() == callB() && callA() == callC()) {}
// callA twice is unwanted? rewritten as:

var a = callA();
if (a == callB() && a == callC()) {}

one liner: simplified
if (callA() &&== (callB(), callC())) {}
```

### big/little endian

research: How this affects the language?!

* test and swap functions (https://bitbucket.org/thejeshgn/quake/src/14ba6369f82c/WinQuake/common.c?fileviewer=file-view-default#cl-1125)

* able to print platform in/dependant binary


### inplace functions

Something like macros but type checked.
