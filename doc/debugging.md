# flang debugging

## Loggin (**TODO**)

flang include a loggin system, so every app can use the default loggin system,
Be a good neighbour!

flang declare 5 levels.

* `log_error`
* `log_warning`
* `log_info`
* `log_verbose`
* `log_debug`
* `log_silly`

`log` is an alias of `log_debug`

*Warning*: log will execute your expression, if it's an assignament it will done.

`log` is designed to be prepend to any expression at statement level.

## log expression (**TODO**)

Print to stdout the type and value of given expression.

Example to illustrate the warning above.
```
var x = 1;
log x; // $(u64) 1
log x = 2; // $(u64) 2
log x = 3; // $(u64) 3
log x; // $(u64) 3
```

## $log expression

Print to stdout the type, code and value of given expression.

```
$log 1+2+3; //$(i64) 1+2+3 = 6
$log v2; // $(struct vec2 { i8 x, i8 y, i8 stop, }) v2 = {x = 100, y = -56, stop = 0}
```

## $$log expression (**TODO**)

Print to stdout the type, code and value of each chunk of the expression.
